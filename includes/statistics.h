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
#pragma once

/*
	Include standard libraries
*/
#include <vector>
#include <math.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

/*
	Include my libraries
*/
#include "triangulation.h"
#include "vertex.h"

/*
	Name spaces of the XML facilities
*/
using boost::property_tree::ptree;
using boost::property_tree::write_xml;
using boost::property_tree::xml_writer_settings;
using boost::property_tree::xml_writer_make_settings;

class Statistics{

private:

	// These variables save the results of the functions
	static double radialDistDev;
	static double twistMin;
	static double twistMax;
	static double maxTwist;
	static unsigned int twistNumber;
	static unsigned int directionChanges;
	

public:

	/*
		Total number of translation which have been checked
	*/
	static unsigned long long translationTries;

	/*
		Number of translations which have been performed at least partially
	*/
	static unsigned long long translationsPerf;

	/*
		Number of split translations
	*/
	static unsigned long long splits;

	/*
		Number of translation which had to be undone due to errors in the
		triangulation found by the local checking
	*/
	static unsigned int undone;

	/*
		Counts how often checkEdge() gets called	
	*/
	static unsigned long long nrChecks;

	/*
		Counts the number of triangles of the SP in all calls of checkEdge()
	*/
	static unsigned long long nrSPTriangles;

	/*
		The maximum number of triangles of an SP in checkEdge()
	*/
	static unsigned int maxSPTriangles;

	/*
		Number of passed triangles in all calls of checkEdge()
	*/
	static unsigned long long nrTriangles;

	/*
		Maximum number of passed triangles in any call of checkEdge()
	*/
	static unsigned int maxTriangles;


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
	static void calculateRadialDistanceDistribution(Triangulation const * const T, const double width);

	
	/*
		The following functions calculate the criteria stated in my theses.
		For further information on these criteria take a look at my theses.
	*/
	static void calculateRadialDistanceDeviation(Triangulation const * const T);

	static void calculateMaxTwist(Triangulation const * const T);

	static void calculateDirectionChanges(Triangulation const * const T);

	static void calculateTwistNumber(Triangulation const * const T);

	/*
		Prints the calculated statistics to stderr. 

		@param 	T 	The triangulation the polygon lives in

		Note:
			- Before the stats are printed, they must be calculated
	*/
	static void printStats(Triangulation const * const T);

	/*
		Writes the calculated statistics to an xml-file. 

		@param 	T 	The triangulation the polygon lives in

		Note:
			- Before the stats are printed, they must be calculated
	*/
	static void writeStatsFile(Triangulation const * const T);
};
