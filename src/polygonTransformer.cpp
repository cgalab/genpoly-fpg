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

#include "polygonTransformer.h"


/*
	The function transformPolygonByMoves() transforms a polygon by randomly selecting a
	vertex and a direction to move and then randomly computing a distance to shift in
	the chosen direction with the constraint that the movement should be
	applicable with reasonable effort. This is done iterations times.

	@param 	T 			The triangulation the polygon lives in
	@param 	iterations	The number of moves which should be tried
	@return 			The number of moves which were executed at least partially

	Note:
		- The number of really performed moves is most likely less then the number of moves
			which should be tried
		- For polygons with holes the vertex also is randomly selected, i.e. it can be from
			the outer polygon as well as of one of the inner polygons
*/
int transformPolygonByMoves(Triangulation * const T, const int iterations){
	int index = 0;
	double dx = 0, dy = 0, stddev, alpha, r;
	bool simple, orientationChange;
	Translation *trans;
	int n = (*T).getActualNumberOfVertices();
	int performedTranslations = 0;
	Vertex *v;
	enum Executed ex = Executed::FULL;
	int div;

	if(iterations <= 1000)
		div = 0.1 * iterations;
	else if(iterations <= 10000)
		div = 0.05 * iterations;
	else if(iterations <= 100000)
		div = 0.02 * iterations;
	else
		div = 0.01 * iterations;

	if(div == 0)
		div = 1;

	// Try the given number of moves
	for(int i = 0; i < iterations; i++){

		// Chose a vertex randomly
		index = (*Settings::generator).getRandomIndex(n);

		v = (*T).getVertex(index);

		// Chose a direction randomly and get a suitable stddev for the distance
		alpha = (*Settings::generator).getDoubleUniform(- M_PI, M_PI);
		stddev = (*v).getDirectedEdgeLength(alpha);

		// Generate a random distance
		r = (*Settings::generator).getDoubleNormal(stddev / 2, stddev / Settings::stddevDiv);

		// Split the translation into x- and y-components
		dx = r * cos(alpha);
		dy = r * sin(alpha);

		if(Settings::kinetic)
			trans = new TranslationKinetic(T, index, dx, dy);
		else
			trans = new TranslationRetriangulation(T, index, dy, dx);

		// Check for an orientation change
		orientationChange = (*trans).checkOrientation();

		if(!orientationChange){
			// Check whether the translation leads to a simple polygon
			simple = (*trans).checkSimplicityOfTranslation();

			if(simple){
				// Try to execute the translation
				ex = (*trans).execute();

				// Count executed translations
				if(ex != Executed::REJECTED){
					performedTranslations++;

					switch(ex){
						case Executed::FULL:
							Statistics::translationsPerf++;
							break;
						case Executed::PARTIAL:
							Statistics::translationsPerf++;
							break;
						case Executed::UNDONE:
							Statistics::undone++;
							performedTranslations--;
							break;
						case Executed::REJECTED:
						default:
							break;
					}

				}
			}
		}

		delete trans;

		if(i % div == 0 && Settings::feedback != FeedbackMode::MUTE)
			fprintf(stderr, "%.1f%% of %d translations performed after %f seconds \n", (double)i / (double)iterations * 100, iterations, (*Settings::timer).elapsedTime());
	}

	return performedTranslations;
}


