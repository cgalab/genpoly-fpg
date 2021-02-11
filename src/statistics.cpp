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

#include "statistics.h"

/*
	Initialise private variables
*/
double Statistics::radialDistDev = 0;
double Statistics::twistMin = 0;
double Statistics::twistMax = 0;
double Statistics:: maxTwist = 0;
unsigned int Statistics::twistNumber = 0;
unsigned int Statistics::directionChanges = 0;

/*
	Initialise global variables
*/
unsigned long long Statistics::translationTries = 0;
unsigned long long Statistics::translationsPerf = 0;
unsigned long long Statistics::splits = 0;
unsigned int Statistics::undone = 0;
unsigned long long Statistics::nrChecks = 0;
unsigned long long Statistics::nrSPTriangles = 0;
unsigned int Statistics::maxSPTriangles = 0;
unsigned long long Statistics::nrTriangles = 0;
unsigned int Statistics::maxTriangles = 0;


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
// TODO:
// Possibly kick that out
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
	unsigned int i;
	unsigned int n = (*T).getActualNumberOfVertices();
	Vertex *v;

	for(i = 0; i < n; i++){
		v = (*T).getVertex(i);

		dist = (*v).getDistanceToOrigin();

		sum = sum + pow(dist - radius, 2);
	}

	sum = sum / n;

	sum = sqrt(sum);

	radialDistDev = sum;
}

void Statistics::calculateMaxTwist(Triangulation const * const T){
	int n = (*T).getActualNumberOfVertices(0);
	double min, max, sum, angle, average;
	Vertex *start, *v;

	// Calculate the average inside angle of the polygon
	average = 180 * (1 - 2.0 / n);

	start = (*T).getVertex(0, 0);

	angle = (*start).getInsideAngle() / M_PI * 180;
	angle = angle - average;
	sum = angle;
	min = angle;
	max = angle;

	v = (*start).getNext();
	while(*v != *start){
		angle = (*v).getInsideAngle() / M_PI * 180;
		angle = angle - average;

		sum = sum + angle;

		if(sum < min)
			min = sum;

		if(sum > max)
			max = sum;

		v = (*v).getNext();
	}

	twistMin = min;
	twistMax = max;
	maxTwist = fabs(min) + fabs(max);
}

void Statistics::calculateDirectionChanges(Triangulation const * const T){
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
	}while(*v != *start);

	directionChanges = n;
}

void Statistics::calculateTwistNumber(Triangulation const * const T){
	double t;
	double alpha, beta;
	Vertex *start, *v;
	unsigned int count = 0;
	unsigned int n = (*T).getActualNrInnerPolygons();
	unsigned int i;

	for(i = 0; i <= n; i++){
		
		start = (*T).getVertex(i, 0);
		v = start;

		t = 0;
		do{
			v = (*v).getNext();

			alpha = (*v).getInsideAngle();
			beta = alpha - M_PI;

			t = t + beta;

			if(t <= -M_PI){
				count++;
				t = t + M_PI;
			} else if(t >= M_PI){
				count++;
				t = t - M_PI;
			}
		}while(*v != *start);

		twistNumber = (((count - 1) / 2) + 1) * 2;
	}
}

