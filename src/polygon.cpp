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

#include "polygon.h"

void Polygon::triangulateStar(){
	PolygonVertex *v0, *v1, *v2;
	PolygonEdge *e0, *e1, *e2;
	TEdge *newEdge;
	bool inside = false;

	// Get the first three vertices and their edges
	v0 = startVertex;
	e0 = v0 -> nextE;
	v1 = e0 -> nextV;
	e1 = v1 -> nextE;
	v2 = e1 -> nextV;

	while(n > 3){
		
		inside = insideTriangle(v0 -> v, v1 -> v, v2 -> v, kernel);

		// In case the kernel point is not in the interior of the given ear,
		// we can clip it
		if(!inside){
			newEdge = new TEdge(v0 -> v, v2 -> v);
			(*T).addEdge(newEdge, 0);

			new Triangle(e0 -> e, e1 -> e, newEdge, v0 -> v, v1 -> v, v2 -> v);

			// Remove the entries of the cut off entities
			free(e0);
			free(e1);
			free(v1);

			// Create an entry for the new edge
			e0 = (PolygonEdge*)malloc(sizeof(PolygonEdge));
			e0 -> e = newEdge;
			e0 -> prevV = v0;
			e0 -> nextV = v2;

			v1 = v2;
			e1 = v1 -> nextE;
			v2 = e1 -> nextV;

			n--;
		}else{
			v0 = v1;
			e0 = e1;
			v1 = v2;
			e1 = v1 -> nextE;
			v2 = e1 -> nextV;
		}
	}

	e2 = v2 -> nextE;

	new Triangle(e0 -> e, e1 -> e, e2 -> e, v0 -> v, v1 -> v, v2 -> v);

	free(e0);
	free(e1);
	free(e2);
	free(v0);
	free(v1);
	free(v2);

	n = 0;
}

void Polygon::triangulateVisible(){
	double referenceDet, area;
	PolygonVertex *v0, *v1, *v2, *additionalV;
	PolygonEdge *e0, *e1, *e2;
	Triangle *t;
	TEdge * newEdge;

	// Get the first three vertices and their edges
	v0 = startVertex;
	e0 = v0 -> nextE;
	v1 = e0 -> nextV;
	e1 = v1 -> nextE;
	v2 = e1 -> nextV;

	// At first we have to find out whether positive or negative determinant values
	// mark convex vertices
	// For that we can exploit the fact that the vertices incident to the base edge
	// must always be convex
	additionalV = startVertex -> prevE-> prevV;
	t = new Triangle(additionalV -> v, v0 -> v, v1 -> v);
	referenceDet = (*t).signedArea();
	delete t;

	while(n > 3){

		// Check whether the recent three vertices are in convex position
		t = new Triangle(v0 -> v, v1 -> v, v2 -> v);
		area = (*t).signedArea();
		delete t;

		if(v1 != startVertex && signbit(area) == signbit(referenceDet)){
			newEdge = new TEdge(v0 -> v, v2 -> v);
			(*T).addEdge(newEdge, 0);

			new Triangle(e0 -> e, e1 -> e, newEdge, v0 -> v, v1 -> v, v2 -> v);

			// Remove the entries of the cut off entities
			free(e0);
			free(e1);
			free(v1);

			v1 = v0;
			e0 = v1 -> prevE;
			v0 = e0 -> prevV;

			// Create an entry for the new edge
			e1 = (PolygonEdge*)malloc(sizeof(PolygonEdge));
			e1 -> e = newEdge;
			e1 -> prevV = v1;
			e1 -> nextV = v2;

			n--;
		}else{
			v0 = v1;
			e0 = e1;
			v1 = v2;
			e1 = v1 -> nextE;
			v2 = e1 -> nextV;
		}
	}
	
	e2 = v2 -> nextE;

	new Triangle(e0 -> e, e1 -> e, e2 -> e, v0 -> v, v1 -> v, v2 -> v);

	free(e0);
	free(e1);
	free(e2);
	free(v0);
	free(v1);
	free(v2);

	n = 0;
}

