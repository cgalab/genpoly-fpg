/* 
 * Copyright 2021 Philipp Mayer - pmayer@cs.sbg.ac.at
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "translationKinetic.h"

/*
	P ~ R ~ I ~ V ~ A ~ T ~ E 	M ~ E ~ M ~ B ~ E ~ R 	F ~ U ~ N ~ C ~ T ~ I ~ O ~ N ~ S
*/

/*
	Private Constructor:
	Generates a new translation of any type, whereas the public constructor is just capable
	of generating translations of type DEFAULT. Pick the moving vertex and its neighbors from
	the triangulations vertices list and initialize the translation path.

	@param 	Tr 		The triangulation the moving vertex lives in
	@param 	i 		The vertex to move
	@param 	dX 		The x-component of the translation vector
	@param 	dY 		The y-component of the translation vector
	@param	type 	The type of the translation

	Note:
		A translation of non-DEFAULT type can just be generated by a DEFAULT translation which
		must be split.
*/
TranslationKinetic::TranslationKinetic(Triangulation *Tr, Vertex *v, double dX, double dY, TranslationType tp) : 
	Translation(Tr, v, dX, dY), split(false), type(tp), actualTime(0){

	transPath = new TEdge(oldV, newV);

	Q = new EventQueue(original, oldV, newV);
}

/*
	The function checkSplit() determines whether a translation can be executed directly or must
	be split into two translation. This corresponds to the question whether the translation path
	intersects any polygon edge or not. If the translation must be split the function sets the
	internal split flag. If the split flag was already set by the function checkOrientation() it 
	does nothing.
*/
void TranslationKinetic::checkSplit(){
	// In case checkOrientation() has already demanded a split
	if(split)
		return;

	split = !checkEdge(original, transPath);
}

/*
	The function generateInitialQueue() generates the initial version of the event queue. Therefore
	it decides for each triangle containing the moving vertex, whether the triangle will collapse
	during the translation. To achieve this it gets checked whether the opposite edge of the triangle
	lays between the start and the target position of the moving vertex.
	After inserting all collapsing triangles with their estimated collapse times into the event queue
	it lets check the event queue the stability of the event ordering.

	@return 	True if the event queue is stable, otherwise false

	Note:
		This function also checks whether any of the triangles is zero and tries to repair it. If
		it finds something not repairable it errors with exit code 7.
*/
bool TranslationKinetic::generateInitialQueue(){
	double t;
	std::list<Triangle*> triangles = (*original).getTriangles();
	TEdge *opposite;
	Vertex *v0, *v1;
	double areaOld, areaNew;
	Triangle *tr;
	bool ok;

	for(auto& i : triangles){
		opposite = (*i).getEdgeNotContaining(original);
		v0 = (*opposite).getV0();
		v1 = (*opposite).getV1();

		tr = new Triangle(v0, v1, oldV);
		areaOld = (*tr).signedArea();
		delete tr;

		// If the vertex lays at an edge at the beginning of the translation then try to flip
		if(areaOld == 0){
			opposite = (*i).getLongestEdgeAlt();

			// If the longest edge is a polygon edge, then we have an error here
			if((*opposite).getEdgeType() == EdgeType::POLYGON){
				fprintf(stderr, "The vertex %llu to be translated lays exactly on a polygon edge :0\n", (*original).getID());
				exit(7);
			}

			// Otherwise we can do a flip, but will reject the translation because this flip will
			// crash the iterator of this loop
			if(Settings::feedback == FeedbackMode::VERBOSE)
				fprintf(stderr, "Numerical correction: The moving vertex lays exactly on an edge before the translation -> security flip\n");

			flip(i, true);
			return false;
		}

		tr = new Triangle(v0, v1, newV);
		areaNew = (*tr).signedArea();
		delete tr;

		// Note: the triangle will also collapse if areaNew is exactly zero (zero can have both signs)
		if((areaNew != 0) && (signbit(areaOld) == signbit(areaNew)))
			continue;
		else{
			t = (*i).calculateCollapseTime(original, dx, dy);

			if(t < 0){
				if(Settings::feedback == FeedbackMode::VERBOSE)
					fprintf(stderr, "Numerical correction: Collapse time of collapsing triangle was %.20f \n", t);

				t = 0;
			}

			if(t > 1){
				if(Settings::feedback == FeedbackMode::VERBOSE)
					fprintf(stderr, "Numerical correction: Collapse time of collapsing triangle was %.20f \n", t);

				t = 1;
			}

			(*i).enqueue();

			(*Q).insertWithoutCheck(t, i);
		}
	}

	ok = (*Q).makeStable(true);

	return ok;
}

