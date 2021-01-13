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

#include "vertex.h"

/*
	S ~ T ~ A ~ T ~ I ~ C 	V ~ A ~ R ~ I ~ A ~ B ~ L ~ E ~ S
*/

/*
	The number of already generated vertices
*/
unsigned long long Vertex::n = 0;

/*
	The number of deleted vertices
*/
unsigned long long Vertex::deleted = 0;


/*
	P ~ R ~ I ~ V ~ A ~ T ~ E 	M ~ E ~ M ~ B ~ E ~ R 	F ~ U ~ N ~ C ~ T ~ I ~ O ~ N ~ S
*/

/*
	The function getEnvironment() recursively inserts all adjacent edges and vertices into
	maps. As key the IDs are used. This function is used for debug outputs.

	@param 	es 		The map of edges
	@param 	vs 		The map of vertices
	@param 	depth 	The number of recursive steps to be done, basically a depth of n means
					that all vertices with distance less then an to this vertex are included
*/
void Vertex::getEnvironment(std::map<const unsigned long long, const TEdge*> &es,
	std::map<const unsigned long long, const Vertex*> &vs, const int depth) const{
	
	Vertex *v;

	vs.insert(std::make_pair(id, this));

	if(depth > 0){
		for(auto const& i : edges){
			es.insert(std::make_pair((*i).getID(), i));
			v = (*i).getOtherVertex(this);
			(*v).getEnvironment(es, vs, depth - 1);
		}
	}
	
}


/*
	C ~ O ~ N ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R ~ S
*/

/*
	Constructor:
	Sets the coordinates and the ID of the new vertex. Also it sets the property rectangleVertex
	to false, i.e. the new vertex is not part of the bounding box.

	@param 	X 	The x coordinate of the vertex
	@param 	Y 	The y coordinate of the vertex
*/
Vertex::Vertex(const double X, const double Y) :
	T(NULL), P(NULL), x(X), y(Y), toPrev(NULL), toNext(NULL), rectangleVertex(false), id(n) {

	n++;
}

/*
	Constructor:
	Sets the coordinates and the ID of the new vertex.
	The property rectangleVertex can be directly set here with the parameter RV.

	@param 	X 	The x coordinate of the vertex
	@param 	Y 	The y coordinate of the vertex
	@param 	RV 	Determines whether the new vertex is part of the bounding box
*/
Vertex::Vertex(const double X, const double Y, const bool RV) :
	T(NULL), P(NULL), x(X), y(Y), toPrev(NULL), toNext(NULL), rectangleVertex(RV), id(n) {

	n++;
}

/*
	The function getTranslated() is an implicit constructor which generates a new vertex
	out of a already existing vertex and a translation vector. The position of the new
	vertex is the position of the old vertex plus the translation vector.
	The property rectangleVertex is set to false.

	@param 	dx 	X-component of the translation vector
	@param 	dy 	Y-component of the translation vector
	@return		The translated vertex
*/
Vertex *Vertex::getTranslated(const double dx, const double dy) const{
	Vertex *v = new Vertex(x + dx, y + dy);
	
	return v;
}


/*
	S ~ E ~ T ~ T ~ E ~ R ~ S
*/

/*
	@param 	t 	The triangulation the vertex lives in
*/
void Vertex::setTriangulation(Triangulation * const t){
	T = t;
}

/*
	@param 	p 	The polygon the vertex belongs to
*/
void Vertex::setPolygon(TPolygon *p){
	P = p;
}

/*
	@param 	X 	The new x-coordinate of the vertex
	@param 	Y 	The new y-coordinate of the vertex
*/
void Vertex::setPosition(const double X, const double Y){
	x = X;
	y = Y;
}

/*
	Adds the edge e to the edges list of the vertex.

	@param 	e 	The edge to be added to the edges list

	Note:
		- Each edge contained in the vertex's edges list should contain the vertex!
		- While constructing an edge this function is automatically called to add the
			edge to the edges lists of its vertices
*/
void Vertex::addEdge(TEdge * const e){
	edges.push_back(e);
}

