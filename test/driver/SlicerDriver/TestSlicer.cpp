/**
 *  @file         test/driver/testDependenceGraph.cpp
 *
 *  @version       1.0
 *  @created       01/25/2013 11:49:14 PM
 *  @revision      $Id$
 *
 *  @author        Ryan Huang <ryanhuang@cs.ucsd.edu>
 *  @organization  University of California, San Diego
 *  
 *  Copyright (c) 2013, Ryan Huang
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *  http://www.apache.org/licenses/LICENSE-2.0
 *     
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  @section       DESCRIPTION
 *  
 *  Test driver for DependenceGraph
 *
 */

#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Instruction.h"
#include "llvm/Value.h"

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"

#include "dependence/DepIter.h"
#include "dependence/DepGraph.h"
#include "dependence/DepGraphBuilder.h"
#include "slicer/Slicer.h"

using namespace llvm;

namespace {
  struct SlicerTestPass: public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    SlicerTestPass() : FunctionPass(ID) {
    }

    virtual bool runOnFunction(Function &F) {
      DepGraphBuilder & builder = getAnalysis<DepGraphBuilder>();
      DepGraph * graph = builder.getDepGraph();
      //graph->print(errs());
      for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; I++) {
        Instruction * inst = &*I;
        if (inst) {
          errs() << " --------------------------------\n";
          errs() << "|" << *inst << "\n";
          for (User::op_iterator OI = inst->op_begin(), OE = inst->op_end(); OI != OE; OI++) {
            Instruction * def = dyn_cast<Instruction>(*OI);
            if (def != NULL)
              errs() << " <" << *def << "\n";
          }
          for (Value::use_iterator UI = inst->use_begin(), UE = inst->use_end(); UI != UE; UI++) {
            Instruction * use = dyn_cast<Instruction>(*UI);
            if (use != NULL)
              errs() << " >" << *use << "\n";
          }
          errs() << " ========\n";
          errs() << " Slicing:\n";
          Slicer * slicer = new Slicer(graph, Criterion(0, inst, true, AllDep));
          slicer->print(errs());
          errs() << " =======\n\n";
          errs() << " --------------------------------\n";
        }
      }
      return false;
    }
    
    // We don't modify the program, so we preserve all analyses
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
      AU.addRequired<AliasAnalysis>();
      AU.addRequired<DepGraphBuilder>();
      AU.addRequired<MemoryDependenceAnalysis>();
    }
  };
}

char SlicerTestPass::ID = 0;
static RegisterPass<SlicerTestPass> X("tslicer", "Slicer Test Pass");
// Register this pass...