/*
	Prints the calculated statistics to stderr. 

	@param 	T 	The triangulation the polygon lives in

	Note:
		- Before the stats are printed, they must be calculated
*/
void Statistics::printStats(Triangulation const * const T){
	unsigned int i;
	unsigned int nrOfHoles;

	fprintf(stderr, "\n");

	fprintf(stderr, "S T A T I S T I C S\n");

	fprintf(stderr, "\n");

	fprintf(stderr, "Polygon:\n");
	fprintf(stderr, "Number of vertices: %d\n", (*T).getActualNumberOfVertices(0));
	nrOfHoles = (*T).getActualNrInnerPolygons();
	fprintf(stderr, "Number of holes: %d\n", nrOfHoles);
	if(nrOfHoles > 0){
		fprintf(stderr, "Hole sizes:\n");
		for(i = 1; i <= nrOfHoles; i++){
			fprintf(stderr, "%d %d\n", i, (*T).getActualNumberOfVertices(i));
		}
	}
	fprintf(stderr, "Radius of the start polygon: %.2f\n", Settings::radiusPolygon);
	fprintf(stderr, "Edge length of the bounding box: %.2f\n", Settings::boxSize);

	fprintf(stderr, "\n");

	fprintf(stderr, "Translations:\n");
	fprintf(stderr, "Number of checked translations: %llu\n", translationTries);
	fprintf(stderr, "Number of performed translation: %llu\n", translationsPerf);
	if(Settings::kinetic)
		fprintf(stderr, "Number of split translations: %llu\n", splits);
	if(Settings::kinetic)
		fprintf(stderr, "Number of undone translations: %u\n", undone);
	else
		fprintf(stderr, "Number of aborted translations: %u\n", undone);
	fprintf(stderr, "Average number of SP triangles: %.2f\n",
		(double)nrSPTriangles / (double) nrChecks);
	fprintf(stderr, "Max number of SP triangles: %d\n", maxSPTriangles);
	fprintf(stderr, "Average number of passed triangles: %.2f\n",
		(double)nrTriangles / (double) nrChecks);
	fprintf(stderr, "Max number of passed triangles: %d\n", maxTriangles);

	fprintf(stderr, "\n");

	fprintf(stderr, "Shape:\n");
	fprintf(stderr, "Radial deviation from the start polygon: %.2f\n", radialDistDev);
	fprintf(stderr, "Number of direction changes: %d\n", directionChanges);
	fprintf(stderr, "Max inside twist: %.2f°\n", twistMin);
	fprintf(stderr, "Max outside twist: %.2f°\n", twistMax);
	fprintf(stderr, "Overall max twist: %.2f°\n", maxTwist);
	fprintf(stderr, "Number of twists by pi: %u\n", twistNumber);
}

/*
	Writes the calculated statistics to an xml-file. 

	@param 	T 	The triangulation the polygon lives in

	Note:
		- Before the stats are printed, they must be calculated
*/
void Statistics::writeStatsFile(Triangulation const * const T){
	ptree tree;
	unsigned int i;
	unsigned int nrOfHoles = (*T).getActualNrInnerPolygons();

	ptree& stats = tree.add("statistics", "");

	ptree& polygon = stats.add("polygon", "");
	polygon.add("size", (*T).getActualNumberOfVertices(0));
	polygon.add("nrofholes", nrOfHoles);

	ptree& holeSizes = polygon.add("holesizes", "");
	for(i = 1; i <= nrOfHoles; i++){
		ptree& hole = holeSizes.add("hole", "");
		hole.add("<xmlattr>.id", i);
		hole.add("size", (*T).getActualNumberOfVertices(i));
	}

	polygon.add("startradius", Settings::radiusPolygon);
	polygon.add("boxsize", Settings::boxSize);


	ptree& trans = stats.add("translations", "");
	trans.add("checked", translationTries);
	trans.add("performed", translationsPerf);
	trans.add("splits", splits);
	if(Settings::kinetic)
		trans.add("undone", undone);
	else
		trans.add("aborted", undone);
	trans.add("averagesp", (double)nrSPTriangles / (double) nrChecks);
	trans.add("maxsp", maxSPTriangles);
	trans.add("averagepassed", (double)nrTriangles / (double) nrChecks);
	trans.add("maxpassed", maxSPTriangles);
	trans.add("timing", Settings::timing);


	ptree& shape = stats.add("shape", "");
	shape.add("radialdev", radialDistDev);
	shape.add("dir_change", directionChanges);
	shape.add("maxinsidetwist", twistMin);
	shape.add("maxoutsidetwist", twistMax);
	shape.add("maxtwist", maxTwist);
	shape.add("twistnumber", twistNumber);


	write_xml(Settings::statisticsFile, tree,
        std::locale(),
        xml_writer_make_settings<std::string>('\t', 1));
}