/*
	The function repairEnd() checks at the end of a translation whether there exist any triangle
	with area 0 connected to the moving vertex. If such a triangle exists it tries to flip it
	if possible, otherwise it tries to move the vertex back by 10% of the translation.

	Note:
		As the checkIntersection functions tries to keep vertices far from edges this should never
		be the case besides such split translation where the vertex is moved exactly to a
		triangulation edge on purpose.
*/
void TranslationKinetic::repairEnd(){
	std::list<Triangle*> triangles;
	double area;
	TEdge *edge;
	TranslationKinetic *trans;
	enum Executed ex;

	triangles = (*original).getTriangles();

	for(auto& i : triangles){
		area = (*i).signedArea();

		if(area == 0){
			if(type == TranslationType::DEFAULT && Settings::feedback == FeedbackMode::VERBOSE){
				fprintf(stderr, "Translation: Triangle area = 0 after translation...");
			}

			edge = (*i).getLongestEdgeAlt();

			// Try to do a security flip
			if((*edge).getEdgeType() != EdgeType::POLYGON)
				flip(i, true);
			// Otherwise try to move the vertex back a bit
			else{

				trans = new TranslationKinetic(T, original, - dx * 0.1, - dy * 0.1);
				ex = (*trans).execute();
				delete trans;

				if(ex == Executed::REJECTED){
					fprintf(stderr, "\nTriangle area = 0 after translation: PE can not be fliped\n");
					exit(2);
				}
			}

			if(type == TranslationType::DEFAULT && Settings::feedback == FeedbackMode::VERBOSE)
				fprintf(stderr, "corrected! \n");
		}
	}
}

/*
	The function executeSplitRetainSide() decomposes one translation, which can not be executed
	directly and where the moving vertex stays at the same side of the edge connecting the
	neighboring vertices, and executes the resulting translations. The decomposition is done
	in the following way:
	1. The vertex is moved to the position where the edge from the previous vertex to the old
		position intersects the edge from the next vertex to the new position resp. the edge
		from the next vertex to the old position intersects the edge from the previous vertex
		to the new position (exactly one of the two pairs of edges intersects!)
	2. The vertex is moved from the intersection position of the first part to the final
		position.

	@return 	Indicates whether the execution was rejected, aborted or fully processed

	Note:
		For detailed information why we can do that take a look at my Master Thesis
*/
enum Executed TranslationKinetic::executeSplitRetainSide() const{
	Vertex *intersectionPoint;
	double transX, transY;
	TranslationKinetic *trans;
	enum Executed ex;

	Statistics::splits++;

	// Compute the intersection point to split the translation
	intersectionPoint = getIntersectionPoint(prevOldE, nextNewE);
	if(intersectionPoint == NULL)
		intersectionPoint = getIntersectionPoint(nextOldE, prevNewE);
	if(intersectionPoint == NULL){
		return Executed::REJECTED;
	}

	// First part of the translation to the intersection point
	transX = (*intersectionPoint).getX() - (*oldV).getX();
	transY = (*intersectionPoint).getY() - (*oldV).getY();

	trans = new TranslationKinetic(T, original, transX, transY, TranslationType::SPLIT_PART_1);
	ex = (*trans).execute();

	delete intersectionPoint;
	delete trans;

	if(ex != Executed::FULL)
		return ex;

	// Second part of the translation from the intersection point to the target point
	transX = (*newV).getX() - (*original).getX();
	transY = (*newV).getY() - (*original).getY();

	trans = new TranslationKinetic(T, original, transX, transY, TranslationType::SPLIT_PART_2);
	ex = (*trans).execute();

	delete trans;

	if(ex == Executed::FULL)
		return ex;
	else
		return Executed::PARTIAL;
}

/*
	The function executeSplitChangeSide() decomposes one translation, which can not be
	executed directly and where the moving vertex changes the side of the edge connecting
	the neighboring vertices, and executes the resulting translations. The decomposition
	is done in the following way:
	1. The vertex is moved exactly at the middle between the two neighboring vertices.
	2. The vertex is moved from the position between the two neighboring vertices to its
		final position.

	@return 	Indicates whether the execution was rejected, aborted or fully processed

	Note:
		For detailed information why we can do that take a look at my Master Thesis
*/
enum Executed TranslationKinetic::executeSplitChangeSide(){
	double middleX, middleY;
	double transX, transY;
	TranslationKinetic *trans;
	enum Executed ex;
	TEdge *edge;
	Triangle *t;

