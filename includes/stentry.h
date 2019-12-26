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

class STEntry{

private:

	TEdge *edge;

	STEntry *parent;
	STEntry *leftChild;
	STEntry *rightChild;

	double entryLength;
	double leftLength;
	double rightLength;
	double totalLength;

	unsigned int nrElementsLeft;
	unsigned int nrElementsRight;
	unsigned int nrElementsTotal;

public:

	STEntry(TEdge *e, STEntry *prt);

	STEntry *getLighterSubtree();

	void addChild(STEntry *child);

	STEntry *getRandomChild();

	double getTotalLength();

	TEdge *getEdge();

	void update();

};

#endif