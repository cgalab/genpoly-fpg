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
#pragma once

/*
	Include standard libraries
*/


/*
	Include my headers	
*/
#include "tedge.h"
#include "vertex.h"
#include "triangle.h"

#ifndef __POLYGON_H_
#define __POLYGON_H_

/*
	This class is used to represent polygonal holes in the triangulation which
	have to be retriangulated. It is capable of retriangulating star-shaped and
	edge-visible polygons. For edge-visible polygons by convention the base edge
	has to close the chain and therefore is the edge before the startVertex.

	Definition: Edge-visible Polygon
	A polygon is edge-visible if there exists one edge e which can be seen internally
	from each vertex not adjacent to e. The edge e is called the base edge.
*/


/*
	Different types of polygons
*/
enum class PolygonType {STARSHAPED, EDGEVISIBLE};


/*
	Element types
*/

struct PolygonVertex;
struct PolygonEdge;

// Vertex
struct PolygonVertex{
	Vertex *v;
	PolygonEdge *prevE;
	PolygonEdge *nextE;
};

//Edge
struct PolygonEdge{
	TEdge *e;
	PolygonVertex *prevV;
	PolygonVertex *nextV;
};


class Polygon{

private:

	/*
		The type of the polygon represented
	*/
	PolygonType type;

	/*
		The triangulation the polygon lives in
	*/
	Triangulation * const T;

	/*
		Recent number of vertices/edges of the polygon
	*/
	int n;

	/*
		The first vertex of the polygon.
		In case of an edge-visible polygon, the predecessor entity of the start
		vertex is the base edge.
	*/
	PolygonVertex *startVertex;

	/*
		A flag whether the polygonal chain is already closed
	*/
	bool closed;

	/*
		The vertex inserted last, in case a vertex has been inserted last.
		In case an edge has been inserted last, then NULL.
	*/
	PolygonVertex *lastVUsed;

	/*
		The edge inserted last, in case an edge has been inserted last.
		In case a vertex has been inserted last, then NULL.
	*/
	PolygonEdge *lastEUsed;

	/*
		One point of the kernel in case of a star-shaped polygon
	*/
	Vertex *kernel;


	void triangulateStar();
	void triangulateVisible();

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
	bool insideTriangle(Vertex * const v0, Vertex * const v1, Vertex * const v2,
		Vertex * const toCheck);

public:

	Polygon(Triangulation *triang, PolygonType tp);

	void addVertex(Vertex *v);

	void addEdge(TEdge *e);

	void setKernel(Vertex *k);

	void changeType(PolygonType tp);

	void close(TEdge *e);

	void print();

	void triangulate();
};

#endif
