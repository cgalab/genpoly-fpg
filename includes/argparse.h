/* 
 * Copyright 2019 Philipp Mayer - pmayer@cs.sbg.ac.at
 *                Günther Eder  - geder@cs.sbg.ac.at
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

#include <getopt.h>
#include <boost/algorithm/string.hpp>

static const char* short_options = "hc:v::S:D";
static struct option long_options[] = {
		{ "help"               , no_argument      , 0, 'h'},
		{ "nrofholes"          , required_argument, 0, 'n'},
		{ "polygonsize"        , required_argument, 0, 'p'},
		{ "startsize"          , required_argument, 0, 'i'},
		{ "seed"               , required_argument, 0, 's'},
		{ "fixedseed"          , no_argument,       0, 'f'},
		{ "arithmetic"         , no_argument,       0, 'a'},
		{ "holesizes"          , required_argument, 0, 'h'},
		{ "outputformat"       , required_argument, 0, 'o'},
		{ "noexecutioninfo"    , no_argument,       0, 'e'},
		{ "numericalcorrectioninfo" , no_argument,  0, 'c'},
		{ "enablelocalchecks"       , no_argument,  0, 'l'},
		{ "enableglobalchecks"      , no_argument,  0, 'g'},
		{ "notcompletemute"    , no_argument,       0, 'm'},
		{ 0, 0, 0, 0}
};

[[noreturn]]
 static void
 usage(const char *progname, int err) {
	FILE *f = err ? stderr : stdout;

	fprintf(f,"Usage: %s [options] <OUTPUT>\n", progname);
	fprintf(f,"  options: --nrofholes <num>          state number of holes (default: 0).\n");
	fprintf(f,"           --polygonsize <num>        polygon size (default 100).\n");
	fprintf(f,"           --startsize <num>          polygon start-size, initial-size (default 10).\n");
	fprintf(f,"           --seed <num>               seed for rnd (default random).\n");
	fprintf(f,"           --fixedseed                set seed as fixed ?\n");
	fprintf(f,"           --arithmetic               enable 'exact' arithmetic? (default off).\n");
	fprintf(f,"           --holesizes <a,b,c,...>    define hole sizes.\n");
	fprintf(f,"           --outputformat <format>    dat,line, or graphml (default graphml).\n");

	fprintf(f,"           --noexecutioninfo          (default on).\n");
	fprintf(f,"           --numericalcorrectioninfo  (default off).\n");
	fprintf(f,"           --enablelocalchecks        (default off).\n");
	fprintf(f,"           --enableglobalchecks       (default off).\n");
	fprintf(f,"           --notcompletemute          (default on).\n");
	fprintf(f,"\n");
	fprintf(f,"  holesizes example:  --holesizes 3,6,7\n");
	exit(err);
}

#include <vector>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>

#include "settings.h"

class Parser : Settings {
public:
	Parser(int argc, char *argv[]) {
		polygonFile = (char*)"/dev/stdout";
		outputFormat = OutputFormat::GRAPHML;
		mute = true;
		argParse(argc,argv);
	}

private:
	void argParse(int argc, char *argv[]) {
		while (1) {
			int option_index = 0;
			int r = getopt_long(argc, argv, short_options, long_options, &option_index);
			char *end_ptr;

			if (r == -1) { break; }
			switch (r) {
			case 'h': {
				usage(argv[0], 0);
				break;
			}
			case 'n': {
				nrInnerPolygons = (unsigned int)strtoul(optarg,&end_ptr,10);
				if (*end_ptr != '\0') {
					std::cerr << "Invalid nrInnerPolygons " << optarg << "." << std::endl;
					exit(1);
				}
				break;
			}
			case 'p': {
				outerSize = (unsigned int)strtoul(optarg,&end_ptr,10);
				if (*end_ptr != '\0' || outerSize == 0) {
					std::cerr << "Invalid outerSize " << optarg << "." << std::endl;
					exit(1);
				}
				break;
			}
			case 'i': {
				initialSize = (unsigned int)strtoul(optarg,&end_ptr,10);
				if (*end_ptr != '\0' || initialSize == 0) {
					std::cerr << "Invalid initialSize " << optarg << "." << std::endl;
					exit(1);
				}
				break;
			}
			case 's': {
				seed = (unsigned int)strtoul(optarg,&end_ptr,10);
				if (*end_ptr != '\0') {
					std::cerr << "Invalid seed " << optarg << "." << std::endl;
					exit(1);
				}
				break;
			}
			case 'f': {
				fixedSeed = true;
				break;
			}
			case 'a': {
				arithmetics = Arithmetics::EXACT;
				break;
			}
			case 'H': {
				std::vector<std::string> nr;
				boost::split(nr, optarg, boost::is_any_of(","));
				for(auto n : nr) {
					innerSizes.push_back(std::stoi(n));
				}
				break;
			}
			case 'o': {
				auto format = boost::algorithm::to_lower_copy(std::string(optarg));
				if (!strcmp(optarg, "dat")) {
					Settings::outputFormat = OutputFormat::DAT;
				} else if (!strcmp(optarg, "line")) {
					Settings::outputFormat = OutputFormat::LINE;
				} else {
					Settings::outputFormat = OutputFormat::GRAPHML;
				}
				break;
			}
			case 'e': {
				executionInfo = false;
				break;
			}
			case 'c': {
				correctionInfo = true;
				break;
			}
			case 'l': {
				localChecking = true;
				break;
			}
			case 'g': {
				globalChecking = true;
				break;
			}
			case 'm': {
				mute = false;
				break;
			}
			default:{
				std::cerr << "Invalid option " << (char)r << std::endl;
				exit(14);
			}

			} /* end switch */
		} /* end while */


		if (argc - optind > 1) {
			usage(argv[0], 1);
		}

		if (argc - optind >= 1) {
			std::string fn(argv[optind]);
			if (fn != "-") {
				polygonFile = argv[optind];
			}
		}

		std::cout << "hmm..3 " << polygonFile << std::endl;
		if(nrInnerPolygons == 0 && innerSizes.size() > 0) {nrInnerPolygons = innerSizes.size();}
		if(nrInnerPolygons > 0) {
			if(innerSizes.size() > nrInnerPolygons) {innerSizes.resize(nrInnerPolygons);}
			else if(innerSizes.size() < nrInnerPolygons) {
				for(size_t i = 0; i < (nrInnerPolygons - innerSizes.size()); ++i) {
					innerSizes.push_back(10);
				}
			}
		}

		if(outerSize == 0)   {outerSize = 100;}
		if(initialSize == 0) {initialSize = 10;}
	}
};