	Statistics::splits++;

	// Get translation to end position of the first part which is the middle between the
	// neighboring vertices
	middleX = ((*prevV).getX() + (*nextV).getX()) / 2;
	middleY = ((*prevV).getY() + (*nextV).getY()) / 2;

	// Compute translation vector
	transX = middleX - (*oldV).getX();
	transY = middleY - (*oldV).getY();

	trans = new TranslationKinetic(T, original, transX, transY, TranslationType::SPLIT_PART_1);
	ex = (*trans).execute();

	delete trans;

	if(ex != Executed::FULL)
		return ex;

	// For numerical reasons it is possible that the triangle of the old vertex and the
	// neighboring vertices does not vanish at the time when the vertex arrives between its
	// neighbors, therefore this must be checked and corrected before starting the second
	// translation
	edge = (*prevV).getEdgeTo(nextV);
	if(edge != NULL){
		t = (*edge).getTriangleContaining(original);
		flip(t, true);
	}

	// Get translation from middle to the target position
	transX = (*newV).getX() - (*original).getX();
	transY = (*newV).getY() - (*original).getY();

	trans = new TranslationKinetic(T, original, transX, transY, TranslationType::SPLIT_PART_2);
	ex = (*trans).execute();

	delete trans;

	if(ex == Executed::FULL)
		return ex;
	else
		return Executed::PARTIAL;
}

/*
	The function flip() executes one event by removing the longest edge of the collapsing
	triangle and inserting the other diagonal of the resulting quadrilateral. It errors
	with exit code 3 if the longest edge is a polygon edge. If the flip is no singleFlip
	it also computes whether the the two resulting triangles will also collapse during the
	further translation and in case insert them into the event queue and check its stability.
	This computation is outsourced to the functions insertAfterOppositeFlip() and
	insertAfterNonOppositeFlip().

	@param 	t0 			The collapsing triangle
	@param 	singleFlip 	Indicates whether this flip is part of working off the event queue,
						or it is just a single security flip
	@return 			True if the event queue is still stable, otherwise false

	Note:
		- It is assumed that the area of the collapsing triangle is zero (or at least close
			to 0), i.e. the moving vertex is already shifted to the event time
		- Checking whether a resulting triangle will collapse during the further translation
			is a highly sensible thing! It is not recommend to use the actual position of
			the moving vertex therefore, because the small errors in its position can lead
			to wrong decisions
		- For more information on the method of deciding take a look into my Master Thesis
*/
bool TranslationKinetic::flip(Triangle *t0, const bool singleFlip){
	TEdge *e, *e1, *e2;
	Triangle *t1;
	Vertex *vj0, *vj1; // Joint vertices
	Vertex *vn0, *vn1; // Non-joint vertices
	// We call it an opposite flip, if the flipped edge is the one opposite to the moving vertex,
	// i.e. it does not contain the moving vertex
	bool oppositeFlip = false;
	double x, y;
	// Indicates whether a new triangle has been inserted into the event queue
	bool insertion = false; 
	Vertex *common, *opposite;
	bool internal;

	if(!singleFlip)
		// Move vertex to event time
		(*original).setPosition((*oldV).getX() + dx * actualTime, (*oldV).getY() + dy * actualTime);

	// Get the edge which should be flipped
	e = (*t0).getLongestEdgeAlt();
	if((*e).getEdgeType() == EdgeType::POLYGON){
		fprintf(stderr, "Flip: polygon edge gets deleted\n");
		fprintf(stderr, "id: %llu dx: %f dy: %f \n", (*original).getID(), dx, dy);

		(*T).check();
		exit(3);
	}

	// Check for flip type
	if(!(*e).contains(original))
		oppositeFlip = true;

	// Remove the other triangle from the event queue if it is enqueued
	t1 =(*e).getOtherTriangle(t0);
	if((*t1).isEnqueued())
		(*Q).remove(t1);

	// Get all vertices of the triangles which are removed
	vj0 = (*e).getV0();
	vj1 = (*e).getV1();
	vn0 = (*t0).getOtherVertex(e);
	vn1 = (*t1).getOtherVertex(e);

	// Find out whether the triangles are internal or external
	// As e is not allowed to be a polygon edge, both triangles must
	// be at the same side of the polygon
	internal = (*t0).isInternal();

	// This automatically also deletes the two triangles
	delete e;

	// New triangle vn0, vn1, vj0
	e = new TEdge(vn0, vn1);
	(*T).addEdge(e, 0);

	e1 = (*vj0).getEdgeTo(vn0);
	e2 = (*vj0).getEdgeTo(vn1);

	t0 = new Triangle(e, e1, e2, vn0, vn1, vj0, internal);

	// New triangle vn0, vn1, vj1
	(*T).addEdge(e, 0);

	e1 = (*vj1).getEdgeTo(vn0);
	e2 = (*vj1).getEdgeTo(vn1);

	t1 = new Triangle(e, e1, e2, vn0, vn1, vj1, internal);

	if(!singleFlip){
		
		// Add the flip to the flip stack
		if(Settings::localChecking)
			FlipStack.push(new Flip(vj0, vj1, vn0, vn1));

		// Reset coordinates temporarily to original position for the calculation of the event time
		x = (*original).getX();
		y = (*original).getY();
		(*original).setPosition((*oldV).getX(), (*oldV).getY());

		if(oppositeFlip){
			
			// Decide which vertex is the non-moving vertex both new triangles share for an
			// opposite flip 
			if(*vn0 == *original)
				common = vn1;
			else
				common = vn0;

			insertion = insertAfterOppositeFlip(t0, t1, vj0, vj1, common);

		}else{
			
			// Decide which vertex is the non-shared vertex of the triangle which does not
			// change anymore
			if(*vj0 == *original)
				opposite = vj1;
			else
				opposite = vj0;

			// Get the triangle which still contains the moving vertex
			if((*t0).contains(original))
				insertion = insertAfterNonOppositeFlip(t0, vn0, vn1, opposite);
			else
				insertion = insertAfterNonOppositeFlip(t1, vn0, vn1, opposite);
		}

		// Get original back to its actual position
		(*original).setPosition(x, y);		

		if(insertion)
			return (*Q).makeStable(false);
		else
			return true;
	}

	return true;
}