/*
	The function growPolygonBy() grows a polygon by n insertions.

	@param 	T 		The triangulation the polygon lives in
	@param	pID 	The ID of the polygon
	@param 	n 		The number of vertices to insert

	Note:
		This function works just for polygons without holes!
*/
void growPolygonBy(Triangulation * const T, const unsigned int pID, const int n){
	int index, actualN, i;
	Insertion *in;
	bool ok;
	int div;
	int counter = 0;
	TEdge *e = NULL;

	if(n <= 1000)
		div = 0.1 * n;
	else if(n <= 10000)
		div = 0.05 * n;
	else if(n <= 100000)
		div = 0.02 * n;
	else
		div = 0.01 * n;

	if(div == 0)
		div = 1;

	for(i = 0; i < n;){

		actualN = (*T).getActualNumberOfVertices(pID);

		// Chose randomly an edge to insert in
		if(Settings::weightedEdgeSelection){
			e = (*T).getRandomEdgeWeighted(pID);

			in = new Insertion(T, pID, e);
		}else{
			index = (*Settings::generator).getRandomIndex(actualN);

			in = new Insertion(T, pID, index);
		}

		// Check whether the chosen edge fulfills the stability criteria for insertions
		ok = (*in).checkStability();

		// Recognized when it is hard to find a suitable edge to insert in
		if(!ok){
			delete in;

			counter++;
			if(counter > 10000 && Settings::feedback == FeedbackMode::VERBOSE)
				fprintf(stderr, "Warning: %d unsuccessful tries to find a suitable edge to insert in\n", counter);
			
			continue;
		}

		counter = 0;

		// Execute the insertion and try to move the new vertex away from the edge
		(*in).execute();

		(*in).translate();

		delete in;

		// Just increase the iteration count if a vertex has really been inserted
		i++;

		if(i % div == 0 && Settings::feedback != FeedbackMode::MUTE)
			fprintf(stderr, "%.1f%% of %d insertions performed after %f seconds \n", (double)i / (double)n * 100,
				n, (*Settings::timer).elapsedTime());
	}
}


/*
	This strategy starts with a small regular polygon, applies a bunch of translations
	to it, then it grows the polygon directly to the desired number of vertices
	and afterwards applies another bunch of translations.

	@param 	T 	The triangulation the polygon lives in
*/
void strategyNoHoles0(Triangulation * const T){
	int performed;


	// Transform the initial polygon
	if(Settings::feedback != FeedbackMode::MUTE)
		fprintf(stderr, "Transform the initial polygon with %d translations:\n",
			Settings::initialTranslationNumber);

	performed = transformPolygonByMoves(T, Settings::initialTranslationNumber);

	if(Settings::feedback != FeedbackMode::MUTE)
		fprintf(stderr, "Transformed the initial polygon with %d of %d translations after %f seconds\n\n",
			performed, Settings::initialTranslationNumber, (*Settings::timer).elapsedTime());

	if(!(*T).check()){
		fprintf(stderr, "Triangulation error: something is wrong in the triangulation at the end of transforming the initial polygon\n");
		exit(9);
	}


	// Grow the polygon
	if(Settings::feedback != FeedbackMode::MUTE)
		fprintf(stderr, "Grow the initial polygon to %d vertices:\n",
			Settings::outerSize);

	growPolygonBy(T, 0, Settings::outerSize - Settings::initialSize);

	if(Settings::feedback != FeedbackMode::MUTE)
		fprintf(stderr, "Grew initial polygon to %d vertices after %f seconds \n\n",
			Settings::outerSize, (*Settings::timer).elapsedTime());

	if(!(*T).check()){
		fprintf(stderr, "Triangulation error: something is wrong in the triangulation after growing the polygon\n");
		exit(9);
	}


	// Additional translation
	if(Settings::additionalTrans > 0){
		if(Settings::feedback != FeedbackMode::MUTE)
			fprintf(stderr, "Transform the polygon with %d translations:\n",
				Settings::additionalTrans);

		performed = transformPolygonByMoves(T, Settings::additionalTrans);

		if(Settings::feedback != FeedbackMode::MUTE)
			fprintf(stderr, "Transformed the polygon with %d of %d translations after %f seconds\n\n", performed,
				Settings::additionalTrans, (*Settings::timer).elapsedTime());

		if(!(*T).check()){
			fprintf(stderr, "Triangulation error: something is wrong in the triangulation at the end\n");
			exit(9);
		}
	}

	Settings::timing = (*Settings::timer).elapsedTime();
}


