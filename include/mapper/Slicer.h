//===- Slicer.h - Simple Slicing Tool ----*- C++ -*-===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// This file implements a simple slicer based on DependenceGraph 
//===----------------------------------------------------------------------===//
#ifndef PERF_SCOPE_SLCIER__H
#define PERF_SCOPE_SLCIER__H

#include "DependenceGraph.h"
#include "PgmDependenceGraph.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Instruction.h"
#include <iterator>
#include <stack>
#include <vector>

namespace llvm {

class Slicer {
    private:
      std::stack<Instruction *> sliceStack;
      SmallSet<Instruction *, 32> visitedInstr;
      // Don't use SmallPtrSet here because count(ptr), ptr cannot be const
      // SmallPtrSet<Instruction *, 32> visitedInstr;
      DependenceGraph* funcDepGraph;
      bool sliceInited;


    public:
      Slicer(DependenceGraph *graph) : funcDepGraph(graph), sliceInited(false) 
      { assert(graph != NULL); }

      bool sliceInit(Instruction &I, PDGIteratorFlags whichDeps = AllDeps); 
      Instruction * sliceNext(); 
  
    private:
      void addToSliceStack(DepGraphNode *dgNode);
};


} // End llvm namespace
#endif
