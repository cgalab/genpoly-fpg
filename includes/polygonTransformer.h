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
#include <stdio.h>
#include <list>
#include <vector>
#include <map>
#include <math.h>

/*
	Include my headers	
*/
#include "triangulation.h"
#include "tedge.h"
#include "vertex.h"
#include "triangle.h"
#include "translationKinetic.h"
#include "translationRetriangulation.h"
#include "translation.h"
#include "insertion.h"
#include "settings.h"
#include "holeInsertion.h"


/*
	The function transformPolygonByMoves() transforms a polygon by randomly selecting a
	vertex and a direction to move and then randomly computing a distance to shift in
	the chosen direction which with the constraint that the movement should be
	applicable with reasonable effort. This is done iterations times.

	@param 	T 			The triangulation the polygon lives in
	@param 	iterations	The number of moves which should be tried
	@return 			The number of moves which were executed at least partially

	Note:
		- The number of really performed moves is most likely less then the number of moves
			which should be tried
		- For polygons with holes the vertex also is randomly selected, i.e. it can be from
			the outer polygon as well as of one of the inner polygons
*/
int transformPolygonByMoves(Triangulation * const T, const int iterations);


/*
	The function growPolygonBy() grows a polygon by n insertions.

	@param 	T 		The triangulation the polygon lives in
	@param	pID 	The ID of the polygon
	@param 	n 		The number of vertices to insert

	Note:
		This function works just for polygons without holes!
*/
void growPolygonBy(Triangulation * const T, const unsigned int pID,  const int n);


/*
	This strategy starts with a small regular polygon, applies a bunch of translations
	to it, then it grows the polygon directly to the desired number of vertices
	and afterwards applies another bunch of translations.

	@param 	T 	The triangulation the polygon lives in
*/
void strategyNoHoles0(Triangulation * const T);


/*
	This strategy starts with a small regular polygon and then does the following until
	the desired number of vertices is reached:
		- Apply a bunch of translations
		- Double the number of vertices

	@param 	T 	The triangulation the polygon lives in

	Note:
		- Unused at the moment
*/
void strategyNoHoles1(Triangulation * const T);


/*
	Strategy for the generation of polygons with holes:
		- Assumption: Holes are already inserted into the start polygon
		- Apply a bunch of translations to the initial polygon
		- Until all polygons have reached the desired number of vertices:
			- Double up the size of the inner polygons
			- Double up the size of the outer polygon

	@param 	T 	The triangulation the polygon lives in
*/
void strategyWithHoles0(Triangulation * const T);


/*
	Strategy for the generation of polygons with holes:
		- Assumption: Holes have not been inserted into the start polygon
		- Apply a bunch of translations to the initial polygon
		- Grow the outer polygon to the size 10 * the number of holes 
			(if possible)
		- Insert the holes
		- Grow all holes to size 20
		- Apply a bunch of translations
		- Apply 10 inflate stages
		- Apply 10 shrinking stages
		- Apply a bunch of translations
		- Until all polygons have reached the desired number of vertices:
			- Double up the size of the inner polygons
			- Double up the size of the outer polygon

	@param 	T 	The triangulation the polygon lives in
*/
void strategyWithHoles1(Triangulation * const T);


/*
	The function shrinkAroundHole() tries to shrink the polygon around the hole with
	id holeIndex. For that it repeatedly selects a random vertex of the hole and searches
	for an adjacent vertex of the polygon or another hole. Then it tries a translation
	of the adjacent vertex in direction to the vertex of the hole. The number of times
	this is done equals the number of vertices of the selected hole.

	@param	T 			The triangulation the polygon lives in
	@param 	holeIndex 	The id of the selected hole
	@param 	offset 		A parameter for selecting an adjacent vertex
*/
void shrinkAroundHole(Triangulation * const T, int holeIndex, int index);


/*
	The function inflateHole() tries to inflate the hole with id holeIndex. For that
	it repeatedly selects a random vertex of the hole and tries to execute a 
	translation of it in direction to the its exterior. The number of times
	this is done equals the number of vertices of the selected hole.

	@param 	T 			The triangulation the polygon lives in
	@param 	holeIndex 	The id of the selected hole
*/
void inflateHole(Triangulation * const T, int holeIndex);


/*
	The function getAdjacentPolygonVertex() returns any Vertex which is adjacent to v in the
	triangulation and belongs to the polygon or another hole. If there is no such
	vertex, it returns NULL.

	@param 	v 	A vertex of a hole
	@return 	Any adjacent vertex to v of the polygon (with polygon id equals 0)
*/
Vertex *getAdjacentPolygonVertex(Vertex const * const v, int index);