/*
	This strategy starts with a small regular polygon and then does the following until
	the desired number of vertices is reached:
		- Apply a bunch of translations
		- Double the number of vertices

	@param 	T 	The triangulation the polygon lives in

	Note:
		- Unused at the moment
*/
void strategyNoHoles1(Triangulation * const T){
	int performed;
	int left;
	int grow;


	// Transform the initial polygon
	if(Settings::feedback != FeedbackMode::MUTE)
		fprintf(stderr, "Transform the initial polygon with %d translations:\n",
			Settings::initialTranslationNumber);

	performed = transformPolygonByMoves(T, Settings::initialTranslationNumber);

	if(Settings::feedback != FeedbackMode::MUTE)
		fprintf(stderr, "Transformed the initial polygon with %d of %d translations after %f seconds\n\n",
			performed, Settings::initialTranslationNumber, (*Settings::timer).elapsedTime());

	if(!(*T).check()){
		fprintf(stderr, "Triangulation error: something is wrong in the triangulation at the end of transforming the initial polygon\n");
		exit(9);
	}

	
	// Switch between growing and transforming
	left = Settings::outerSize - Settings::initialSize;
	while(left > 0){
		
		// Double up the number of vertices
		grow = (*T).getActualNumberOfVertices(0);

		if(grow > left)
			grow = left;

		if(Settings::feedback != FeedbackMode::MUTE)
			fprintf(stderr, "Grow the polygon by %d vertices:\n", grow);

		growPolygonBy(T, 0, grow);
		left = left - grow;

		if(Settings::feedback != FeedbackMode::MUTE)
		fprintf(stderr, "Grew the polygon to %d vertices afters %f seconds \n\n",
			(*T).getActualNumberOfVertices(0), (*Settings::timer).elapsedTime());


		// Transform the polygon
		grow = (*T).getActualNumberOfVertices(0);
		if(Settings::feedback != FeedbackMode::MUTE)
			fprintf(stderr, "Transform the polygon with %d translations:\n", grow);

		performed = transformPolygonByMoves(T, grow);

		if(Settings::feedback != FeedbackMode::MUTE)
			fprintf(stderr, "Transformed the polygon with %d of %d translations in %f \
				seconds\n\n", performed, grow, (*Settings::timer).elapsedTime());
	}

	Settings::timing = (*Settings::timer).elapsedTime();
}


