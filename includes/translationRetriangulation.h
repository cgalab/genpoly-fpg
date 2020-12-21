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
	Include standard libraries
*/
#include <list>
#include <map>

/*
	Include my headers	
*/
#include "settings.h"
#include "translation.h"
#include "polygon.h"

#ifndef __TRANSLATION_RETRIANGULATION_H_
#define __TRANSLATION_RETRIANGULATION_H_

class TranslationRetriangulation : public Translation{

private:

	/*
		Flag whether the moving vertex changes the side relative to its neighbors
	*/
	bool sideChange;

	/*
		The polygons we have to retriangulate during the translation. Depending on the
		type of translation not all of these polygons are used.
	*/
	// The polygon in opposite direction to the translation direction
	Polygon *p0;
	Polygon *p1;
	Polygon *p2;
	Polygon *p3;

	void buildPolygonsSideChange();
	void buildPolygonsSideRemainCase1();
	void buildPolygonsSideRemainCase2();
	void buildPolygonsSideRemainCase3();
	void bPSCOppositeDirection();
	void bPSRC1TranslationDirection();
	void bPSRC1OppositeDirection();
	void bPSRC2OppositeDirection();
	void bPSRC3OppositeDirection(Vertex *primaryV, Vertex *secondaryV, TEdge *primaryE,
		TEdge *secondaryE);
	void bPSCTranslationDirection();
	TEdge *bPSRC3SPOld(Vertex *primaryV, Vertex *secondaryV, TEdge *primaryE,
		TEdge *secondaryE, TEdge *primaryNewE, TEdge *secondaryNewE);
	void bPSRC3TranslationDirection(Vertex *primaryV, Vertex *secondaryV, TEdge *borderE,
		TEdge *primaryE, TEdge *primaryNewE, TEdge *secondaryE, TEdge *secondaryNewE);
	bool insideTriangle(Triangle *t, Vertex *toCheck);

public:

	/*
		LIST OF PUBLIC MEMBER FUNCTIONS

		CONSTRUCTORS:
		
						TranslationKinetic(Triangulation *Tr, int i, double dX, double dY)

		OTHERS:

		bool 			checkOrientation()
		enum Executed 	execute()
		bool 			checkSimplicityOfTranslation()
	*/

	
	/*
		C ~ O ~ N ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R ~ S
	*/

	/*
		Constructor:
		Generates a new Translation of type DEFAULT. Picks the moving vertex and its
		neighbors from the triangulations vertices list and initialize the translation path.

		@param 	Tr 		The triangulation the moving vertex lives in
		@param 	i 		The index of the moving vertex in the triangulation
		@param 	dX 		The x-component of the translation vector
		@param 	dY 		The y-component of the translation vector

		Note:
			Translations of other types can just be generated by the translation class itself.
	*/
	TranslationRetriangulation(Triangulation *Tr, int i, double dX, double dY);


	/*
		O ~ T ~ H ~ E ~ R ~ S
	*/

	/*
		The function execute() processes a translation. First it calls checkSplit() for checking
		whether the translation can be executed directly or must be split. If the flag split is set,
		it calls one of the functions which split the translation into two translations
		(depending on the geometric case) and executes them, otherwise execute() executes the
		translation by successively working through the events in the event queue.

		@return 	Indicates whether the execution was rejected, aborted or fully processed

		Note:
			For more information on the splits see my Master Thesis
	*/
	enum Executed execute() override;


	/*
		D ~ E ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R
	*/

	/*
		Destructor:
		Checks the surrounding polygon of the moved vertex and deletes all the remaining construction
		vertices and edges. It errors with exit code 6 if the surrounding polygon check fails. It also
		deletes the flip stack.
	*/	
	~TranslationRetriangulation() override;
};

#endif
