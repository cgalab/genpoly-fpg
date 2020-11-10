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
#include <queue>

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
	Define and include headers to the entities contained by SelectionTrees
*/
template<class T> class STEntry;

#include "stentry.h"


/*
	SelectionTree provides a method for selecting an TEdge of a polygon uniform
	at random weighted by the lengths of the TEdges. For that it utilizes an 
	unordered binary tree containing all edges. The binary tree stays balanced
	by containing the elements in each subtree.
*/
template<class T> class SelectionTree{

private:
	
	/*
		The root entry of the binary tree
	*/
	STEntry<T> *root;

	/*
		A queue which keeps track of the empty nodes of the SelectionTree
	*/
	std::queue<STEntry<T>*> emptyNodes;

	/*
		Flag whether the select should be weighted or not
	*/
	bool weighted;

public:

	/*
		C ~ O ~ N ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R ~ S
	*/

	SelectionTree<T>(bool w) : root(NULL) {

		weighted = w;
	}



	/*
		S ~ E ~ T ~ T ~ E ~ R ~ S
	*/

	/*
		The function insert() inserts an object into the binary tree while making
		sure that the binary tree stays balanced. If there are empty nodes available,
		these nodes are used first.

		@param 	e 	A new object which has to be inserted in the binary tree
	*/
	void insert(T e){
		STEntry<T> *entry, *child;

		// First check whether there is any empty node and use it in case
		if(!emptyNodes.empty()){
			entry = emptyNodes.front();
			emptyNodes.pop();

			(*entry).setObject(e);
		}else{

			if(root == NULL)
				root = new STEntry<T>(e, NULL, this);
			else{
				entry = root;
				child = root;

				while(child != NULL){
					entry = child;
					child = (*entry).getLighterSubtree();
				}

				(*entry).addChild(new STEntry<T>(e, entry, this));
			}
		}
	}

	/*
		Adds a node which element has been removed to the queue for empty nodes.
	*/
	void addNodeToQueue(STEntry<T> *node){
		emptyNodes.push(node);
	}


	/*
		G ~ E ~ T ~ T ~ E ~ R ~ S
	*/

	/*
		The function getRandomObject() selects an entry of the binary tree
		uniformly at random weighted by the objects' weights.

		@return 	The randomly selected object
	*/
	T getRandomObject(){
		STEntry<T> *entry, *last;

		if(root == NULL)
			return NULL;

		last = NULL;
		entry = root;

		while(last != entry){
			last = entry;
			entry = (*last).getRandomChild();
		}

		return (*entry).getObject();
	}

	/*
		@return 	True if the SelectionTree is weighted, otherwise false
	*/
	bool isWeighted(){
		return weighted;
	}


	/*
		O ~ T ~ H ~ E ~ R ~ S
	*/

	/*
		The function check() iterates through the whole binary tree to check
		whether the object of each node can still be found.
	*/
	void check(){
	
		if(root == NULL){
			fprintf(stderr, "This SelectionTree is empty!\n");
			return;
		}

		fprintf(stderr, "Total number of elements: %d\n", (*root).getNrElementsTotal());

		(*root).check();
	}
};

#endif