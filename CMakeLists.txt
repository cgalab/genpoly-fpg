cmake_minimum_required (VERSION 3.10)
project (fpg)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

OPTION(ENABLE_CLI "using command line parser instead of config file" ON) # switch to OFF if you want to use fpg with config files

set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall" )

set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -W -Wundef -Wpointer-arith" )
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wwrite-strings" )
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wredundant-decls -Wchar-subscripts -Wcomment -Wformat=2" )
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wwrite-strings -Wmissing-declarations -Wredundant-decls" )
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wswitch-enum" )

set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Winit-self -Wmissing-field-initializers" )
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Waddress -Wmissing-noreturn -Wstrict-overflow=1" )
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wextra -Warray-bounds" )
#set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wshadow" )
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pedantic" )

set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Werror=switch-enum" )

### XXX REMOVE EVENTUALLY
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-unused-parameter" )
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-unused-variable" )

if( NOT CMAKE_BUILD_TYPE )
    set( CMAKE_BUILD_TYPE Debug CACHE STRING
       "Choose the type of build, options are: Debug Release."
       FORCE )
endif()

string(TOUPPER "${CMAKE_BUILD_TYPE}" UPPERCASE_CMAKE_BUILD_TYPE)

if (NOT UPPERCASE_CMAKE_BUILD_TYPE MATCHES "^(DEBUG|RELEASE)$")
  message(FATAL_ERROR "Invalid value for CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif()

set_source_files_properties(src/predicates.cpp PROPERTIES COMPILE_FLAGS "-Wno-float-equal -Wno-missing-declarations")


add_executable(fpg
                  src/eventQueue.cpp
                  src/initialPolygonGenerator.cpp
                  src/insertion.cpp
                  src/main.cpp
                  src/polygonTransformer.cpp
                  src/predicates.cpp
                  src/settings.cpp
                  src/statistics.cpp
                  src/tedge.cpp
                  src/tpolygon.cpp
                  src/translation.cpp
                  src/triangle.cpp
                  src/triangulation.cpp
                  src/vertex.cpp
                  src/translationKinetic.cpp
                  src/translationRetriangulation.cpp
                  src/polygon.cpp
               )
include_directories(includes)
