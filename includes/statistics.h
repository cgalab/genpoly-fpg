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

	/*
		Total number of translation which have been checked
	*/
	static unsigned long long translationTries;

	/*
		Number of translations which have been performed at least partially
	*/
	static unsigned long long translationsPerf;

public:

	static void calculateRadialDistanceDistribution(Triangulation const * const T, const double width);

	static void calculateRadialDistanceDeviation(Triangulation const * const T);

	static void calculateMaxTwist(Triangulation const * const T);

	static void calculateSinuosity(Triangulation const * const T);
};