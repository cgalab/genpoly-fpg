/* 
 * Copyright 2021 Philipp Mayer - philmay1992@gmail.com
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
		Flag for aborted translations
	*/
	bool aborted;

	/*
		The polygons we have to retriangulate during the translation. Depending on the
		type of translation not all of these polygons are used.
	*/
	Polygon *p0;
	Polygon *p1;
	Polygon *p2;
	Polygon *p3;


	/*
		P ~ R ~ I ~ V ~ A ~ T ~ E 	M ~ E ~ M ~ B ~ E ~ R 	F ~ U ~ N ~ C ~ T ~ I ~ O ~ N ~ S
	*/

	/*
		The function buildPolygonsSideChange() finds the polygons to retriangulate in
		case of a translation where the moving vertex changes the side relative to its
		adjacent vertices in the polygon. In this case we have up to two edge-visible
		polygons (relative to the polygon edges incident to original) in translation
		direction and one star-shaped polygon in opposite direction with the original
		position as point in the kernel.
	*/
	void buildPolygonsSideChange();

	/*
		The function buildPolygonsSideRemainCase1() finds the polygons to retriangulate in
		case of a translation with the following properties:
			(1) The moving vertex remains at the same side relative to its adjacent vertices
				in the polygon
			(2) The vertex moves closer to the edge between the two adjacent vertices, i.e.
				it is in the interior of the triangle defined by the original position and
				the two adjacent vertices
		In this case we get two edge-visible polygons (relative to the polygon edges incident
		to original) in translation direction and one star-shaped polygon in the other
		direction with the original position as point in the kernel.
	*/
	void buildPolygonsSideRemainCase1();

	/*
		The function buildPolygonsSideRemainCase2() finds the polygons to retriangulate in
		case of a translation with the following properties:
			(1) The moving vertex remains at the same side relative to its adjacent vertices
				in the polygon
			(2) The vertex moves farther from the edge between the two adjacent vertices, i.e.
				the original position of the vertex is in the interior of the triangle defined
				by the new position and the two adjacent vertices in the polygon
		In this case we get two edge-visible polygons (relative to the polygon edges incident
		to original) in translation direction and one star-shaped polygon in the other
		direction with the original position as point in the kernel.
	*/
	void buildPolygonsSideRemainCase2();

	/*
		The function buildPolygonsSideRemainCase3() finds the polygons to retriangulate in
		case of a translation with the following properties:
			(1) The moving vertex remains at the same side relative to its adjacent vertices
				in the polygon
			(2) The vertex moves left/right, i.e. one of the incident edges before the
				translation intersects one of the edges after the translation.
		Such a translation can also be characterized by the fact that it has a non-simple
		translation quadrilateral (oldV, prevV, newV, nextV).
		In this case we get two edge-visible polygons (relative to the polygon edges incident
		to original) in translation direction, one edge-visible polygon (relative to primaryE)
		in the opposite direction and one either edge-visible, or star-shaped (depending on
		the original position) around the original position of the vertex.

		Note:
			- There is one case which can not be solved by the method. Details can be found
				in bPSRC3SPOld().
	*/
	void buildPolygonsSideRemainCase3();

	/*
		The function bPSCOppositeDirection() builds a star-shaped polygon opposite to the
		translation direction for a translation which leads to a side change of the moving
		vertex relative to its adjacent vertices. The original position can be used as
		kernel point. The polygon can be build by a walk along the SP of the moving vertex
		between the two adjacent vertices.
	*/
	void bPSCOppositeDirection();

	/*
		The function bPSCOppositeDirection() builds a star-shaped polygon opposite to the
		translation direction for a translation which does not lead to a side change and
		the new position of the moving vertex is inside of the triangle defined by its
		original position and the two adjacent vertices. The original position can be used
		as kernel point. The polygon can be build by a walk along the SP of the moving
		vertex between the two adjacent vertices.
	*/
	void bPSRC1OppositeDirection();

	/*
		The function bPSCOppositeDirection() builds a star-shaped polygon opposite to the
		translation direction for a translation which does not lead to a side change and
		the old position of the moving vertex is inside of the triangle defined by its
		new position and the two adjacent vertices. The original position can be used
		as kernel point. The polygon can be build by a walk along the SP of the moving
		vertex between the two adjacent vertices.
	*/
	void bPSRC2OppositeDirection();

	/*
		The function bPSRC3OppositeDirection() builds an edge-visible polygon to primaryE
		opposite to the translation direction for a translation with a non-simple
		translation quadrilateral. The polygon can be build by a walk along the SP of the
		moving vertex between the two adjacent vertices.

		@param 	primaryE 	The old edge which new version is intersected by one of the
							old edges, so either prevOldE or nextOldE
		@param 	secondaryE 	The other old edge
		@param 	primaryV 	The adjacent vertex incident to primaryE
		@param 	secondaryV 	The adjacent vertex incident to secondaryE
	*/
	void bPSRC3OppositeDirection(Vertex *primaryV, Vertex *secondaryV, TEdge *primaryE,
		TEdge *secondaryE);

	/*
		The function bPSCTranslationDirection() builds to edge-visible polygons along
		the new edges in translation direction. It can be used if the moving vertex changes
		its side relative to the adjacent vertices or moves farther away (side remain case 2).
		For that it traces the two new vertices through the triangulation.
	*/
	void bPSCTranslationDirection();

	/*
		The function bPSRC1TranslationDirection() builds up to two edge-visible polygon in
		translation direction relative to the two edges incident to the moving vertices for
		a translation which moves the vertex closer to the line defined by the two adjacent
		vertices. For that it goes along the SP of the moving vertex and checks for each
		vertex whether the incident line to the original position intersects prevNewE or
		nextNewE. Depending on which of the new edges is intersected the vertices can be
		assigned to one of the polygons.
	*/
	void bPSRC1TranslationDirection();

	/*
		The function bPSRC3SPOld() builds the polygon in the area of the original position
		for a translation with non-simple quadrilateral. For that it goes along the SP of
		the moving vertex starting at primaryV until it reaches the triangle in which the
		translation and or through which the translations leaves the SP. The first SP vertex
		of this triangle is called borderV. In case the translation leaves SP and borderV can
		not be seen from primaryV through the interior of the SP, the translation must be
		aborted. Otherwise if the translation leaves the SP, an edge between these two vertices
		closes the polygon. Depending on whether the original position is inside of the polygon,
		it is star-shaped with the old position in its kernel, or edge-visible relative to
		the newly inserted edge.

		@param 	primaryE 		The old edge which new version is intersected by one of the
								old edges, so either prevOldE or nextOldE
		@param 	secondaryE 		The other old edge
		@param 	primaryV 		The adjacent vertex incident to primaryE
		@param 	secondaryV 		The adjacent vertex incident to secondaryE
		@param 	primaryNewE 	The new version of primaryE
		@param 	secondaryNewE 	The new version of secondaryE
		@return 				The SP edge which is intersected by the new edges, if the
								translation leaves SP, or NULL, if it does not leave the SP or
								must be aborted
	*/
	TEdge *bPSRC3SPOld(Vertex *primaryV, Vertex *secondaryV, TEdge *primaryE,
		TEdge *secondaryE, TEdge *primaryNewE, TEdge *secondaryNewE);

	/*
		The function bPSRC3TranslationDirection() builds to edge-visible polygon (relative
		to the two edges incident to the moving vertex) in translation direction for a
		translation with non-simple translation quadrilateral. For that it starts at secondaryV
		and goes along the SP of the moving vertex until it reaches borderE. Then it traces the
		new edges through the triangulation starting at borderE.

		@param 	primaryE 		The old edge which new version is intersected by one of the
								old edges, so either prevOldE or nextOldE
		@param 	secondaryE 		The other old edge
		@param 	primaryV 		The adjacent vertex incident to primaryE
		@param 	secondaryV 		The adjacent vertex incident to secondaryE
		@param 	primaryNewE 	The new version of primaryE
		@param 	secondaryNewE 	The new version of secondaryE
		@param 	borderE			The SP edge which is intersected by the new edges, if the
								translation leaves SP, or NULL, if it does not leave the SP or
								must be aborted
	*/
	void bPSRC3TranslationDirection(Vertex *primaryV, Vertex *secondaryV, TEdge *borderE,
		TEdge *primaryE, TEdge *primaryNewE, TEdge *secondaryE, TEdge *secondaryNewE);

	/*
		The function checkVisibility() checks whether v0 is visible to v1 through the interior
		of the SP of the moving vertex. For that it traces the edge between the two vertices
		through the triangulation.

		@param 	v0 		The first vertex to check for visibility
		@param 	v1 		The second vertex of interest
		@param 	refV 	A reference vertex which indicates the side of the interior of SP
		@return 		True if v0 is visible to v1, otherwise false
	*/
	bool checkVisibility(Vertex *v0, Vertex *v1, Vertex *refV);


