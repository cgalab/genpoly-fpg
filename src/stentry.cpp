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

#include "stentry.h"

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
STEntry::STEntry(TEdge *e, STEntry *prt) : edge(e), parent(prt), leftChild(NULL),
	rightChild(NULL), leftLength(0), rightLength(0), nrElementsLeft(0), 
	nrElementsRight(0), nrElementsTotal(1) {
	
	entryLength = (*edge).length();
	totalLength = entryLength;

	// Register the entry at the edge
	(*e).setSTEntry(this);
}


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
void STEntry::addChild(STEntry *child){
	
	if(nrElementsLeft <= nrElementsRight){
		leftChild = child;
		nrElementsLeft = 1;
		leftLength = (*leftChild).getTotalLength();
	}else{
		rightChild = child;
		nrElementsRight = 1;
		rightLength = (*rightChild).getTotalLength();
	}

	nrElementsTotal = nrElementsRight + nrElementsLeft + 1;
	totalLength = entryLength + leftLength + rightLength;

	if(parent != NULL)
		(*parent).update();

}

/*
	The function replaceEdge() replaces the TEdge of this entry by the TEdge
	e. Afterwards it updates the lengths of all ancestors.

	@param 	e 	The new TEdge
*/
void STEntry::replaceEdge(TEdge *e){
	
	edge = e;

	// Register the entry at the edge
	(*edge).setSTEntry(this);

	entryLength = (*edge).length();

	totalLength = entryLength + leftLength + rightLength;

	if(parent != NULL)
		(*parent).update();
}


/*
	G ~ E ~ T ~ T ~ E ~ R ~ S
*/

/*
	@return 	The root of the subtree with less elements
*/
STEntry *STEntry::getLighterSubtree(){
	
	if(nrElementsLeft <= nrElementsRight)
		return leftChild;
	else
		return rightChild;
}

/*
	The function getRandomChild() randomly selects a child entry of this entry
	with probabilities corresponding to the total length of the edges in it.
	If it returns this entry again, the TEdge of this entry got selected.

	@return 	The STEntry of the randomly selected TEdge
*/
STEntry *STEntry::getRandomChild(){
	double random = (*Settings::generator).getDoubleUniform(0, totalLength);

	if(nrElementsLeft != 0 && random < leftLength)
		return leftChild;

	if(nrElementsRight != 0 && random < leftLength + rightLength)
		return rightChild;

	return this;
}

/*
	@return 	The total length of all TEdges in the subtree with this entry
				as root
*/
double STEntry::getTotalLength(){
	return totalLength;
}

/*
	@return 	The TEdge of this entry
*/
TEdge *STEntry::getEdge(){
	return edge;
}

/*
	@return 	The total number of elements of the subtree with this entry
				as root
*/
unsigned int STEntry::getNrElementsTotal(){
	return nrElementsTotal;
}


/*
	O ~ T ~ H ~ E ~ R ~ S
*/

/*
	The function update() updates the lengths and the numbers of elements of
	this entry and calls update() for the parent entry.
*/
void STEntry::update(){

	if(leftChild != NULL){
		leftLength = (*leftChild).getTotalLength();
		nrElementsLeft = (*leftChild).getNrElementsTotal();
	}

	if(rightChild != NULL){
		rightLength = (*rightChild).getTotalLength();
		nrElementsRight = (*rightChild).getNrElementsTotal();
	}

	entryLength = (*edge).length();

	totalLength = entryLength + leftLength + rightLength;
	nrElementsTotal = nrElementsLeft + nrElementsRight + 1;

	if(parent != NULL)
		(*parent).update();
}

/*
	The function check() checks whether the edge of the entry is still there
	and calls check() for all child entries.
*/
void STEntry::check(){
	
	// Just take a look whether the edge still exists
	(*edge).length();

	if(leftChild != NULL)
		(*leftChild).check();

	if(rightChild != NULL)
		(*rightChild).check();
}