/*
	Adds the triangle t to the triangles list of the vertex.

	@param 	t 	The triangle to be added to the triangles list

	Note:
		- Each triangle contained in the vertex's triangles list should contain the vertex!
		- While constructing a triangle this function is automatically called to add the
			triangle to the triangles lists of its vertices
*/
void Vertex::addTriangle(Triangle * const t){
	triangles.push_back(t);
}

/*
	The function setToPrev() sets the pointer for the polygon edge which connects the vertex
	to its predecessor in the polygon.

	@param 	e 	The edge to the predecessor of the vertex in the polygon

	Note:
		This function is automatically called when a new polygon edge is constructed.
*/
void Vertex::setToPrev(TEdge * const e){
	toPrev = e;
}

/*
	The function setToNext() sets the pointer for the polygon edge which connects the vertex
	to its successor in the polygon.

	@param 	e 	The edge to the successor of the vertex in the polygon

	Note:
		This function is automatically called when a new polygon edge is constructed.
*/
void Vertex::setToNext(TEdge * const e){
	toNext = e;
}


/*
	G ~ E ~ T ~ T ~ E ~ R ~ S
*/

/*
	@return 	The x-coordinate of the vertex's position
*/
double Vertex::getX() const{
	return x;
}

/*
	@return 	The y-coordinate of the vertex's position
*/
double Vertex::getY() const{
	return y;
}

/*
	@return 	The triangles list of the vertex
*/
std::list<Triangle*> Vertex::getTriangles() const{ 
	return triangles;
}

/*
	@return 	The ID of the vertex
*/
unsigned long long Vertex::getID() const{
	return id;
}


/*
	The function getEdgeTo() searches the edges list for an edge to the vertex toV
	and returns it if existing. Otherwise it returns NULL.

	@param 	toV 	The target vertex of the searched edge
	@return 		The edge from the vertex to the vertex toV if existing, otherwise
					NULL
*/
TEdge *Vertex::getEdgeTo(Vertex * const toV) const{
	Vertex *v;

	for(auto const& i : edges){
		v = (*i).getOtherVertex(this);
		if(*v == *toV)
			return i;
	}

	return NULL;
}

/*
	The function getSurroundingEdges() returns a list of all surrounding edges of the
	vertex which form the surrounding polygon of the vertex. I.e. basically a list of
	all edges which are contained by a triangle containing the vertex, but do not
	contain the vertex itself.

	@return 	A list of all surrounding edges
*/
std::vector<TEdge*> Vertex::getSurroundingEdges() const{
	std::vector<TEdge*> out(triangles.size());
	int n = 0;

	for(auto const& i : triangles){
		out[n] = (*i).getEdgeNotContaining(this);
		n++;
	}

	return out;
}

/*
	@return 	A list of all (2) polygon edges incident to the vertex
*/
std::list<TEdge*> Vertex::getPolygonEdges() const{
	std::list<TEdge*> out;

	out.push_back(toPrev);
	out.push_back(toNext);

	return out;
}

/*
	@return 	A list of all edges incident to the vertex
*/
std::list<TEdge*> Vertex::getEdges() const{
	return edges;
}

/*
	@return 	True if the vertex is part of the bounding box, otherwise false
*/
bool Vertex::isRectangleVertex() const{
	return rectangleVertex;
}

/*
	The function getMediumEdgeLength() computes the mean of the lengths of all
	incident edges to the vertex. This can be used as estimate for appropriate 
	distribution parameters.

	@return 	The mean of the lengths of all incident edges
*/
double Vertex::getMediumEdgeLength() const{
	int n = edges.size();
	double sum = 0;

	for(auto const& i : edges){
		sum = sum + (*i).length();
	}

	return sum / n;
}

