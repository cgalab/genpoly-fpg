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
#pragma once

/*
	Include standard libraries
*/
#include <vector>
#include <stdio.h>
#include <map>
#include <fstream>

/*
	Include my headers	
*/
#include "settings.h"

/*
	Define the class Triangulation
*/
#ifndef __TRIANGULATION_H_
#define __TRIANGULATION_H_

/*
	Define and include headers to the entities contained by triangulations
*/
class Vertex;
class TEdge;
class Triangle;
class TPolygon;

#include "vertex.h"
#include "tedge.h"
#include "triangle.h"
#include "tpolygon.h"
#include "selectionTree.h"

class Triangulation{

private:

	/*
		The polygons living in the triangulation
	*/
	TPolygon *outerPolygon;
	std::vector<TPolygon*> innerPolygons;
	
	/*
		A vector of all polygon vertices contained by the triangulation
	*/
	std::vector<Vertex*> vertices;

	/*
		A map of all edges contained by the triangulation
		Note:
			- Keeping this is not necessary for running the polygon generator, but it's pretty 
				useful for debugging, because otherwise it's hard to print the whole triangulation
			- An unordered map with a hash table could be faster, but I do not know how efficient 
				the deletion of elements is implemented their
	*/
	std::map<int, TEdge*> edges;

	/*
		A selection tree for selecting a triangle interior to the polygon		
	*/
	SelectionTree<Triangle*> *internalTriangles;

	/*
		Vertices of the bounding box
	*/
	Vertex *Rectangle0;
	Vertex *Rectangle1;
	Vertex *Rectangle2;
	Vertex *Rectangle3;

	/*
		The target number of polygon vertices (including the vertices of inner polygons)
	*/
	int N;


public:
	
	/*
		LIST OF PUBLIC MEMBER FUNCTIONS

		CONSTRUCTORS:

						Triangulation()

		SETTER:
		
		void 			addInnerPolygon(TPolygon * const p)
		void 			addVertex(Vertex * const v, const unsigned int pID)
		void 			changeVertex(const int i, const unsigned int fromP,
						const unsigned int toP)
		void 			addEdge(TEdge * const e , const unsigned int pID)
		void 			addInternalTriangle(Triangle *t)
		void 			setRectangle(Vertex * const v0, Vertex * const v1, Vertex * const v2,
						Vertex * const v3)

		GETTER:
		
		unsigned int 	getActualNrInnerPolygons() const
		int 			getTargetNumberOfVertices() const
		int 			getActualNumberOfVertices() const
		int 			getActualNumberOfVertices(const unsigned int pID) const
		Vertex*			getVertex(const int i, const unsigned int pID) const
		Vertex* 		getVertex(const int i) const
		TEdge*			getRandomEdgeWeighted(const unsigned int pID) const;
		Triangle* 		getRandomInternalTriangleWeighted() const;

		REMOVER:

		void 			removeVertex(const int index)
		void 			removeEdge(TEdge * const e)

		PRINTER:
		
		void 			writeTriangulation(const char *filename) const
		void 			writePolygon(const char *filename) const
		void 			writePolygonToDat(const char *filename) const
		void 			writePolygonToLine(const char *filename) const

		OTHERS:

		bool 			check() const
		void 			checkST() const
		void 			stretch(const double factor)
		void			checkSimplicity() const
	*/


	/*
		C ~ O ~ N ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R ~ S
	*/

	/*
		Constructor:
		Already allocates memory for the vector of vertices and generates the TPolygon
		instances for the outer polygon.
	*/
	Triangulation();


	/*
		S ~ E ~ T ~ T ~ E ~ R ~ S
	*/

	/*
		@param 	p 	The new inner polygon
	*/
	void addInnerPolygon(TPolygon * const p);

	/*
		The function addVertex() inserts a vertex into the vertices list of the triangulation
		and also calls the addVertex() function of the polygon the vertex belongs to. It errors
		with exit code 12 if pID is greater then the number of inner polygons.

		@param	v 		Vertex to be added to the vertices vector
		@param 	pID 	The polygon the vertex belongs to (pID = 0 outer polygon, else inner
						polygon)
	*/
	void addVertex(Vertex * const v, const unsigned int pID);

	/*
		The function changeVertex() removes the vertex at index i from the polygon with ID
		fromP and adds it to the polygon with ID toP.

		@param	i 		The index of the vertex to be moved in the polygon with ID fromP
		@param 	fromP 	The ID of the polygon the vertex lives in originally
		@param 	toP 	The ID of the polygon the vertex should be moved to
	*/
	void changeVertex(const int i, const unsigned int fromP, const unsigned int toP);

	/*
		Adds a new edge to the edge map of the triangulation if printing the whole
		triangulation is required. Polygon edges get add to the SelectionTree of its 
		polygon if, their STEntry is not already set. If the edge is already in the
		edge map, the edge map won't get changed.

		@param	e 		Edge to be added to the edge map
		@param 	pID 	For polygon edges the ID of the polygon, for other edges no
						meaning
	*/
	void addEdge(TEdge * const e , const unsigned int pID);

	/*
		Adds an internal triangle to the selection tree of internal triangles.

		@param 	t 	The triangle
	*/
	void addInternalTriangle(Triangle *t);

