# Author: Thomas Braun

# Commands: 1.) cd %srcDir%
#	  2.) mkdir build
#	  3.) cd build
#	  4.) cmake -G "MinGW Makefiles" ..
#	  5.) make

cmake_minimum_required(VERSION 2.6)
# BEGIN VARIABLES

SET(projectName matrix-file-reader)

# define all source files here, the file suffix should be kept for now
# _one_ file per line
SET(src
	api-layout.h
	error-codes.h
   )
# END VARIABLES

PROJECT(${projectName} C)

Find_Package(Doxygen)

add_custom_target(
	doc
	COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
	DEPENDS ${CMAKE_SOURCE_DIR}/Doxyfile
	DEPENDS ${src}
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)


Find_Package(LaTeX)

add_custom_target(
	pdf
	COMMAND ${PDFLATEX_COMPILER} ${CMAKE_SOURCE_DIR}/latex/refman.tex
	DEPENDS ${CMAKE_SOURCE_DIR}/latex/refman.tex
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/latex
)

