/* 
 * Copyright 2021 Philipp Mayer - pmayer@cs.sbg.ac.at
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
#include <stdio.h>
#include <vector>
#include <string>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#include <math.h>
#endif

/*
	Include my headers	
*/
#include "predicates.h"
#include "stentry.h"

#ifndef __TRIANGLE_H_
#define __TRIANGLE_H_

/*
	Define and include headers to the entities used in Triangle
*/
class TEdge;
class Vertex;

#include "tedge.h"
#include "vertex.h"

class Triangle{

private:
	
	/*
		The edges building the triangle
	*/
	TEdge * const e0;
	TEdge * const e1;
	TEdge * const e2;

	/*
		The corresponding vertices
	*/
	Vertex * const v0;
	Vertex * const v1;
	Vertex * const v2;

	/*
		Indicates whether the triangle is internal to the polygon or not.
		Triangles in the interior of a hole are counted as external.
	*/
	const bool internal;

	/*
		Indicates whether the triangle is element of the actual event queue
	*/
	bool enqueued;

	/*
		The triangles selection tree entry
	*/
	STEntry<Triangle*> *entry;

	/*
		The unique ID of the triangle
	*/
	const unsigned long long id;

	/*
		The number of already generated triangles
	*/
	static unsigned long long n;

	/*
		Number of triangles existing at the moment
	*/
	static unsigned long long existing; 


	/*
		P ~ R ~ I ~ V ~ A ~ T ~ E 	M ~ E ~ M ~ B ~ E ~ R 	F ~ U ~ N ~ C ~ T ~ I ~ O ~ N ~ S
	*/

	/*
		The function signedAreaExact() computes an estimate of the signed area of the triangle
		by using Shewchuks "Routines for Arbitrary Precision Floating-point Arithmetic and Fast
		Robust Geometric Predicates". So in fact the sign of this signed area is absolutely
		trustworthy, whereas the exact value might still have numerical errors. Also 0 is
		trustworthy.

		@return 	The estimate for the signed area

		Note:
			In fact this functions computes 2 times the signed area of the triangle.
	*/
	double signedAreaExact() const;

	/*
		The function signedAreaDouble() computes an estimate of the signed area of the triangle
		with standard floating-point arithmetics. To provide stability it orders the vertices by
		reserveID before computing the determinant, such that the vertices in the determinant
		always have the same order independent of the ordering of the vertices in the triangle
		class. The correct sign therefore is ensured by the case distinctions here.

		@return 	The estimate for the signed area

		Note:
			In fact this functions computes 2 times the signed area of the triangle.
	*/
	double signedAreaDouble() const;

	/*
		The function det() computes the determinant of three 2D vertices. It asserts that the
		input vertices are already sorted by their reserveID! Then it shifts the vertices in
		a way that V0 is exactly at the origin to reduce the number of necessary calculations
		and therefore improve the numerical correctness of the result.

		@param 	V0 	Vertex placed in the first row of the matrix
		@param 	V1 	Vertex placed in the second row of the matrix
		@param 	V2 	Vertex placed in the third row of the matrix
		@return 	The determinant of the vertices matrix

		Note:
			This is just default floating-point arithmetic!
	*/
	double det(Vertex * const V0, Vertex * const V1, Vertex * const V2) const;

public:

	bool operator ==(const Triangle& t) const{

		if(id == t.id)
			return true;
		else
			return false;
	}

	bool operator !=(const Triangle& t) const{

		if(id != t.id)
			return true;
		else
			return false;
	}
	