public:

	/*
		LIST OF PUBLIC MEMBER FUNCTIONS

		CONSTRUCTORS:
		
						TranslationKinetic(Triangulation *Tr, int i, double dX, double dY)
						TranslationKinetic(Triangulation *Tr, Vertex *v, double dX, double dY)

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
		Constructor:
		Generates a new Translation of type DEFAULT. Picks the moving vertex and its
		neighbors from the triangulations vertices list and initialize the translation path.

		@param 	Tr 		The triangulation the moving vertex lives in
		@param 	v 		The vertex to move
		@param 	dX 		The x-component of the translation vector
		@param 	dY 		The y-component of the translation vector

		Note:
			Translations of other types can just be generated by the translation class itself.
	*/
	TranslationRetriangulation(Triangulation *Tr, Vertex *v, double dX, double dY);


	/*
		O ~ T ~ H ~ E ~ R ~ S
	*/

	/*
		The function execute() processes a translation. First it calls the suitable function
		for building the polygons to retriangulate, then it moves the vertex to its target
		position and calls the functions to retriangulate the polygons.

		@return 	Indicates whether the execution was aborted or fully processed
	*/
	enum Executed execute() override;


	/*
		D ~ E ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R
	*/

	/*
		Destructor:
		Checks the surrounding polygon of the moved vertex and deletes all the remaining construction
		vertices and edges. It errors with exit code 6 if the surrounding polygon check fails. It also
		deletes the additional entities used during the translation.
	*/	
	~TranslationRetriangulation() override;
};

#endif