	/*
		The function setRectangle() sets the vertices of the Rectangle0, ..., Rectangle3 of
		bounding box for the polygons

		@param	v0, v1, v2, v3 	Vertices building the bounding box for the polygon (ordering
								doesn't matter as the vertices are connected by their edges)
	*/
	void setRectangle(Vertex * const v0, Vertex * const v1, Vertex * const v2, Vertex * const v3);


	/*
		G ~ E ~ T ~ T ~ E ~ R ~ S
	*/

	/*
		@return 	The actual number of inner polygons
	*/
	unsigned int getActualNrInnerPolygons() const;

	/*
		@return		Final number of vertices the polygon will contain (including the vertices
					of inner polygons)
	*/
	int getTargetNumberOfVertices() const;

	/*
		@return		The number of vertices the polygon does contain now (including the vertices
					of inner polygons)
	*/
	int getActualNumberOfVertices() const;

	/*
		@param 	pID The polygon of interest
		@return		The number of vertices the polygon with pID does contain now
	*/
	int getActualNumberOfVertices(const unsigned int pID) const;

	/*
		@param	i 	Index of the vertex in the vertices vector of the polygon with pID
		@param 	pID	The ID of the polygon of interest
		@return 	The vertex at index i in the vertices vector, NULL if no polygon with pID
					exists

		Note:
			- Be n the actual number of vertices in the vertex vector, then i < 0 
				returns the vertex with index n + i and i >= n returns the vertex at index
				i - n. This is helpful to get the previous and next vertex while generating
				the initial polygon.
			- This will not work after inserting additional vertices, as the vertices won't be 
				in the same order in the vertices vector as they are in the polygon
	*/
	Vertex *getVertex(const int i, const unsigned int pID) const;

	/*
		@param 	i 	The index of the vertex in the vertices vector
		@return 	The vertex at index i in the vertices vector
	*/
	Vertex *getVertex(const int i) const;

	/*
		@param 	pID 	The ID of the polygon of interest
		@return 		Any edge of the polygon selected uniformly at random by its length
	*/
	TEdge *getRandomEdgeWeighted(const unsigned int pID) const;

	/*
		@return 	A randomly selected (based on their weights) triangle in the interior
					of the polygon
	*/
	Triangle *getRandomInternalTriangleWeighted() const;


	/*
		R ~ E ~ M ~ O ~ V ~ E ~ R
	*/

	/*
		The function removeVertex() removes one vertex from the vector of vertices and sets the
		entry NULL

		@param	i 	Index of the vertex to be removed

		Note:
			- This function is just for debugging purposes and should normally not be used
				anywhere in the code
	*/
	void removeVertex(const int i);

	/*
		The function removeEdge() searches one edge by its ID in the edges map and removes it

		@param	e 	The edge to be removed
	*/
	void removeEdge(TEdge * const e);

	
	/*
		P ~ R ~ I ~ N ~ T ~ E ~ R
	*/

	/*
		The function writeTriangulation() writes the whole triangulation in .graphml style into a file

		@param	filename	The name of the .graphml file

		Note:
			- This function just works, if the triangulation stores all edges
			- Works properly with Gephi (scaling factor is required for that)
	*/
	void writeTriangulation(const char *filename) const;

	/*
		The function writePolygon() writes just the polygon in .graphml style into a file

		@param	filename	The name of the .graphml file

		Note:
			- Graphml: https://de.wikipedia.org/wiki/GraphML
			- Works properly with Gephi (scaling factor is required for that)
	*/
	void writePolygon(const char *filename) const;

	/*
		The function writePolygonToDat() writes all polygons to a .dat file which can be
		interpreted by gnuplot.

		@param 	filename 	The name of the .dat file
	*/
	void writePolygonToDat(const char *filename) const;

	/*
		The function writePolygonToLine() writes all polygons to a .line file

		@param 	filename 	The name of the .line file
	*/
	void writePolygonToLine(const char *filename) const;


	/*
		O ~ T ~ H ~ E ~ R ~ S
	*/

	/*
		The function check() checks for errors in the triangulation. It checks the following
		criteria:
			- Has each edge of the bounding box exactly one triangle assigned
			- Has each other edge exactly two triangles assigned
			- Has each edge to different vertices assigned
			- Has each vertex a previous and a next vertex connected by a polygon edge
			- Stays each vertex inside of its surrounding polygon

		@return 	true if everything is alright, otherwise false
	*/
	bool check() const;

	/*
		Checks the correctness of the SelectionTrees
	*/
	void checkST() const;

	/*
		The function stretch() stretches the whole polygon by a constant factor, i.e. the
		x- and y-coordinates of all vertices get multiplied by the factor.

		@param	factor	The factor by which all vertex coordinates get multiplied

		Note:
			- It is not checked, whether this operations is numerically stable!
			- It is not used anywhere at the moment
	*/
	void stretch(const double factor);

	/*
	The function checkSimplicity() checks whether a polygon is simple by checking for self-
	intersections. So it basically compares each edge with each other edge. To do so it uses
	two nested for-loops: The outer one loops over all edges, whereas the inner one just loops
	over the edges which have not been compared yet with the edge of the outer one. If it finds
	any intersection it errors with exit code 11.

	Note:
		- This function does not consider edges of other polygons!
		- Should not be called for larger polygons, because it is highly inefficient.
*/
	void checkSimplicity() const;
};

#endif
