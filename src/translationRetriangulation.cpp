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

void TranslationRetriangulation::buildPolygonsSideRemainCase1(){
	bPSRC1OppositeDirection();
	bPSRC1TranslationDirection();
}

void TranslationRetriangulation::buildPolygonsSideRemainCase2(){
	bPSRC2OppositeDirection();
	bPSCTranslationDirection();
}

void TranslationRetriangulation::buildPolygonsSideRemainCase3(){
	Vertex *primaryV, *secondaryV;
	TEdge *primaryE, *secondaryE, *primaryNewE, *secondaryNewE, *borderE = NULL;
	IntersectionType i;


	// First of all we have to find out in which direction the translation goes,
	// i.e. either the vertex is moved from prevV away to nextV or vice versa
	i = checkIntersection(prevNewE, nextOldE, true);
	if(i != IntersectionType::NONE){
		primaryV = prevV;
		primaryE = prevOldE;
		primaryNewE = prevNewE;
		secondaryV = nextV;
		secondaryE = nextOldE;
		secondaryNewE = nextNewE;
	}else{
		primaryV = nextV;
		primaryE = nextOldE;
		primaryNewE = nextNewE;
		secondaryV = prevV;
		secondaryE = prevOldE;
		secondaryNewE = prevNewE;
	}

	bPSRC3OppositeDirection(primaryV, secondaryV, primaryE, secondaryE);

	/*if(id == 1999)
		(*T).writeTriangulation("polygons1.graphml");
	
	if(id == 1999 && p0 != NULL){
		printf("P0:\n");
		(*p0).print();
	}*/

	borderE = bPSRC3SPOld(primaryV, secondaryV, primaryE, secondaryE, primaryNewE, secondaryNewE);

	
	/*if(id == 1999 && p1 != NULL){
		printf("P1:\n");
		(*p1).print();

		(*T).writeTriangulation("polygons2.graphml");
	}*/
	
	

	if(borderE != NULL)
		bPSRC3TranslationDirection(primaryV, secondaryV, borderE, primaryE, primaryNewE, secondaryE, secondaryNewE);	

	/*if(id == 1999 && p2 != NULL){
		printf("P2\n");
		(*p2).print();
	}

	if(id == 1999 && p3 != NULL){
		printf("P3\n");
		(*p3).print();
	}*/
}

void TranslationRetriangulation::bPSRC3OppositeDirection(Vertex *primaryV, Vertex *secondaryV,
	TEdge *primaryE, TEdge *secondaryE){
	TEdge *e = NULL, *SPEdge;
	Vertex *v;
	double area0, area1;
	Triangle *t, *test;
	std::list<TEdge*> edgesToRemove;

	/*
		Find the polygon at the other side of (prevV, nextV)
	*/

	// In case the triangle prevV, original, nextV does already exist,
	// we do not have to do something in this direction
	e = (*primaryV).getEdgeTo(secondaryV);

	if(e == NULL) {
		p0 = new Polygon(T, PolygonType::EDGEVISIBLE);
		(*p0).addVertex(primaryV);

		// Find the right triangle
		t = (*primaryE).getT0();
		v = (*t).getOtherVertex(primaryE);

		test = new Triangle(primaryV, oldV, v);
		area0 = (*test).signedArea();
		delete test;

		test = new Triangle(primaryV, oldV, newV);
		area1 = (*test).signedArea();
		delete test;

		if(signbit(area0) != signbit(area1))
			t = (*primaryE).getT1();

		e = (*t).getOtherEdgeContaining(original, primaryE);
		while((*e).getID() != (*secondaryE).getID()){
			edgesToRemove.push_back(e);

			SPEdge = (*t).getEdgeNotContaining(original);

			(*p0).addEdge(SPEdge);
			(*p0).addVertex((*e).getOtherVertex(original));

			t = (*e).getOtherTriangle(t);
			e = (*t).getOtherEdgeContaining(original, e);
		}

		SPEdge = (*t).getEdgeNotContaining(original);
		(*p0).addEdge(SPEdge);
		(*p0).addVertex(secondaryV);

		(*p0).addEdge(secondaryE);
		(*p0).addVertex(original);
		(*p0).close(primaryE);

		for(auto& i : edgesToRemove)
			delete i;
	}
}