	/*
		LIST OF PUBLIC MEMBER FUNCTIONS

		CONSTRUCTORS:

									Triangle(TEdge *E0, TEdge *E1, TEdge *E2, Vertex *V0,
									Vertex *V1, Vertex *V2)
									Triangle(Vertex *V0, Vertex *V1, Vertex *V2)

		GETTER:

		unsigned long long 			getID() const
		Vertex* 					getVertex(const int index) const
		TEdge* 						getEdge(const int index) const
		Vertex* 					getOtherVertex(TEdge * const e) const
		TEdge* 						getEdgeNotContaining(Vertex const * const v) const
		TEdge* 						getEdgeContaining(Vertex const * const v) const
		TEdge* 						getOtherEdgeContaining(Vertex const * const v, TEdge const *
									const e) const
		TEdge* 						getEdgeContaining(Vertex const * const v0, Vertex const *
									const v1) const
		std::vector<TEdge*> 		getOtherEdges(TEdge * const e) const
		TEdge* 						getLongestEdge(const double epsilon) const
		TEdge* 						getLongestEdgeAlt() const
		double 						getRange(Vertex const * const v, const double alpha) const
		TEdge* 						getNotIntersectedEdge() const
		double 						getWeight() const

		SETTER:

		void 						setSTEntry(STEntry<Triangle*> *e)

		PRINTER:

		void 						print() const

		OTHERS:

		bool 						contains(Vertex const * const v) const
		bool 						contains(TEdge const * const e) const
		void 						enqueue()
		void 						dequeue()
		bool 						isEnqueued() const
		bool 						isInternal() const
		double 						calculateCollapseTime(Vertex * const moving, const double dx,
									const double dy) const
		double 						signedArea() const
		bool 						inside(Vertex *v) const
		void 						updateSTEntry() const


		LIST OF STATIC FUNCTIONS

		static bool  				insideTriangle(Vertex *v0, Vertex *v1, Vertex *v2, Vertex *v)
		static unsigned long long 	getNumberOfExistingTriangles() const
	*/


	/*
		C ~ O ~ N ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R ~ S
	*/

	/*
		Constructor:
		Builds a triangle by their vertices and edges. It automatically registers the new triangle
		at their vertices and edges and sets the property enqueued to false. Additionally it checks
		whether a triangle built of the same edges already exists and in case errors with exit
		code 5.

		@param 	E0 		The first edge of the triangle
		@param 	E1 		The second edge of the triangle
		@param 	E2 		The third edge of the triangle
		@param 	V0 		The first vertex of the triangle
		@param 	V1 		The second vertex of the triangle
		@param 	V2 		The third vertex of the triangle
		@param 	intern 	Indicates whether the triangle is internal or external to the polygon.
						Triangles in the interior of a hole a counted as external.

		Note:
			- The order of the edges is not important if relative to the order of the vertices
			- The order of the vertices is just important if you want to check the orientation
				of the triangle
	*/
	Triangle(TEdge *E0, TEdge *E1, TEdge *E2, Vertex *V0, Vertex *V1, Vertex *V2, bool intern);

	/*
		Constructor:
		This constructor generates triangles just for orientation tests, i.e. these triangles
		are not fully generated as they have not assigned any edges and are not assigned to any
		edges. So the triangles are basically just their three vertices and can be used for
		computing the signed area of the vertex group only.

		@param 	V0 	The first vertex of the triangle
		@param 	V1 	The second vertex of the triangle
		@param 	V2 	The third vertex of the triangle

		Note:
			The order of the vertices is important to check the orientation. It is always the
			ordering v0 -> v1 -> v2 considered.
	*/
	Triangle(Vertex *V0, Vertex *V1, Vertex *V2);

	/*
		G ~ E ~ T ~ T ~ E ~ R ~ S
	*/

	/*
		@return 	The ID of the vertex
	*/
	unsigned long long getID() const;

	/*
		The function getVertex() returns the vertex which is assigned to the position index which
		depends just on the ordering the vertices were given to the constructor. If the index is
		not 0, 1 or 2 it returns NULL.

		@param 	index 	The index of the vertex you want
		@return 		The vertex you want if the index is reasonable, otherwise NULL

		Note:
			This function is basically just useful if you want to enumerate all vertices of the
			triangle.
	*/
	Vertex *getVertex(const int index) const;

