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

class SelectionTree{

private:
	STEntry *root;

public:

	SelectionTree();

	void insert(TEdge *e);

	TEdge *getRandomEdge();

	void check();
};

#endif