TEdge *TranslationRetriangulation::bPSRC3SPOld(Vertex *primaryV, Vertex *secondaryV, TEdge *primaryE,
		TEdge *secondaryE, TEdge *primaryNewE, TEdge * secondaryNewE){
	Triangle *t = NULL;
	TEdge *e, *SPEdge, *e0, *e1, *e2, *borderE = NULL;
	Vertex *v, *borderV;
	std::list<TEdge*> edgesToRemove;

	// First we have to find the right triangle
	// For that we can exploit the fact, that we have already deleted
	// the triangles in the other direction (except if there is just one triangle)
	t = (*primaryE).getT0();

	if(t == NULL || (*(*t).getOtherVertex(primaryE)).getID() == (*secondaryV).getID())
		t = (*primaryE).getT1();

	p1 = new Polygon(T, PolygonType::STARSHAPED);
	(*p1).addVertex(primaryV);

	e = (*t).getOtherEdgeContaining(original, primaryE);
	v = (*t).getOtherVertex(primaryE);
	SPEdge = (*t).getEdgeNotContaining(original);

	// Note: In the first iteration SPEdge contains primaryV and thus, intersects primaryNewW
	// This intersection must be filtered
	while((*SPEdge).contains(primaryV) || (!insideTriangle(t, newV) && checkIntersection(SPEdge, primaryNewE, true) == IntersectionType::NONE)){
		(*p1).addEdge(SPEdge);
		(*p1).addVertex(v);

		edgesToRemove.push_back(e);

		t = (*e).getOtherTriangle(t);
		e = (*t).getOtherEdgeContaining(original, e);
		SPEdge = (*t).getEdgeNotContaining(original);
		v = (*e).getOtherVertex(original);
	}

	borderV = (*t).getOtherVertex(e);

	if(insideTriangle(t, newV)){

		// Close p1
		e1 = new TEdge(borderV, original);
		(*T).addEdge(e1, 0);

		(*p1).addEdge(e1);
		(*p1).addVertex(original);
		(*p1).close(primaryE);

		(*p1).setKernel(oldV);

		// Remove all marked edges now to avoid registering to many triangles
		// at one edge
		// At first we have to engage to the next triangle
		t = (*e).getOtherTriangle(t);

		for(auto & i : edgesToRemove)
			delete i;
		edgesToRemove.clear();


		/*
			Start the second polygon along secondaryE
		*/

		// First check, whether a second polygon is required
		// It is not required if the actual triangle contains secondaryV
		if((*v).getID() == (*secondaryV).getID())			

			new Triangle(SPEdge, secondaryE, e1, original, borderV, secondaryV);

		else{

			e2 = new TEdge(v, original);
			(*T).addEdge(e2, 0);
			new Triangle(SPEdge, e1, e2, original, borderV, v);

			p2 = new Polygon(T, PolygonType::EDGEVISIBLE);

			(*p2).addVertex(original);
			(*p2).addEdge(e2);
			(*p2).addVertex(v);

			while((*e).getID() != (*secondaryE).getID()){
				edgesToRemove.push_back(e);

				e = (*t).getOtherEdgeContaining(original, e);
				v = (*e).getOtherVertex(original);
				SPEdge = (*t).getEdgeNotContaining(original);

				(*p2).addEdge(SPEdge);
				(*p2).addVertex(v);

				t = (*e).getOtherTriangle(t);
			}

			(*p2).close(secondaryE);
		}

		for(auto & i : edgesToRemove)
			delete i;

		borderE = NULL;

	}else{

		// First of all we have to check whether borderV can see primaryV
		// Otherwise we can not insert an edge between them and thus, have
		// to abort the translation
		if(!checkVisibility(borderV, primaryV, secondaryV)){

			delete p1;
			p1 = NULL;

			aborted = true;

			return NULL;
		}


		// Remove all triangles now to avoid registering to much triangles
		// at one edge	
		for(auto & i : edgesToRemove)
			delete i;

		borderE = SPEdge;

		// Close p1, in case it has at least three vertices
		if((*primaryV).getEdgeTo(borderV) != NULL){

			delete p1;
			p1 = NULL;

		}else{

			e = new TEdge(borderV, primaryV);
			(*T).addEdge(e, 0);

			(*p1).close(e);

			if(Translation::insideTriangle(borderV, newV, primaryV, oldV))
				(*p1).changeType(PolygonType::EDGEVISIBLE);
			else
				(*p1).setKernel(oldV);
		}

	}

	return borderE;
}

