##===- projects/sample/tools/Makefile ----------------------*- Makefile -*-===##

#
# Relative path to the top of the source tree.
#
LEVEL=../..

#
# List all of the subdirectories that we will compile.
#

TOOLNAME=perfscope

USEDLIBS=parser.a riskeval.a costmodel.a diffengine.a mappercore.a slicer.a llvmslicer.a callgraph.a mods.a points.a dependence.a commons.a language.a

#USEDLIBS=parser.a mappercore.a riskeval.a costmodel.a diffengine.a dependence.a commons.a 

# LLVMLIBS=LLVMSupport.a LLVMCore.a LLVMSupport.a LLVMCore.a LLVMBitReader.a LLVMAsmParser.a \
				LLVMAnalysis.a LLVMTransformUtils.a LLVMScalarOpts.a LLVMTarget.a

LINK_COMPONENTS = all

include $(LEVEL)/Makefile.common