/*
	Strategy for the generation of polygons with holes:
		- Assumption: Holes are already inserted into the start polygon
		- Apply a bunch of translations to the initial polygon
		- Until all polygons have reached the desired number of vertices:
			- Double up the size of the inner polygons
			- Double up the size of the outer polygon

	@param 	T 	The triangulation the polygon lives in
*/
void strategyWithHoles0(Triangulation * const T){
	int performed;
	int nrInsertions;
	unsigned int i, k;
	int actualN;


	// Transform the initial polygon
	if(Settings::feedback != FeedbackMode::MUTE)
		fprintf(stderr, "Transform the initial polygon with %d translations:\n",
			Settings::initialTranslationNumber);

	performed = transformPolygonByMoves(T, Settings::initialTranslationNumber);

	if(Settings::feedback != FeedbackMode::MUTE)
		fprintf(stderr, "Transformed the initial polygon with %d of %d translations after %f seconds\n\n",
			performed, Settings::initialTranslationNumber, (*Settings::timer).elapsedTime());

	if(!(*T).check()){
		fprintf(stderr, "Triangulation error: something is wrong in the triangulation at the end of transforming the initial polygon\n");
		exit(9);
	}


	// Repeatedly doubling the number of vertices of the holes and the polygon
	performed = 1;

	// TODO:
	// What the hell is this k doing?
	// Ok....maybe it is an insurance in case no further insertions can be executed such that
	// the program would run forever?!
	k = 0;
	while(performed != 0 && k < 20){
		performed = 0;

		// Double up the sizes of the inner polygons (if still possible)
		for(i = 1; i <= Settings::nrInnerPolygons; i++){
			
			actualN = (*T).getActualNumberOfVertices(i);

			if(Settings::innerSizes[i - 1] >= (unsigned int)(2 * actualN))
				nrInsertions = actualN;
			else
				nrInsertions = Settings::innerSizes[i - 1] - actualN;

			if(nrInsertions != 0 && Settings::feedback != FeedbackMode::MUTE)
				fprintf(stderr, "Grow the inner polygon with ID %d by %d vertices to %d vertices:\n",
					i, nrInsertions, nrInsertions + actualN);

			growPolygonBy(T, i, nrInsertions);

			performed = performed + nrInsertions;

			if(nrInsertions != 0 && Settings::feedback != FeedbackMode::MUTE)
				fprintf(stderr, "Grew the inner polygon with ID %d by %d vertices to %d vertices\n\n",
					i, nrInsertions, nrInsertions + actualN);
		}

		// Double up the size of the outer polygon
		actualN = (*T).getActualNumberOfVertices(0);

		if(Settings::outerSize >= (unsigned int)(2 * actualN))
			nrInsertions = actualN;
		else
			nrInsertions = Settings::outerSize - actualN;

		if(nrInsertions != 0 && Settings::feedback != FeedbackMode::MUTE)
			fprintf(stderr, "Grow the outer polygon by %d vertices to %d vertices:\n", nrInsertions,
				nrInsertions + actualN);

		growPolygonBy(T, 0, nrInsertions);

		performed = performed + nrInsertions;

		if(nrInsertions != 0 && Settings::feedback != FeedbackMode::MUTE)
			fprintf(stderr, "Grew the outer polygon by %d vertices to %d vertices\n\n", nrInsertions,
				nrInsertions + actualN);

		k++;
	}

	if(!(*T).check()){
		fprintf(stderr, "Triangulation error: something is wrong in the triangulation after growing the polygon to its final size\n");
		exit(9);
	}

	Settings::timing = (*Settings::timer).elapsedTime();
}


