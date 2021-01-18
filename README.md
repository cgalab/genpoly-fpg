# FPG

Philipp Mayer's Fast Polygon Generator. Starting from a regular polygon of given size FPG randomly translates the polygon's vertices across the plane by maintaining the polygon's simplicity using a triangulation. To adjust the triangulation to the translations, FPG has implemented two different approaches: A kinetic triangulation approach and an approach which finds obsolete parts of the triangulation and replaces them by new triangulation.

# Algorithm

Details of the algorithm are described in the Master's Thesis [Generating Random Simple Polygons by Vertex Translations and Insertions](https://www.researchgate.net/publication/348275206_Generating_Random_Simple_Polygons_by_Vertex_Translations_and_Insertions)
.

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

	./fpg [--help] [--verbose] [--mute] [--seed <num>] [--arithmetic] [--kinetic] [--disableweightedselection] [--disablelocalchecks] [--enableglobalchecks] [--startsize <num>] [--initialholes] [--nrofholes <num>] [--holesizes <num,num,...>] [--outputformat <format>] [--enablestats][--statsfile <filename>] [--printtriang <filename>] <POLYGON_SIZE> [OUTPUT_FILE]

| options       | shortform | description   |
| -------------:|----------:|:------------- |
|  --help       | -h        | print help    |
|  --verbose    | -v        | verbose mode, shows additional information on error correction |
|  --mute       | -m        | suppress all command line outputs                              |
|  --seed       | -s        | set the seed for the random generator (default: random)        |
|  --arithmetic | -a        | enable Shewchuk's 'exact' arithmetic                           |
|  --kinetic    | -k        | uses the kinetic triangulation approach instead of replacing the obsolete triangulation parts  |
|  --disableweightedselection | -w | disable the weighted edge selection for inserting new vertices |
|  --disablelocalchecks  | -l    | disable the local checking and error correction after each translation       |
|  --enableglobalchecks  | -g    | enable the global checking after each stage               |
|  --startsize  | -i        | give the number of vertices of the start polygon (default: 10) |
|  --initialholes  | -I     | insert holes into the start polygon instead of during the growth   |
|  --nrofholes  | -n        | state the number of holes (default: 0)                         |
|  --holesizes  | -H        | give a list for the number of vertices of the holes            |
|  --outputformat  | -o     | select between dat, line or graphml (default: graphml)         |
|  --enablestats   | -t     | enable the computation of statistics, results are printed      |
|  --statsfile       | -T   | give a file to print in statistics in XML format               |
|  --printtriang  | -p      | print the whole triangulation to a file in graphml format      |

# License

FPG is licensed under [GPLv3](https://www.gnu.org/licenses/gpl-3.0.html).
