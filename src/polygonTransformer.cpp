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

#include "polygonTransformer.h"


/*
	The function transformPolygonByMoves() transforms a polygon by randomly selecting a
	vertex and a direction to move and then randomly computing a distance to shift in
	the chosen direction which with the constraint that the movement should be
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
	enum Executed ex;
	int div;

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

		trans = new TranslationKinetic(T, index, dx, dy);

		// Check for an orientation change
		orientationChange = (*trans).checkOrientation();

		if(!orientationChange){
			// Check whether the translation leads to a simple polygon
			simple = (*trans).checkSimplicityOfTranslation();

			if(simple){
				// Try to execute the translation
				ex = (*trans).execute();

				// Count executed translations
				if(ex != Executed::REJECTED)
					performedTranslations++;
			}
		}

		delete trans;

		if(i % div == 0 && Settings::feedback != FeedbackMode::MUTE)
			fprintf(stderr, "%f%% of %d translations performed after %f seconds \n", (double)i / (double)iterations * 100, iterations, (*Settings::timer).elapsedTime());
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
			fprintf(stderr, "%f%% of %d insertions performed after %f seconds \n", (double)i / (double)n * 100,
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

	performed = transformPolygonByMoves(T, Settings::initialTranslationNumber);

	if(Settings::feedback != FeedbackMode::MUTE)
		fprintf(stderr, "Transformed initial polygon with %d of %d translations in %f \
			seconds\n\n", performed, Settings::initialTranslationNumber,
			(*Settings::timer).elapsedTime());

	if(!(*T).check()){
		fprintf(stderr, "Triangulation error: something is wrong in the triangulation at the \
			end of transforming the initial polygon\n");
		exit(9);
	}

	growPolygonBy(T, 0, Settings::outerSize - Settings::initialSize);

	if(Settings::feedback != FeedbackMode::MUTE)
		fprintf(stderr, "Grew initial polygon to %d vertices afters %f seconds \n\n",
			Settings::outerSize, (*Settings::timer).elapsedTime());

	if(!(*T).check()){
		fprintf(stderr, "Triangulation error: something is wrong in the triangulation after growing the \
			polygon\n");
		exit(9);
	}

	if(Settings::additionalTrans > 0){
		performed = transformPolygonByMoves(T, Settings::additionalTrans);

		if(Settings::feedback != FeedbackMode::MUTE)
			fprintf(stderr, "Transformed polygon with %d of %d translations in %f seconds\n\n", performed,
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

	performed = transformPolygonByMoves(T, Settings::initialTranslationNumber);

	if(Settings::feedback != FeedbackMode::MUTE)
		fprintf(stderr, "Transformed initial polygon with %d of %d translations in %f \
			seconds\n\n", performed, Settings::initialTranslationNumber,
			(*Settings::timer).elapsedTime());

	if(!(*T).check()){
		fprintf(stderr, "Triangulation error: something is wrong in the triangulation at the \
			end of transforming the initial polygon\n");
		exit(9);
	}

	
	left = Settings::outerSize - Settings::initialSize;
	while(left > 0){
		grow = (*T).getActualNumberOfVertices(0);

		if(grow > left)
			grow = left;

		growPolygonBy(T, 0, grow);
		left = left - grow;

		if(Settings::feedback != FeedbackMode::MUTE)
		fprintf(stderr, "Grew polygon to %d vertices afters %f seconds \n\n",
			(*T).getActualNumberOfVertices(0), (*Settings::timer).elapsedTime());

		grow = (*T).getActualNumberOfVertices(0);
		performed = transformPolygonByMoves(T, grow);

		if(Settings::feedback != FeedbackMode::MUTE)
			fprintf(stderr, "Transformed initial polygon with %d of %d translations in %f \
				seconds\n\n", performed, grow, (*Settings::timer).elapsedTime());
	}

	Settings::timing = (*Settings::timer).elapsedTime();
}


/*
	Strategy for the generation of polygons with holes:
		- Apply a bunch of translations to the initial polygon
		- Until all polygons have reached the desired number of vertices:
			- Double up the size of the inner polygons
			- Double up the size of the outer polygon
			- Apply a bunch of translations

	@param 	T 	The triangulation the polygon lives in
*/
void strategyWithHoles0(Triangulation * const T){
	int performed;
	int nrInsertions;
	unsigned int i, k;
	int actualN;

	performed = transformPolygonByMoves(T, Settings::initialTranslationNumber);

	if(Settings::feedback != FeedbackMode::MUTE)
		fprintf(stderr, "Transformed initial polygon with %d of %d translations in %f seconds\n\n",
			performed, Settings::initialTranslationNumber, (*Settings::timer).elapsedTime());

	if(!(*T).check()){
		fprintf(stderr, "Triangulation error: something is wrong in the triangulation at the \
			end of transforming the initial polygon\n");
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

		growPolygonBy(T, 0, nrInsertions);

		performed = performed + nrInsertions;

		if(nrInsertions != 0 && Settings::feedback != FeedbackMode::MUTE)
			fprintf(stderr, "Grew outer polygon by %d vertices to %d vertices\n\n", nrInsertions,
				nrInsertions + actualN);

		k++;
	}

	if(!(*T).check()){
		fprintf(stderr, "Triangulation error: something is wrong in the triangulation after \
			growing the initial polygon\n");
		exit(9);
	}

	Settings::timing = (*Settings::timer).elapsedTime();
}