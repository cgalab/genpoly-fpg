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

#include "triangle.h"

/*
	S ~ T ~ A ~ T ~ I ~ C 	V ~ A ~ R ~ I ~ A ~ B ~ L ~ E ~ S
*/

/*
	The number of already generated triangles
*/
unsigned long long Triangle::n = 0;

/*
	Number of triangles existing at the moment
*/
unsigned long long Triangle::existing = 0; 


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
double Triangle::signedAreaExact() const{
	point p0, p1, p2;

	p0.x = (*v0).getX();
	p0.y = (*v0).getY();

	p1.x = (*v1).getX();
	p1.y = (*v1).getY();

	p2.x = (*v2).getX();
	p2.y = (*v2).getY();

	return orient2d(p0, p1, p2);
}

/*
	The function signedAreaDouble() computes an estimate of the signed area of the triangle
	with standard floating-point arithmetics. To provide stability it orders the vertices by
	reserveID before computing the determinant, such that the vertices in the determinant always
	have the same order independent of the ordering of the vertices in the triangle class. The
	correct sign therefore is ensured by the case distinctions here.

	@return 	The estimate for the signed area

	Note:
		- In fact this functions computes 2 times the signed area of the triangle.
*/
double Triangle::signedAreaDouble() const{

	if(*v0 < *v1 && *v0 < *v2){
		if(*v1 < *v2)
			return det(v0, v1, v2);
		else
			return - det(v0, v2, v1);
	}else if(*v1 < *v0 && *v1 < *v2){
		if(*v0 < *v2)
			return- det(v1, v0, v2);
		else
			return det(v1, v2, v0);
	}else{
		if(*v0 < *v1)
			return det(v2, v0, v1);
		else
			return - det(v2, v1, v0);
	}
}

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
double Triangle::det(Vertex * const V0, Vertex * const V1, Vertex * const V2) const{
	double area;
	double ax, ay, bx, by, cx, cy;

	ax = (*V0).getX();
	ay = (*V0).getY();

	bx = (*V1).getX() - ax;
	by = (*V1).getY() - ay;

	cx = (*V2).getX() - ax;
	cy = (*V2).getY() - ay;

	area = cy * bx - by * cx;

	return area;
}


/*
	C ~ O ~ N ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R ~ S
*/

/*
	Constructor:
	Builds a triangle by their vertices and edges. It automatically registers the new triangle
	at their vertices and edges and sets the property enqueued to false. Additionally it checks
	whether a triangle built of the same edges already exists and in case errors with exit code 5.

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
Triangle::Triangle(TEdge *E0, TEdge *E1, TEdge *E2, Vertex *V0, Vertex *V1, Vertex *V2,
	bool intern) : e0(E0), e1(E1), e2(E2), v0(V0), v1(V1), v2(V2), internal(intern),
	enqueued(false), entry(NULL), id(n) {

	Triangle *t;
	Triangulation *T;

	if(*v0 == *v1 || *v0 == *v2 || *v2 == *v1){
		fprintf(stderr, "Two vertices of the new triangle are identical!\n");
		exit(5);
	}

	(*e0).setTriangle(this);
	(*e1).setTriangle(this);
	(*e2).setTriangle(this);

	(*v0).addTriangle(this);
	(*v1).addTriangle(this);
	(*v2).addTriangle(this);

	// Check whether a triangle with the same edges already exists
	t = (*e0).getOtherTriangle(this);
	if(t != NULL && (*t).contains(e1) && (*t).contains(e2)){
		fprintf(stderr, "The same triangle already exists\n");
		exit(5);
	}

	// Add the triangle to the selection tree if it is internal
	if(internal && !Settings::holeInsertionAtStart){
		T = (*v0).getTriangulation();
		(*T).addInternalTriangle(this);
	}

	n++;

	existing++;
}

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
Triangle::Triangle(Vertex *V0, Vertex *V1, Vertex *V2) :
	e0(NULL), e1(NULL), e2(NULL), v0(V0), v1(V1), v2(V2), internal(false), enqueued(false),
	entry(NULL), id(n) {

	(*v0).addTriangle(this);
	(*v1).addTriangle(this);
	(*v2).addTriangle(this);

	n++;

	existing++;
}


/*
	G ~ E ~ T ~ T ~ E ~ R ~ S
*/

/*
	@return 	The ID of the vertex
*/
unsigned long long Triangle::getID() const{
	return id;
}

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
Vertex *Triangle::getVertex(const int index) const{
	if(index == 0)
		return v0;
	if(index == 1)
		return v1;
	if(index == 2)
		return v2;

	return NULL;
}

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
TEdge *Triangle::getEdge(const int index) const{
	if(index == 0)
		return e0;
	if(index == 1)
		return e1;
	if(index == 2)
		return e2;

	return NULL;
}