/*
	The function getDirectedEdgeLength() finds the triangle incident to the vertex
	in direction alpha (alpha = 0 corresponds to the positive x-direction) and
	computes the mean edge length of the edges of the triangle containing the
	vertex. This value can be used as estimate for appropriate distribution
	parameters.
	If the function is not able to find the right triangle it uses getMediumEdgeLength()
	instead the get an estimate.

	@param 	alpha 	Direction of the planed translation
	@return 		Mean length of the triangles edge in direction alpha

	Note:
		This gives a better estimate then getMediumEdgeLength(), but is more
		expensive to compute
*/
double Vertex::getDirectedEdgeLength(const double alpha) const{
	double length;

	for(auto const& i : triangles){
		// The function getRange() checks whether the triangle is in direction
		// alpha and returns the range if true, otherwise it returns -1
		length = (*i).getRange(this, alpha);

		if(length > 0){
			return length;
		}
	}
	
	if(Settings::feedback == FeedbackMode::VERBOSE)
		fprintf(stderr, "Was not able to find the triangle for vertex %llu in direction %f\n", id, alpha);
	
	return - getMediumEdgeLength();
}

/*
	@return 	The edge to the predecessor of the vertex in the polygon
*/
TEdge *Vertex::getToPrev() const{
	return toPrev;
}

/*
	@return 	The edge to the successor of the vertex in the polygon
*/
TEdge *Vertex::getToNext() const{
	return toNext;
}

/*
	@return 	 The predecessor of the vertex in the polygon
*/
Vertex *Vertex::getPrev() const{
	return (*toPrev).getOtherVertex(this);
}

/*
	@return 	The successor of the vertex in the polygon
*/
Vertex *Vertex::getNext() const{
	return (*toNext).getOtherVertex(this);
}

/*
	@return 	The triangulation the vertex lives in
*/
Triangulation *Vertex::getTriangulation() const{
	return T;
}

/*
	The function getTriangleWith() searches for a triangle in the triangles list
	which is formed by the vertex and the vertices v0 and v1. If such a triangle
	does not exist, it returns NULL

	@param 	v0 	Second vertex contained by the searched triangle
	@param 	v1 	Third vertex contained by the searched triangle
	@return 	The searched triangle if it exists, otherwise NULL
*/
Triangle *Vertex::getTriangleWith(Vertex const * const v0, Vertex const * const v1) const{

	for(auto const& i : triangles){
		if((*i).contains(v0) && (*i).contains(v1))
			return i;
	}

	return NULL;
}

/*
	@return 	The id of the polygon the vertex belongs to, -1 if it does not
				belong to a polygon.
*/
unsigned int Vertex::getPID() const{
	if(!rectangleVertex)
		return (*P).getID();
	else
		return -1;
}

/*
	@return 	The actual size of the polygon the vertex belongs to
*/
int Vertex::getActualPolygonSize() const{
	return (*P).getActualPolygonSize();
}

/*
	The function getInsideAngle() computes the angle in the interior of the polygon
	(in counter-clockwise orientation) between the to polygon edges incident to the
	vertex.

	@return 	The interior angle of the polygon at this vertex
*/
double Vertex::getInsideAngle() const{
	double alpha0, alpha1, angle;
	Vertex *prev = (*toPrev).getOtherVertex(this);
	Vertex *next = (*toNext).getOtherVertex(this);

	// The angles between the incident edges and the x-axis
	alpha0 = fabs((*toPrev).getAngle(this));
	alpha1 = fabs((*toNext).getAngle(this));

	// The computation of the inside angle depends on the configuration of the edges
	// First we have to check whether the edges are increasing or decreasing (directed
	// edges from the lower index in the polygon to the higher one)
	if(y - (*prev).getY() < 0){
		// First down, second down
		if((*next).getY() - y < 0){
			angle = alpha0 + alpha1;
		// First down, second up
		}else{
			if(alpha1 > alpha0){
				angle = 2 * M_PI - ( alpha1 - alpha0);
			}else{
				angle = alpha0 - alpha1;
			}
		}
	}else{
		// First up, second down
		if((*next).getY() - y < 0){
			if(alpha1 > alpha0){
				angle = alpha1 - alpha0;
			}else{
				angle = 2 * M_PI - (alpha0 - alpha1);
			}
		// First up, second up
		}else{
			angle = 2 * M_PI - alpha0 - alpha1;
		}
	}

	return angle;
}


