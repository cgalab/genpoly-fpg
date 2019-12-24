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

#include "predicates.h"
#include "settings.h"

/*
	Polygon parameters
*/
unsigned int Settings::nrInnerPolygons = 0;
int Settings::outerSize = 0;
std::vector<int> Settings::innerSizes;
const std::string Settings::dummyFileName = "dummy.fpg";

/*
	Initial polygon
*/
int Settings::initialSize = 10;
double Settings::radiusPolygon = 0.1;
double Settings::radiusHole = 0.05;
double Settings::boxSize = 3.0;
int Settings::initialTranslationFactor = 1000;
int Settings::initialTranslationNumber = -1;


/*
	Translation parameters
*/
Arithmetics Settings::arithmetics = Arithmetics::DOUBLE;
bool Settings::localChecking = true;
bool Settings::globalChecking = false;


/*
	Insertion parameters
*/
int Settings::insertionTries = 100;
double Settings::minLength = 0.0000001;


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
FeedbackMode Settings::feedback = FeedbackMode::EXECUTION;
bool Settings::executionInfo = true;
bool Settings::correctionInfo = false;
bool Settings::enableStats = false;
bool Settings::mute = false;
OutputFormat Settings::outputFormat = OutputFormat::DAT;
bool Settings::simplicityCheck = false;
char* Settings::polygonFile = (char*)"polygon.dat";
bool Settings::triangulationOutputRequired = false;
char* Settings::triangulationFile = NULL;
char* Settings::statisticsFile = NULL;