/*
	Strategy for the generation of polygons with holes:
		- Assumption: Holes have not been inserted into the start polygon
		- Apply a bunch of translations to the initial polygon
		- Grow the outer polygon to the size 10 * the number of holes 
			(if possible)
		- Insert the holes
		- Grow all holes to size 20
		- Apply a bunch of translations
		- Apply 10 inflate stages
		- Apply 10 shrinking stages
		- Apply a bunch of translations
		- Until all polygons have reached the desired number of vertices:
			- Double up the size of the inner polygons
			- Double up the size of the outer polygon

	@param 	T 	The triangulation the polygon lives in
*/
void strategyWithHoles1(Triangulation * const T){
	int performed;
	int nrInsertions;
	unsigned int i, k;
	int actualN;


	// Do some translations
	if(Settings::feedback != FeedbackMode::MUTE)
		fprintf(stderr, "Transform the initial polygon with %d translations:\n",
			Settings::initialTranslationNumber);

	performed = transformPolygonByMoves(T, Settings::initialTranslationNumber);

	if(Settings::feedback != FeedbackMode::MUTE)
		fprintf(stderr, "Transformed initial polygon with %d of %d translations in %f seconds\n\n",
			performed, Settings::initialTranslationNumber, (*Settings::timer).elapsedTime());

	if(!(*T).check()){
		fprintf(stderr, "Triangulation error: something is wrong in the triangulation at the end of transforming the initial polygon\n");
		exit(9);
	}



	// Grow the outer polygon to the size 5 * number of holes if possible
	actualN = (*T).getActualNumberOfVertices(0);
	k = Settings::nrInnerPolygons;

	if(Settings::outerSize >= (unsigned int)(10 * k - actualN))
		nrInsertions = 5 * k - actualN;
	else
		nrInsertions = Settings::outerSize - actualN;

	if(nrInsertions > 0){
		if(nrInsertions != 0 && Settings::feedback != FeedbackMode::MUTE)
			fprintf(stderr, "Grow the polygon by %d vertices to %d vertices:\n", nrInsertions,
				nrInsertions + actualN);

		growPolygonBy(T, 0, nrInsertions);

		if(nrInsertions != 0 && Settings::feedback != FeedbackMode::MUTE)
			fprintf(stderr, "Grew the polygon by %d vertices to %d vertices\n\n", nrInsertions,
				nrInsertions + actualN);
	}



	// Insert the holes into the the polygon
	for(i = 0; i < Settings::nrInnerPolygons; i++){
		insertHole(T);
	}

	if(Settings::feedback != FeedbackMode::MUTE)
		fprintf(stderr, "Inserted %d holes into the polygon\n\n", Settings::nrInnerPolygons);



	// Grow all inner polygon to size 20
	for(i = 1; i <= Settings::nrInnerPolygons; i++){
		if(Settings::innerSizes[i - 1] >= (unsigned int) 20)
			nrInsertions = 17;
		else
			nrInsertions = Settings::innerSizes[i - 1] - 3;

		if(nrInsertions > 0){

			if(nrInsertions != 0 && Settings::feedback != FeedbackMode::MUTE)
				fprintf(stderr, "Grow the inner polygon with ID %d by %d vertices to %d vertices:\n",
					i, nrInsertions, nrInsertions + 3);

			growPolygonBy(T, i, nrInsertions);

			if(nrInsertions != 0 && Settings::feedback != FeedbackMode::MUTE)
				fprintf(stderr, "Grew the inner polygon with ID %d by %d vertices to %d vertices\n\n",
					i, nrInsertions, nrInsertions + 3);
		}
	}



	// Do some random shifting
	if(Settings::feedback != FeedbackMode::MUTE)
		fprintf(stderr, "Transform the polygons with %d translations:\n",
			Settings::initialTranslationNumber);

	performed = transformPolygonByMoves(T, Settings::initialTranslationNumber);

	if(Settings::feedback != FeedbackMode::MUTE)
		fprintf(stderr, "Transformed the polygons with %d of %d translations in %f seconds\n\n",
			performed, Settings::initialTranslationNumber, (*Settings::timer).elapsedTime());

	if(!(*T).check()){
		fprintf(stderr, "Triangulation error: something is wrong in the triangulation at the end of transforming the initial polygon\n");
		exit(9);
	}



	// Try to inflate the holes and shrink the polygon around them
	for(k = 0; k < 10; k++){
		for(i = 1; i <= Settings::nrInnerPolygons; i++){
			if(Settings::feedback != FeedbackMode::MUTE)
				fprintf(stderr, "Inflate the hole with ID %d\n", i);

			inflateHole(T, i);
		}

		fprintf(stderr, "\n");
	}

	for(k = 0; k < 10; k++){
		for(i = 1; i <= Settings::nrInnerPolygons; i++){
			if(Settings::feedback != FeedbackMode::MUTE)
				fprintf(stderr, "Shrink the polygon around the hole with ID %d\n", i);

			shrinkAroundHole(T, i, k);
		}

		fprintf(stderr, "\n");
	}



	// Do some random shifting
	if(Settings::feedback != FeedbackMode::MUTE)
		fprintf(stderr, "Transform the polygons with %d translations:\n",
			Settings::initialTranslationNumber);

	performed = transformPolygonByMoves(T, Settings::initialTranslationNumber);

	if(Settings::feedback != FeedbackMode::MUTE)
		fprintf(stderr, "Transformed polygons with %d of %d translations after %f seconds\n\n",
			performed, Settings::initialTranslationNumber, (*Settings::timer).elapsedTime());

	if(!(*T).check()){
		fprintf(stderr, "Triangulation error: something is wrong in the triangulation at the end of transforming the initial polygon\n");
		exit(9);
	}



	performed = 1;

	// TODO:
	// What the hell is this k doing?
	// Ok....maybe it is an insurance in case no further insertions can be executed such that
	// the program would run forever?!
	k = 0;
	while(performed != 0 && k < 20){
		performed = 0;

		// Double up the sizes of the inner polygons (if still possible)
		for(i = 1; i <= Settings::nrInnerPolygons; i++){

			actualN = (*T).getActualNumberOfVertices(i);

			if(Settings::innerSizes[i - 1] >= (unsigned int)(2 * actualN))
				nrInsertions = actualN;
			else
				nrInsertions = Settings::innerSizes[i - 1] - actualN;

			if(nrInsertions != 0 && Settings::feedback != FeedbackMode::MUTE)
				fprintf(stderr, "Grow the inner polygon with ID %d by %d vertices to %d vertices:\n",
					i, nrInsertions, nrInsertions + actualN);

			growPolygonBy(T, i, nrInsertions);

			performed = performed + nrInsertions;

			if(nrInsertions != 0 && Settings::feedback != FeedbackMode::MUTE)
				fprintf(stderr, "Grew the inner polygon with ID %d by %d vertices to %d vertices\n\n",
					i, nrInsertions, nrInsertions + actualN);
		}

		// Double up the size of the outer polygon
		actualN = (*T).getActualNumberOfVertices(0);

		if(Settings::outerSize >= (unsigned int)(2 * actualN))
			nrInsertions = actualN;
		else
			nrInsertions = Settings::outerSize - actualN;

		if(nrInsertions != 0 && Settings::feedback != FeedbackMode::MUTE)
			fprintf(stderr, "Grow the outer polygon by %d vertices to %d vertices:\n", nrInsertions,
				nrInsertions + actualN);

		growPolygonBy(T, 0, nrInsertions);

		performed = performed + nrInsertions;

		if(nrInsertions != 0 && Settings::feedback != FeedbackMode::MUTE)
			fprintf(stderr, "Grew outer polygon by %d vertices to %d vertices\n\n", nrInsertions,
				nrInsertions + actualN);

		k++;
	}



	if(!(*T).check()){
		fprintf(stderr, "Triangulation error: something is wrong in the triangulation after growing the polygon to its final size\n");
		exit(9);
	}

	Settings::timing = (*Settings::timer).elapsedTime();
}