/*
	The function insertAfterOppositeFlip() decides whether newly generated triangles after
	an opposite flip (the edge not containing the moving vertex has been flipped) has to be
	inserted into the event queue. Therefore it does not use the actual position of the 
	moving vertex as this position may be corrupted by numerical inaccuracies. Instead just
	all static vertices of the triangles are used.

	@param 	leftT 	The left one of the new triangles
	@param 	rightT 	The right one of the new triangles
	@param 	leftV 	The vertex which is only contained by the left triangle
	@param 	rightV 	The vertex which is only contained by the right triangle
	@param 	common 	The vertex which is contained by both triangles, but is not the moving
					vertex
	@return 		True if one of the triangles has been inserted into the event queue,
					otherwise false

	Note:
		- For detailed information on the decision criteria take a look into my Master Thesis
		- Left and right may not be really left and right, but this does not matter as long
			as leftV corresponds to leftT and rightV to rightT
*/
bool TranslationKinetic::insertAfterOppositeFlip(Triangle * leftT, Triangle * rightT, Vertex *leftV,
	Vertex * rightV, Vertex * common) const{

	Vertex *dummyVertex;
	Triangle *dummyTriangle;
	double area0, area1, time;
	bool insertion = false;

	// First decide whether the common vertex is inside the corridor built by the two lines
	// parallel to the transition line through the non-shared vertices
	dummyVertex = (*leftV).getTranslated(dx, dy);
	dummyTriangle = new Triangle(leftV, dummyVertex, common);
	area0 = (*dummyTriangle).signedArea();
	delete dummyTriangle;
	delete dummyVertex;

	dummyVertex = (*rightV).getTranslated(dx, dy);
	dummyTriangle = new Triangle(rightV, dummyVertex, common);
	area1 = (*dummyTriangle).signedArea();
	delete dummyTriangle;
	delete dummyVertex;

	// The common vertex is inside the corridor,
	// i.e. both new triangles will collapse in the future
	if(signbit(area0) != signbit(area1)){
		// Now we have to check for both new triangles, whether they collapse before
		// the end of the translation

		// For leftT (consisting of leftV, common and original)
		// we have to check the edge from the common vertex to leftV
		dummyTriangle = new Triangle(leftV, common, oldV);
		area0 = (*dummyTriangle).signedArea();
		delete dummyTriangle;

		dummyTriangle = new Triangle(leftV, common, newV);
		area1 = (*dummyTriangle).signedArea();
		delete dummyTriangle;

		// Note: the triangle will also collapse if the test triangle with the new
		// vertex is exact zero (which can have both signs)
		if((area1 == 0) || (signbit(area0) != signbit(area1))){
			time = (*leftT).calculateCollapseTime(original, dx, dy);
			(*Q).insertWithoutCheck(time, leftT);
			(*leftT).enqueue();
			insertion = true;
		}

		// For rightT (consisting of rightV, common and original)
		// we have to check the edge from the common vertex to rightV
		dummyTriangle = new Triangle(rightV, common, oldV);
		area0 = (*dummyTriangle).signedArea();
		delete dummyTriangle;

		dummyTriangle = new Triangle(rightV, common, newV);
		area1 = (*dummyTriangle).signedArea();
		delete dummyTriangle;

		// Note: the triangle will also collapse if the test triangle with the new
		// vertex is exact zero (which can have both signs)
		if((area1 == 0) || (signbit(area0) != signbit(area1))){
			time = (*rightT).calculateCollapseTime(original, dx, dy);
			(*Q).insertWithoutCheck(time, rightT);
			(*rightT).enqueue();
			insertion = true;
		}

	// The common vertex is outside of the corridor
	}else{
		// Now we have to find out which of the two new triangles is the one which collapses
		// in the future

		// It holds:
		// If the common vertex lays out to the left, then rightT will collapse

		// Now we can take a look whether the common vertex and the moving vertex are on
		// different sides of the line parallel to the transition line through the leftV

		dummyVertex = (*leftV).getTranslated(dx, dy);

		dummyTriangle = new Triangle(leftV, dummyVertex, common);
		area0 = (*dummyTriangle).signedArea();
		delete dummyTriangle;

		// ALARM:
		// I accidentally assigned both areas to area0, but it worked :O
		// Hopefully it also works if I correct it
		dummyTriangle = new Triangle(leftV, dummyVertex, original);
		area1 = (*dummyTriangle).signedArea();
		delete dummyTriangle;

		delete dummyVertex;

		// If they are on different sides, then rightT is the triangle which will collapse
		// in the future
		if(signbit(area0) == signbit(area1)){
			// Now we have to check whether leftT collapses before the end of the translation,
			// i.e. start and end position have to be on different sides of the edge from leftV
			// to common

			dummyTriangle = new Triangle(leftV, common, oldV);
			area0 = (*dummyTriangle).signedArea();
			delete dummyTriangle;

			dummyTriangle = new Triangle(leftV, common, newV);
			area1 = (*dummyTriangle).signedArea();
			delete dummyTriangle;

			if((area1 == 0) || (signbit(area0) != signbit(area1))){
				time = (*leftT).calculateCollapseTime(original, dx, dy);
				(*Q).insertWithoutCheck(time, leftT);
				(*leftT).enqueue();
				insertion = true;
			}
		}else{
			// Now we have to check whether righT collapses before the end of the translation,
			// i.e. start and end position have to be on different side of the edge from rightV
			// to common

			dummyTriangle = new Triangle(rightV, common, oldV);
			area0 = (*dummyTriangle).signedArea();
			delete dummyTriangle;

			dummyTriangle = new Triangle(rightV, common, newV);
			area1 = (*dummyTriangle).signedArea();
			delete dummyTriangle;

			if((area1 == 0) || (signbit(area0) != signbit(area1))){
				time = (*rightT).calculateCollapseTime(original, dx, dy);
				(*Q).insertWithoutCheck(time, rightT);
				(*rightT).enqueue();
				insertion = true;
			}
		}
	}

	return insertion;
}

