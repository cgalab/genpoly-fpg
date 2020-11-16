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

# Usage

	./fpg [--help] [--verbose] [--mute] [--nrofholes <num>] [--startsize <num>] [--seed <num>] [--arithmetic] [--holesizes <num,num,...>] [--outputformat <format>] [--statsfile <filename>] [--disablelocalchecks] [--enableglobalchecks] [--enablestats] [--disableweightedselection] [--printtriang <filename>] <POLYGON_SIZE> [OUTPUT_FILE]

| options       | shortform | description   |
| -------------:|----------:|:------------- |
|  --help       | -h        | print help    |
|  --verbose    | -v        | verbose mode, shows additional information on error correction |
|  --mute       | -m        | suppress all command line outputs                              |
|  --nrofholes  | -n        | state the number of holes (default: 0)                         |
|  --startsize  | -i        | give the number of vertices of the start polygon (default: 10) |
|  --seed       | -s        | set the seed for the random generator (default: random)        |
|  --arithmetic | -a        | enable Shewchuk's 'exact' arithmetic                           |
|  --holesizes  | -H        | give a list for the number of vertices of the holes            |
|  --outputformat  | -o     | select between dat, line or graphml (default: graphml)         |
|  --enablestats   | -t     | enable the computation of statistics, results are printed      |
|  --statsfile       | -T   | give a file to print in statistics in XML format               |
|  --disablelocalchecks  | -l    | disable the local checking and error correction after each translation       |
|  --enableglobalchecks  | -g    | enable the global checking after each stage               |
|  --disableweightedselection | -w | disable the weighted edge selection for inserting new vertices |
|  --printtriang  | -p      | print the whole triangulation to a file in graphml format      |

# License

FPG is licensed under [GPLv3](https://www.gnu.org/licenses/gpl-3.0.html).