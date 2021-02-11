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

#include "tpolygon.h"


/*
	S ~ T ~ A ~ T ~ I ~ C 	V ~ A ~ R ~ I ~ A ~ B ~ L ~ E ~ S
*/

/*
	The number of already generated polygons
*/
unsigned int TPolygon::N = 0;


/*
	C ~ O ~ N ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R ~ S
*/

/*
	Constructor:
	Already allocates memory for all vertices which will be inserted in the future.

	@param 	t 	The triangulation the new polygon lives in
	@param 	n 	The target number of vertices of the new polygon
*/
TPolygon::TPolygon(Triangulation const * const t, const int n) :
	T(t), tree(new SelectionTree<TEdge*>(true)), id(N) {

	vertices.reserve(n);

	N++;
}


/*
	S ~ E ~ T ~ T ~ E ~ R ~ S
*/

/*
	@param	v 	Vertex to be added to the vertices vector
*/
void TPolygon::addVertex(Vertex * const v){
	vertices.push_back(v);

	(*v).setPolygon(this);
}

/*
	@param	e 	Edge to be added to the SelectionTree
*/
void TPolygon::addEdge(TEdge * const e){
	(*tree).insert(e);
}


/*
	G ~ E ~ T ~ T ~ E ~ R ~ S
*/

/*
	@return 	The id of the polygon
*/
unsigned int TPolygon::getID() const{
	return id;
}

/*
	@param	i 	Index of the vertex in the vertices vector
	@return 	The vertex at index i in the vertices vector

	Note:
		- Be n the actual number of vertices in the vertex vector, then i < 0 
			returns the vertex with index n + i and i >= n returns the vertex at index
			i - n. This is helpful to get the previous and next vertex while generating
			the initial polygon.
		- This will not work after inserting additional vertices, as the vertices won't be 
			in the same order in the vertices vector as they are in the polygon
*/
Vertex *TPolygon::getVertex(const int i) const{ 	
	int n;

	n = vertices.size();

	return vertices[i % n];
}

/*
	@return 	The actual size of the polygon
*/
int TPolygon::getActualPolygonSize() const{
	return vertices.size();
}

/*
	@return 	Any edge of the polygon selected uniformly at random by its length
*/
TEdge *TPolygon::getRandomEdgeWeighted() const{
	return (*tree).getRandomObject();
}


/*
	R ~ E ~ M ~ O ~ V ~ E ~ R
*/

/*
	@param 	i 	The index of the vertex to be removed in the vertices list
	@return 	The vertex to be removed
*/
Vertex *TPolygon::removeVertex(const int i){
	Vertex *v;

	v = vertices[i];

	vertices.erase(vertices.begin() + i);

	return v;
}


/*
	O ~ T ~ H ~ E ~ R ~ S
*/

/*
	Write this polygon to os in line format.
	@param  os      Output stream
*/
void TPolygon::writeToLine(std::ostream& os) const {
	unsigned cnt = 0;
	unsigned num_v = vertices.size() + 1;
	os << num_v << std::endl;
	const Vertex * const start = getVertex(0);
	const Vertex * other = start;

	do {
		os << other->getX() << " " << other->getY() << std::endl;
		++cnt;
		other = other->getNext();
	} while (other != start);

	os << other->getX() << " " << other->getY() << std::endl;
	++cnt;
	assert(cnt == num_v);
	os << std::endl;
}

/*
	Check the correctness of the SelectionTree
*/
void TPolygon::checkST() const{
	fprintf(stderr, "Start checking the SelectionTree of polygon %d\n", id);

	(*tree).check();

	fprintf(stderr, "SelectionTree successfully checked\n");
}
