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

#include "statistics.h"

/*
	Initialise global variables
*/
unsigned long long Statistics::translationTries = 0;
unsigned long long Statistics::translationsPerf = 0;

/*
	The function calculateDistanceDistribution() calculates the distance distribution
	for all vertices (including the vertices of the holes) and prints it to stdout.
	Therefore it splits the bounding box into ring layers of diameter width, counts
	the vertices in each layer and compute the vertex densities in the layers.
	The number of layers is computed by the distance of the origin to the bounding 
	box divided by width.

	Note:
		As the box is a rectangle the outer layer is in fact no ring anymore, but is
		defined as the rectangle minus the circle defined by the outer border of the
		second outer layer.
*/
void Statistics::calculateRadialDistanceDistribution(Triangulation const * const T, const double width){
	int n_seg; // The number of segments
	int n_vert; // The number of vertices of the polygon
	int i, cat;
	std::vector<int> segments;
	Vertex *v;
	double distance, area = 0;

	n_seg = ceil(Settings::boxSize / (2 * width)) + 1;
	segments.reserve(n_seg);

	for(i = 0; i < n_seg; i++)
		segments.push_back(0);

	n_vert = (*T).getActualNumberOfVertices();

	for(i = 0; i < n_vert; i++){
		v = (*T).getVertex(i);
		distance =(*v).getDistanceToOrigin();

		cat = floor(distance / width);
		if(cat < n_seg)
			segments[cat]++;
		else
			segments[n_seg - 1]++;
	}

	for(i = 0; i < n_seg; i++)
		fprintf(stderr, "%.4f \t", width * i + width / 2);

	fprintf(stderr, "\n");

	for(i = 0; i < n_seg - 1; i++){
		area = pow((i + 1) * width, 2) * M_PI - pow(i * width, 2) * M_PI;
		fprintf(stderr, "%6.f \t", segments[i] / area);
	}
	area = pow(Settings::boxSize, 2) - area;
	fprintf(stderr, "%6.f \t", segments[n_seg - 1] / area);

	fprintf(stderr, "\n");
}

void Statistics::calculateRadialDistanceDeviation(Triangulation const * const T){
	double sum = 0;
	double radius = Settings::radiusPolygon;
	double dist;
	Vertex *start, *v;

	start = (*T).getVertex(0, 0);
	v = start;

	do{
		dist = (*v).getDistanceToOrigin();

		sum = sum + pow(dist - radius, 2);

		v = (*v).getNext();

	}while((*v).getID() != (*start).getID());

	sum = sum / ((*T).getActualNumberOfVertices(0) - 1);

	sum = sqrt(sum);

	fprintf(stderr, "Initial radius: %.3f\n", radius);
	fprintf(stderr, "Box size: %.3f\n", Settings::boxSize);
	fprintf(stderr, "Radial distance deviation: %.3f\n", sum);
}

void Statistics::calculateMaxTwist(Triangulation const * const T){
	int n = (*T).getActualNumberOfVertices(0);
	double min, max, sum, angle, average;
	Vertex *start, *v;

	// Calculate the average inside angle of the polygon
	average = 180 * (1 - 2.0 / n);
	fprintf(stderr, "Average inside angle: %.3f\n", average);

	start = (*T).getVertex(0, 0);

	angle = (*start).getInsideAngle() / M_PI * 180;
	angle = angle - average;
	sum = angle;
	min = angle;
	max = angle;

	v = (*start).getNext();
	while((*v).getID() != (*start).getID()){
		angle = (*v).getInsideAngle() / M_PI * 180;
		angle = angle - average;

		sum = sum + angle;

		if(sum < min)
			min = sum;

		if(sum > max)
			max = sum;

		v = (*v).getNext();
	}

	fprintf(stderr, "sum at end: %.3f\n", sum);
	fprintf(stderr, "min: %.3f max: %.3f amplitude: %.3f\n", min, max, fabs(min) + fabs(max));

}

void Statistics::calculateSinuosity(Triangulation const * const T){
	int n = 0;
	double angle;
	Vertex *start, *v;
	bool toRight = false;

	start = (*T).getVertex(0, 0);

	// Initialise direction
	angle = (*start).getInsideAngle();
	if(angle > M_PI)
		toRight = true;

	v = start;
	do{
		v = (*v).getNext();

		angle = (*v).getInsideAngle();

		// Angles greater then pi a considered as curves to the right,
		// others as curve to the left
		if(angle > M_PI){
			if(!toRight)
				n++;
			toRight = true;
		}else{
			if(toRight)
				n++;
			toRight = false;
		}
	}while((*v).getID() != (*start).getID());

	fprintf(stderr, "Number of orientation changes: %d\n", n);
}