/*
	@param 	e 	One of the triangle's edges
	@return 	The vertex which is contained by the triangle, but not by the edge e
*/
Vertex *Triangle::getOtherVertex(TEdge * const e) const{
	if(!(*e).contains(v0))
		return v0;
	else if(!(*e).contains(v1))
		return v1;
	else
		return v2;
}

/*
	@param 	v 	One of the triangle's vertices
	@return 	The edge of the triangle which does not contain v

	Note:
		If all edges contain v, the function returns NULL, but then anything is wrong anyway
*/
TEdge *Triangle::getEdgeNotContaining(Vertex const * const v) const{
	if(!(*e0).contains(v)) return e0;
	if(!(*e1).contains(v)) return e1;
	if(!(*e2).contains(v)) return e2;

	fprintf(stderr, "error: all edges are containing the vertex %llu \n", (*v).getID());
	return NULL;
}

/*
	The function getEdgeContaining() gets one of the two edges of the triangle which contains
	the vertex v (if the vertex v is part of the triangle, otherwise it returns NULL). Which
	of the two edges you get just depends on the ordering of the edges given to the constructor.

	@param 	v 	The vertex which should be contained by the edge
	@return 	Any edge contained by the triangle which contains v (NULL of v is not part of
				the triangle)
*/
TEdge *Triangle::getEdgeContaining(Vertex const * const v) const{
	if((*e0).contains(v)) return e0;
	if((*e1).contains(v)) return e1;
	if((*e2).contains(v)) return e2;

	fprintf(stderr, "error: none of the edges is containing the vertex %llu \n", (*v).getID());
	return NULL;
}


/*
	The function getOtherEdgeContaining() gets you the edge of the triangle which is not e
	but contains the vertex v.

	@param 	v 	The vertex the edge should contain
	@param 	e 	The edge you do not want
	@return 	The edge you search for or NULL if such an edge does not exist (then something
				is wrong)
*/
TEdge *Triangle::getOtherEdgeContaining(Vertex const * const v, TEdge const * const e) const{
	if(*e != *e0 && (*e0).contains(v)) return e0;
	if(*e != *e1 && (*e1).contains(v)) return e1;
	if(*e != *e2 && (*e2).contains(v)) return e2;

	(*e0).print();
	(*e1).print();
	(*e2).print();
	fprintf(stderr, "error: no other edge is containing the vertex %llu \n", (*v).getID());
	return NULL;
}

/*
	The function getEdgeContaining() gets you the edge of the triangle which contains both
	given vertices.

	@param 	v0 	One of the vertices the edge must contain
	@param 	v1 	The other vertex the edge must contain
	@return 	The edge which contains both vertices or NULL if no such edge exists
*/
TEdge *Triangle::getEdgeContaining(Vertex const * const v0, Vertex const * const v1) const{
	if((*e0).contains(v0) && (*e0).contains(v1)) return e0;
	if((*e1).contains(v0) && (*e1).contains(v1)) return e1;
	if((*e2).contains(v0) && (*e2).contains(v1)) return e2;

	return NULL;
}


