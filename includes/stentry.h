/* 
 * Copyright 2020 Philipp Mayer - pmayer@cs.sbg.ac.at
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
	Define and include headers to the entities contained by a STEntry
*/
template<class T> class SelectionTree;

#include "selectionTree.h"


/*
	The class STEntry represents one entry of a SelectionTree. It contains
	a TEdge, information on the length of the TEdge an the TEdges in its
	subtrees and information on the number of entries in its subtrees.
*/
template<class T> class STEntry{

private:

	/*
		The SelectionTree this element belongs to.
	*/
	SelectionTree<T> *tree;

	/*
		The object this entry represents
	*/
	T element;

	/*
		Parent and child nodes of this entry in a SelectionTree
	*/
	STEntry<T> *parent;
	STEntry<T> *leftChild;
	STEntry<T> *rightChild;

	/*
		Weight of the object and the sum of the objects' weights in its subtrees
	*/
	double elementWeight;
	double leftWeight;
	double rightWeight;

	/*
		Sum of weights of all objects in the subtree which has this entry as root
	*/
	double totalWeight;

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
		Sets the parent entry in the SelectionTree and the weight of its
		object.

		@param 	e 	The object this STEntry represents
		@param 	prt	The parent entry in the SelectionTree
		@param 	st 	The SelectionTree the new STEntry belongs to
	*/
	STEntry<T>(T e, STEntry<T> *prt, SelectionTree<T> *st) : tree(st), element(e),
		parent(prt), leftChild(NULL), rightChild(NULL), leftWeight(0), rightWeight(0),
		nrElementsLeft(0), nrElementsRight(0), nrElementsTotal(1) {
		
		if((*tree).isWeighted())
			elementWeight = (*element).getWeight();
		else
			elementWeight = 1;
		
		totalWeight = elementWeight;

		// Register the entry at the object
		(*e).setSTEntry(this);
	}


	/*
		S ~ E ~ T ~ T ~ E ~ R ~ S
	*/

	/*
		The function addChild() adds a new child entry to this STEntry. The new
		child gets added left if there is no child already, otherwise it gets
		added right. Afterwards the number of elements and the total weights get
		updated for all ancestors.

		@param 	child 	The new child entry

		Note:
			This function assumes, that there is still one child pointer unset,
			so it overrides entries if all child pointers are set.
	*/
	void addChild(STEntry<T> *child){
		
		if(nrElementsLeft <= nrElementsRight){
			leftChild = child;
			nrElementsLeft = 1;
			leftWeight = (*leftChild).getTotalWeight();
		}else{
			rightChild = child;
			nrElementsRight = 1;
			rightWeight = (*rightChild).getTotalWeight();
		}

		nrElementsTotal = nrElementsRight + nrElementsLeft + 1;
		totalWeight = elementWeight + leftWeight + rightWeight;

		if(parent != NULL)
			(*parent).update();
	}

	/*
		Assign a new object to the entry.
	*/
	void setObject(T e){
		//TODO: some checking whether the entry really is empty at the moment

		element = e;
		(*e).setSTEntry(this);

		update();
	}


	/*
		R ~ E ~ M ~ O ~ V ~ E ~ R
	*/

	/*
		Removes the object of the STEntry and adds the entry to the queue of
		empty entries.
	*/
	void removeObject(){
		element = NULL;
		elementWeight = 0;
		totalWeight = leftWeight + rightWeight;

		if(parent != NULL)
			(*parent).update();

		(*tree).addNodeToQueue(this);
	}


	/*
		G ~ E ~ T ~ T ~ E ~ R ~ S
	*/

	/*
		@return 	The root of the subtree with less elements
	*/
	STEntry<T> *getLighterSubtree(){
	
		if(nrElementsLeft <= nrElementsRight)
			return leftChild;
		else
			return rightChild;
	}

	/*
		The function getRandomChild() randomly selects a child entry of this entry
		with probabilities corresponding to the total weight of the objects in it.
		If it returns this entry again, the object of this entry got selected.

		@return 	The STEntry of the randomly selected object
	*/
	STEntry<T> *getRandomChild(){
		double random = (*Settings::generator).getDoubleUniform(0, totalWeight);

		if(nrElementsLeft != 0 && random < leftWeight)
			return leftChild;

		if(nrElementsRight != 0 && random < leftWeight + rightWeight)
			return rightChild;

		return this;
	}

	/*
		@return 	The total weight of all objects in the subtree with this entry
					as root
	*/
	double getTotalWeight(){
		return totalWeight;
	}

	/*
		@return 	The object of this entry
	*/
	T getObject(){
		return element;
	}

	/*
		@return 	The total number of elements of the subtree with this entry
					as root
	*/
	unsigned int getNrElementsTotal(){
		return nrElementsTotal;
	}


	/*
		O ~ T ~ H ~ E ~ R ~ S
	*/

	/*
		The function update() updates the lengths and the numbers of elements of
		this entry and calls update() for the parent entry.
	*/
	void update(){

		if(!(*tree).isWeighted())
			elementWeight = 1;
		else if(element != NULL)
			elementWeight = (*element).getWeight();
		else
			elementWeight = 0;

		if(leftChild != NULL){
			leftWeight = (*leftChild).getTotalWeight();
			nrElementsLeft = (*leftChild).getNrElementsTotal();
		}

		if(rightChild != NULL){
			rightWeight = (*rightChild).getTotalWeight();
			nrElementsRight = (*rightChild).getNrElementsTotal();
		}
		
		totalWeight = elementWeight + leftWeight + rightWeight;
		nrElementsTotal = nrElementsLeft + nrElementsRight + 1;

		if(parent != NULL)
			(*parent).update();
	}

	/*
		The function check() checks whether the object of the element is still there
		and calls check() for all child entries.
	*/
	void check(){
	
		// Just take a look whether the element still exists
		(*element).getWeight();

		printf("Number of elements in the subtrees: %d (left)  %d (right)\n", nrElementsLeft, nrElementsRight);

		if(leftChild != NULL)
			(*leftChild).check();

		if(rightChild != NULL)
			(*rightChild).check();
	}
};

#endif