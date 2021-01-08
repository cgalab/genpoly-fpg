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
	has to close the chain and therefore is the edge connecting startVertex with its
	previous vertex.

	Definition: Edge-visible Polygon
	A polygon is edge-visible if there exists one edge e which can (partially) 
	be seen internally from each vertex not adjacent to e. The edge e is called
	base edge.
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

	*/
	bool internal;

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


	/*
		P ~ R ~ I ~ V ~ A ~ T ~ E 	M ~ E ~ M ~ B ~ E ~ R 	F ~ U ~ N ~ C ~ T ~ I ~ O ~ N ~ S
	*/

	/*
		The function triangulateStar() triangulates a star-shaped polygon using an ear clipping
		approach. For that it successively cuts of ears which do not contain the given kernel
		point. This ensures that the polygon is still star-shaped after a cut. To ensure a linear
		time complexity the function uses a backtracking approach after each cut. Thus, the
		triangulation is done after one walk around the polygon.
	*/
	void triangulateStar();

	/*
		The function triangulateVisible() triangulates an edge-visible polygon using an ear
		clipping approach. For that it successively cuts of ears going along the polygonal chain.
		For edge visible polygons hold that each convex vertex not incident to the base edge
		defines an ear and cutting of one of these ears lets the polygon remain edge-visible to
		the given base edge. To ensure a linear time complexity the function uses a backtracking
		approach after each cut. Thus, the triangulation is done after one walk around the polygon.
	*/
	void triangulateVisible();


public:

	/*
		LIST OF PUBLIC MEMBER FUNCTIONS

		CONSTRUCTORS:
		
			Polygon(Triangulation *triang, PolygonType tp)

		SETTERS:

		void 	addVertex(Vertex *v)
		void 	addEdge(TEdge *e)
		void 	setKernel(Vertex *v)
		void 	changeType(PolygonType tp)
		void 	close(TEdge *e)

		OTHERS:

		void 	print()
		void 	triangulate()
	*/


	/*
		C ~ O ~ N ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R ~ S
	*/

	/*
		Constructor:
		Generates a new Polygon of type tp in the triangulation triang.

		@param 	triang 	The triangulation the polygon lives in
		@param 	tp 		The PolygonType of the polygon
		@param 	intern 	Indicates whether the polygon is internal or
						external to the polygon. The interior of a hole
						is counted as exterior.
	*/
	Polygon(Triangulation *triang, PolygonType tp, bool intern);


	/*
		S ~ E ~ T ~ T ~ E ~ R ~ S
	*/

	/*
		Adds a new vertex to the polygon which becomes lastVUsed. In case
		the last added entity was also a vertex or the polygon is already
		closed, this function throws a polygon build error.

		@param 	v 	The new vertex
	*/
	void addVertex(Vertex *v);

	/*
		Adds a new edge to the polygon which becomes lastEUsed. In case
		the last added entity was also an edge or the polygon is already
		closed, this function throws a polygon build error.

		@param 	e 	The new edge
	*/
	void addEdge(TEdge *e);

	/*
		This function closes the polygon with an edge. Afterwards no other
		entity can be added to the polygonal chain. In case the last inserted
		entity was an edge, this function throws a polygon build error.

		@param 	e 	The edge to close the polygon with
	*/
	void close(TEdge *e);

	/*
		Sets a kernel point for a star-shaped polygon. In case the polygon is
		not star-shaped, the function throws a polygon build error.

		@param 	k 	The kernel point
	*/
	void setKernel(Vertex *k);

	/*
		Changes the type of a polygon to tp. In case the polygon has a kernel set
		and is changed to another type than star-shaped, this function throws a
		polygon build error.

		@param 	tp 	The new PolygonType of the polygon
	*/
	void changeType(PolygonType tp);


	/*
		P ~ R ~ I ~ N ~ T ~ E ~ R
	*/

	/*
		Prints the polygon to stderr.
	*/
	void print();


	/*
		O ~ T ~ H ~ E ~ R ~ S
	*/

	/*
		Triangulates the polygon with the suitable triangulation function. If
		the polygon is star-shaped, but no kernel point is set, the function
		throws a triangulation error.
	*/
	void triangulate();


	/*
		D ~ E ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R
	*/

	/*
		Destructor:
		Cleans up the polygonal chain if necessary.
	*/
	~Polygon();
};

#endif