/*
	The function insertAfterNonOppositeFlip() decides whether newly generated triangles after
	a non-opposite flip (one of the edges containing the moving vertex has been flipped) has to
	be inserted into the event queue. Therefore it does not use the actual position of the 
	moving vertex as this position may be corrupted by numerical inaccuracies. Instead just
	all static vertices of the triangles are used.

	@param 	t 			The one new triangle which can potentially collapse during the further
						translation
	@param 	shared0 	One of the two vertices which are shared by the two new triangles
	@param 	shared1 	The other of the two shared vertices
	@param 	opposite 	The non-shared vertex of the triangle which does not contain the
						moving vertex
	@return 			True if the triangles has been inserted into the event queue,
						otherwise false

	Note:
		- For detailed information on the decision criteria take a look into my Master Thesis
*/
bool TranslationKinetic::insertAfterNonOppositeFlip(Triangle *t, Vertex * shared0, Vertex * shared1,
	Vertex *opposite) const{

	Triangle *dummyTriangle;
	double area0, area1, time;

	// The new triangle will collapse in the future if the non-joint vertex of the triangle
	// which won't change anymore is on the same side of the new edge as the target position
	// is

	dummyTriangle = new Triangle(shared0, shared1, opposite);
	area0 = (*dummyTriangle).signedArea();
	delete dummyTriangle;

	dummyTriangle = new Triangle(shared0, shared1, newV);
	area1 = (*dummyTriangle).signedArea();
	delete dummyTriangle;

	// If the vertices are not on different sides, none of the triangles will collapse in
	// the future
	// UNSAVE:
	// area1 == 0
	if((signbit(area0) == signbit(area1))){
		// Now we have to check whether the new triangle will collapse before the
		// translation ends, i.e. the start position end the end position of the movements
		// are on different sides of the new edge which is given by vn0 and vn1

		dummyTriangle = new Triangle(shared0, shared1, oldV);
		area0 = (*dummyTriangle).signedArea();
		delete dummyTriangle;

		if((area1 == 0) || (signbit(area0) != signbit(area1))){
			time = (*t).calculateCollapseTime(original, dx, dy);
			(*Q).insertWithoutCheck(time, t);
			(*t).enqueue();
			return true;
		}
	}

	return false;
}

