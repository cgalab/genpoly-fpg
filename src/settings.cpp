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

#include "predicates.h"
#include "settings.h"

/*
	Polygon parameters
*/
unsigned int Settings::nrInnerPolygons = 0;
unsigned int Settings::outerSize = 0;
std::vector<unsigned int> Settings::innerSizes;

/*
	Initial polygon
*/
unsigned int Settings::initialSize = 20;
double Settings::radiusPolygon = 0.1;
double Settings::radiusHole = 0.05;
double Settings::boxSize = 3.0;
unsigned int Settings::initialTranslationFactor = 100;
int Settings::initialTranslationNumber = -1;


/*
	Translation parameters
*/
bool Settings::kinetic = false;
Arithmetics Settings::arithmetics = Arithmetics::DOUBLE;
int Settings::stddevDiv = 3;
bool Settings::localChecking = true;
bool Settings::globalChecking = false;
unsigned int Settings::additionalTrans = 0;

/*
	Insertion parameters
*/
unsigned int Settings::insertionTries = 100;
double Settings::minLength = 0.0000001;
bool Settings::weightedEdgeSelection = true;


/*
	Utilities
*/
Timer* Settings::timer = NULL;


/*
	Random generator
*/
bool Settings::fixedSeed = false;
unsigned int Settings::seed = 0;
RandomGenerator* Settings::generator = NULL;


/*
	Feedback settings
*/
FeedbackMode Settings::feedback = FeedbackMode::DEFAULT;
OutputFormat Settings::outputFormat = OutputFormat::GRAPHML;
char* Settings::polygonFile = NULL;
bool Settings::triangulationOutputRequired = false;
char* Settings::triangulationFile = NULL;
bool Settings::enableStats = false;
char* Settings::statisticsFile = NULL;

bool Settings::simplicityCheck = false;
double Settings::timing = 0;


void Settings::printSettings(){
	unsigned int i;

	fprintf(stderr, "Polygon settings:\n");
	fprintf(stderr, "Number of holes: %d\n", nrInnerPolygons);
	fprintf(stderr, "Number of vertices (start polygon): %d\n", initialSize);
	fprintf(stderr, "Target number of vertices (polygon): %d\n", outerSize);

	if(nrInnerPolygons > 0){
		fprintf(stderr, "Target number of vertices (holes):\n");
		for(i = 0; i < nrInnerPolygons; ++i){
			fprintf(stderr, "%d\n", innerSizes[i]);
		}
	}

	fprintf(stderr, "\n");

	fprintf(stderr, "Machine settings:\n");
	if(arithmetics == Arithmetics::DOUBLE)
		fprintf(stderr, "Arithmetic: DOUBLE\n");
	else
		fprintf(stderr, "Arithmetic: EXACT\n");
	if(fixedSeed)
		fprintf(stderr, "Seed configuration: FIXED\n");
	else
		fprintf(stderr, "Seed configuration: RANDOM\n");
	fprintf(stderr, "Seed: %u\n", seed);
	if(localChecking)
		fprintf(stderr, "Local checking: enabled\n");
	else
		fprintf(stderr, "Local checking: disbaled\n");
	if(globalChecking)
		fprintf(stderr, "Global checking: enabled\n");
	else
		fprintf(stderr, "Global checking: disbaled\n");

	fprintf(stderr, "\n");

	fprintf(stderr, "Output settings:\n");
	fprintf(stderr, "Polygon file: %s\n", polygonFile);
	switch (outputFormat) {
		case OutputFormat::DAT: fprintf(stderr, "OutputFormat: dat\n"); break;
		case OutputFormat::LINE: fprintf(stderr, "OutputFormat: line\n"); break;
		case OutputFormat::GRAPHML: fprintf(stderr, "OutputFormat: graphml\n"); break;
	}
	if(triangulationOutputRequired)
		fprintf(stderr, "Triangulation file: %s\n", triangulationFile);
	if(statisticsFile != NULL)
		fprintf(stderr, "Statistics file: %s\n", statisticsFile);
	if(Settings::feedback != FeedbackMode::MUTE)
		fprintf(stderr, "Print execution information: true\n");
	else
		fprintf(stderr, "Print execution information: false\n");
	if(Settings::feedback == FeedbackMode::VERBOSE)
		fprintf(stderr, "Print numerical correction information: true\n");
	else
		fprintf(stderr, "Print numerical correction information: false\n");

	fprintf(stderr, "\n");
}


// Checks whether all necessary settings are given
void Settings::checkAndApplySettings(){
	unsigned int i;
	bool conflict = false;
	fprintf(stderr, "Checking for configuration conflicts...");

	if(outerSize < 3){
		fprintf(stderr, "The polygon must have at least 3 vertices, given number %d\n", outerSize);
		exit(14);
	}

	if(outerSize < initialSize){
		fprintf(stderr, "The size of the start polygon is not allowed to exceed the target size:\n");
		fprintf(stderr, "Given start size: %d Given target size: %d\n", initialSize, outerSize);
		exit(14);
	}

	if(nrInnerPolygons != innerSizes.size()){
		fprintf(stderr, "Conflicting number of holes:\n");
		fprintf(stderr, "Given number: %u Given number of sizes: %lu\n", nrInnerPolygons,
			innerSizes.size());
		exit(14);
	}

	for(i = 0; i < nrInnerPolygons; i++){
		if(innerSizes[i] < 3){
			fprintf(stderr, "Holes must have a size of at least 3, given size for polygon with id %d: %d\n",
				i + 1, innerSizes[i]);
			exit(14);
		}
	}

	if(fixedSeed){
		if(seed == 0){
			fprintf(stderr, "\nNote: FixedSeed is marked to use, but no seed is given!\n");
			conflict = true;
		}
	}else{
		if(seed != 0){
			fprintf(stderr, "\nNote: A seed is given but FixedSeed is not marked to use!\n");
			conflict = true;
		}
	}

	if(!conflict)
		fprintf(stderr, "no conflicts\n\n");
	else
		fprintf(stderr, "\n");


	// Apply settings

	if(feedback == FeedbackMode::MUTE)
		fprintf(stderr, "Command line muted!\n");

	// Generate and start Timer
	timer = new Timer();
	(*timer).start();

	// Compute the number of translations for the initial polygon
	if(initialSize < outerSize){
		initialTranslationNumber = initialTranslationFactor * initialSize;
	}else{
		initialTranslationNumber = initialTranslationFactor * outerSize;
		initialSize = outerSize;
	}

	// Initialize the RandomGenerator
	generator = new RandomGenerator(fixedSeed, seed);

	// Enable triangulation output
	if(triangulationFile != NULL)
		triangulationOutputRequired = true;

	// Initialize the exact arithmetic
	if(arithmetics == Arithmetics::EXACT)
		exactinit();
}
