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

#include "holeInsertion.h"

/*
	The function insertHole() inserts one hole into an inner triangle of the
	polygon. The inner triangle is selected randomly based on triangle specific
	weights. The inserted hole has the same shape as the triangle into which
	it is inserted, but is shrunk by 50% in each direction. The area in the
	triangle, but external to the hole gets retriangulated.

	@param 	T 	The triangulation the polygon lives in
*/
void insertHole(Triangulation *T){
	// Randomly select the triangle to insert the hole in
	Triangle *t = (*T).getRandomInternalTriangleWeighted();
	Vertex *v0, *v1, *v2, *holeV0, *holeV1, *holeV2;
	TEdge *e0, *e1, *e2, *holeE0, *holeE1, *holeE2;
	TEdge *con0, *con1, *con2, *e;
	double mx, my;
	unsigned long long index;

	// Get the vertices of the triangle
	v0 = (*t).getVertex(0);
	v1 = (*t).getVertex(1);
	v2 = (*t).getVertex(2);

	// Do not use the triangle's function for getting edges here
	// otherwise it is not sure that e0 will be the edge between
	// v0 and v1
	e0 = (*v0).getEdgeTo(v1);
	e1 = (*v1).getEdgeTo(v2);
	e2 = (*v2).getEdgeTo(v0);

	// Remove the triangle
	delete t;

	// Generate the new polygon in the triangulation
	index = (*T).getActualNrInnerPolygons() + 1;
	(*T).addInnerPolygon(new TPolygon(T, Settings::innerSizes[index]));

	// Compute the mean value of the triangles' vertices
	mx = ((*v0).getX() + (*v1).getX() + (*v2).getX()) / 3;
	my = ((*v0).getY() + (*v1).getY() + (*v2).getY()) / 3;

	// Generate new vertices for the holes by dividing the line segments
	// between the mean point and the vertices of the triangle
	holeV0 = new Vertex(((*v0).getX() + mx) / 2, ((*v0).getY() + my) / 2);
	holeV1 = new Vertex(((*v1).getX() + mx) / 2, ((*v1).getY() + my) / 2);
	holeV2 = new Vertex(((*v2).getX() + mx) / 2, ((*v2).getY() + my) / 2);
	(*T).addVertex(holeV0, index);
	(*T).addVertex(holeV1, index);
	(*T).addVertex(holeV2, index);

	// Generate the edges of the hole
	holeE0 = new TEdge(holeV0, holeV1, EdgeType::POLYGON);
	holeE1 = new TEdge(holeV1, holeV2, EdgeType::POLYGON);
	holeE2 = new TEdge(holeV2, holeV0, EdgeType::POLYGON);
	(*T).addEdge(holeE0, index);
	(*T).addEdge(holeE1, index);
	(*T).addEdge(holeE2, index);

	// Triangulate the hole
	new Triangle(holeE0, holeE1, holeE2, holeV0, holeV1, holeV2, false);

	// Triangulate the area around the new hole
	con0 = new TEdge(v0, holeV0);
	con1 = new TEdge(v1, holeV1);
	con2 = new TEdge(v2, holeV2);
	(*T).addEdge(con0, 0);
	(*T).addEdge(con1, 0);
	(*T).addEdge(con2, 0);

	// First third
	e = new TEdge(v0, holeV1);
	(*T).addEdge(e, 0);

	new Triangle(con0, holeE0, e, v0, holeV0, holeV1, true);
	new Triangle(e, e0, con1, v0, v1, holeV1, true);

	// Second third
	e = new TEdge(v1, holeV2);
	(*T).addEdge(e, 0);

	new Triangle(con1, holeE1, e, v1, holeV1, holeV2, true);
	new Triangle(e, e1, con2, v1, v2, holeV2, true);

	// Third third
	e = new TEdge(v2, holeV0);
	(*T).addEdge(e, 0);

	new Triangle(con2, holeE2, e, v2, holeV2, holeV0, true);
	new Triangle(e, e2, con0, v2, v0, holeV0, true);
}
