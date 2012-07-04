# Makefile for the diff parser
#
# Author: Ryan Huang <ryanhuang@cs.ucsd.edu>


SHELL = /bin/sh

CC := gcc
CXX := g++
DEFS = -DHAVE_CONFIG_H

#TOPDIR := $(CURDIR)
TOPDIR :=.

SRC_DIR := $(TOPDIR)/src
INCLUDE_DIR := $(TOPDIR)/include

CXXFLAGS =-Wall -fno-exceptions
CXXOPTIONS =-I$(INCLUDE_DIR)

CSRC := $(wildcard $(SRC_DIR)/*.c)
CCSRC := $(wildcard $(SRC_DIR)/*.cc)
SRC := $(CSRC) $(CCSRC)

COBJS := $(patsubst $(SRC_DIR)/%.c, $(SRC_DIR)/%.o, $(CSRC))
CCOBJS := $(patsubst $(SRC_DIR)/%.cc, $(SRC_DIR)/%.o, $(CCSRC))
OBJS := $(COBJS) $(CCOBJS)

all: parser

parser: $(OBJS)
	@echo Linking...
	$(CXX) -o $@ $(CXXFLAGS) $(DEFS) $^

$(COBJS) : %.o : %.c
	$(CC) -o $@ -c $(CXXOPTIONS) $(CXXFLAGS) $(DEFS) $<

$(CCOBJS) : %.o : %.cc
	$(CC) -o $@ -c $(CXXOPTIONS) $(CXXFLAGS) $(DEFS) $<

depend: .deps
	@echo Dependency built.

.deps: $(SRC)
	@echo Building dependency...
	$(CC) $(CXXOPTIONS) $(CFLAGS) $(DEFS) -MM $^ >> .deps

-include .deps

clean:
	-find . -type f -name "*.o" -exec rm {} \;
	-rm .deps

debug:
	@echo Source dir: $(SRC_DIR)
	@echo Source files: $(SRC)
	@echo Obj from c files: $(COBJS)
	@echo Obj from cc files: $(CCOBJS)

.PHONY: clean debug depend
