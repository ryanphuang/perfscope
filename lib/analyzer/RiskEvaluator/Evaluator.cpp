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
#include "llvm/IntrinsicInst.h"

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
      errs() << "Callee unknown\n";
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

bool RiskEvaluator::expensive(Instruction * I)
{
  return true;
}

bool RiskEvaluator::inhot(Instruction *I)
{
  assert(LI && SE && "Require Loop information and ScalarEvolution");
  unsigned depth = 0;
  const BasicBlock * BB = I->getParent();
  Loop * loop = LI->getLoopFor(BB);
  while (loop) {
    depth++;
    SmallVector<BasicBlock *, 4> exits;
    loop->getExitingBlocks(exits);
    unsigned count = 0;
    for (SmallVector<BasicBlock *, 4>::iterator ei = exits.begin(), ee = exits.end();
        ei != ee; ++ei) {
      if (*ei) {
        unsigned c = SE->getSmallConstantTripCount(loop, *ei); 
        if (c > count)
          count = c;
      }
    }
    errs() << "   * L" << depth << " trip count: " << count << "\n";
    loop = loop->getParentLoop();
  }
  errs() << "  loop depth: " << depth << "\n";
  return true;
}

bool RiskEvaluator::runOnFunction(Function &F)
{
  if (!m_inst_map.count(&F)) {
    errs() << F.getName() << " not in the target\n";
    return false;
  }
  InstVecTy &inst_vec = m_inst_map[&F];
  LI = &getAnalysis<LoopInfo>(); 
  SE = &getAnalysis<ScalarEvolution>(); 
  for (InstVecIter I = inst_vec.begin(), E = inst_vec.end(); I != E; I++) {
    Instruction* inst = *I;
    errs() << *inst << "\n";
    if (isa<IntrinsicInst>(inst)) {
      errs() << "  intrinsic\n";
      continue;
    }
    /*
    if (cost_model)
      errs() << "  cost: " << cost_model->getInstructionCost(inst) << "\n";
    errs() << "  hotness: ";
    if (assess(inst))
      errs() << " hot";
    else
      errs() << " cold";
    errs() << "\n";
    */
    if (LI->empty())
      continue;
    inhot(inst);
  }
  return false;
}

char RiskEvaluator::ID = 0;
const char * RiskEvaluator::PassName = "Risk evaluator pass";
