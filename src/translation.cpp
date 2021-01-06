/* 
 * Copyright 2021 Philipp Mayer - philmay1992@gmail.com
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

#include "translation.h"

/*
	S ~ T ~ A ~ T ~ I ~ C 	V ~ A ~ R ~ I ~ A ~ B ~ L ~ E ~ S
*/

/*
	The number of already generated vertices
*/
unsigned long long Translation::n = 0;


/*
	P ~ R ~ I ~ V ~ A ~ T ~ E 	M ~ E ~ M ~ B ~ E ~ R 	F ~ U ~ N ~ C ~ T ~ I ~ O ~ N ~ S
*/

/*
	The function insideQuadrilateral() checks whether the vertex v lays inside of a quadrilateral
	formed by the edge from oldV to its neighboring  vertices and from newV to its neighboring
	vertices.
	For that it generates a dummy vertex with the same y-coordinate as v and a x-coordinate which
	is the maximum x-coordinate of all vertices of the quadrilateral plus 10. So the dummy vertex
	definitely lays outside of the quadrilateral. Then it checks how often the edge between v
	and the dummy vertex intersects the edges of the quadrilateral. If the number of intersections
	is odd, then v must lay inside of the quadrilateral.

	@param 	v 	The vertex of interest
	@return 	True if v is inside of the quadrilateral, otherwise false

	Note:
		- Source: https://www.geeksforgeeks.org/how-to-check-if-a-given-point-lies-inside-a-polygon/
		- If the dummy edge intersects exactly at a vertex, we will get intersections with two edges,
			which makes the function think, that the vertex is outside of the quadrilateral.
			Therefore the function automatically returns true, if it gets two or more vertex
			intersection anywhere. This leads to rejecting the translation.
*/
bool Translation::insideQuadrilateral(Vertex * const v) const{
	Vertex *dummyVertex;
	TEdge *dummyEdge;
	double maxX, x;
	int count = 0;
	IntersectionType intersection;
	bool vertexInt = false;

	// Find maximum x value
	maxX = (*oldV).getX();
	x = (*newV).getX();
	if(x > maxX)
		maxX = x;
	x = (*prevV).getX();
	if(x > maxX)
		maxX = x;
	x = (*nextV).getX();
	if(x > maxX)
		maxX = x;

	// If v has a greater x-coordinate then all quadrilateral vertices, it can not be inside
	if((*v).getX() > maxX)
		return false;

	// Generate the dummy vertex outside of the quadrilateral
	maxX = maxX + 10;
	dummyVertex = new Vertex(maxX, (*v).getY());

	dummyEdge = new TEdge(v, dummyVertex);

	// Count the intersection
	intersection = checkIntersection(dummyEdge, prevOldE, false);
	if(intersection == IntersectionType::VERTEX)
		vertexInt = true;
	if(intersection != IntersectionType::NONE)
		count++;

	intersection = checkIntersection(dummyEdge, nextOldE, false);
	if(intersection == IntersectionType::VERTEX)
		vertexInt = true;
	if(intersection != IntersectionType::NONE)
		count++;

	intersection = checkIntersection(dummyEdge, prevNewE, false);
	if(intersection == IntersectionType::VERTEX)
		vertexInt = true;
	if(intersection != IntersectionType::NONE)
		count++;

	intersection = checkIntersection(dummyEdge, nextNewE, false);
	if(intersection == IntersectionType::VERTEX)
		vertexInt = true;
	if(intersection != IntersectionType::NONE)
		count++;

	delete dummyEdge;
	delete dummyVertex;

	// If there was any intersection of type vertex return false, so the translation gets
	// refused. The point is, for an vertex intersection we can not say anything for sure
	// especially as the dummyEdge could just intersect with any point of the quadrilateral.
	if(vertexInt)
		return false;

	if(count % 2 == 1)
		return true;
	else
		return false;
}