/*
	R ~ E ~ M ~ O ~ V ~ E ~ R
*/

/*
	@param 	e 	Edge to be removed from the edges list
*/
void Vertex::removeEdge(TEdge * const e){
	edges.remove(e);
}

/*
	@param 	t 	Triangle to be removed from the triangles list
*/
void Vertex::removeTriangle(Triangle * const t){
	triangles.remove(t);
}


/*
	P ~ R ~ I ~ N ~ T ~ E ~ R
*/

/*
	The function write() writes the information of a vertex as node into a .graphml file.
	The node information contains the ID of the vertex, its coordinates and its ID as
	mainText. This functions also provides a scaling factor for the coordinates as some
	graphml-viewers are not capable of scaling. The setting of the scaling factor is
	trial-and-error.

	@param 	f 		The .graphml file to print in
	@param 	factor 	The used scaling factor

	Note:
		This function just prints one node into a .graphml file, to print the hole
		triangulation use the print functions of the Triangulation class
*/
void Vertex::write(FILE * const f, double factor) const{
	int n;
	int text;

	fprintf(f, "<node id=\"%llu\">"
	           "<data key=\"x\">%.16f</data>"
	           "<data key=\"y\">%.16f</data>"
	           "</node>\n", id, x * factor, y * factor);
}

/*
	The function writeToDat() writes the coordinates of a vertex to a .dat file such that it
	can be interpreted by gnuplot.

	@param 	f 	A handle for the .dat file
*/
void Vertex::writeToDat(FILE * const f) const{
	fprintf(f, "%f %f\n", x, y);
}

/*
	The function print() prints the basic information of a vertex to standard out.
	It prints the ID and both coordinates in a precision of 15 decimal digits.
*/
void Vertex::print() const{
	fprintf(stderr, "Vertex %llu at (%.15f, %.15f)\n", id, x, y);
}

/*
	The function writeEnvironment() writes a local part of the triangulation around
	the vertex to a -graphml file. It collects the data recursively by the function
	getEnvironment().

	@param 	depth 		The number of recursive steps to be done, basically a depth of n
						means that all vertices with distance less then n to this vertex
						are included
	@param 	filename 	The name of the file to write to
*/
void Vertex::writeEnvironment(const int depth, const char *filename) const{
	FILE *f;
	std::map<const unsigned long long, const TEdge*> es;
	std::map<const unsigned long long, const Vertex*> vs;
	TEdge const *e;
	Vertex const *v;

	f = fopen(filename, "w");

	// Collect the data of the environment
	getEnvironment(es, vs, depth);

	// Write a graphml header
	fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(f, "<graphml>\n");
	fprintf(f, "<graph id=\"Graph\" edgeDefault=\"undirected\">\n");

	// Write all nodes
	fprintf(f, "<nodes>\n");

	for(auto const& i : vs){
		v = i.second;
		(*v).write(f, 2000);
	}
	fprintf(f, "</nodes>\n");

	// Write all edges
	fprintf(f, "<edges>\n");
	for(auto const& i : es){
		e = i.second;
		(*e).write(f);
	}
	fprintf(f, "</edges>\n");

	fprintf(f, "</graph>\n");
	fprintf(f, "</graphml>\n");

	fclose(f);
}

