##===- projects/sample/lib/sample/Makefile -----------------*- Makefile -*-===##

#
# Indicate where we are relative to the top of the source tree.
#
LEVEL=../../..

#
# Give the name of a library.  This will build a dynamic version.
#
LIBRARYNAME = LLVMSlicer
USEDLIBS = mappercore.a callgraph.a language.a mods.a points.a 
BUILD_ARCHIVE = 1
LOADABLE_MODULE = 1

#
# Include Makefile.common so we know what to do.
#
include $(LEVEL)/Makefile.common