/*
	@param 	e 	One edge which should be part of the triangle
	@return 	A vector of all other edges part of the triangle
*/
std::vector<TEdge*> Triangle::getOtherEdges(TEdge * const e) const{
	std::vector<TEdge*> out(2);
	int index = 0;

	if(*e != *e0){
		out[index] = e0;
		index++;
	}
	if(*e != *e1){
		out[index] = e1;
		index++;
	}
	if(*e != *e2){
		out[index] = e2;
		index++;
	}

	return out;
}

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
TEdge *Triangle::getLongestEdge(const double epsilon) const{
	double l0 = (*e0).length();
	double l1 = (*e1).length();
	double l2 = (*e2).length();
	double longest, second;
	TEdge *longestE, *secondE;

	if(l0 >= l1 && l0 >= l2){
		longest = l0;
		longestE = e0;

		if(l1 >= l2){
			second = l1;
			secondE = e1;
		}else{
			second = l2;
			secondE = e2;
		}
	}else if(l1 >= l2){
		longest = l1;
		longestE = e1;

		if(l0 >= l2){
			second = l0;
			secondE = e0;
		}else{
			second = l2;
			secondE = e2;
		}
	}else{
		longest = l2;
		longestE = e2;

		if(l0 >= l1){
			second = l0;
			secondE = e0;
		}else{
			second = l1;
			secondE = e1;
		}
	}

	// Check whether the longest edge is a polygon edge and replace it if possible
	if((*longestE).getEdgeType() == EdgeType::POLYGON){
		if(longest - epsilon <= second)
			longestE = secondE;
	}

	return longestE;
}

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
TEdge *Triangle::getLongestEdgeAlt() const{
	Vertex *v;

	v = getOtherVertex(e0);
	if((*e0).isBetween(v))
		return e0;

	v = getOtherVertex(e1);
	if((*e1).isBetween(v))
		return e1;

	v = getOtherVertex(e2);
	if((*e2).isBetween(v))
		return e2;

	fprintf(stderr, "was not able to detect a longest edge by comparison\n");
	fprintf(stderr, "triangle area: %.16f \n", signedArea());
	print();
	(*v0).print();
	(*v1).print();
	(*v2).print();

	// Questionable whether this is really helpful
	return getLongestEdge(0.0001);
}

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
double Triangle::getRange(Vertex const * const v, const double alpha) const{
	TEdge *e, *f, *g;
	double alpha1, alpha2, l;

	if(!(*e0).contains(v)){
		e = e1;
		f = e2;
	}else if(!(*e1).contains(v)){
		e = e0;
		f = e2;
	}else{
		e = e0;
		f = e1;
	}

	alpha1 = (*e).getAngle(v);
	alpha2 = (*f).getAngle(v);

	// Order alpha1 and alpha2
	if(alpha1 < alpha2){
		l = alpha1;
		alpha1 = alpha2;
		alpha2 = l;

		g = e;
		e = f;
		f = g;
	}

	l = ((*e).length() + (*f).length()) / 2;

	// Assume: alpha1 >= alpha2
	if(alpha1 - alpha2 <= M_PI){
		if(alpha <= alpha1 && alpha >= alpha2)
			return l;
	}else{
		if(alpha >= alpha1 || alpha <= alpha2)
			return l;
	}

	// Alpha is not inside this triangle
	return -1;
}

/*
	@return 	The first edge which has its intersected flag not set
*/
TEdge *Triangle::getNotIntersectedEdge() const{
	if(!(*e0).isIntersected()) return e0;
	if(!(*e1).isIntersected()) return e1;
	if(!(*e2).isIntersected()) return e2;

	return NULL;
}

/*
	@return 	A weight for the triangle, implemented as (1 + k) ^ 2
				where k is the number of its polygon edges 
*/
double Triangle::getWeight() const{
	int n = 1;
	unsigned long long pid;
	Vertex *v;

	if((*e0).getEdgeType() == EdgeType::POLYGON){
		v = (*e0).getV0();
		pid = (*v).getPID();
		if(pid == 0)
			n++;

	}

	if((*e1).getEdgeType() == EdgeType::POLYGON){
		v = (*e1).getV0();
		pid = (*v).getPID();
		if(pid == 0)
			n++;

	}

	if((*e2).getEdgeType() == EdgeType::POLYGON){
		v = (*e2).getV0();
		pid = (*v).getPID();
		if(pid == 0)
			n++;

	}

	n = n * n;
	
	return (double)n;
}


/*
	S ~ E ~ T ~ T ~ E ~ R ~ S
*/

/*
	Adds the selection tree entry of the triangle.

	@param 	e 	The selection tree entry
*/
void Triangle::setSTEntry(STEntry<Triangle*> *e){
	entry = e;
}


/*
	P ~ R ~ I ~ N ~ T ~ E ~ R
*/

/*
	The function print() prints all three vertices of the triangle
*/
void Triangle::print() const{
	fprintf(stderr, "Triangle %llu:\n", id);
	(*v0).print();
	(*v1).print();
	(*v2).print();
}


/*
	O ~ T ~ H ~ E ~ R ~ S
*/

/*
	The function contains() checks whether the triangle contains the vertex v.

	@param 	v 	The vertex of interest
	@return 	True if the triangle contains v, otherwise false
*/
bool Triangle::contains(Vertex const * const v) const{

	if(*v0 == *v) return true;
	if(*v1 == *v) return true;
	if(*v2 == *v) return true;
	return false;
}

/*
	The function contains() checks whether the triangle contains the edge e.

	@param 	e 	The edge of interest
	@return 	True if the triangle contains e, otherwise false
*/
bool Triangle::contains(TEdge const * const e) const{

	if(*e0 == *e) return true;
	if(*e1 == *e) return true;
	if(*e2 == *e) return true;
	return false;
}