	/*
		The function getEdge() returns the edge which is assigned to the position index which
		depends just on the ordering the edges were given to the constructor. If the index is
		not 0, 1 or 2 it returns NULL.

		@param 	index 	The index of the edge you want
		@return 		The edge you want if the index is reasonable, otherwise NULL

		Note:
			This function is basically just useful if you want to enumerate all edges of the
			triangle.
	*/
	TEdge *getEdge(const int index) const;

	/*
		@param 	e 	One of the triangle's edges
		@return 	The vertex which is contained by the triangle, but not by the edge e
	*/
	Vertex *getOtherVertex(TEdge * const e) const;

	/*
		@param 	v 	One of the triangle's vertices
		@return 	The edge of the triangle which does not contain v

		Note:
			If all edges contain v, the function returns NULL, but then anything is wrong anyway
	*/
	TEdge *getEdgeNotContaining(Vertex const * const v) const;

	/*
		The function getEdgeContaining() gets one of the two edges of the triangle which contains
		the vertex v (if the vertex v is part of the triangle, otherwise it returns NULL). Which
		of the two edges you get just depends on the ordering of the edges given to the constructor.

		@param 	v 	The vertex which should be contained by the edge
		@return 	Any edge contained by the triangle which contains v (NULL of v is not part of
					the triangle)
	*/
	TEdge *getEdgeContaining(Vertex const * const v)const;

	/*
		The function getOtherEdgeContaining() gets you the edge of the triangle which is not e
		but contains the vertex v.

		@param 	v 	The vertex the edge should contain
		@param 	e 	The edge you do not want
		@return 	The edge you search for or NULL if such an edge does not exist (then something
					is wrong)
	*/
	TEdge *getOtherEdgeContaining(Vertex const * const v, TEdge const * const e) const;

	/*
		The function getEdgeContaining() gets you the edge of the triangle which contains both
		given vertices.

		@param 	v0 	One of the vertices the edge must contain
		@param 	v1 	The other vertex the edge must contain
		@return 	The edge which contains both vertices or NULL if no such edge exists
	*/
	TEdge *getEdgeContaining(Vertex const * const v0, Vertex const * const v1) const;

	/*
		@param 	e 	One edge which should be part of the triangle
		@return 	A vector of all other edges part of the triangle
	*/
	std::vector<TEdge*> getOtherEdges(TEdge * const e) const;

	/*
		The function getLongestEdge() computes the euclidean length of all edges and returns
		the longest one. If the longest one is a polygon edge it checks whether there is another
		edge which is less then epsilon shorter then the longest edge and returns this if existing.

		@param 	epsilon 	The length difference which is considered the be equal
		@return 			The longest edge

		Note:
			As the computation of the euclidean distance requires two multiplication and a square
			root this function could be very faulty, therefore it is by default replaced by
			getLongestEdgeAlt() 
	*/
	TEdge *getLongestEdge(const double epsilon) const;

	/*
		The function getLongestEdgeAlt() finds the longest edge of the triangle not by computing
		any length, but by comparing the coordinates of the vertices. Therefore it is assumed that
		all vertices lay very close to one line, then it is possible to get the middle vertex by
		comparing the coordinates. The longest edge is the edge which does not contain the middle
		vertex. If something should go wrong there the function calls the function getLongestEdge()
		as replacement.
		
		@return 	The longest edge of the triangle

		Note:
			This function works just if all vertices lay very close to one line which should be the
			case at each flip event!
	*/
	TEdge *getLongestEdgeAlt() const;

	/*
		The function getRange() checks whether the triangle lays in direction alpha seen from
		vertex v. If it does so, it returns the mean length of the two edges of the triangle
		incident to v, otherwise it returns -1. The result can be used as estimate for the
		distance of the vertex v to the opposite edge of the triangle.

		@param 	v 		The vertex which will move
		@param 	alpha 	The direction in which v will move (-pi < alpha <= pi)
		@return 		Estimate for the distance v can move in direction alpha till it crashes
						into an edge
	*/
	double getRange(Vertex const * const v, const double alpha) const;

	/*
		@return 	The first edge which has its intersected flag not set
	*/
	TEdge *getNotIntersectedEdge() const;