void Settings::readConfigFile(char *filename){
	std::fstream file;
	std::string line;
	char *token;
	char delimiters[] = "= ,{}\n:\"";
	unsigned int i;
	bool found;

	fprintf(stderr, "Read configuration file %s...", filename);

	file.open(filename, std::ios::in);
	if(!file.is_open()){
		fprintf(stderr, "\nCannot open configuration file: \"%s\"\n", filename);
		exit(100);
	}

	while(getline(file, line)){
		if(!(token = strtok(stringToChar(line), delimiters)))
            continue;
        
        if(token[0] == '#')
            continue;

        for (i = 0; i < strlen(token); i++)
            token[i] = (char) toupper(token[i]);
        if(!strcmp(token, "NROFHOLES")){
        	if(!(token = strtok(0, delimiters)) || !sscanf(token, "%d", &nrInnerPolygons)){
        		fprintf(stderr, "NrOfHoles: expected integer, got \"%s\"!\n", token);
        		exit(13);
        	}
        }else if(!strcmp(token, "POLYGONSIZE")){
        	if(!(token = strtok(0, delimiters)) || !sscanf(token, "%d", &outerSize)){
        		fprintf(stderr, "PolygonSize: expected integer, got \"%s\"!\n", token);
        		exit(13);
        	}
        }else if(!strcmp(token, "STARTSIZE")){
        	if(!(token = strtok(0, delimiters)) || !sscanf(token, "%d", &initialSize)){
        		fprintf(stderr, "StartSize: expected integer, got \"%s\"!\n", token);
        		exit(13);
        	}
        }else if(!strcmp(token, "SEED")){
        	if(!(token = strtok(0, delimiters)) || !sscanf(token, "%d", &seed)){
        		fprintf(stderr, "Seed: expected integer, got \"%s\"!\n", token);
        		exit(13);
        	}
        }else if(!strcmp(token, "FIXEDSEED")){
        	fixedSeed = Settings::readBoolean(found);
        	if(!found){
        		fprintf(stderr, "FixedSeed: boolean expected!\n");
        		exit(13);
        	}
        }else if(!strcmp(token, "ARITHMETIC")){
        	arithmetics = Settings::readArithmeticType(found);
        	if(!found){
        		fprintf(stderr, "Arithmetic: \"EXACT\" or \"DOUBLE\" expected!\n");
        		exit(13);
        	}
        }else if(!strcmp(token, "HOLESIZES")){
        	Settings::readHoleSizes(found);
        	if(!found){
        		fprintf(stderr, "HoleSizes: single integer or list of integers expected!\n");
        		exit(13);
        	}
        }else if(!strcmp(token, "POLYGONFILE")){
        	if(!(token = strtok(0, delimiters))){
        		fprintf(stderr, "PolygonFile: missing argument!\n");
        		exit(13);
        	}
        	Settings::polygonFile = token;
        }else if(!strcmp(token, "OUTPUTFORMAT")){
        	if(!(token = strtok(0, delimiters))){
        		fprintf(stderr, "OutputFormat: missing argument!\n");
        		exit(13);
        	}
        	for (i = 0; i < strlen(token); i++)
        		token[i] = (char) toupper(token[i]);
        	if (!strcmp(token, "DAT")) {
        		Settings::outputFormat = OutputFormat::DAT;
        	} else if (!strcmp(token, "LINE")) {
        		Settings::outputFormat = OutputFormat::LINE;
        	} else if (!strcmp(token, "GRAPHML")) {
        		Settings::outputFormat = OutputFormat::GRAPHML;
        	} else {
        		fprintf(stderr, "OutputFormat: unknown format %s!\n", token);
        		exit(13);
        	}
        }else if(!strcmp(token, "TRIANGULATIONFILE")){
        	if(!(token = strtok(0, delimiters))){
        		fprintf(stderr, "TriangulationFile: missing argument!\n");
        		exit(13);
        	}
        	Settings::triangulationFile = token;
        }else if(!strcmp(token, "STATISTICSFILE")){
        	enableStats = true;
        	if(!(token = strtok(0, delimiters))){
        		fprintf(stderr, "StatisticsFile: missing argument!\n");
        		exit(13);
        	}
        	Settings::statisticsFile = token;
        }else if(!strcmp(token, "PRINTEXECUTIONINFO")){
        	executionInfo = Settings::readBoolean(found);
        	if(!found){
        		fprintf(stderr, "PrintExecutionInfo: boolean expected!\n");
        		exit(13);
        	}
        }else if(!strcmp(token, "NUMERICALCORRECTIONINFO")){
        	correctionInfo = Settings::readBoolean(found);
        	if(!found){
        		fprintf(stderr, "NumericalCorrectionInfo: boolean expected!\n");
        		exit(13);
        	}
        }else if(!strcmp(token, "ENABLELOCALCHECKS")){
        	localChecking = Settings::readBoolean(found);
        	if(!found){
        		fprintf(stderr, "EnableLocalChecks: boolean expected!\n");
        		exit(13);
        	}
        }else if(!strcmp(token, "ENABLEGLOBALCHECKS")){
        	globalChecking = Settings::readBoolean(found);
        	if(!found){
        		fprintf(stderr, "EnableGlobalChecks: boolean expected!\n");
        		exit(13);
        	}
        }else if(!strcmp(token, "COMPLETEMUTE")){
        	mute = Settings::readBoolean(found);
        	if(!found){
        		fprintf(stderr, "CompleteMute: boolean expected!\n");
        		exit(13);
        	}
        }else{
        	fprintf(stderr, "Unknown token %s\n", token);
        	exit(13);
        }
	}

	file.close();

	fprintf(stderr, "successful\n\n");
}

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
	fprintf(stderr, "Seed: %llu\n", seed);
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
	if(executionInfo)
		fprintf(stderr, "Print execution information: true\n");
	else
		fprintf(stderr, "Print execution information: false\n");
	if(correctionInfo)
		fprintf(stderr, "Print numerical correction information: true\n");
	else
		fprintf(stderr, "Print numerical correction information: false\n");

	fprintf(stderr, "\n");
}

// cast string to char*
char* Settings::stringToChar(std::string str){
	char * writable = new char[str.size() + 1];
	strcpy(writable, str.c_str());

	return writable;
}

// Reads booleans
bool Settings::readBoolean(bool &found){
    bool out = false;
    unsigned int i;
    char *token = strtok(0, "= ,{}\n");

    found = true;
    if(token){
        for(i = 0; i < strlen(token); i++)
            token[i] = (char) toupper(token[i]);

        if(!strcmp(token, "TRUE"))
            out = true;
        else if(!strcmp(token, "FALSE"))
            out = false;
        else
            found = false;
    }

    return out;
}