/*
	The function undo() checks whether the moving vertex still lays inside of its 
	surrounding polygon. If it does not the function undoes all executed flips in
	reversed order and sets the moving vertex back to its original position.

	@return 	True if the translation has been undone, otherwise false
*/
bool TranslationKinetic::undo(){
	struct Flip *f;
	bool ok;
	Vertex *oldD0, *oldD1;
	Vertex *newD0, *newD1;
	TEdge *e;
	Triangle *t;
	bool internal;

	if(!Settings::localChecking)
		return false;

	ok = (*original).checkSurroundingPolygon();

	if(!ok){

		if(Settings::feedback == FeedbackMode::VERBOSE)
			fprintf(stderr, "Surrounding polygon check after abortion failed...");

		// Undo all flips
		while(!FlipStack.empty()){
			
			f = FlipStack.top();
			FlipStack.pop();

			oldD0 = f -> oldDV0;
			oldD1 = f -> oldDV1;
			newD0 = f -> newDV0;
			newD1 = f -> newDV1;

			// Get the new edge to delete it
			e = (*newD0).getEdgeTo(newD1);

			// Find out whether the triangle are internal or external
			t = (*e).getT0();
			internal = (*t).isInternal();

			delete e;

			// Recreate the old edge
			e = new TEdge(oldD0, oldD1);
			(*T).addEdge(e, 0);

			// And the old triangles
			new Triangle(e, (*oldD0).getEdgeTo(newD0), (*oldD1).getEdgeTo(newD0), oldD0, oldD1, newD0, internal);
			new Triangle(e, (*oldD0).getEdgeTo(newD1), (*oldD1).getEdgeTo(newD1), oldD0, oldD1, newD1, internal);

			delete f;
		}

		// Reset the vertex to the start position
		(*original).setPosition((*oldV).getX(), (*oldV).getY());

		if(Settings::feedback == FeedbackMode::VERBOSE)
			fprintf(stderr, "translation undone!\n");

		return true;
	}

	return false;
}


/*
	C ~ O ~ N ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R ~ S
*/

/*
	Constructor:
	Generates a new Translation of type DEFAULT. Picks the moving vertex and its
	neighbors from the triangulations vertices list and initialize the translation path.

	@param 	Tr 		The triangulation the moving vertex lives in
	@param 	i 		The index of the moving vertex in the triangulation
	@param 	dX 		The x-component of the translation vector
	@param 	dY 		The y-component of the translation vector

	Note:
		Translations of other types can just be generated by the translation class itself.
*/
TranslationKinetic::TranslationKinetic(Triangulation *Tr, int i, double dX, double dY) :
	Translation(Tr, i, dX, dY), split(false), type(TranslationType::DEFAULT), actualTime(0){

	transPath = new TEdge(oldV, newV);

	Q = new EventQueue(original, oldV, newV);
}