	/*
		@return 	A weight for the triangle, implemented as (1 + k) ^ 2
					where k is the number of its polygon edges 
	*/
	double getWeight() const;


	/*
		S ~ E ~ T ~ T ~ E ~ R ~ S
	*/

	/*
		Adds the selection tree entry of the triangle.

		@param 	e 	The selection tree entry
	*/
	void setSTEntry(STEntry<Triangle*> *e);


	/*
		P ~ R ~ I ~ N ~ T ~ E ~ R
	*/

	/*
		The function print() prints all three vertices of the triangle
	*/
	void print() const;

	
	/*
		O ~ T ~ H ~ E ~ R ~ S
	*/

	/*
		The function contains() checks whether the triangle contains the vertex v.

		@param 	v 	The vertex of interest
		@return 	True if the triangle contains v, otherwise false
	*/
	bool contains(Vertex const * const v) const;

	/*
		The function contains() checks whether the triangle contains the edge e.

		@param 	e 	The edge of interest
		@return 	True if the triangle contains e, otherwise false
	*/
	bool contains(TEdge const * const e) const;

	/*
		The function enqueue() sets the triangle's enqueued flag.
	*/
	void enqueue();

	/*
		The function dequeue() unsets the triangle's enqueued flag.
	*/
	void dequeue();

	/*
		@return 	True if the triangle is enqueued in the event queue at the moment, otherwise
					false
	*/
	bool isEnqueued() const;

	/*
		@return 	False it the triangle is external to the polygon or internal to a hole,
					otherwise true
	*/
	bool isInternal() const;

	/*
		The function calculateCollapseTime() computes at which time of a translation of the vertex
		moving in direction (dx, dy) this triangle will collapse, i.e. its area gets zero. The time
		is the start time of the translation, the time 1 is the time at which the vertex is
		translated exactly by (dx, dy). The time is calculated by the quotient of the triangle
		areas of the two triangles consisting of the opposite edge of this triangle and the start
		and end position of the moving vertex. For details take a look at my Master Thesis.

		@param 	moving 	The vertex to be translated
		@param 	dx 		The x-component of the translation vector
		@param 	dy 		The y-component of the translation vector
		@return 		The time when the triangles area gets zero during the translation

		Note:
			This function also can give you collapse times not between 0 and 1! This means that
			the triangle will not become zero during this translation.
	*/
	double calculateCollapseTime(Vertex * const moving, const double dx, const double dy) const;

	/*
		The function signedArea() computes an estimate of the signed area of the triangle.
		Depending on Settings::arithmetics it uses Shewchuck's predicates or simple floating-
		point arithmetics to do so.

		Note:
			In fact, it computes two times the signed area of the triangle.
	*/
	double signedArea() const;

	/*
		Checks whether the vertex v is in the interior of the triangle.

		@param 	v 	The vertex of interest
		@return 	True if v is in the interior of the triangle, otherwise false
	*/
	bool inside(Vertex *v) const;

	/*
		Updates the selection tree entry of the triangle.
	*/
	void updateSTEntry() const;
	
	
	/*
		D ~ E ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R
	*/

	/*
		Destructor:
		Removes also the dangling pointers from the vertices' an edges' triangles' lists.
	*/
	~Triangle();


	/*
		S ~ T ~ A ~ T ~ I ~ C   F ~ U ~ N ~ C ~ T ~ I ~ O ~ N ~ S
	*/

	/*
		@return 	The recent number of existing instances of triangle
	*/
	static unsigned long long getNumberOfExistingTriangles();

	/*
		The function insideTriangle() checks whether the vertex v is inside the triangle
		formed by the vertices v0, v1 and v2.

		@param	v0 			First vertex of the triangle
		@param 	v1 			Second vertex of the triangle
		@param 	v2 			Third vertex of the triangle
		@param 	v 			The vertex for which should be checked whether it lies inside the
							triangle or not
		@return 			True if v lies inside the triangle, otherwise false
	*/
	static bool insideTriangle(Vertex  *v0, Vertex *v1, Vertex *v2, Vertex *v);
};
	
#endif
