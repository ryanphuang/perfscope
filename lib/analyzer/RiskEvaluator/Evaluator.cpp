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

static int INDENT = 0;

#define EVALUATOR_DEBUG

gen_dbg(eval)

#ifdef EVALUATOR_DEBUG
gen_dbg_impl(eval)
#else
gen_dbg_nop(eval)
#endif

#ifdef EVALUATOR_DEBUG
inline void errind(int addition = 0)
{
  indent(INDENT + addition);
}
#else
inline void errind(int addition = 0)
{
}
#endif

static RiskEvaluator::RiskLevel Judge[2][2] = {
  {RiskEvaluator::LowRisk, RiskEvaluator::MediumRisk},
  {RiskEvaluator::MediumRisk, RiskEvaluator::HighRisk}
};

RiskEvaluator::RiskLevel RiskEvaluator::assess(Instruction *I, 
      std::map<Loop *, unsigned> & LoopDepthMap)
{
  errs() << *I << "\n";
  errind();
  if (isa<IntrinsicInst>(I)) {
    eval_debug("intrinsic\n");
    return NoRisk;
  }
  eval_debug("expensive:\n");
  bool exp = expensive(I);
  errind(2);
  eval_debug("%s\n", (exp ? "yes" : "no"));
  bool hot = false;
  if (!LI->empty()) {
    errind();
    eval_debug("hot:\n");
    hot = inhot(I, LoopDepthMap);
    errind(2);
    eval_debug("%s\n", (hot ? "yes" : "no"));
  }
  return Judge[hot][exp];
}

unsigned RiskEvaluator::getLoopDepth(Loop *L, std::map<Loop *, unsigned> & LoopDepthMap)
{
  std::map<Loop *, unsigned>::iterator it = LoopDepthMap.find(L);
  if (it != LoopDepthMap.end())
    return it->second;
  SmallVector<BasicBlock *, 4> exits;
  L->getExitingBlocks(exits);
  unsigned count = 0;
  for (SmallVector<BasicBlock *, 4>::iterator ei = exits.begin(), ee = exits.end();
      ei != ee; ++ei) {
    if (*ei) {
      unsigned c = SE->getSmallConstantTripCount(L, *ei); 
      if (c > count)
        count = c;
    }
  }
  LoopDepthMap[L] = count;
  return count;
}

bool RiskEvaluator::expensive(Instruction * I)
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
          errind(2);
          eval_debug("*%s*\n",toStr(it->first)); 
          return true;
        }
      }
    }
  }
  if(cost_model) {
    unsigned cost = cost_model->getInstructionCost(I); 
    errind(2);
    eval_debug("cost: %u\n", cost);
  }
  return false;
}

bool RiskEvaluator::inhot(Instruction *I, std::map<Loop *, unsigned> & LoopDepthMap)
{
  assert(LI && SE && "Require Loop information and ScalarEvolution");
  const BasicBlock * BB = I->getParent();
  Loop * loop = LI->getLoopFor(BB);
  unsigned depth = 0;
  while (loop) {
    depth++;
    errind(2);
    eval_debug("L%u trip count:%u\n", depth, getLoopDepth(loop, LoopDepthMap));
    loop = loop->getParentLoop();
  }
  return true;
}

bool RiskEvaluator::runOnFunction(Function &F)
{
  if (!m_inst_map.count(&F)) {
    errs() << F.getName() << " not in the target\n";
    return false;
  }
  LI = &getAnalysis<LoopInfo>(); 
  SE = &getAnalysis<ScalarEvolution>(); 
  INDENT = 4;
  InstVecTy &inst_vec = m_inst_map[&F];
  std::map<Loop *, unsigned> LoopDepthMap;
  for (InstVecIter I = inst_vec.begin(), E = inst_vec.end(); I != E; I++) {
    Instruction* inst = *I;
    assess(inst, LoopDepthMap);
  }
  return false;
}

char RiskEvaluator::ID = 0;
const char * RiskEvaluator::PassName = "Risk evaluator pass";