/*
	The function insideTriangle() checks whether the vertex toCheck is inside the triangle
	formed by the vertices v0, v1 and v2.

	@param	v0 			First vertex of the triangle
	@param 	v1 			Second vertex of the triangle
	@param 	v2 			Third vertex of the triangle
	@param 	toCheck 	The vertex for which should be checked whether it lays inside the
						triangle or not
	@return 			True if toCheck lays inside the triangle, otherwise false
*/
bool Polygon::insideTriangle(Vertex * const v0, Vertex * const v1, Vertex * const v2,
	Vertex * const toCheck){
	double area0, area1;
	Triangle *t;

	t = new Triangle(v0, v1, toCheck);
	area0 = (*t).signedArea();
	delete t;

	t = new Triangle(v1, v2, toCheck);
	area1 = (*t).signedArea();
	delete t;

	if(signbit(area0) != signbit(area1))
		return false;

	t = new Triangle(v2, v0, toCheck);
	area1 = (*t).signedArea();
	delete t;

	if(signbit(area0) != signbit(area1))
		return false;

	return true;
}



Polygon::Polygon(Triangulation *triang, PolygonType tp) :
	type(tp), T(triang), n(0), startVertex(NULL), closed(false), lastVUsed(NULL), lastEUsed(NULL) {

}

void Polygon::addVertex(Vertex *v){
	PolygonVertex *entry;

	if(closed){
		fprintf(stderr, "Polygon build error: polygon has already been closed!\n");
		exit(15);
	}

	if(n != 0 && lastVUsed != NULL){
		fprintf(stderr, "Polygon build error: something went wrong while building a polygon to retriangulate....two vertices next to each other!\n");
		exit(15);
	}

	entry = (PolygonVertex*)malloc(sizeof(PolygonVertex));

	entry -> v = v;
	entry -> prevE = lastEUsed;
	entry -> nextE = NULL;

	if(n == 0)
		startVertex = entry;
	else
		lastEUsed -> nextV = entry;

	lastVUsed = entry;
	lastEUsed = NULL;

	n++;
}

void Polygon::addEdge(TEdge *e){
	PolygonEdge *entry;

	if(closed){
		fprintf(stderr, "Polygon build error: polygon has already been closed!\n");
		exit(15);
	}

	if(n == 0){
		fprintf(stderr, "Polygon build error: something went wrong while building a polygon to retriangulate....the polygon is not allowed to start with an edge!\n");
		exit(15);
	}

	if(lastEUsed != NULL){
		fprintf(stderr, "Polygon build error: something went wrong while building a polygon to retriangulate....two edges next to each other!\n");
		exit(15);
	}

	entry = (PolygonEdge*)malloc(sizeof(PolygonEdge));

	entry -> e = e;
	entry -> prevV = lastVUsed;
	entry -> nextV = NULL;

	lastVUsed -> nextE = entry;

	lastVUsed = NULL;
	lastEUsed = entry;
}

void Polygon::setKernel(Vertex *k){
	kernel = k;
}

void Polygon::close(TEdge *e){
	PolygonEdge *entry;

	if(n < 3){
		fprintf(stderr, "Polygon build error: something went wrong while building a polygon to retriangulate....the polygon is not allowed to have less then three vertices!\n");
		exit(15);
	}

	if(lastEUsed != NULL){
		fprintf(stderr, "Polygon build error: something went wrong while building a polygon to retriangulate....two edges next to each other!\n");
		exit(15);
	}

	entry = (PolygonEdge*)malloc(sizeof(PolygonEdge));

	entry -> e = e;
	entry -> prevV = lastVUsed;
	entry -> nextV = startVertex;
	lastVUsed -> nextE = entry;
	startVertex -> prevE = entry;

	lastVUsed = NULL;

	closed = true;
}

void Polygon::print(){
	PolygonVertex *vertex;
	PolygonEdge *edge;

	fprintf(stderr, "Polygon:\n");

	vertex = startVertex;
	edge = vertex -> nextE;
	do{
		
		fprintf(stderr, "Vertex %llu\n", (*vertex -> v).getID());
		fprintf(stderr, "Edge %llu from Vertex %llu to Vertex %llu\n", (*edge -> e).getID(),
			(*(*edge -> e).getV0()).getID(), (*(*edge -> e).getV1()).getID());

		vertex = edge -> nextV;
		edge = vertex -> nextE;
		
	}while(vertex != startVertex);

	fprintf(stderr, "End of polygon\n\n");
}

void Polygon::triangulate(){
	if(type == PolygonType::STARSHAPED){
		
		if(kernel == NULL){
			fprintf(stderr, "Triangulation error: Star-shaped polygon can not be triangulated, if no kernel point is given!");
			exit(16);
		}

		triangulateStar();
	}
	else if(type == PolygonType::EDGEVISIBLE)
		triangulateVisible();
}
