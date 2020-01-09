# FPG

Philipp Mayer's Fast Polygon Generator. Starting from a regular polygon of given size FPG randomly translates the poylgon verties across the plane by maintaining the polygons' simplicity.

# Compiling 

	git clone https://github.com/cgalab/genpoly-fpg
	mkdir -p genpoly-fpg/build && cd genpoly-fpg/build
	cmake .. 
	make -j 6

# Usage

	./fpg <POLYGON_SIZE>
	
