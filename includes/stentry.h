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
	Include standard libraries
*/

/*
	Include my headers
*/
#include "settings.h"


/*
	Define the class STEntry
*/
#ifndef __STENTRY_H_
#define __STENTRY_H_

/*
	Define and include headers to the entites contained by a STEntry
*/
class TEdge;

#include "tedge.h"

/*
	The class STEntry represents one entry of a SelectionTree. It contains
	a TEdge, information on the length of the TEdge an the TEdges in its
	subtrees and information on the number of entries in its subtrees.
*/
class STEntry{

private:

	/*
		The TEdge this entry represents
	*/
	TEdge *edge;

	/*
		Parent and child nodes of this entry in a SelectionTree
	*/
	STEntry *parent;
	STEntry *leftChild;
	STEntry *rightChild;

	/*
		Lengths of the TEdge and the TEdges in its subtrees
	*/
	double entryLength;
	double leftLength;
	double rightLength;

	/*
		Sum of length of all TEdges in the subtree which has this entry as root
	*/
	double totalLength;

	/*
		Element counts for the subtrees
	*/
	unsigned int nrElementsLeft;
	unsigned int nrElementsRight;
	unsigned int nrElementsTotal;

public:

	/*
		C ~ O ~ N ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R ~ S
	*/

	/*
		Constructor:
		Sets the parent entry int the Selection and computes the length of its
		TEdge

		@param 	e 	The TEdge this STEntry represents
		@param 	prt	The parent entry in the SelectionTree
	*/
	STEntry(TEdge *e, STEntry *prt);

	/*
		S ~ E ~ T ~ T ~ E ~ R ~ S
	*/

	/*
		The function addChild() adds a new child entry to this STEntry. The new
		child gets added left if there is no child already, otherwise it gets
		added right. Afterwards the number of elements and the total length gets
		updated for all ancestors.

		@param 	child 	The new child entry

		Note:
			This function assumes, that there is still one child pointer unset,
			so it overrides entries if all child pointers are set.
	*/
	void addChild(STEntry *child);

	/*
		The function replaceEdge() replaces the TEdge of this entry by the TEdge
		e. Afterwards it updates the lengths of all ancestors.

		@param 	e 	The new TEdge
	*/
	void replaceEdge(TEdge *e);


	/*
		G ~ E ~ T ~ T ~ E ~ R ~ S
	*/

	/*
		@return 	The root of the subtree with less elements
	*/
	STEntry *getLighterSubtree();

	/*
		The function getRandomChild() randomly selects a child entry of this entry
		with probabilities corresponding to the total length of the edges in it.
		If it returns this entry again, the TEdge of this entry got selected.

		@return 	The STEntry of the randomly selected TEdge
	*/
	STEntry *getRandomChild();

	/*
		@return 	The total length of all TEdges in the subtree with this entry
					as root
	*/
	double getTotalLength();

	/*
		@return 	The TEdge of this entry
	*/
	TEdge *getEdge();

	/*
		@return 	The total number of elements of the subtree with this entry
					as root
	*/
	unsigned int getNrElementsTotal();


	/*
		O ~ T ~ H ~ E ~ R ~ S
	*/

	/*
		The function update() updates the lengths and the numbers of elements of
		this entry and calls update() for the parent entry.
	*/
	void update();

	/*
		The function check() checks whether the edge of the entry is still there
		and calls check() for all child entries.
	*/
	void check();
};

#endif