/*
	Constructor:
	Generates a new Translation of type DEFAULT. Picks the moving vertex and its
	neighbors from the triangulations vertices list and initialize the translation path.

	@param 	Tr 		The triangulation the moving vertex lives in
	@param 	v 		The vertex to move
	@param 	dX 		The x-component of the translation vector
	@param 	dY 		The y-component of the translation vector

	Note:
		Translations of other types can just be generated by the translation class itself.
*/
TranslationKinetic::TranslationKinetic(Triangulation *Tr, Vertex *v, double dX, double dY) :
	Translation(Tr, v, dX, dY), split(false), type(TranslationType::DEFAULT), actualTime(0){

	transPath = new TEdge(oldV, newV);

	Q = new EventQueue(original, oldV, newV);
}


/*
	O ~ T ~ H ~ E ~ R ~ S
*/

/*
	The function checkOrientation() checks whether the polygon would change its orientation by
	this translation. This basically means that the moving vertex is shifted across the whole
	polygon which corresponds to all other vertices and edges being inside of the quadrilateral
	formed by the oldV, the newV and their neighboring edges. Obviously if the quadrilateral is
	not simple, this can not happen at all. If the quadrilateral is simple then it corresponds
	to having one of the other vertices of the polygon inside the quadrilateral, because if
	one vertex is inside it follows all other vertices must be inside or at least one is outside
	so it exist at least one edge intersecting the quadrilateral so the translation can not lead
	to a simple polygon at all.
	For polygons with holes it also checks whether the outer polygon rolls over an inner one or
	an inner polygon rolls over another one. Additionally it checks whether a vertex passes one
	of the inner polygons in a way, that one polygon edge would crash into the inner polygon.
	For some of these cases it is possible to solve it by splitting the translation. For these
	cases it sets the split flag of the translation.

	@return 	True if the polygon would change its orientation or rolls over another inner 
				polygon, otherwise false

	Note:
		For more information on how to check which cases can be solved by split translations
		take a look at my Master Thesis
*/
bool TranslationKinetic::checkOrientation(){
	bool orientationChange;
	Vertex *randomV;
	unsigned int i;
	Triangle *dummy;
	double areaOld, areaNew;
	bool inside0, inside1;
	TEdge *triangleEdge;

	// At first we check whether the moving vertex passes by another polygon
	// Note:
	// It can just pass by an inner polygon
	for(i = 1; i <= (*T).getActualNrInnerPolygons(); i++){

		// Skip the changing polygon itself
		if((*original).getPID() == i)
			continue;

		// Get a random vertex of the polygon
		randomV = (*T).getVertex(0, i);

		inside0 = Triangle::insideTriangle(oldV, newV, prevV, randomV);
		inside1 = Triangle::insideTriangle(oldV, newV, nextV, randomV);

		// TODO:
		// is this really correct and not the wrong way around?
		if(inside0 && inside1){
			split = true;
			continue;
		}

		if(inside0 || inside1)
			return true;
	}

	// Now we check whether the polygon rolls over another polygon or itself

	// Check whether the quadrilateral of the chosen Vertex P, its translated version P' and the
	// two neighbors M and N is simple, otherwise there can not be any orientation change
	orientationChange = !(checkIntersection(prevOldE, nextNewE, false) != IntersectionType::NONE || checkIntersection(nextOldE, prevNewE, false) != IntersectionType::NONE);

	if(!orientationChange)
		return false;


	// Now check for the polygon itself whether another random vertex of it lays inside the polygon,
	// i.e. that its orientation would be changed by the translation

	// Special Case:
	// If the polygon has a size of 3, we have to check whether the moving vertex changes the
	// side of the opposing edge
	if((*original).getActualPolygonSize() == 3){
		dummy = new Triangle(prevV, nextV, oldV);
		areaOld = (*dummy).signedArea();
		delete dummy;

		dummy = new Triangle(prevV, nextV, newV);
		areaNew = (*dummy).signedArea();
		delete dummy;

		// This already checks whether the vertex moves onto its opposing edge
		if(signbit(areaOld) != signbit(areaNew) || fabs(areaNew) <= Settings::epsInt)
			return true;

	// Default Case:
	}else{
		// Check for vertex before the previous vertex whether it is inside the quadrilateral
		randomV = (*prevV).getPrev();

		orientationChange = insideQuadrilateral(randomV);

		// Check also for a second vertex to increase the chance to reject non-simple translation
		randomV = (*nextV).getNext();

		orientationChange = orientationChange || insideQuadrilateral(randomV);

		if(orientationChange)
			return true;
	}

	
	// Now we also have to check for all other polygons whether the polygon which is changing
	// rolls over them

	// Note:
	// It is not possible that an inner polygon rolls over the outer one, so here we
	// just have to check the inner ones

	for(i = 1; i <= (*T).getActualNrInnerPolygons(); i++){

		// Skip the changing polygon itself
		if((*original).getPID() == i)
			continue;

		// Get a random vertex of the polygon
		randomV = (*T).getVertex(0, i);

		orientationChange = insideQuadrilateral(randomV);

		if(orientationChange)
			return true;
	}

	return false;
}

