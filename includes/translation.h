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
#include <cmath>
#include <limits>
#include <string>

/*
	Include my headers	
*/
#include "vertex.h"
#include "triangle.h"
#include "tedge.h"
#include "triangulation.h"
#include "settings.h"
#include "statistics.h"

#ifndef __TRANSLATION_H_
#define __TRANSLATION_H_

/*
	Define the different types a translation can end with:
		FULL: 		The translation was fully executed
		REJECTED: 	The translation has even not been started because the result is
					not simple or the event queue is unstable (kinetic)
		PARTIAL: 	The translation has been started, but was aborted due to instability
					of the event queue (kinetic)
		UNDONE: 	The translation has been started, but was aborted due to instability
					if the event queue and afterwards it was undone (kinetic)
*/
enum class Executed {FULL, REJECTED, PARTIAL, UNDONE};

class Translation{

protected:

	/*
		The triangulation the vertex to be translated lives in
	*/
	Triangulation * const T;

	/*
		The index of the vertex in the triangulation's vertex vector
	*/
	const int index;

	/*
		The vertex to be translated, a copy at the start position and a copy at the
		target position
	*/
	Vertex *original;
	Vertex *oldV;
	Vertex *newV;

	/*
		The neighboring vertices in the polygon
	*/
	Vertex *prevV;
	Vertex *nextV;

	/*
		The path the vertex should move along
	*/
	// TODO: maybe this is just used by the kinetic approach
	TEdge *transPath;

	/*
		The actual edges between the vertex and its neighbors
	*/
	TEdge *prevOldE;
	TEdge *nextOldE;

	/*
		The new polygon edges the translation will produce
	*/
	TEdge *prevNewE;
	TEdge *nextNewE;

	/*
		The components of the translation vector
	*/
	const double dx, dy;

	/*
		The ID of the translation
	*/
	unsigned long long id;

	/*
		The number of already performed translations
	*/
	static unsigned long long n;


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
	bool insideQuadrilateral(Vertex * const v) const;

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

	/*
		The function checkEdge() checks whether the edge newE starting at vertex fromV
		intersects any polygon edge. Therefore it first checks whether newE intersects any
		edge of the surrounding polygon of fromV. If it does not, then it can not intersect
		any polygon edge, otherwise we continue the check in the other triangle assigned to
		the intersected edge until newE either intersects a polygon edge or ends in any
		triangle.

		@param 	fromV 	The vertex where the new edge starts
		@param 	newE 	The new edge
		@return 		True if the new edge does not intersect any polygon edge,
						otherwise false

		Note:
			- If the edge goes right through another vertex we also count it as intersecting
				a polygon edge even if the edge we checked was no polygon edge, because a
				new polygon edge obviously is not allowed to go through a vertex.
			- As we had the case that a new edge numerically intersected all three edges of the same
				triangle or multiple edges of the surrounding polygon we do not stop at the first 
				intersection we find, but we count all intersections. If we find more then one, the
				function returns false. But this should now really happen nowhere as the epsilon
				in checkIntersection() should lead to getting all these cases as intersections of
				type VERTEX.
			- Edges of the bounding box are treated as polygon edges
	*/
	bool checkEdge(Vertex * const fromV, TEdge * const newE) const;


public:

	/*
		LIST OF PUBLIC MEMBER FUNCTIONS

		CONSTRUCTORS:
		
						Translation(Triangulation *Tr, int i, double dX, double dY)

		OTHERS:

		bool 			checkOrientation()
		enum Executed 	execute()
		bool 			checkSimplicityOfTranslation()
	*/
	

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
	Translation(Triangulation *Tr, int i, double dX, double dY);


	/*
		O ~ T ~ H ~ E ~ R ~ S
	*/

	/*
		The function checkOrienation() checks whether the polygon would change its orientation by
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

		@return 	True if the polygon would change its orientation or rolls over another inner 
					polygon, otherwise false
	*/
	virtual bool checkOrientation();

	/*
		The function checkSimplicityOfTranslation() determines whether the result of a
		translation is a simple polygon. It does so by checking whether one of the edges
		from the new vertex position to one of its neighbors intersects a polygon edge.

		@return 	True if the resulting polygon is still simple, otherwise false
	*/
	bool checkSimplicityOfTranslation() const;

	/*
		The function execute() processes a translation.

		@return 	Indicates whether the execution was rejected, aborted or fully processed
	*/
	virtual enum Executed execute() = 0;


	/*
		D ~ E ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R
	*/

	/*
		Destructor:
		Checks the surrounding polygon of the moved vertex and deletes all the remaining construction
		vertices and edges. It errors with exit code 6 if the surrounding polygon check fails.
	*/
	virtual ~Translation() {};
};

#endif