/*
	The function writeSurroundingTriangulation() writes a vertex and all vertices and
	edges of its adjacent triangles into a .graphml file.

	@param 	filename 	The name of the .graphml file
*/
void Vertex::writeSurroundingTriangulation(const char *filename) const{
	FILE *f;
	std::map<const unsigned long long, const TEdge*> es;
	std::map<const unsigned long long, const Vertex*> vs;
	TEdge const *e;
	Vertex const *v;

	f = fopen(filename, "w");

	// Collect all entities
	vs.insert(std::make_pair(id, this));

	for(auto const& i : triangles){
		e = (*i).getEdge(0);
		es.insert(std::make_pair((*e).getID(), e));

		e = (*i).getEdge(1);
		es.insert(std::make_pair((*e).getID(), e));

		e = (*i).getEdge(2);
		es.insert(std::make_pair((*e).getID(), e));

		v = (*i).getVertex(0);
		vs.insert(std::make_pair((*v).getID(), v));

		v = (*i).getVertex(1);
		vs.insert(std::make_pair((*v).getID(), v));

		v = (*i).getVertex(2);
		vs.insert(std::make_pair((*v).getID(), v));
	}

	// Write graphml header
	fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(f, "<graphml>\n");
	fprintf(f, "<graph id=\"Graph\" edgeDefault=\"undirected\">\n");

	// Write nodes
	fprintf(f, "<nodes>\n");

	for(auto const& i : vs){
		v = i.second;
		(*v).write(f, 3000);
	}
	fprintf(f, "</nodes>\n");

	// Write edges
	fprintf(f, "<edges>\n");
	for(auto const& i : es){
		e = i.second;
		(*e).write(f);
	}
	fprintf(f, "</edges>\n");

	fprintf(f, "</graph>\n");
	fprintf(f, "</graphml>\n");

	fclose(f);
}

/*
	The function printStats() prints an overview of created, deleted and still existing
	vertices to stdout.
*/
void Vertex::printStats(){
	fprintf(stderr, "created: %llu deleted: %llu still existing: %llu \n", n, deleted, n - deleted);
}


/*
	O ~ T ~ H ~ E ~ R ~ S
*/

/*
	The function check() checks whether the vertex has exactly two polygon edges (except
	the vertex is a vertex of the bounding box) and whether toPrev and toNext are set
	correctly.

	@return 	True if everything is alright, otherwise false
*/
bool Vertex::check(){
	int n = 0;
	bool ok = true;

	if(!rectangleVertex){
		for(auto const& i : edges){
			if((*i).getEdgeType() == EdgeType::POLYGON) 
				n++;
		}

		if(n != 2){
			if(Settings::feedback == FeedbackMode::VERBOSE)
				fprintf(stderr, "Vertex %llu has %d polygon edges\n", id, n);
			ok = false;
		}

		if(toPrev == NULL){
			if(Settings::feedback == FeedbackMode::VERBOSE)
				fprintf(stderr, "Edge to previous vertex is missing for vertex %llu \n", id);
			ok = false;
		}

		if(toNext == NULL){
			if(Settings::feedback == FeedbackMode::VERBOSE)
				fprintf(stderr, "Edge to next vertex is missing for vertex %llu \n", id);
			ok = false;
		}
	}

	return ok;
}

/*
	The function stretch() scales the coordinates of a vertex by a fixed factor.

	@param 	factor 	The scaling factor

	Note:
		- The function should just be called from Triangulation::stretch() to stretch
			the whole triangulation, but not stretch a single vertex and expect the
			triangulation to stay correct
		- It is not checked that a simple polygon after stretching is still simple.
			In theory it has to, but for numerical reasons this might fail
*/
void Vertex::stretch(const double factor){
	x = factor * x;
	y = factor * y;
}

