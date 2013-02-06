/**
 *  @file          lib/analyzer/RiskEvaluator/LoopMeasure.cpp
 *
 *  @version       1.0
 *  @created       02/05/2013 02:55:49 PM
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
 *  Measure the loop performance risk
 *
 */

#include "llvm/Pass.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Instructions.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/InstIterator.h"

using namespace llvm; 

namespace {

struct LoopMeasure : public FunctionPass {
  static char ID;

  LoopMeasure() : FunctionPass(ID) {
  }

  virtual bool runOnFunction(Function &F) {
    LoopInfo &li = getAnalysis<LoopInfo>(); 
    if (li.empty()) {
      errs() << "There's no loop in " << F.getName() << "\n";
      return false;
    }
    errs() << "Found loop in " << F.getName() << "\n";
    ScalarEvolution *SE = &getAnalysis<ScalarEvolution>(); 
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; I++) {
      unsigned depth = li.getLoopDepth(I->getParent());
      errs() << *I << " is in a " << depth << " level nested loop:\n";
      if (depth > 0) {
        Loop * loop = li.getLoopFor(I->getParent());
        while (depth > 0 && loop) {
          BasicBlock * ExitBlock = loop->getExitingBlock();
          if (ExitBlock) {
            unsigned c = SE->getSmallConstantTripCount(loop, ExitBlock); 
            errs() << "   * L" << depth << " trip count: " << c << "\n";
          }
          loop = loop->getParentLoop();
          depth--;
        }
      }
    }
    return false;
  }

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    //AU.addRequired<IndVarSimplify>();
    //AU.addRequiredID(LoopSimplifyID);
    AU.addRequired<LoopInfo>();
    AU.addRequired<ScalarEvolution>(); 
  }
};

char LoopMeasure::ID = 0;
static RegisterPass<LoopMeasure> X("loopmeasure", "Measure Loop");

} // anonymous namespace