/*
	The function shrinkAroundHole() tries to shrink the polygon around the hole with
	id holeIndex. For that it repeatedly selects a random vertex of the hole and searches
	for an adjacent vertex of the polygon or another hole. Then it tries a translation
	of the adjacent vertex in direction to the vertex of the hole. The number of times
	this is done equals the number of vertices of the selected hole.

	@param	T 			The triangulation the polygon lives in
	@param 	holeIndex 	The id of the selected hole
	@param 	offset 		A parameter for selecting an adjacent vertex
*/
void shrinkAroundHole(Triangulation * const T, int holeIndex, int offset){
	int n = (*T).getActualNumberOfVertices(holeIndex);
	int i, index;
	Vertex *vHole, *vPolygon;
	double dx, dy;
	TEdge *e;
	double angle, stddev, r;
	Translation *trans;
	bool orientationChange, simple;
	Executed ex;

	for(i = 0; i < n; i++){

		// Chose a vertex randomly
		index = (*Settings::generator).getRandomIndex(n);

		vHole = (*T).getVertex(index, holeIndex);
		vPolygon = getAdjacentPolygonVertex(vHole, offset);

		if(vPolygon != NULL){

			// Get direction from vPolygon to vHole
			e = (*vHole).getEdgeTo(vPolygon);
			angle = (*e).getAngle(vPolygon);

			// Chose a direction normally distributed around angle
			angle = (*Settings::generator).getDoubleNormal(angle, 0.2);
			stddev = (*vPolygon).getDirectedEdgeLength(angle);

			// Generate a random distance
			r = (*Settings::generator).getDoubleNormal(stddev / 2, stddev / Settings::stddevDiv);

			// Split the translation into x- and y-components
			dx = r * cos(angle);
			dy = r * sin(angle);

			if(Settings::kinetic)
				trans = new TranslationKinetic(T, vPolygon, dx, dy);
			else
				trans = new TranslationRetriangulation(T, vPolygon, dy, dx);

			// Check for an orientation change
			orientationChange = (*trans).checkOrientation();

			if(!orientationChange){
				// Check whether the translation leads to a simple polygon
				simple = (*trans).checkSimplicityOfTranslation();

				if(simple){
					// Try to execute the translation
					ex = (*trans).execute();

					// Count executed translations
					if(ex != Executed::REJECTED){

						switch(ex){
							case Executed::FULL:
								Statistics::translationsPerf++;
								break;
							case Executed::PARTIAL:
								Statistics::translationsPerf++;
								break;
							case Executed::UNDONE:
								Statistics::undone++;
								break;
							case Executed::REJECTED:
							default:
								break;
						}

					}
				}
			}

			delete trans;
		}
	}
}