/*
	The function enqueue() sets the triangle's enqueued flag.
*/
void Triangle::enqueue(){
	enqueued = true;
}

/*
	The function dequeue() unsets the triangle's enqueued flag.
*/
void Triangle::dequeue(){
	enqueued = false;
}

/*
	@return 	True if the triangle is enqueued in the event queue at the moment, otherwise
				false
*/
bool Triangle::isEnqueued() const{
	return enqueued;
}

/*
	@return 	False it the triangle is external to the polygon or internal to a hole,
				otherwise true
*/
bool Triangle::isInternal() const{
	return internal;
}

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
double Triangle::calculateCollapseTime(Vertex * const moving, const double dx, const double dy) const{
	double ax, ay, bx, by, cx, cy, ex, ey;
	double areaOld, areaNew, portion;

	if(!contains(moving))
		return -1;

	cx = (*moving).getX();
	cy = (*moving).getY();

	if(*moving == *v0){
		ax = (*v1).getX();
		ay = (*v1).getY();
		bx = (*v2).getX();
		by = (*v2).getY();
	}else if(*moving == *v1){
		ax = (*v0).getX();
		ay = (*v0).getY();
		bx = (*v2).getX();
		by = (*v2).getY();
	}else{
		ax = (*v0).getX();
		ay = (*v0).getY();
		bx = (*v1).getX();
		by = (*v1).getY();
	}

	// Shift a to zero
	bx = bx - ax;
	by = by - ay;

	cx = cx - ax;
	cy = cy - ay;

	ex = cx + dx;
	ey = cy + dy;

	areaOld = cx * by - cy * bx;
	areaNew = bx * ey - by * ex;

	portion = areaNew / areaOld;

	return 1 / (portion + 1);
}

/*
	The function signedArea() computes an estimate of the signed area of the triangle.
	Depending on Settings::arithmetics it uses Shewchuck's predicates or simple floating-
	point arithmetics to do so.

	Note:
		In fact, it computes two times the signed area of the triangle.
*/
double Triangle::signedArea() const{
	if(Settings::arithmetics == Arithmetics::EXACT)
		return signedAreaExact();
	else
		return signedAreaDouble();
}

/*
	Checks whether the vertex v is in the interior of the triangle.

	@param 	v 	The vertex of interest
	@return 	True if v is in the interior of the triangle, otherwise false
*/
bool Triangle::inside(Vertex *v) const{
	double area0, area1;
	Triangle *t;

	t = new Triangle(v0, v1, v);
	area0 = (*t).signedArea();
	delete t;

	t = new Triangle(v1, v2, v);
	area1 = (*t).signedArea();
	delete t;

	if(signbit(area0) != signbit(area1))
		return false;

	t = new Triangle(v2, v0, v);
	area1 = (*t).signedArea();
	delete t;

	if(signbit(area0) != signbit(area1))
		return false;

	return true;
}

/*
	Updates the selection tree entry of the triangle.
*/
void Triangle::updateSTEntry() const{
	if(entry != NULL)
		(*entry).update();
}


/*
	D ~ E ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R
*/

/*
	Destructor:
	Removes also the dangling pointers from the vertices' and edges' triangles' lists.
*/
Triangle::~Triangle(){

	(*v0).removeTriangle(this);
	(*v1).removeTriangle(this);
	(*v2).removeTriangle(this);

	// Some test triangles may not have assigned edges
	if(e0 != NULL)
		(*e0).removeTriangle(this);
	if(e0 != NULL)
		(*e1).removeTriangle(this);
	if(e0 != NULL)
		(*e2).removeTriangle(this);

	existing--;

	if(entry != NULL)
		(*entry).removeObject();
}


/*
	S ~ T ~ A ~ T ~ I ~ C   F ~ U ~ N ~ C ~ T ~ I ~ O ~ N ~ S
*/

/*
	@return 	The recent number of existing instances of triangle
*/
unsigned long long Triangle::getNumberOfExistingTriangles(){
	return existing;
}

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
bool Triangle::insideTriangle(Vertex  *v0, Vertex *v1, Vertex *v2, Vertex *v){
	double area0, area1;
	Triangle *t;

	t = new Triangle(v0, v1, v);
	area0 = (*t).signedArea();
	delete t;

	t = new Triangle(v1, v2, v);
	area1 = (*t).signedArea();
	delete t;

	if(signbit(area0) != signbit(area1))
		return false;

	t = new Triangle(v2, v0, v);
	area1 = (*t).signedArea();
	delete t;

	if(signbit(area0) != signbit(area1))
		return false;

	return true;
}