// Read arithmetic type
Arithmetics Settings::readArithmeticType(bool &found){
    Arithmetics out = Arithmetics::DOUBLE;
    unsigned int i;
    char *token = strtok(0, "= ,{}\n");

    found = true;
    if(token){
        for(i = 0; i < strlen(token); i++)
            token[i] = (char) toupper(token[i]);

        if(!strcmp(token, "EXACT"))
            out = Arithmetics::EXACT;
        else if(!strcmp(token, "DOUBLE"))
            out = Arithmetics::DOUBLE;
        else
            found = false;
    }

    return out;
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
		fprintf(stderr, "Given number: %d Given number of sizes: %d\n", nrInnerPolygons,
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

	// Mute overrides all other command line output settings
	if(mute){
		executionInfo = false;
		correctionInfo = false;

		fprintf(stderr, "Command line muted!\n");
	}

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

// Get hole sizes
void Settings::readHoleSizes(bool &found){
	int n, k;
	unsigned int i;
	char delimiters[] = "= ,{}\n:";
	char *token;

	// Get the first given size
	if(!(token = strtok(0, delimiters)) || !sscanf(token, "%d", &n)){
		found = false;
		return;
	}
	innerSizes.push_back(n);
	k = n;

	// No we have to check whether there is an additional size given
	if(!(token = strtok(0, delimiters)) || !sscanf(token, "%d", &n)){
		// If no additional size is given, all holes may have the same size
		if(nrInnerPolygons == 0){
			fprintf(stderr, "HoleSizes: Can not be specified as single integer before specifying\
				the number of holes\n");
			found = false;
			return;
		}

		for(i = 1; i < nrInnerPolygons; i++)
			innerSizes.push_back(k);
	}

	// Hole sizes are given as a list
	while(token != NULL){
		innerSizes.push_back(n);

		// Get next
		token = strtok(0, delimiters);

		if(token != NULL){
			if(!sscanf(token, "%d", &n)){
				fprintf(stderr, "HoleSizes: integer expected, got %s\n", token);
			}
		}
	}
	found = true;
}

//Print a dummy configuration file
void Settings::printDummyFile(){
	FILE *f;

	f = fopen(dummyFileName.c_str(), "a+");
	fseek(f,0,SEEK_END);
	if(ftell(f) > 0 ) {
		std::cout << "file " << dummyFileName << " already exists." << std::endl;
		fclose(f);
		return;
	}

	fprintf(f, "##################################################################################\n");
	fprintf(f, "# This is an example configuration file for FPG                                  #\n");
	fprintf(f, "# All lines starting with '#' are ignored!                                       #\n");
	fprintf(f, "##################################################################################\n");

	fprintf(f, "\n\n");


	fprintf(f, "##################################################################################\n");
	fprintf(f, "# Specify the number of holes (optional; default: 0)                             #\n");
	fprintf(f, "##################################################################################\n");
	fprintf(f, "NrOfHoles = 2\n");

	fprintf(f, "\n");

	fprintf(f, "##################################################################################\n");
	fprintf(f, "# Specify the target number of vertices for the polygon (mandatory)              #\n");
	fprintf(f, "##################################################################################\n");
	fprintf(f, "PolygonSize = 1000\n");

	fprintf(f, "\n");

	fprintf(f, "##################################################################################\n");
	fprintf(f, "# Specifies the the size of the start polygon (optional; default: 10)            #\n");
	fprintf(f, "# Note that the StartSize must be less or equal to PolygonSize!                  #\n");
	fprintf(f, "##################################################################################\n");
	fprintf(f, "StartSize = 5\n");

	fprintf(f, "\n");

	fprintf(f, "##################################################################################\n");
	fprintf(f, "# Specify the number of vertices of the holes either as one integer, so all      #\n");
	fprintf(f, "# holes have the same size, or as comma-separated list surrounded by {}.         #\n");
	fprintf(f, "# If it is specified as one integer, the number of holes must have been          #\n");
	fprintf(f, "# specified before!                                                              #\n");
	fprintf(f, "# (mandatory if the number of holes is not 0)                                    #\n");
	fprintf(f, "##################################################################################\n");
	fprintf(f, "HoleSizes = {25, 20}\n");

	fprintf(f, "\n\n");


	fprintf(f, "##################################################################################\n");
	fprintf(f, "# Specifies the type of arithmetics for determinants, either standard            #\n");
	fprintf(f, "# double (DOUBLE) or Shewchuk's predicates (EXACT) (optional; default: DOUBLE)   #\n");
	fprintf(f, "##################################################################################\n");
	fprintf(f, "Arithmetic = EXACT\n");

	fprintf(f, "\n");

	fprintf(f, "##################################################################################\n");
	fprintf(f, "# Enables local correctness checking after aborted translations.                 #\n");
	fprintf(f, "# (optional; default: true)                                                      #\n");
	fprintf(f, "##################################################################################\n");
	fprintf(f, "EnableLocalChecks = false\n");

	fprintf(f, "\n");

	fprintf(f, "##################################################################################\n");
	fprintf(f, "# Enables global correctness checking between stages and after the last stage.   #\n");
	fprintf(f, "# (optional; default: false)                                                     #\n");
	fprintf(f, "##################################################################################\n");
	fprintf(f, "EnableGlobalChecks = true\n");

	fprintf(f, "\n");

	fprintf(f, "##################################################################################\n");
	fprintf(f, "# Specifies whether a given seed should be used (TRUE) or a seed generated by    #\n");
	fprintf(f, "# the machine (FALSE) (optional; default: FALSE)                                 #\n");
	fprintf(f, "##################################################################################\n");
	fprintf(f, "FixedSeed = false\n");

	fprintf(f, "\n");

	fprintf(f, "##################################################################################\n");
	fprintf(f, "# The seed to use (mandatory if FixedSeed = TRUE; 0 is permitted)                #\n");
	fprintf(f, "##################################################################################\n");
	fprintf(f, "Seed = 123456\n");

	fprintf(f, "\n\n");


	fprintf(f, "##################################################################################\n");
	fprintf(f, "# Specifies the output file for the polygon (optional; default: polygon.dat)     #\n");
	fprintf(f, "##################################################################################\n");
	fprintf(f, "PolygonFile = \"poly.dat\"\n");

	fprintf(f, "\n\n");

	fprintf(f, "##################################################################################\n");
	fprintf(f, "# Specifies the output format for the polygon (dat, line, graphml)               #\n");
	fprintf(f, "##################################################################################\n");
	fprintf(f, "OutputFormat = \"dat\"\n");

	fprintf(f, "\n");

	fprintf(f, "##################################################################################\n");
	fprintf(f, "# Specifies a graphml-file to print the whole triangulation in.                  #\n");
	fprintf(f, "# (optional; default: NULL)                                                      #\n");
	fprintf(f, "##################################################################################\n");
	fprintf(f, "TriangulationFile = \"triangulation.graphml\"\n");

	fprintf(f, "\n");

	fprintf(f, "##################################################################################\n");
	fprintf(f, "# Specifies a file to print the statistics of the polygon and generation in.     #\n");
	fprintf(f, "# (optional; default: NULL)                                                      #\n");
	fprintf(f, "##################################################################################\n");
	fprintf(f, "StatisticsFile = \"stats.dat\"\n");

	fprintf(f, "\n\n");


	fprintf(f, "##################################################################################\n");
	fprintf(f, "# Mute all output to stdout. (optional; default: false)                          #\n");
	fprintf(f, "##################################################################################\n");
	fprintf(f, "CompleteMute = false\n");

	fprintf(f, "\n");

	fprintf(f, "##################################################################################\n");
	fprintf(f, "# Specify whether FPG should print information on the state of execution during  #\n");
	fprintf(f, "# the execution. (optional; default:true)                                        #\n");
	fprintf(f, "##################################################################################\n");
	fprintf(f, "PrintExecutionInfo = false\n");

	fprintf(f, "\n");

	fprintf(f, "##################################################################################\n");
	fprintf(f, "# Specify whether FPG should print note numerical corrections at stdout.         #\n");
	fprintf(f, "# (optional; default:false)                                                      #\n");
	fprintf(f, "##################################################################################\n");
	fprintf(f, "NumericalCorrectionInfo = true\n");

	fclose(f);

	fprintf(stderr, "Printed a dummy config file named %s\n",dummyFileName.c_str());
}
