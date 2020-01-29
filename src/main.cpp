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



#include "initialPolygonGenerator.h"
#include <stdlib.h>
#include <stdio.h>
#include "triangulation.h"
#include "polygonTransformer.h"
#include "settings.h"
#include "statistics.h"
#ifdef ENABLE_CLI
#include "argparse.h"
#endif
/*
list of exit codes:
code 	name						meaning
1		Circle edge					An edge starts at the same vertex as it ends
2		Vertex at PE end			A vertex lies exactly at a polygon edge after a translation
3		PE flip 					The edge to be flipped at an event is a polygon edge
4		Triangle overflow			A new triangle is assigned to an edge which already has two triangles
5		Triangle redundancy			A new triangle is assigned to an edge which already has the same 
									triangle registered
6		Surrounding polygon fail	The moving vertex is not anymore inside of its surrounding polygon
									at the end of a translation
7		Vertex at PE start 			The moving vertex lies at a polygon edge at the beginning of a translation
8		temporarily empty
9		Triangulation error			Some vertex has left its surrounding polygon but it hasn't been recognized
									during the translation
10		Vertex at PE end 			The surrounding polygon check recognizes a vertex exactly laying on a PE
11 		Not simple					The check for simplicity found an intersection
12 		Vertex insertion error 		The pID passed to Triangulation::addVertex() exceeds the number of inner
									polygons
13 		Parameter error				A parameter given in the configuration file has a wrong type
14 		Setting error				Some mandatory settings are not given or settings are conflicting
15 		Not implemented yet
*/

int main(int argc, char *argv[]){
	Triangulation* T;

#ifdef ENABLE_CLI
	Parser parse(argc, argv);
	Settings::checkAndApplySettings();
#else
	if(argc != 2){
		fprintf(stderr, "Usage: fpg <CONFIG FILE>\n");
		Settings::printDummyFile();
		exit(14);
	}else{
		Settings::readConfigFile(argv[1]);
		Settings::checkAndApplySettings();	
	}
#endif

	if(!Settings::mute)
			Settings::printSettings();

	T = generateRegularPolygon();

	(*T).check();

	if(Settings::executionInfo)
		fprintf(stderr, "Initial polygon with %d vertices in regular shape computed after %f seconds\n",
			Settings::initialSize, (*Settings::timer).elapsedTime());

	if(Settings::nrInnerPolygons == 0)
		strategyNoHoles0(T);
	else if(Settings::nrInnerPolygons == 1)
		strategyWithHoles0(T);
	else
		strategyWithHoles0(T);

	switch (Settings::outputFormat) {
		case OutputFormat::DAT: (*T).writePolygonToDat(Settings::polygonFile); break;
		case OutputFormat::LINE: (*T).writePolygonToLine(Settings::polygonFile); break;
		case OutputFormat::GRAPHML: (*T).writePolygon(Settings::polygonFile); break;
	};

	if(Settings::triangulationOutputRequired)
		(*T).writeTriangulation(Settings::triangulationFile);

	if(Settings::enableStats){
		Statistics::calculateMaxTwist(T);
		Statistics::calculateSinuosity(T);
		Statistics::calculateRadialDistanceDeviation(T);

		if(!Settings::mute)
			Statistics::printStats(T);

		if(Settings::statisticsFile != NULL)
			Statistics::writeStatsFile(T);
	}
	
	exit(0);
}
