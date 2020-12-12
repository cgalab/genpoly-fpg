/* 
 * Copyright 2020 Philipp Mayer - philmay1992@gmail.com
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

#include "translationRetriangulation.h"

void TranslationRetriangulation::buildPolygonsSideChange(){
	bPSCOppositeDirection();
	bPSCTranslationDirection();
}

void TranslationRetriangulation::bPSCOppositeDirection(){
	Triangle *t, *tTest, *oldTriangle;
	Vertex *v;
	TEdge *e, *oldEdge, *edgeToRemove;
	double areaOld, areaNew;

	// Find the polygon in opposite direction and remove the corresponding triangles

	// First of all we have to find the triangle in the correct direction of the
	// surrounding polygon
	tTest = new Triangle(prevV, oldV, nextV);
	areaNew = (*tTest).signedArea();
	delete tTest;

	// We simple check one triangle and take the other one if it is not the right
	// one
	t = (*prevOldE).getT0();
	v = (*t).getOtherVertex(prevOldE);
	tTest = new Triangle(prevV, oldV, v);
	areaOld = (*tTest).signedArea();
	delete tTest;

	// Make sure to take the triangle in the right direction
	// TODO:
	// Maybe this condition is not complete
	if(signbit(areaOld) == signbit(areaNew) || (*v).getID() == (*nextV).getID())
		t = (*prevOldE).getT1();
	
	// Start building the polygon
	p0 = new Polygon(T, PolygonType::STARSHAPED);

	(*p0).addVertex(prevV);

	// Now we go along the surrounding polygon from prevOldE until we reach
	// nextOldE and remove all triangles and edges inside the surrounding polygon
	e = prevOldE;
	oldEdge = NULL;
	edgeToRemove = NULL;
	while((*e).getID() != (*nextOldE).getID()){
		(*p0).addEdge((*t).getEdgeNotContaining(original));

		// Go further to the next triangle
		e = (*t).getOtherEdgeContaining(original, e);
		(*p0).addVertex((*e).getOtherVertex(original));
		oldTriangle = t;
		t = (*e).getOtherTriangle(oldTriangle);

		// Remove the previous triangle
		delete oldTriangle;

		// Make sure to only remove the right edges
		// The first remove takes place in the second loop iteration and
		// is the edge of the first iteration, so it is delayed
		if(edgeToRemove != NULL)
			delete edgeToRemove;
		if(oldEdge != NULL)
			edgeToRemove = oldEdge;
		oldEdge = e;
	}

	// Remove the remaining edge
	if(edgeToRemove != NULL)
		delete edgeToRemove;

	// Close the polygon with the edge between the two adjacent vertices
	e = (*prevV).getEdgeTo(nextV);

	// In case the triangulation does not consist this edge already, we have to
	// generate it now
	if(e == NULL){
		e = new TEdge(prevV, nextV);
		(*T).addEdge(e, 0);

		// Add a new triangle to the triangulation
		new Triangle(e, prevOldE, nextOldE, prevV, nextV, original);
	}
	
	(*p0).close(e);
	(*p0).setKernel(oldV);
}

void TranslationRetriangulation::bPSCTranslationDirection(){
	std::list<TEdge*> edgesToRemove;
	Triangle *t = NULL, *test, *t1, *t2, *t3;
	TEdge *e = NULL, *e1, *e2, *e3;
	Vertex *v = NULL, *v1, *v2, *v3;
	std::vector<TEdge*> surEdges;
	double areaOther, areaTest;
	bool leavesSP1 = true, leavesSP2 = true;

	/*
		P O L Y G O N   C O N T A I N I N G   P R E V V
	*/

	// Start the polygon containing prevV
	p1 = new Polygon(T, PolygonType::EDGEVISIBLE);
	(*p1).addVertex(prevV);

	// Get all edges of the surrounding polygon of prevV
	surEdges = (*prevV).getSurroundingEdges();

	// Search for the triangle through which the new edge leaves the surrounding polygon
	for(auto& i : surEdges){

		if(checkIntersection(prevNewE, i, false) != IntersectionType::NONE){
			(*i).setIntersected();
			edgesToRemove.push_back(i);
			e = i;
			break;
		}
	}

	// In case the new edge does not leave the surrounding polygon, we are already
	// done with this edge
	// Otherwise we have to go further through the triangulation until the edge
	// ends in one triangle
	if(e == NULL)
		leavesSP1 = false;
	else{
		// Check the orientation of nextV relative to prevNewE
		test = new Triangle(prevV, newV, nextV);
		areaOther = (*test).signedArea();
		delete test;

		// Get the right vertex
		v = (*e).getV0();
		test = new Triangle(prevV, newV, v);
		areaTest = (*test).signedArea();
		delete test;

		if(signbit(areaOther) == signbit(areaTest))
			v = (*e).getV1();

		t = (*e).getTriangleContaining(prevV);
		(*p1).addEdge((*t).getEdgeContaining(prevV, v));

		// Advance to the next triangle
		t = (*e).getOtherTriangle(t);

		while(true){
			// Get the other edges of the triangle we are currently in
			surEdges = (*t).getOtherEdges(e);

			// prevNewE intersects the first edge
			if(checkIntersection(prevNewE, surEdges[0], false) != IntersectionType::NONE)
				e = surEdges[0];
			// prevNewE intersects the other edge
			else if(checkIntersection(prevNewE, surEdges[1], false) != IntersectionType::NONE)
				e = surEdges[1];
			// prevNewE does not intersect any further edge
			else
				break;

			(*e).setIntersected();
			edgesToRemove.push_back(e);

			v = (*t).getOtherVertex(e);
			test = new Triangle(prevV, newV, v);
			areaTest = (*test).signedArea();
			delete test;

			if(signbit(areaTest) != signbit(areaOther)){	
				(*p1).addVertex(v);
				(*p1).addEdge((*t).getNotIntersectedEdge());
			}

			// Advance to the next triangle
			t = (*e).getOtherTriangle(t);
		}
	}

	// store the last edge intersected, in case nextV does not intersect any edge
	e1 = e;



	/*
		P O L Y G O N   C O N T A I N I N G   N E X T V
	*/

	e = NULL;

	// Start the polygon containing nextV
	p2 = new Polygon(T, PolygonType::EDGEVISIBLE);
	(*p2).addVertex(nextV);

	// Get all edges of the surrounding polygon of nextV
	surEdges = (*nextV).getSurroundingEdges();

	// Search for the triangle through which the new edge leaves the surrounding polygon
	for(auto& i : surEdges){

		if(checkIntersection(nextNewE, i, false) != IntersectionType::NONE){
			if(!(*i).isIntersected()){
				(*i).setIntersected();
				edgesToRemove.push_back(i);
			}
			e = i;
			break;
		}
	}

	// In case the new edge does not leave the surrounding polygon, we are already
	// done with this edge
	// Otherwise we have to go further through the triangulation until the edge
	// ends in one triangle
	if(e == NULL)
		leavesSP2 = false;
	else{
		// Check the orientation of prevV relative to nextNewE
		test = new Triangle(nextV, newV, prevV);
		areaOther = (*test).signedArea();
		delete test;

		// Get the right vertex
		v = (*e).getV0();
		test = new Triangle(nextV, newV, v);
		areaTest = (*test).signedArea();
		delete test;

		if(signbit(areaOther) == signbit(areaTest))
			v = (*e).getV1();

		t = (*e).getTriangleContaining(nextV);
		(*p2).addEdge((*t).getEdgeContaining(nextV, v));

		// Advance to the next triangle
		t = (*e).getOtherTriangle(t);
		
		while(true){
			// Get the other edges of the triangle we are currently in
			surEdges = (*t).getOtherEdges(e);

			// prevNewE intersects the first edge
			if(checkIntersection(nextNewE, surEdges[0], false) != IntersectionType::NONE)
				e = surEdges[0];
			// prevNewE intersects the other edge
			else if(checkIntersection(nextNewE, surEdges[1], false) != IntersectionType::NONE)
				e = surEdges[1];
			// prevNewE does not intersect any further edge
			else
				break;

			if(!(*e).isIntersected()){
				(*e).setIntersected();
				edgesToRemove.push_back(e);
			}

			v = (*t).getOtherVertex(e);
			test = new Triangle(nextV, newV, v);
			areaTest = (*test).signedArea();
			delete test;

			if(signbit(areaTest) != signbit(areaOther)){
				(*p2).addVertex(v);
				(*p2).addEdge((*t).getNotIntersectedEdge());
			}

			// Advance to the next triangle
			t = (*e).getOtherTriangle(t);
		}
	}

	// In case the new vertex lies in the surrounding polygon of both adjacent vertices, 
	// we can repair the triangulation with one single edge
	if(!leavesSP1 && !leavesSP2){

		// So we do not need the polygons to retriangulate here
		delete p1;
		delete p2;

		p1 = NULL;
		p2 = NULL;

		// Find the other entities of the triangle in which the translation ends and
		// split it into two triangles
		e = (*prevV).getEdgeTo(nextV);
		t = (*e).getTriangleNotContaining(original);
		v = (*t).getOtherVertex(e);

		e1 = new TEdge(original, v);
		(*T).addEdge(e1, 0);

		// Remove the old triangle
		delete t;

		// Fill the empty space with two new triangles
		new Triangle(prevOldE, e1, (*prevV).getEdgeTo(v), prevV, original, v);
		new Triangle(nextOldE, e1, (*nextV).getEdgeTo(v), nextV, original, v);

		return;
	}

	// Restore the last edge intersected of prevOldE, in case nextOldE has not intersected any edge
	if(e == NULL)
		e = e1;

	// Get the vertices of the final triangle
	v1 = (*e).getV0();
	v2 = (*e).getV1();
	v3 = (*t).getOtherVertex(e);

	// Delete all intersected edges
	for(auto& i : edgesToRemove)
		delete i;

	// Case: 
	// Translation ends in a triangle which is incident to the moving vertex
	if((*v1).getID() == (*original).getID() || (*v2).getID() == (*original).getID()
		|| (*v3).getID() == (*original).getID()){

		// The triangle is also incident to prevV
		// => v3 is prevV
		if(!leavesSP1){

			delete p1;
			p1 = NULL;

			if((*v1).getID() == (*original).getID()){
				// => v2 is another vertex to link with
				e1 = new TEdge(original, v2);
				new Triangle(prevOldE, e1, (*v3).getEdgeTo(v2), prevV, original, v2);

				(*p2).addVertex(v2);
			}else{
				// => v1 is another vertex to link with
				e1 = new TEdge(original, v1);
				new Triangle(prevOldE, e1, (*v3).getEdgeTo(v1), prevV, original, v1);

				(*p2).addVertex(v1);
			}

			(*p2).addEdge(e1);
			(*p2).addVertex(original);
			(*p2).close(nextOldE);

			(*T).addEdge(e1, 0);

		// The triangle is also incident to nextV
		// => v3 is nextV
		}else if(!leavesSP2){

			delete p2;
			p2 = NULL;

			if((*v1).getID() == (*original).getID()){
				// => v2 is another vertex to link with
				e1 = new TEdge(original, v2);
				new Triangle(nextOldE, e1, (*v3).getEdgeTo(v2), nextV, original, v2);

				(*p1).addVertex(v2);
			}else{
				// => v1 is another vertex to link with
				e1 = new TEdge(original, v1);
				new Triangle(nextOldE, e1, (*v3).getEdgeTo(v1), nextV, original, v1);

				(*p1).addVertex(v1);
			}

			(*p1).addEdge(e1);
			(*p1).addVertex(original);
			(*p1).close(prevOldE);

			(*T).addEdge(e1, 0);

		// The triangle is not incident to prevV or nextV (it is not possible that
		// it is incident to original, prevV and nextV)
		}else{
			// => v3 is the vertex to close the chain of prevOldE

			e1 = new TEdge(original, v3);
			(*T).addEdge(e1, 0);

			(*p1).addVertex(v3);
			(*p1).addEdge(e1);
			(*p1).addVertex(original);
			(*p1).close(prevOldE);

			if((*original).getID() == (*v1).getID()){
				// => v2 is the vertex to close the chain of nextOldE
				e2 = new TEdge(original, v2);
				(*T).addEdge(e2, 0);

				new Triangle(e1, e2, (*v3).getEdgeTo(v2), original, v2, v3);

				(*p2).addVertex(v2);
				(*p2).addEdge(e2);
				(*p2).addVertex(original);
				(*p2).close(nextOldE);
			}else{
				// => v1 is the vertex to close the chain of nextOldE
				e2 = new TEdge(original, v1);
				(*T).addEdge(e2, 0);

				new Triangle(e1, e2, (*v3).getEdgeTo(v1), original, v1, v3);

				(*p2).addVertex(v1);
				(*p2).addEdge(e2);
				(*p2).addVertex(original);
				(*p2).close(nextOldE);
			}
		}
	// Translation ends in an triangle not incident to original
	}else{

		// v3 is always the vertex opposite to the last intersected edge

		// prevE has not left its SP
		if(!leavesSP1){

			delete p1;
			p1 = NULL;

			e1 = new TEdge(original, v3);
			(*T).addEdge(e1, 0);

			new Triangle(prevOldE, e1, (*prevV).getEdgeTo(v3), original, prevV, v3);

			// Case:
			// v1 is the prevV
			// => v2 will close the chain of nextV
			if((*prevV).getID() == (*v1).getID()){
				
				e2 = new TEdge(original, v2);
				(*T).addEdge(e2, 0);

				new Triangle(e1, e2, (*v3).getEdgeTo(v2), original, v2, v3);

				(*p2).addVertex(v2);
				(*p2).addEdge(e2);
				(*p2).addVertex(original);
				(*p2).close(nextOldE);

			// v2 is the prevV
			// => v1 will close the chain of nextV
			}else{

				e2 = new TEdge(original, v1);
				(*T).addEdge(e2, 0);

				new Triangle(e1, e2, (*v3).getEdgeTo(v1), original, v1, v3);

				(*p2).addVertex(v1);
				(*p2).addEdge(e2);
				(*p2).addVertex(original);
				(*p2).close(nextOldE);
			}

		// nextE has not left its SP
		}else if(!leavesSP2){

			delete p2;
			p2 = NULL;

			e1 = new TEdge(original, v3);
			(*T).addEdge(e1, 0);

			new Triangle(nextOldE, e1, (*nextV).getEdgeTo(v3), original, nextV, v3);

			// Case:
			// v1 is the nextV
			// => v2 will close the chain of prevV
			if((*nextV).getID() == (*v1).getID()){
		
				e2 = new TEdge(original, v2);
				(*T).addEdge(e2, 0);

				new Triangle(e1, e2, (*v3).getEdgeTo(v2), original, v2, v3);

				(*p1).addVertex(v2);
				(*p1).addEdge(e2);
				(*p1).addVertex(original);
				(*p1).close(prevOldE);

			// v2 is the nextV
			// => v1 will close the chain of prevV
			}else{

				e2 = new TEdge(original, v1);
				(*T).addEdge(e2, 0);

				new Triangle(e1, e2, (*v3).getEdgeTo(v1), original, v1, v3);

				(*p1).addVertex(v1);
				(*p1).addEdge(e2);
				(*p1).addVertex(original);
				(*p1).close(prevOldE);
			}

		// Both, prevE and nextE, have left their SP
		}else{

			e1 = new TEdge(v1, original);
			e2 = new TEdge(v2, original);
			e3 = new TEdge(v3, original);

			(*T).addEdge(e1, 0);
			(*T).addEdge(e2, 0);
			(*T).addEdge(e3, 0);

			new Triangle(e1, e3, (*v1).getEdgeTo(v3), v1, original, v3);
			new Triangle(e2, e3, (*v2).getEdgeTo(v3), original, v2, v3);

			t = new Triangle(nextV, newV, prevV);
			areaOther = (*t).signedArea();
			delete t;

			t = new Triangle(nextV, newV, v1);
			areaTest = (*t).signedArea();
			delete t;

			// v1 lies at the same side of (nextV, newV) as prevV and thus,
			// it closes p1 and v2 closes p2
			if(signbit(areaOther) == signbit(areaTest)){

				(*p1).addVertex(v1);
				(*p1).addEdge(e1);
				(*p1).addVertex(original);
				(*p1).close(prevOldE);

				(*p2).addVertex(v2);
				(*p2).addEdge(e2);
				(*p2).addVertex(original);
				(*p2).close(nextOldE);

			// v1 lies at the opposite side of (nextV, newV) than prevV and thus,
			// it closes p2 and v2 closes p1
			}else{
				(*p1).addVertex(v2);
				(*p1).addEdge(e2);
				(*p1).addVertex(original);
				(*p1).close(prevOldE);

				(*p2).addVertex(v1);
				(*p2).addEdge(e1);
				(*p2).addVertex(original);
				(*p2).close(nextOldE);
			}
		}
	}
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
TranslationRetriangulation::TranslationRetriangulation(Triangulation *Tr, int i, double dX, double dY) :
	Translation(Tr, i, dX, dY), p0(NULL), p1(NULL), p2(NULL), p3(NULL){

	Triangle *t0, *t1;
	double areaOld, areaNew;

	t0 = new Triangle(prevV, nextV, oldV);
	t1 = new Triangle(prevV, nextV, newV);

	areaOld = (*t0).signedArea();
	areaNew = (*t1).signedArea();

	sideChange = signbit(areaOld) != signbit(areaNew);

	delete t0;
	delete t1;
}


/*
	O ~ T ~ H ~ E ~ R ~ S
*/

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
enum Executed TranslationRetriangulation::execute(){

	// Find the polygons to retriangulate
	if(sideChange){
		buildPolygonsSideChange();
		(*original).setPosition((*newV).getX(), (*newV).getY());
	}

	// Move the vertex to its target position
	//(*original).setPosition((*newV).getX(), (*newV).getY());

	if(p0 != NULL)
		(*p0).triangulate();

	if(p1 != NULL)
		(*p1).triangulate();

	if(p2 != NULL)
		(*p2).triangulate();

	return Executed::FULL;
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

TranslationRetriangulation::~TranslationRetriangulation(){
	bool ok;

	// Update the edge lengths in the SelectionTree
	if(Settings::weightedEdgeSelection){
		(*prevOldE).updateSTEntry();
		(*nextOldE).updateSTEntry();
	}

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
