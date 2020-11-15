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
	Include my headers	
*/

#include "vertex.h"
#include "triangulation.h"
#include "tedge.h"
#include "triangle.h"
#include "translation.h"
#include "settings.h"


/*
	Define the class Insertion:
	The class insertion holds all utilities to insert a vertex into an edge, repair the
	surrounding triangulation and move the new vertex off the edge.
*/
#ifndef __INSERTION_H_
#define __INSERTION_H_

class Insertion{

private:
	
	/*
		The triangulation the polygon lives in
	*/
	Triangulation * const T;

	/*

	*/
	unsigned int pID;

	/*
		The edge to insert in
	*/
	TEdge *e;

	/*
		The two vertices forming the edge
	*/
	Vertex *v0, *v1;

	/*
		The inserted vertex
	*/
	Vertex *newV;
	

public:

	/*
		LIST OF PUBLIC MEMBER FUNCTIONS

		CONSTRUCTORS:
		
				Insertion(Triangulation * const t, const unsigned int pid, const int index)
				Insertion(Triangulation * const t, const unsigned int pid, TEdge *edge)

		OTHERS:

		bool 	checkStability() const
		void 	execute()
		void 	translate() const
	*/


	/*
		C ~ O ~ N ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R ~ S
	*/

	/*
		Constructor:
		The constructor is given an vertex index. The insertion is done into the
		edge between the vertex and its successor in the polygon

		@param 	T 		The triangulation the polygon lives in
		@param 	pid 	The id of the polygon to insert in
		@param 	index 	The index of the vertex
	*/
	Insertion(Triangulation * const t, const unsigned int pid, const int index);

	/*
		Constructor:
		This constructor directly takes the edge into which the new vertex will
		be inserted

		@param 	T 		The triangulation the polygon lives in
		@param 	pid 	The id of the polygon to insert in
		@param 	e 	 	The edge to insert in
	*/
	Insertion(Triangulation * const t, const unsigned int pid, TEdge *edge);



	/*
		O ~ T ~ H ~ E ~ R ~ S
	*/

	/*
		The function checkStability() checks whether the insertion can be done without
		risking numerical problems. Therefore the chosen edge must have at least the
		length Settings::minLength and, in case we use the double arithmetics, both
		adjacent triangles' determinants must be at least Settings::minDetInsertion.

		@return 	True if the chosen edge fulfills the criteria, otherwise false
	*/
	bool checkStability() const;

	/*
		The function execute() inserts a vertex at the middle of the chosen edge.
		Therefore at first it deletes the chosen edge and its adjacent triangle,
		then it inserts the new vertex, adds edges to the four surrounding vertices
		and generate the new triangles for this quadrilateral.
	*/
	void execute();

	/*
		The function translate() tries to move the new vertex away from the edge.
		Therefore it tries up to Settings::insertionTries times to find a translation
		which leads to a simple polygon and execute it. If it has not found a suitable
		translation till the limit is reached, the new vertex will just stay were it is.
	*/
	void translate() const;
};

#endif