void TranslationRetriangulation::bPSRC3TranslationDirection(Vertex *primaryV, Vertex *secondaryV,
	TEdge *borderE, TEdge *primaryE, TEdge *primaryNewE, TEdge *secondaryE, TEdge *secondaryNewE){
	
	Triangle *t, *test;
	std::vector<TEdge*> surEdges;
	TEdge *e = NULL, *SPEdge, *e2, *e3;
	Vertex *v, *v2 = secondaryV, *v3; // v2/v3 is the vertex last added to p2/p3,
	std::list<TEdge*> edgesToRemove;
	double areaRef, area;
	bool containsSecondaryV;

	p2 = new Polygon(T, PolygonType::EDGEVISIBLE);
	p3 = new Polygon(T, PolygonType::EDGEVISIBLE);

	(*p2).addVertex(secondaryV);

	// Now we have to check whether borderE does contain secondaryV
	// In case it does, we can directly start with the incident triangle
	// (Note: The other incident triangle has already been deleted before)
	// Otherwise we must at first find a path from secondaryV to the triangle
	if(!(*borderE).contains(secondaryV)){

		surEdges = (*secondaryV).getSurroundingEdges();

		for(auto& i : surEdges){

			if(checkIntersection(secondaryNewE, i, false) != IntersectionType::NONE){
				edgesToRemove.push_back(i);
				e = i;
				break;
			}
		}

		if(e != NULL){
			
			v2 = (*e).getOtherVertex(original);

			(*p2).addEdge((*v2).getEdgeTo(secondaryV));
			(*p2).addVertex(v2);

			// Find the triangle in the right direction
			t = (*e).getTriangleNotContaining(secondaryV);

			while(t != NULL){
				e = (*t).getOtherEdgeContaining(original, e);
				edgesToRemove.push_back(e);

				v2 = (*e).getOtherVertex(original);

				(*p2).addEdge((*t).getEdgeNotContaining(original));
				(*p2).addVertex(v2);
				
				t = (*e).getOtherTriangle(t);
			}

			// If we can not find a next triangle (t == NULL), then we reached borderE
		}		

	}

	// Get the other border vertex
	v3 = (*borderE).getOtherVertex(v2);

	// Now we have to go further starting at borderE
	// We can do both polygons in parallel
	(*p3).addVertex(primaryV);
	(*p3).addEdge((*primaryV).getEdgeTo(v3));
	(*p3).addVertex(v3);

	// Get the triangle in the right direction
	// For that we can exploit the fact that the triangle in the wrong direction
	// has already been deleted
	t = (*borderE).getT0();

	if(t == NULL)
		t = (*borderE).getT1();

	// We need a reference value the check on which side of primaryNewE a vertex is
	test = new Triangle(primaryV, newV, v2);
	areaRef = (*test).signedArea();
	delete test;

	e = borderE;
	edgesToRemove.push_back(e);
	while(true){

		// Get the vertex opposite to the edge by which the triangle was entered
		v = (*t).getOtherVertex(e);

		surEdges = (*t).getOtherEdges(e);

		if(checkIntersection(surEdges[0], primaryNewE, true) != IntersectionType::NONE)
			e = surEdges[0];
		else if(checkIntersection(surEdges[1], primaryNewE, true) != IntersectionType::NONE)
			e = surEdges[1];
		else
			break;

		edgesToRemove.push_back(e);

		test = new Triangle(primaryV, newV, v);
		area = (*test).signedArea();
		delete test;

		if(signbit(areaRef) == signbit(area)){
			(*p2).addEdge((*v2).getEdgeTo(v));
			(*p2).addVertex(v);
			v2 = v;
		}else{
			(*p3).addEdge((*v3).getEdgeTo(v));
			(*p3).addVertex(v);
			v3 = v;
		}

		t = (*e).getOtherTriangle(t);
	}

	v = (*t).getOtherVertex(e);

	// We have to check this before we delete the edges and incident triangles
	containsSecondaryV = (*t).contains(secondaryV);

	// Delete the old edges
	for(auto& i : edgesToRemove)
		delete i;

	// Now we have to close the polygons

	e = new TEdge(original, v);
	(*T).addEdge(e, 0);
	e3 = new TEdge(original, v3);
	(*T).addEdge(e3, 0);

	// The triangle in which the translations ends is incident to secondaryV
	if(containsSecondaryV){
		// In this case p2 is just a triangle which can directly be inserted
		delete p2;
		p2 = NULL;

		new Triangle((*v).getEdgeTo(secondaryV), e, secondaryE, original, v, secondaryV);

	}else{

		e2 = new TEdge(original, v2);
		(*T).addEdge(e2, 0);

		new Triangle(e2, e, (*v2).getEdgeTo(v), v2, original, v);

		(*p2).addEdge(e2);
		(*p2).addVertex(original);
		(*p2).close(secondaryE);
	}

	new Triangle(e3, e, (*v3).getEdgeTo(v), v3, original, v);

	(*p3).addEdge(e3);
	(*p3).addVertex(original);
	(*p3).close(primaryE);
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

	// In case the triangulation does not contain this edge already, we have to
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

void TranslationRetriangulation::bPSRC2OppositeDirection(){
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
	if(signbit(areaOld) != signbit(areaNew))
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

	(*p0).addEdge(nextOldE);
	(*p0).addVertex(original);	
	(*p0).close(prevOldE);

	(*p0).setKernel(oldV);
}

void TranslationRetriangulation::bPSRC1OppositeDirection(){
	Triangle *t, *tTest, *oldTriangle;
	Vertex *v;
	TEdge *e, *oldEdge, *edgeToRemove;
	double areaOld, areaNew;

	// Find the polygon in opposite direction and remove the corresponding triangles

	// First of all we have to find the triangle in the correct direction of the
	// surrounding polygon
	tTest = new Triangle(prevV, oldV, newV);
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
	if(signbit(areaOld) == signbit(areaNew))
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

	(*p0).addEdge(nextOldE);
	(*p0).addVertex(original);	
	(*p0).close(prevOldE);

	(*p0).setKernel(oldV);
}

void TranslationRetriangulation::bPSRC1TranslationDirection(){
	std::list<TEdge*> edgesToRemove;
	Vertex *v;
	TEdge *e = NULL, *newEdge;
	Triangle *t = NULL;
	IntersectionType i;

	// Check Whether there exists the triangle build by prevV, original
	// and nextV
	e = (*prevV).getEdgeTo(nextV);

	// In case it does, we do not have to retriangulate anything here
	if(e != NULL)
		return;


	// First of all we have to find the triangle in the correct direction of the
	// surrounding polygon. For that it helps that the triangle in the other
	// direction has already been removed by the previous function call

	// We simple check one triangle and take the other one if this triangle 
	// does not exist anymore
	t = (*prevOldE).getT0();
	if(t == NULL)
		t = (*prevOldE).getT1();

	// We start with the polygon containing prevV
	p1 = new Polygon(T, PolygonType::EDGEVISIBLE);
	(*p1).addVertex(prevV);

	e = (*t).getOtherEdgeContaining(original, prevOldE);
	i = checkIntersection(e, prevNewE, true);
	while(i == IntersectionType::EDGE && (*e).getID() != (*nextOldE).getID()){
		(*p1).addEdge((*t).getEdgeNotContaining(original));
		(*p1).addVertex((*e).getOtherVertex(original));
		edgesToRemove.push_back(e);

		t = (*e).getOtherTriangle(t);
		e = (*t).getOtherEdgeContaining(original, e);
		i = checkIntersection(e, prevNewE, true);
	}

	// Now e is the first edge which does not intersect prevNewE anymore,
	// but nextNewE
	(*p1).addEdge((*t).getEdgeNotContaining(original));
	v = (*e).getOtherVertex(original);
	(*p1).addVertex(v);

	// So we close the first polygon with a new edge and start the second one
	// (only in case we have not reached nextOldE, otherwise we simply
	// close p1 and do not need a second polygon)
	if((*e).getID() == (*nextOldE).getID()){
		// Close
		(*p1).addEdge(nextOldE);
		(*p1).addVertex(original);
		(*p1).close(prevOldE);
	}else{
		newEdge = new TEdge(v, original);
		(*T).addEdge(newEdge, 0);

		// Close
		(*p1).addEdge(newEdge);
		(*p1).addVertex(original);
		(*p1).close(prevOldE);

		// Start new polygon
		p2 = new Polygon(T, PolygonType::EDGEVISIBLE);
		(*p2).addVertex(original);
		(*p2).addEdge(newEdge);
		(*p2).addVertex(v);

		t = (*e).getOtherTriangle(t);
		edgesToRemove.push_back(e);
		e = (*t).getOtherEdgeContaining(original, e);
		while((*e).getID() != (*nextOldE).getID()){
			(*p2).addEdge((*t).getEdgeNotContaining(original));
			(*p2).addVertex((*e).getOtherVertex(original));
			edgesToRemove.push_back(e);

			t = (*e).getOtherTriangle(t);
			e = (*t).getOtherEdgeContaining(original, e);
		}

		(*p2).addEdge((*t).getEdgeNotContaining(original));
		v = (*e).getOtherVertex(original);
		(*p2).addVertex(v);
		(*p2).close(nextOldE);
	}

	for(auto& i : edgesToRemove)
		delete i;
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

bool TranslationRetriangulation::insideTriangle(Triangle *t, Vertex *toCheck){
	return Translation::insideTriangle((*t).getVertex(0), (*t).getVertex(1), (*t).getVertex(2), toCheck);
}

bool TranslationRetriangulation::checkVisibility(Vertex *borderV, Vertex *primaryV, Vertex *secondaryV){
	TEdge *e = new TEdge(borderV, primaryV), *intersected = NULL;
	bool visible = true;
	std::vector<TEdge*> surEdges;
	IntersectionType it;
	Triangle *t;
	double areaRef, areaBorder;


	// First we have to check whether borderV is turn by at least one pi
	// around original relative to primaryV
	// If it is, then its also visible after the translation
	t = new Triangle(primaryV, original, secondaryV);
	areaRef = (*t).signedArea();
	delete t;

	t = new Triangle(primaryV, original, borderV);
	areaBorder = (*t).signedArea();
	delete t;

	if(signbit(areaRef) == signbit(areaBorder)){
		delete e;

		return true;
	}

	surEdges = (*borderV).getSurroundingEdges();

	for(auto& i : surEdges){

		// Check whether one of the surrounding edges of borderV contains primaryV
		if((*i).contains(primaryV)){
			
			delete e;

			// Now we have to check whether borderV can really see primaryV through
			// the interior of SP of original
			// This is the case, if the edge from borderV to primaryV has an triangle
			// which contains original

			e = (*borderV).getEdgeTo(primaryV);

			t = (*e).getT0();
			if((*t).contains(original))
				return true;

			t = (*e).getT1();
			if((*t).contains(original))
				return true;
			// Otherwise borderV can see primaryV, but only through the exterior
			else
				return false;
		}
		
		it = checkIntersection(e, i, true);

		if(it == IntersectionType::VERTEX){
			visible = false;

			break;
		}else if(it == IntersectionType::EDGE){
			intersected = i;

			visible = (*intersected).contains(original);

			break;
		}
	}


	if(visible && intersected != NULL){

		t = (*intersected).getTriangleNotContaining(borderV);
		surEdges = (*t).getOtherEdges(intersected);

		while(true){

			it = checkIntersection(surEdges[0], e, true);

			if(it == IntersectionType:: NONE){
				it = checkIntersection(surEdges[1], e, true);

				intersected = surEdges[1];
			}else
				intersected = surEdges[0];

			if(it == IntersectionType::NONE)
				break;
			else if(it == IntersectionType::VERTEX){
				visible = false;

				break;
			}else{
				visible = (*intersected).contains(original);

				if(!visible)
					break;

				t = (*intersected).getOtherTriangle(t);

				surEdges = (*t).getOtherEdges(intersected);
			}
		}
	}

	delete e;

	return visible;
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
	Translation(Tr, i, dX, dY), aborted(false), p0(NULL), p1(NULL), p2(NULL), p3(NULL){

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
	bool newInsideOld, oldInsideNew;

	// Find the polygons to retriangulate
	if(sideChange){
		buildPolygonsSideChange();
	}else{
		newInsideOld = Translation::insideTriangle(prevV, oldV, nextV, newV);
		oldInsideNew = Translation::insideTriangle(prevV, newV, nextV, oldV);

		// 1. Case:
		// The vertex is moved into the triangle defined by the old position of the
		// vertex and its adjacent vertices
		if(newInsideOld)

			buildPolygonsSideRemainCase1();

		// 2. Case:
		// The vertex is moved in a way such that the original position of the vertex
		// is inside the triangle defined by the new position and the adjacent vertices
		else if(oldInsideNew)

			buildPolygonsSideRemainCase2();

		// 3. Case:
		// The vertex is moved in a way such that the translation quadrilateral is not
		// simple
		else
			buildPolygonsSideRemainCase3();
	}

	// Move the vertex to its target position
	if(!aborted)
		(*original).setPosition((*newV).getX(), (*newV).getY());

	if(p0 != NULL)
		(*p0).triangulate();

	if(p1 != NULL)
		(*p1).triangulate();

	if(p2 != NULL)
		(*p2).triangulate();

	if(p3 != NULL)
		(*p3).triangulate();

	if(aborted)
		return Executed::UNDONE;
	else
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

	if(p0 != NULL) delete p0;
	if(p1 != NULL) delete p1;
	if(p2 != NULL) delete p2;
	if(p3 != NULL) delete p3;
}
