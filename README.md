# FPG

Philipp Mayer's Fast Polygon Generator. Starting from a regular polygon of given size FPG randomly translates the poylgon verties across the plane by maintaining the polygons' simplicity.

# Algorithm

TBD: add reference to my thesis

# Output

Writes the polygon generated either as GraphML (.graphml) format, or as line format, or as dat format.

# Requirements 
- C++11 enabled complier (gcc,clang)
- Linux or Mac OS(?)
- BOOST library (>= 1.67, with libboost_graph, libboost_regex, https://www.boost.org)

# Compiling 

	git clone https://github.com/cgalab/genpoly-fpg
	mkdir -p genpoly-fpg/build && cd genpoly-fpg/build
	cmake .. 
	make -j 6

For the `RELEASE` version use `cmake -D CMAKE_BUILD_TYPE=Release ..`

By default FPG uses a command line interface, this can be changed to use a configuration file instead with `cmake -D ENABLE_CLI=OFF ..`
# Usage

	./fpg <POLYGON_SIZE> <OUTPUT_FILE>

| options       | shortform | description   |
| -------------:|----------:|:------------- |
|  --help       |           | print help    |
|  --verbose    | --v       | verbose mode, shows information about the computation |
|  --nrofholes  |           | state the number of holes (default: 0)                |
|  --startsize  |           | give the number of vertices of the start polygon (default: 10) |
|  --seed       |           | set the seed for the random generator (default: random)        |	
|  --arithmetic | --a       | enable Shewchuk's 'exact' arithmetic                           |
|  --holesizes  |           | give a list for the number of vertices of the holes            |
|  --outputformat  | --o    | select between dat, line or graphml (default: graphml)         |
|  --noexecutioninfo |      | disables the execution info, in case of verbose (default: on)  |
|  --statsfile       |      | give a file to print in statistics of the generation and the polygon in XML format |
|  --numericalcorrectioninfo  |  | enable information on numerical corrections (default: off)|
|  --disablelocalchecks  |       | disables the local checking and error correction after each translation       |
|  --enableglobalchecks  |       | enables the global checking after each stage              |
|  --enablestats   |         | enables the computation of statistics, in case of verbose the stats are printed to command line       |
|  --disableweightedselection |  | disables the weighted edge selection for inserting new vertices |

# License

FPG is licenced under [GPLv3](https://www.gnu.org/licenses/gpl-3.0.html).