/*
	The function checkSurroundingPolygon() examines whether the vertex still is inside
	of its surrounding polygon. It errors with exit code 10 if a vertex lays exactly at
	a polygon edge.

	Definition surrounding polygon:
		Be T the set of triangles containing the vertex v. Be E the set of edges
		contained by the triangles of T. Be E' the subset of E which only contains
		those elements of E which do not contain v.
		Then all elements of E' together form a polygon around v.

		For a correct triangulation it holds that each vertex must be inside of its
		surrounding polygon.

	@return 	True if the vertex is inside of its surrounding polygon, otherwise false
*/
bool Vertex::checkSurroundingPolygon(){
	std::queue<Vertex*> Q;
	double area0, area;
	Vertex *first, *second;
	Triangle *t, *t0;
	TEdge *e;
    
    // Insert all vertices into the Q in the order they form the surrounding polygon
    // the start vertex is contained a second time at the end of the queue
	t = triangles.front();
	t0 = t;
	e = (*t).getEdgeContaining(this);
	first = (*e).getOtherVertex(this);
	Q.push(first);

	e = (*t).getOtherEdgeContaining(this, e);
	first = (*e).getOtherVertex(this);
	Q.push(first);

	t = (*e).getOtherTriangle(t);
	while((*t).getID() != (*t0).getID()){
		e = (*t).getOtherEdgeContaining(this, e);
		first = (*e).getOtherVertex(this);
		Q.push(first);

		t = (*e).getOtherTriangle(t);
	}

	// Compute the area of the first triangle outside of the loop to get the right sign
	first = Q.front();
	Q.pop();
	second = Q.front();
	Q.pop();

	t = new Triangle(first, second, this);
	area0 = (*t).signedArea();
	delete t;

	// If the vertex lies exactly at an edge, check whether the edge is a PE
	if(area0 == 0){
		t = getTriangleWith(first, second);
		e = (*t).getLongestEdgeAlt();

		if(Settings::feedback == FeedbackMode::VERBOSE)
			fprintf(stderr, "surrouding polygon check: area is exactly 0!\n");

		if((*e).getEdgeType() == EdgeType::POLYGON){
			fprintf(stderr, "surrouding polygon check: a vertex lies exactly on a polygon edge!\n");
			exit(10);
		}
	}

	while(!Q.empty()){
		first = second;
		second = Q.front();
		Q.pop();
        
		t = new Triangle(first, second, this);
		area = (*t).signedArea();
		delete t;

		// If the vertex lies exactly at an edge, check whether the edge is a PE
		if(area == 0){
			t = getTriangleWith(first, second);
			e = (*t).getLongestEdgeAlt();
				
			if(Settings::feedback == FeedbackMode::VERBOSE)
				fprintf(stderr, "surrouding polygon check: area is exactly 0!\n");

			if((*e).getEdgeType() == EdgeType::POLYGON){
				fprintf(stderr, "surrouding polygon check: a vertex lies exactly on a polygon edge!\n");
				exit(10);
			}else
				continue;
		}

		// Compare orientation with the orientation of the first triangle
		if(signbit(area) != signbit(area0)){
			// In the case that the first area was exactly zero we can take the next area as
			// first area
			if(area0 == 0){
				area0 = area;
				continue;
			}

			return false;
		}
	}

	return true;
}

/*
	@return 	The Euclidean distance to the origin
*/
double Vertex::getDistanceToOrigin() const{
	return sqrt(x * x + y * y);
}


/*
	D ~ E ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R
*/

/*
	Destructor:
	The Destructor deletes a vertex if it is not linked to any other entity. If it is
	still link, the Destructor throws a vertex deletion error.

	ATTENTIONE:
		- This code should only be used for dummy vertices!
		- If a vertex of the triangulation is deleted, you have to make sure that
			there is no pointer to the vertex in the triangulation's vertices vector left!
*/
Vertex::~Vertex(){
	int nEdges = edges.size();
	int nTriangles = triangles.size();

	if(nEdges != 0 || nTriangles != 0){
		fprintf(stderr, "Vertex deletion error: the deleted vertex was still part of an edge or triangle!\n");
		exit(8);
	}

	if(T != NULL){
		fprintf(stderr, "Vertex deletion error: the vertex is part of the triangulation!\n");
		exit(8);
	}

	deleted++;
}
