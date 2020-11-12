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

#include "insertion.h"

/*
	C ~ O ~ N ~ S ~ T ~ R ~ U ~ C ~ T ~ O ~ R ~ S
*/

/*
	Contructor:
	The contructor is given an vertex index. The insertion is done into the
	edge between the vertex and its successor in the polygon

	@param 	T 		The triangulation the polygon lives in
	@param 	pid 	The id of the polygon to insert in
	@param 	index 	The index of the vertex
*/
Insertion::Insertion(Triangulation * const t, const unsigned int pid, const int index) :
	T(t), pID(pid) {

	v0 = (*T).getVertex(index, pID);
	v1 = (*v0).getNext();
	e = (*v0).getToNext();
}

/*
	Contructor:
	This constructor directly takes the edge into which the new vertex will
	be inserted

	@param 	T 		The triangulation the polygon lives in
	@param 	pid 	The id of the polygon to insert in
	@param 	e 	 	The edge to insert in
*/
Insertion::Insertion(Triangulation * const t, const unsigned int pid, TEdge *edge) :
	T(t), pID(pid), e(edge){

	v0 = (*e).getV0();
	v1 = (*e).getV1();
}


/*
	O ~ T ~ H ~ E ~ R ~ S
*/

/*
	The function checkStability() checks whether the insertion can be done without
	risking numerical problems. Therefore the choosen edge must have at least the
	length Settings::minLength and, in case we use the double arithmetics, both
	adjacent triangles' determinants must be at least Settings::minDetInsertion.

	@return 	True if the chosen edge fulfils the criteria, otherwise false
*/
bool Insertion::checkStability() const{
	double l = (*e).length();
	Triangle *t;

	// Check the length of the edge
	if(l < Settings::minLength)
		return false;

	// Check the triangle areas
	if(Settings::arithmetics == Arithmetics::DOUBLE){
		t = (*e).getT0();
		if(fabs((*t).signedArea()) < Settings::minDetInsertion)
			return false;

		t = (*e).getT1();
		if(fabs((*t).signedArea()) < Settings::minDetInsertion)
			return false;
	}

	return true;
}

/*
	The function execute() inserts a vertex at the middle of the choosen edge.
	Therefore at first it deletes the choosen edge and its adjacent triangle,
	then it inserts the new vertex, adds edges to the four surrouding vertices
	and generate the new triangles for this quadrilateral.
*/
void Insertion::execute(){
	Triangle *t0, *t1;
	double x, y;
	Vertex *other0, *other1;
	TEdge *fromV0ToOther0, *fromV0ToOther1, *fromV1ToOther0, *fromV1ToOther1;
	TEdge *fromV0ToNew, *fromV1ToNew;
	TEdge *fromNewToOther0, *fromNewToOther1;

	// Calculate the coordinates of the new vertex
	x = (*v0).getX() + ((*v1).getX() - (*v0).getX()) / 2;
	y = (*v0).getY() + ((*v1).getY() - (*v0).getY()) / 2;

	// Generate the new vertex
	newV = new Vertex(x, y);
	(*T).addVertex(newV, pID);

	t0 = (*e).getT0();
	t1 = (*e).getT1();

	// Get the other two vertices of the concerned quadrilateral
	other0 = (*t0).getOtherVertex(e);
	other1 = (*t1).getOtherVertex(e);

	// Delete the edge and the old triangles with it
	delete e;

	// Get the four edges of the surrounding quadrilateral
	fromV0ToOther0 = (*v0).getEdgeTo(other0);
	fromV0ToOther1 = (*v0).getEdgeTo(other1);
	fromV1ToOther0 = (*v1).getEdgeTo(other0);
	fromV1ToOther1 = (*v1).getEdgeTo(other1);

	// Generate the edges between the new vertex and the other four vertices
	fromV0ToNew = new TEdge(v0, newV, EdgeType::POLYGON);
	fromV1ToNew = new TEdge(newV, v1, EdgeType::POLYGON);
	(*T).addEdge(fromV0ToNew, pID);
	(*T).addEdge(fromV1ToNew, pID);

	fromNewToOther0 = new TEdge(newV, other0);
	fromNewToOther1 = new TEdge(newV, other1);
	(*T).addEdge(fromNewToOther0, 0);
	(*T).addEdge(fromNewToOther1, 0);

	// Generate the new triangles
	new Triangle(fromV0ToNew, fromV0ToOther0, fromNewToOther0, v0, newV, other0);
	new Triangle(fromV0ToNew, fromV0ToOther1, fromNewToOther1, v0, newV, other1);
	new Triangle(fromV1ToNew, fromV1ToOther0, fromNewToOther0, v1, newV, other0);
	new Triangle(fromV1ToNew, fromV1ToOther1, fromNewToOther1, v1, newV, other1);
}

/*
	The function translate() tries to move the new vertex away from the edge.
	Therefore it tries up to Settings::insertionTries times to find a translation
	which leads to a simple polygon and execute it. If it has not found a suitable
	translation till the limit is reached, the new vertex will just stay were it is.
*/
void Insertion::translate() const{
	int index;
	bool overroll, simple = false;
	double alpha, stddev, r, dx, dy;
	Translation *trans;
	unsigned int count = 0;

	// The new vertex is the last one in the vertices vector of the triangulation
	index = (*T).getActualNumberOfVertices() - 1;

	while(!simple && count < Settings::insertionTries){
		// Chose a random direction for the movement
		alpha = (*Settings::generator).getDoubleUniform(- M_PI, M_PI);
		// Compute suitable distribution parameters
		stddev = (*newV).getDirectedEdgeLength(alpha);

		// Generate the translation distance
		r = (*Settings::generator).getDoubleNormal(stddev / 2, stddev / 6);

		// Split the translation in x- and y-components
		dx = r * cos(alpha);
		dy = r * sin(alpha);

		trans = new Translation(T, index, dx, dy);

		// Check for a potential overroll
		overroll = (*trans).checkOverroll();

		if(!overroll){
			// Check the simplicity of the translation
			simple = (*trans).checkSimplicityOfTranslation();

			// Execute the translation
			if(simple){
				(*trans).checkSplit();

				(*trans).execute();
			}
		}

		delete trans;

		count++;
	}

	if(Settings::feedback == FeedbackMode::VERBOSE && count == Settings::insertionTries)
		fprintf(stderr, "Warning: Was not able to find a suitable translation after the insertion!\n");
}