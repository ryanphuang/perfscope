##===- projects/sample/tools/Makefile ----------------------*- Makefile -*-===##

#
# Relative path to the top of the source tree.
#
LEVEL=../../..

#
# List all of the subdirectories that we will compile.
#

TOOLNAME=riskevaldriver
#
# LIBRARYNAME = LLVMCostDriver
# LOADABLE_MODULE = 1
USEDLIBS = riskeval.a costmodel.a llvmslicer.a callgraph.a mods.a points.a dependence.a language.a mappercore.a slicer.a dependence.a commons.a 

# LLVMLIBS = LLVMSupport.a
LINK_COMPONENTS = all

include $(LEVEL)/Makefile.common