/*
	The function checkEdge() checks whether the edge newE starting at vertex fromV intersects any
	polygon edge. Therefore it first checks whether newE intersects any edge of the surrounding
	polygon of fromV. If it does not, then it can not intersect any polygon edge, otherwise we
	continue the check in the other triangle assigned to the intersected edge until newE either
	intersects a polygon edge or ends in any triangle.

	@param 	fromV 	The vertex where the new edge starts
	@param 	newE 	The new edge
	@return 		True if the new edge does not intersect any polygon edge, otherwise false

	Note:
		- If the edge goes right through another vertex we also count it as intersecting a polygon
			edge even if the edge we checked was no polygon edge, because a new polygon edge
			obviously is not allowed to go through a vertex.
		- As we had the case that a new edge numerically intersected all three edges of the same
			triangle or multiple edges of the surrounding polygon we do not stop at the first 
			intersection we find, but we count all intersections. If we find more then one, the
			function returns false. But this should now really happen nowhere as the epsilon
			in checkIntersection() should lead to getting all these cases as intersections of
			type VERTEX.
		- Edges of the bounding box are treated as polygon edges
*/
bool Translation::checkEdge(Vertex * const fromV, TEdge * const newE) const{
	std::vector<TEdge*> surEdges;
	enum IntersectionType iType = IntersectionType::NONE;
	enum IntersectionType iType0, iType1;
	TEdge *intersectedE = NULL;
	EdgeType eType;
	Triangle *nextT = NULL;
	int count = 0;
	unsigned int size;
	unsigned int passedTriangles = 1;

	Statistics::nrChecks++;
	Statistics::nrTriangles++;

	surEdges = (*fromV).getSurroundingEdges();

	size = surEdges.size();

	Statistics::nrSPTriangles = Statistics::nrSPTriangles + size;
	if(size > Statistics::maxSPTriangles)
		Statistics::maxSPTriangles = size;

	// Iterate over all edges of the surrounding polygon
	for(auto& i : surEdges){
		iType = checkIntersection(newE, i, false);

		// New edge hits vertex of surrounding polygon
		if(iType == IntersectionType::VERTEX)
			return false;

		// Count intersections to detect numerical errors
		if(iType == IntersectionType::EDGE){
			count++;
			intersectedE = i;
		}
	}

	// No intersection -> the new vertex stays in the surrounding polygon
	if(count == 0){
		return true;
	// Multiple intersections -> numerical error
	}else if(count > 1){
		if(Settings::feedback == FeedbackMode::VERBOSE)
			fprintf(stderr, "CheckEdge: new edge intersects multiple edges of the surrounding polygon -> translation rejected due to numerical problem\n");
		
		return false;
	}

	// One real intersection with an edge of the surrounding polygon
	eType = (*intersectedE).getEdgeType();

	// Intersected edge is a polygon or frame edge
	if(eType != EdgeType::TRIANGULATION){
		return false;
	// Intersected edge is just a triangulation edge
	}else{
		nextT = (*intersectedE).getTriangleNotContaining(fromV);
		surEdges = (*nextT).getOtherEdges(intersectedE);
	}

	passedTriangles++;
	Statistics::nrTriangles++;

	// Iterate over the adjacent triangles if there was an intersection with a triangulation edge
	// Here surEdges always have the length 2
	while(true){
		iType0 = checkIntersection(newE, surEdges[0], false);
		iType1 = checkIntersection(newE, surEdges[1], false);

		// The new edge does not intersect any further edges
		if(iType0 == IntersectionType::NONE && iType1 == IntersectionType::NONE)
			return true;

		// New edge hits a vertex
		if(iType0 == IntersectionType::VERTEX) 
			return false;
		if(iType1 == IntersectionType::VERTEX) 
			return false;

		// Check for numerical problems
		if(iType0 != IntersectionType::NONE && iType1 != IntersectionType::NONE){
			if(Settings::feedback == FeedbackMode::VERBOSE)
				fprintf(stderr, "CheckEdge: new edge intersects multiple edges of the actual triangle -> translation rejected due to numerical problem\n");
			
			return false;
		}

		// Choose the intersected edge
		if(iType0 != IntersectionType::NONE){
			iType = iType0;
			intersectedE = surEdges[0];
		}else{
			iType = iType1;
			intersectedE = surEdges[1];
		}

		eType = (*intersectedE).getEdgeType();

		// intersected edge is a polygon or frame edge
		if(eType != EdgeType::TRIANGULATION){
			return false;
		// Intersected edge is just a triangulation edge
		}else{
			nextT = (*intersectedE).getOtherTriangle(nextT);
			surEdges = (*nextT).getOtherEdges(intersectedE);
		}

		passedTriangles++;
		Statistics::nrTriangles++;

		if(passedTriangles > Statistics::maxTriangles)
			Statistics::maxTriangles = passedTriangles;
	}

	return true;
}


/*
	C ~ O ~ N ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R ~ S
*/

/*
	Constructor:
	Generates a new Translation. Picks the moving vertex and its neighbors from the
	triangulations vertices list and initialize the translation path.

	@param 	Tr 		The triangulation the moving vertex lives in
	@param 	i 		The index of the moving vertex in the triangulation
	@param 	dX 		The x-component of the translation vector
	@param 	dY 		The y-component of the translation vector
*/
Translation::Translation(Triangulation *Tr, int i, double dX, double dY) :
	T(Tr), index(i), dx(dX), dy(dY), id(n){

	original = (*T).getVertex(index);

	prevV = (*original).getPrev();
	nextV = (*original).getNext();

	oldV = (*original).getTranslated(0, 0);
	newV = (*original).getTranslated(dx, dy);

	prevOldE = (*original).getToPrev();
	nextOldE = (*original).getToNext();

	prevNewE = new TEdge(prevV, newV);
	nextNewE = new TEdge(newV, nextV);

	n++;
	Statistics::translationTries++;
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
bool Translation::checkOrientation(){
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
	for(i = 1; i <= Settings::nrInnerPolygons; i++){

		// Skip the changing polygon itself
		if((*original).getPID() == i)
			continue;

		// Get a random vertex of the polygon
		randomV = (*T).getVertex(0, i);

		inside0 = Triangle::insideTriangle(oldV, newV, prevV, randomV);
		inside1 = Triangle::insideTriangle(oldV, newV, nextV, randomV);

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

	for(i = 1; i <= Settings::nrInnerPolygons; i++){

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
	The function checkSimplicityOfTranslation() determines whether the result of a translation is
	a simple polygon. It does so by checking whether one of the edges from the new vertex position
	to one of its neighbors intersects a polygon edge.

	@return 	True if the resulting polygon is still simple, otherwise false
*/
bool Translation::checkSimplicityOfTranslation() const{
	bool simple;

	(*prevOldE).setEdgeType(EdgeType::TRIANGULATION, true);
	(*nextOldE).setEdgeType(EdgeType::TRIANGULATION, true);

	simple = checkEdge(prevV, prevNewE);
	simple = simple && checkEdge(nextV, nextNewE);

	(*prevOldE).setEdgeType(EdgeType::POLYGON, true);
	(*nextOldE).setEdgeType(EdgeType::POLYGON, true);

	return simple;
}
