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

#include "selectionTree.h"

/*
	C ~ O ~ N ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R ~ S
*/

SelectionTree::SelectionTree() : root(NULL) {}


/*
	S ~ E ~ T ~ T ~ E ~ R ~ S
*/

/*
	The function insert() inserts an TEdge into the binary while making
	sure that the binary stays balanced.

	@param 	e 	A new TEdge which has to be inserted in the binary tree
*/
void SelectionTree::insert(TEdge * e){
	STEntry *entry, *child;

	if(root == NULL){
		root = new STEntry(e, NULL);
	}else{
		entry = root;
		child = root;

		while(child != NULL){
			entry = child;
			child = (*entry).getLighterSubtree();
		}

		(*entry).addChild(new STEntry(e, entry));
	}
}


/*
	G ~ E ~ T ~ T ~ E ~ R ~ S
*/

/*
	The function getRandomEdge() selects an entry of the binary tree
	uniformly at random weighted by the lengths of the TEdges
*/
TEdge *SelectionTree::getRandomEdge(){
	STEntry *entry, *last;

	if(root == NULL)
		return NULL;

	last = NULL;
	entry = root;

	while(last != entry){
		last = entry;
		entry = (*last).getRandomChild();
	}

	return (*entry).getEdge();
}


/*
	O ~ T ~ H ~ E ~ R ~ S
*/

/*
	The function check() iterates through the whole binary tree to check
	whether the TEdge of each not can still be found.
*/
void SelectionTree::check(){
	
	if(root == NULL){
		fprintf(stderr, "This SelectionTree is empty!\n");
		return;
	}

	fprintf(stderr, "Total number of elements: %d\n", (*root).getNrElementsTotal());

	(*root).check();
}