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
#pragma once

#include <vector>
#include <math.h>

#include "triangulation.h"
#include "vertex.h"

class Statistics{

private:

	static double radialDistDev;
	static double twistMin;
	static double twistMax;
	static double maxTwist;
	static unsigned int sinuosity;
	

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


	static void calculateRadialDistanceDistribution(Triangulation const * const T, const double width);

	static void calculateRadialDistanceDeviation(Triangulation const * const T);

	static void calculateMaxTwist(Triangulation const * const T);

	static void calculateSinuosity(Triangulation const * const T);

	static void printStats(Triangulation const * const T);
};