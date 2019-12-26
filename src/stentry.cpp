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

STEntry::STEntry(TEdge *e, STEntry *prt) : edge(e), parent(prt){
	entryLength = (*edge).length();
	leftLength = 0;
	rightLength = 0;
	totalLength = entryLength;

	leftChild = NULL;
	rightChild = NULL;

	nrElementsLeft = 0;
	nrElementsRight = 0;
	nrElementsTotal = 1;
}

STEntry *STEntry::getLighterSubtree(){
	if(nrElementsLeft <= nrElementsRight)
		return leftChild;
	else
		return rightChild;
}

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

STEntry *STEntry::getRandomChild(){
	double random = (*Settings::generator).getDoubleUniform(0, totalLength);

	if(leftLength != 0 && random < leftLength)
		return leftChild;

	if(rightLength != 0 && random < leftLength + rightLength)
		return rightChild;

	return this;
}

double STEntry::getTotalLength(){
	return totalLength;
}

TEdge *STEntry::getEdge(){
	return edge;
}

void STEntry::update(){
	if(leftChild != NULL)
		leftLength = (*leftChild).getTotalLength();

	if(rightChild != NULL)
		rightLength = (*rightChild).getTotalLength();

	entryLength = (*edge).length();

	totalLength = entryLength + leftLength + rightLength;

	if(parent != NULL)
		(*parent).update();
}