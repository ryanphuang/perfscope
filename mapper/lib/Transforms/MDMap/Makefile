##===- lib/Transforms/MDMap/Makefile -----------------------*- Makefile -*-===##

LEVEL = ../../..
LIBRARYNAME = LLVMMDMap
LOADABLE_MODULE = 1
USEDLIBS = mappercore.a

ifneq ($(REQUIRES_RTTI), 1)
ifneq ($(REQUIRES_EH), 1)
EXPORTED_SYMBOL_FILE = $(PROJ_SRC_DIR)/MDMap.exports
endif
endif

include $(LEVEL)/Makefile.common

