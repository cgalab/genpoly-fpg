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
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>

#include "timer.h"
#include "randomGenerator.h"
#include "predicates.h"

#ifndef __SETTINGS_H_
#define __SETTINGS_H_

enum class FeedbackMode {LACONIC, EXECUTION, VERBOSE};
enum class Arithmetics {EXACT, DOUBLE};

enum class OutputFormat {DAT, LINE, GRAPHML};

class Settings{

public:
	
	/*
		GLOBAL VARIABLES
	*/


	/*
		Polygon parameters
	*/

	// The number of the inner polygons
	static unsigned int nrInnerPolygons;

	// Target size of the outer polygon
	static unsigned int outerSize;

	// Target sizes of the inner polygons
	static std::vector<unsigned int> innerSizes;


	/*
		Initial polygon
	*/

	// Size of the initial polygon
	static unsigned int initialSize;

	// Radius of the initial outer polygon
	static double radiusPolygon;

	// Radius of the first initial hole
	static double radiusHole;

	// Size of the bounding box
	static double boxSize;

	// Average number of translations per vertex of the initial polygon
	static unsigned int initialTranslationFactor;

	// Number of translations to perform on the initial polygon
	static int initialTranslationNumber;


	/*
		Translation parameters
	*/

	// Type of used arithmetics
	static Arithmetics arithmetics;

	// Minimal time distance between events in the event queue
	static constexpr double epsEventTime = 0.00001; //10⁻⁵

	// At intersection check all determinants' absolute values less then epsInt
	// are interpreted as zero
	static constexpr double epsInt = 0.000000000001; // 10⁻¹²

	// Minimal allowed determinant for triangles attached to edges for insertion
	// To small triangles on the edge of insertion can lead to the vertex moving
	// out of its surrounding polygon (just with double arithmetics)
	static constexpr double minDetInsertion = 0.000000000001; // 10⁻¹²

	//
	static double distMax;

	//
	static int stddevDiv;

	// Enable local correctness checking after aborted translations
	static bool localChecking;

	// Enable global correctness checking between stages
	static bool globalChecking;

	// Factor for additional translations
	static unsigned int additionalTrans;


	/*
		Insertion parameters
	*/

	// Number of tries to move an inserted vertex
	static unsigned int insertionTries;

	// Minimal allowed edge length for insertions
	static double minLength;

	// Flag for weighted edge selection
	static bool weightedEdgeSelection;


	/*
		Utilities
	*/

	// Global Timer
	static Timer *timer;


	/*
		Random Generator
	*/

	// Enable a fixed seed for the RandomGenerator
	static bool fixedSeed;

	// The fixed seed for the RandomGenerator (if enabled)
	static unsigned int seed;

	// Global RandomGenerator
	static RandomGenerator *generator;


	/*
		Feedback settings
	*/

	// Feedback mode
	static FeedbackMode feedback;

	// Flag whether execution info should be printed
	static bool executionInfo;

	// Flag whether info an numerical correction should be printed
	static bool correctionInfo;

	// Flag to enable statistics on the polygon and its generation
	static bool enableStats;

	// Flag to mute all command line output
	static bool mute;

	// Filename for polygon output file
	static char *polygonFile;

	// Output format for polygon file
	static OutputFormat outputFormat;

	// A flag whether the triangulation is required
	static bool triangulationOutputRequired;

	// Filename for triangulation file
	static char *triangulationFile;

	// Filename for statistics file
	static char *statisticsFile;





	// Function for reading the configuration file
	static void readConfigFile(char *filename);

	// Function for printing the settings
	static void printSettings();

	// cast string to char*
	static char* stringToChar(std::string str);

	// Reads booleans
	static bool readBoolean(bool &found);

	// Read arithmetic type
	static Arithmetics readArithmeticType(bool &found);

	// Checks whether all necessary settings are given
	static void checkAndApplySettings();

	// Get hole sizes
	static void readHoleSizes(bool &found);

	//Print a dummy configuration file
	static void printDummyFile();

	/*
		Check
	*/

	// Enable simplicity check
	static bool simplicityCheck;

	static const std::string dummyFileName;

	static double timing;
};

#endif