/*
	The function execute() processes a translation. First it calls checkSplit() for checking
	whether the translation can be executed directly or must be split. If the flag split is set,
	it calls one of the functions which split the translation into two translations
	(depending on the geometric case) and executes them, otherwise execute() executes the
	translation by successively working through the events in the event queue.

	@return 	Indicates whether the execution was rejected, aborted or fully processed

	Note:
		For more information on the splits see my Master Thesis
*/
enum Executed TranslationKinetic::execute(){
	Triangle *t = NULL;
	std::pair<double, Triangle*> e;
	double oldArea, newArea;
	bool undone;
	
	// First check whether the translation can be executed directly or not
	if(type == TranslationType::DEFAULT)
		checkSplit();

	// The translation must be split into two translations
	if(split){
		t = new Triangle(prevV, nextV, oldV);
		oldArea = (*t).signedArea();
		delete t;

		t = new Triangle(prevV, nextV, newV);
		newArea = (*t).signedArea();
		delete t;

		// Vertex stays on the same side of the edge between the neighboring vertices
		if(signbit(oldArea) == signbit(newArea))
			return executeSplitRetainSide();
		// Vertex changes side
		else
			return executeSplitChangeSide();
	
	// Default translation
	}else{

		if(!generateInitialQueue())
			return Executed::REJECTED;

		// Do till all events are processed
		while((*Q).size() > 0){
			e = (*Q).pop();
			actualTime = e.first;
			t = e.second;

			// Abort if the event queue becomes unstable
			if(!flip(t, false)){
				undone = undo();
				if(undone)
					return Executed::UNDONE;
				else
					return Executed::PARTIAL;
			}
		}

		// Move the vertex to its target position
		(*original).setPosition((*newV).getX(), (*newV).getY());

		return Executed::FULL;
	}
}


/*
	D ~ E ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R
*/

/*
	Destructor:
	Checks the surrounding polygon of the moved vertex and deletes all the remaining construction
	vertices and edges. It errors with exit code 6 if the surrounding polygon check fails. It also
	deletes the flip stack.
*/
// TODO:
// Does it still make sense to check the surrounding polygon here?
TranslationKinetic::~TranslationKinetic(){
	bool ok;
	struct Flip *f;
	std::list<Triangle*> triangles;

	// Update the edge lengths in the SelectionTree
	if(Settings::weightedEdgeSelection){
		(*prevOldE).updateSTEntry();
		(*nextOldE).updateSTEntry();
	}

	// And also the weights of the internal triangles incident to original
	if(!Settings::holeInsertionAtStart){
		triangles = (*original).getTriangles();

		for(auto& i : triangles)
			(*i).updateSTEntry();
	}

	// Delete the flip stack
	while(!FlipStack.empty()){
		f = FlipStack.top();
		FlipStack.pop();

		delete f;
	}

	repairEnd();

	ok = (*original).checkSurroundingPolygon();

	if(!ok){
		fprintf(stderr, "\nstart position:\n");
		(*oldV).print();
		fprintf(stderr, "original position:\n");
		(*original).print();
		fprintf(stderr, "target position:\n");
		(*newV).print();
		fprintf(stderr, "translation vector: dx = %.20f dy = %.20f \n", dx, dy);

		(*T).writeTriangulation("failure.graphml");

		exit(6);
	}
	
	delete transPath;
	delete prevNewE;
	delete nextNewE;
	delete oldV;
	delete newV;
}
