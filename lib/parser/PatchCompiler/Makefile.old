# Makefile for the diff parser
#
# Author: Ryan Huang <ryanhuang@cs.ucsd.edu>


SHELL = /bin/sh

CC := gcc
CXX := g++
DEFS = -DHAVE_CONFIG_H

#TOPDIR := $(CURDIR)
TOPDIR :=.

SRCDIR := $(TOPDIR)/src
INCLUDE_DIR := $(TOPDIR)/include

CXXFLAGS =-Wall -g -fno-exceptions
CXXOPTIONS =-I$(INCLUDE_DIR)

CSRC := $(wildcard $(SRCDIR)/*.c)
CCSRC := $(wildcard $(SRCDIR)/*.cc)
SRC := $(CSRC) $(CCSRC)

COBJS := $(patsubst $(SRCDIR)/%.c, $(SRCDIR)/%.o, $(CSRC))
CCOBJS := $(patsubst $(SRCDIR)/%.cc, $(SRCDIR)/%.o, $(CCSRC))
OBJS := $(COBJS) $(CCOBJS)


# make-dpend(source, obj, output)
define make-depend
	$(CC) $(CXXOPTIONS) $(CFLAGS) $(DEFS) -MM -MP -MT $2 -MF $3 $1
endef


all: parser depend

parser: $(OBJS)
	@echo Linking...
	$(CXX) -o $@ $(CXXFLAGS) $(DEFS) $^

-include .deps

$(COBJS) : %.o : %.c
	$(CC) -o $@ -c $(CXXOPTIONS) $(CXXFLAGS) $(DEFS) $<
#	$(call make-depend, $<, $@, )

$(CCOBJS) : %.o : %.cc
	$(CC) -o $@ -c $(CXXOPTIONS) $(CXXFLAGS) $(DEFS) $<
#	$(call make-depend, $<, $@, )

depend: .deps
	@echo Done

.deps: $(SRC)
	@echo Rebuilding dependency...
	$(CC) $(CXXOPTIONS) $(CFLAGS) $(DEFS) -MM $^ >> .deps

# FIXME:
# simple hack here to preserve the obj prefix in the output dependency target.
# this doesn't work if the source code layout becomes complicated.
	
	@mv .deps .deps.tmp
	@sed -e 's|\(.*\)\.o:|$(SRCDIR)/\1.o:|' < .deps.tmp > .deps
	@rm .deps.tmp

clean:
	@echo cleaning...
	@-find . -type f -name "*.o" -exec rm {} \;
	@-rm .deps
	@echo done..

debug:
	@echo Source dir: $(SRCDIR)
	@echo Source files: $(SRC)
	@echo Obj from c files: $(COBJS)
	@echo Obj from cc files: $(CCOBJS)

.PHONY: clean debug depend
