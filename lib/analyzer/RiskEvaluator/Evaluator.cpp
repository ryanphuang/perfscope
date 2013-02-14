/**
 *  @file          Evaluator.cpp
 *
 *  @version       1.0
 *  @created       02/06/2013 12:03:39 AM
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
 *  Local risk evaluator, which is a function pass and accepts the Instruction set
 *  to be evaluated
 *
 */

#include <algorithm>
#include <iostream>

#include "llvm/Function.h"
#include "llvm/Instruction.h"

#include "llvm/Support/CallSite.h"
#include "llvm/Support/raw_ostream.h"

#include "commons/handy.h"
#include "commons/LLVMHelper.h"
#include "analyzer/Evaluator.h"

using namespace llvm;

bool RiskEvaluator::assess(Instruction *I)
{
  if (isa<CallInst>(I) || isa<InvokeInst>(I)) {
    CallSite cs(I);
    Function *func = cs.getCalledFunction();
    if (func == NULL ) {
      std::cout << "Callee unknown\n";
      return false;
    }
    else
      if(func->isIntrinsic())
        return false;
    if (profile) {
      const char *dname = cpp_demangle(func->getName().data());
      for (Profile::iterator it = profile->begin(), ie = profile->end(); 
          it != ie; ++it) {
        if (std::binary_search(it->second.begin(), it->second.end(), dname)) {
          errs() << "*" << toStr(it->first) << "*";
          return true;
        }
      }
    }
  }
  return false;
}

bool RiskEvaluator::runOnFunction(Function &F)
{
  if (!m_inst_map.count(&F)) {
    errs() << F.getName() << " not in the target\n";
    return false;
  }
  InstVecTy inst_vec = m_inst_map[&F];
  LoopInfo &li = getAnalysis<LoopInfo>(); 
  if (li.empty()) {
    errs() << "There's no loop in " << F.getName() << "\n";
    return false;
  }
  ScalarEvolution *SE = &getAnalysis<ScalarEvolution>(); 
  const BasicBlock * BB = 0;
  unsigned old_depth = 0;
  for (InstVecIter I = inst_vec.begin(), E = inst_vec.end(); I != E; I++) {
    Instruction* inst = *I;
    unsigned depth = old_depth;
    if (inst->getParent() != BB) {
      BB = inst->getParent();
      depth = li.getLoopDepth(BB);
      old_depth = depth;
    }
    errs() << *inst << "\n";
    if (cost_model)
      errs() << "  cost: " << cost_model->getInstructionCost(inst) << "\n";
    errs() << "  loop depth: " << depth << "\n";
    errs() << "  hotness: ";
    if (assess(inst))
      errs() << " hot";
    else
      errs() << " cold";
    errs() << "\n";
    if (depth > 0) {
      Loop * loop = li.getLoopFor(inst->getParent());
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

char RiskEvaluator::ID = 0;
