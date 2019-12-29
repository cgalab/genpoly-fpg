/* 
 * Copyright 2019 Philipp Mayer - pmayer@cs.sbg.ac.at
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
#include "settings.h"


/*
	Define the class SelectionTree
*/
#ifndef __SELECTIONTREE_H_
#define __SELECTIONTREE_H_

/*
	Define and include headers to the entites contained by SelectionTrees
*/
class STEntry;
class TEdge;

#include "stentry.h"
#include "tedge.h"

/*
	SelectionTree provides a method for selecting an TEdge of a polygon uniform
	at random weighted by the lengths of the TEdges. For that it utilizes an 
	unordered binary tree containing all edges. The binary tree stays balanced
	by containing the elements in each subtree.
*/
class SelectionTree{

private:
	
	/*
		The root entry of the binary tree
	*/
	STEntry *root;

public:

	/*
		C ~ O ~ N ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R ~ S
	*/

	SelectionTree();


	/*
		S ~ E ~ T ~ T ~ E ~ R ~ S
	*/

	/*
		The function insert() inserts an TEdge into the binary while making
		sure that the binary stays balanced.

		@param 	e 	A new TEdge which has to be inserted in the binary tree
	*/
	void insert(TEdge *e);


	/*
		G ~ E ~ T ~ T ~ E ~ R ~ S
	*/

	/*
		The function getRandomEdge() selects an entry of the binary tree
		uniformly at random weighted by the lengths of the TEdges
	*/
	TEdge *getRandomEdge();


	/*
		O ~ T ~ H ~ E ~ R ~ S
	*/

	/*
		The function check() iterates through the whole binary tree to check
		whether the TEdge of each not can still be found.
	*/
	void check();
};

#endif