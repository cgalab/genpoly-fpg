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

#include "polygon.h"

void Polygon::triangulateStar(Vertex *kernel){

}

void Polygon::triangulateVisible(){
	
}


Polygon::Polygon(PolygonType tp) :
	type(tp), n(0), startVertex(NULL), closed(false), lastVUsed(NULL), lastEUsed(NULL) {

}

void Polygon::addVertex(Vertex *v){
	PolygonVertex *entry;

	if(n != 0 && lastVUsed != NULL){
		fprintf(stderr, "Polygon build error: something went wrong while building a polygon to retriangulate....two vertices next to each other!\n");
		exit(15);
	}

	entry = (PolygonVertex*)malloc(sizeof(PolygonVertex));

	entry -> v = v;
	entry -> prevE = lastEUsed;
	entry -> nextE = NULL;

	if(n == 0)
		startVertex = entry;
	else
		lastEUsed -> nextV = entry;

	lastVUsed = entry;
	lastEUsed = NULL;

	n++;
}

void Polygon::addEdge(TEdge *e){
	PolygonEdge *entry;

	if(n == 0){
		fprintf(stderr, "Polygon build error: something went wrong while building a polygon to retriangulate....the polygon is not allowed to start with an edge!\n");
		exit(15);
	}

	if(lastEUsed != NULL){
		fprintf(stderr, "Polygon build error: something went wrong while building a polygon to retriangulate....two edges next to each other!\n");
		exit(15);
	}

	entry = (PolygonEdge*)malloc(sizeof(PolygonEdge));

	entry -> e = e;
	entry -> prevV = lastVUsed;
	entry -> nextV = NULL;

	lastVUsed -> nextE = entry;

	lastVUsed = NULL;
	lastEUsed = entry;
}

void Polygon::setKernel(Vertex *k){
	kernel = k;
}

void Polygon::close(TEdge *e){
	PolygonEdge *entry;

	if(n < 3){
		fprintf(stderr, "Polygon build error: something went wrong while building a polygon to retriangulate....the polygon is not allowed to have less then three vertices!\n");
		exit(15);
	}

	if(lastEUsed != NULL){
		fprintf(stderr, "Polygon build error: something went wrong while building a polygon to retriangulate....two edges next to each other!\n");
		exit(15);
	}

	entry = (PolygonEdge*)malloc(sizeof(PolygonEdge));

	entry -> e = e;
	entry -> prevV = lastVUsed;
	entry -> nextV = startVertex;
	lastVUsed -> nextE = entry;
	startVertex -> prevE = entry;

	lastVUsed = NULL;
}

void Polygon::print(){
	PolygonVertex *vertex;
	PolygonEdge *edge;

	fprintf(stderr, "Polygon:\n");

	vertex = startVertex;
	edge = vertex -> nextE;
	do{
		
		fprintf(stderr, "Vertex %llu\n", (*vertex -> v).getID());
		fprintf(stderr, "Edge %llu\n", (*edge -> e).getID());

		vertex = edge -> nextV;
		edge = vertex -> nextE;
		
	}while(vertex != startVertex);

	fprintf(stderr, "End of polygon\n");
}

void Polygon::triangulate(){
	if(type == PolygonType::STARSHAPED)
		triangulateStar(NULL);
	else if(type == PolygonType::EDGEVISIBLE)
		triangulateVisible();
}