/*
	The function inflateHole() tries to inflate the hole with id holeIndex. For that
	it repeatedly selects a random vertex of the hole and tries to execute a 
	translation of it in direction to the its exterior. The number of times
	this is done equals the number of vertices of the selected hole.

	@param 	T 			The triangulation the polygon lives in
	@param 	holeIndex 	The id of the selected hole
*/
void inflateHole(Triangulation * const T, int holeIndex){
	int n = (*T).getActualNumberOfVertices(holeIndex);
	int i;
	Vertex *v;
	double angle, r, stddev, dx, dy;
	Translation *trans;
	bool orientationChange, simple;
	Executed ex;
	int index;

	for(i = 0; i < n; i++){

		// Chose a vertex randomly
		index = (*Settings::generator).getRandomIndex(n);

		v = (*T).getVertex(index, holeIndex);

		// Get direction to the outside
		angle = (*v).getNormalDirectionOutside();

		// Chose a direction normally distributed around angle
		angle = (*Settings::generator).getDoubleNormal(angle, 0.5);
		stddev = (*v).getDirectedEdgeLength(angle);

		// Generate a random distance
		r = (*Settings::generator).getDoubleNormal(stddev / 2, stddev / Settings::stddevDiv);

		// Split the translation into x- and y-components
		dx = r * cos(angle);
		dy = r * sin(angle);

		if(Settings::kinetic)
			trans = new TranslationKinetic(T, v, dx, dy);
		else
			trans = new TranslationRetriangulation(T, v, dy, dx);

		// Check for an orientation change
		orientationChange = (*trans).checkOrientation();

		if(!orientationChange){
			// Check whether the translation leads to a simple polygon
			simple = (*trans).checkSimplicityOfTranslation();

			if(simple){
				// Try to execute the translation
				ex = (*trans).execute();

				// Count executed translations
				if(ex != Executed::REJECTED){

					switch(ex){
						case Executed::FULL:
							Statistics::translationsPerf++;
							break;
						case Executed::PARTIAL:
							Statistics::translationsPerf++;
							break;
						case Executed::UNDONE:
							Statistics::undone++;
							break;
						case Executed::REJECTED:
						default:
							break;
					}
				}
			}
		}

		delete trans;
	}
}


/*
	The function getAdjacentPolygonVertex() returns any Vertex which is adjacent to v in the
	triangulation and belongs to the polygon or another hole. If there is no such
	vertex, it returns NULL.

	@param 	v 	A vertex of a hole
	@return 	Any adjacent vertex to v of the polygon (with polygon id equals 0)
*/
Vertex *getAdjacentPolygonVertex(Vertex const * const v, int index){
	std::list<TEdge*> edges = (*v).getEdges();
	Vertex *otherV;
	TEdge *e;
	int n = edges.size();
	int pid, hid = (*v).getPID();
	auto front = edges.begin();

	index = index % n;

	std::advance(front, index);

	e = *front;
	otherV = (*e).getOtherVertex(v);

	pid = (*otherV).getPID();
	if(pid != hid && pid != -1)
		return otherV;

	return NULL;
}
