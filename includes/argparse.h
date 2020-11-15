/* 
 * Copyright 2020 Philipp Mayer - philmay1992@gmail.com
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

static const char* short_options = "hn:i:s:aH:o:T:lgvmtwp:";
static struct option long_options[] = {
		{ "help"                , no_argument      , 0, 'h'},
		{ "nrofholes"           , required_argument, 0, 'n'},
		{ "startsize"           , required_argument, 0, 'i'},
		{ "seed"                , required_argument, 0, 's'},
		{ "arithmetic"          , no_argument,       0, 'a'},
		{ "holesizes"           , required_argument, 0, 'H'},
		{ "outputformat"        , required_argument, 0, 'o'},
		{ "statsfile"           , required_argument, 0, 'T'},
		{ "disablelocalchecks"       , no_argument,  0, 'l'},
		{ "enableglobalchecks"       , no_argument,  0, 'g'},
		{ "verbose"                  , no_argument,  0, 'v'},
		{ "mute"                     , no_argument,  0, 'm'},
		{ "enablestats"              , no_argument,  0, 't'},
		{ "disableweightedselection" , no_argument,  0, 'w'},
		{ "printtriang"				 , required_argument, 0, 'p'},
		{ 0, 0, 0, 0}
};

[[noreturn]]
 static void
 usage(const char *progname, int err) {
	FILE *f = err ? stderr : stdout;

	fprintf(f,"Usage: %s [options] <nr. of vertices> <output-file>\n", progname);
	fprintf(f,"  options: --nrofholes <num>          state number of holes (default: 0).\n");
	fprintf(f,"           --startsize <num>          polygon start-size, initial-size (default 10).\n");
	fprintf(f,"           --seed <num>               seed for rnd (default random).\n");
	fprintf(f,"           --arithmetic               enable 'exact' arithmetic? (default off).\n");
	fprintf(f,"           --holesizes <a,b,c,...>    define hole sizes.\n");
	fprintf(f,"           --outputformat <format>    dat, line, or graphml (default graphml).\n");
	fprintf(f,"           --disablelocalchecks       \n");
	fprintf(f,"           --enableglobalchecks       (default off).\n");
	fprintf(f,"           --verbose                  (default off).\n");
	fprintf(f,"           --mute                     (default off).\n");
	fprintf(f,"           --enablestats              (default off).\n");
	fprintf(f,"           --statsfile <string>       xml-file for statistics (default none)\n");
	fprintf(f,"           --printtriang <string>     print the triangulation into a graphml file\n");
	fprintf(f,"           --disableweightedselection \n");
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
		feedback = FeedbackMode::DEFAULT;
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
			case 'i': {
				initialSize = (unsigned int)strtoul(optarg,&end_ptr,10);
				if (*end_ptr != '\0' || initialSize == 0) {
					std::cerr << "Invalid initialSize " << optarg << "." << std::endl;
					exit(1);
				}
				break;
			}
			case 's': {
				fixedSeed = true;
				seed = (unsigned int)strtoul(optarg,&end_ptr,10);
				if (*end_ptr != '\0') {
					std::cerr << "Invalid seed " << optarg << "." << std::endl;
					exit(1);
				}
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
			case 'T': {
				enableStats = true;
				statisticsFile = optarg;
				break;
			}
			case 'p': {
				triangulationFile = optarg;
				triangulationOutputRequired = true;
				break;
			}
			case 'l': {
				localChecking = false;
				break;
			}
			case 'g': {
				globalChecking = true;
				break;
			}
			case 'v': {
				feedback = FeedbackMode::VERBOSE;
				break;
			}
			case 'm': {
				feedback = FeedbackMode::MUTE;
				break;
			}case 't': {
				enableStats = true;
				break;
			}
			case 'w': {
				weightedEdgeSelection = false;
				break;
			}
			default:{
				std::cerr << "Invalid option " << (char)r << std::endl;
				exit(14);
			}

			} /* end switch */
		} /* end while */


		if (argc - optind > 2 || argc < 2) {
			usage(argv[0], 1);
		}

		/* get polygon size */
		if (argc - optind >= 1) {
			outerSize = std::stol(argv[optind]);
		}

		/* get output file */
		if (argc - optind >= 2) {
			std::string fn(argv[optind+1]);
			if (fn != "-") {
				polygonFile = argv[optind+1];
			}
		}

		if(nrInnerPolygons == 0 && innerSizes.size() > 0) {nrInnerPolygons = innerSizes.size();}
		if(nrInnerPolygons > 0) {
			if(innerSizes.size() > nrInnerPolygons) {innerSizes.resize(nrInnerPolygons);}
			else if(innerSizes.size() < nrInnerPolygons) {
				for(size_t i = 0; i < (nrInnerPolygons - innerSizes.size()); ++i) {
					innerSizes.push_back(10);
				}
			}
		}

		if(outerSize == 0)   {outerSize = 20;}
		if(initialSize == 0) {initialSize = 10;}
		
		// In the strategy used the number of additional translations after the growth
		// of the polygon equals the desired number of vertices
		additionalTrans = outerSize;
	}
};
