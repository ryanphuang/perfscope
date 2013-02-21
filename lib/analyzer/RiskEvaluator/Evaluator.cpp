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
#include <queue>

#include "llvm/Function.h"

#include "llvm/ADT/SmallPtrSet.h"

#include "llvm/Instruction.h"
#include "llvm/IntrinsicInst.h"

#include "llvm/Support/CallSite.h"
#include "llvm/Support/raw_ostream.h"

#include "commons/handy.h"
#include "commons/CallSiteFinder.h"
#include "commons/LLVMHelper.h"
#include "analyzer/Evaluator.h"

namespace llvm {

static int INDENT = 0;

//#define EVALUATOR_DEBUG

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

static RiskLevel RiskMatrix[HOTNESSES][EXPNESSES] = {
  {LowRisk, LowRisk, ModerateRisk},
  {LowRisk, ModerateRisk, HighRisk},
  {ModerateRisk, HighRisk, ExtremeRisk}
};

static const char * RiskLevelStr[RISKLEVELS] = {
  "no risk",
  "low risk",
  "moderate risk",
  "high risk",
  "extreme risk"
};

static const char * HotStr[HOTNESSES] = {
  "cold",
  "normal",
  "hot"
};

static const char * ExpStr[EXPNESSES] = {
  "minor",
  "normal",
  "expensive"
};

const char * toRiskStr(RiskLevel risk)
{
  if (risk < 0 || risk > RISKLEVELS)
    return "UNKNOWN";
  return RiskLevelStr[risk];
}

const char * toHotStr(Hotness hot)
{
  if (hot < 0 || hot > HOTNESSES)
    return "UNKNOWN";
  return HotStr[hot];
}

const char * toExpStr(Expensiveness exp)
{
  if (exp < 0 || exp > EXPNESSES)
    return "UNKNOWN";
  return ExpStr[exp];
}

RiskLevel RiskEvaluator::assess(Instruction *I, 
      std::map<Loop *, unsigned> & LoopDepthMap, Hotness FuncHotness)
{
  #ifdef EVALUATOR_DEBUG
  errs() << *I << "\n";
  #endif
  errind();
  if (isa<IntrinsicInst>(I)) {
    eval_debug("intrinsic\n");
    return NoRisk;
  }
  eval_debug("expensiveness:\n");
  Expensiveness exp = calcInstExp(I);
  errind(2);
  eval_debug("%s\n", toExpStr(exp)); 
  Hotness hot = Regular;
  if (!LI->empty()) {
    errind();
    eval_debug("hotness:\n");
    if (FuncHotness <= Hot)
      hot = calcInstHotness(I, LoopDepthMap); 
    else
      hot = Hot;
    errind(2);
    eval_debug("%s\n", toHotStr(hot));
  }
  return RiskMatrix[hot][exp];
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

Hotness RiskEvaluator::calcCallerHotness(Function * func, int level)
{
  if (func == NULL || level <= 0)
    return Cold;
  SmallPtrSet<Function *, 4> visited;
  typedef std::pair<Function *, int> FuncDep;
  std::queue<FuncDep> bfsQueue;
  bfsQueue.push(std::make_pair(func, 1));
  eval_debug("Callers:\n");
  unsigned callers = 0;
  while (!bfsQueue.empty()) {
    callers++;
    FuncDep & item = bfsQueue.front();
    errind(2);
    const char *name = item.first->getName().data();
    eval_debug("Depth #%d: %s ", item.second, cpp_demangle(name));
    Hotness hot = calcFuncHotness(name);
    eval_debug("%s", toHotStr(hot));
    if (hot == Hot) {
      eval_debug("\n");
      return Hot;
    }
    // Don't trace upward too much
    if (item.second >= level) {
      eval_debug("\n");
      bfsQueue.pop();
      continue;
    }
    CallSiteFinder csf(item.first);
    CallSiteFinder::cs_iterator i = csf.begin(), e = csf.end();
    if(i == e) {
      eval_debug(", no caller\n"); 
      bfsQueue.pop();
      continue;
    }
    // Push to the queue and increment the depth
    for (; i != e; i++)
      bfsQueue.push(std::make_pair(*i, item.second + 1));
    bfsQueue.pop();
    eval_debug("\n");
  }
  if (callers > CALLERHOT)
    return Hot;
  //TODO define cold function
  return Regular;
}

Hotness RiskEvaluator::calcFuncHotness(const char * funcName)
{
  if (profile) {
    for (Profile::iterator it = profile->begin(), ie = profile->end(); 
        it != ie; ++it) {
      if (std::binary_search(it->second.begin(), it->second.end(), funcName)) {
        errind(2);
        eval_debug("*%s*\n",toHotStr(it->first)); 
        return Hot;
      }
    }
  }
  return Regular;
}

Hotness RiskEvaluator::calcFuncHotness(Function * func)
{
  if (func)
    return calcFuncHotness(cpp_demangle(func->getName().data()));
  return Cold;
}

Expensiveness RiskEvaluator::calcInstExp(Instruction * I)
{
  Expensiveness exp = Minor;
  if (isa<CallInst>(I) || isa<InvokeInst>(I)) {
    CallSite cs(I);
    Function *func = cs.getCalledFunction();
    if (func == NULL || func->isIntrinsic()) {
      #ifdef EVALUATOR_DEBUG
      if (func == NULL)
        errs() << "Callee unknown\n";
      #endif
      return exp;
    }
    //TODO better way
    if (calcFuncHotness(func) == Hot)
      return Expensive;
  }
  if(cost_model) {
    unsigned cost = cost_model->getInstructionCost(I); 
    errind(2);
    eval_debug("cost: %u\n", cost);
    if (cost == 0)
      exp = Minor;
    else
      if (cost > INSTEXP)
        exp = Expensive;
      else
        exp = Normal;
  }
  return exp;
}

Hotness RiskEvaluator::calcInstHotness(Instruction *I, std::map<Loop *, unsigned> & LoopDepthMap)
{
  assert(LI && SE && "Require Loop information and ScalarEvolution");
  const BasicBlock * BB = I->getParent();
  Loop * loop = LI->getLoopFor(BB);
  unsigned depth = 0;
  Hotness hot = Cold;
  while (loop) {
    depth++;
    errind(2);
    unsigned cnt = getLoopDepth(loop, LoopDepthMap);
    eval_debug("L%u trip count:%u\n", depth, cnt);
    // loop count cannot be determined, so it's potentially
    // very tight!
    if (cnt == 0 || cnt > LOOPCOUNTTIGHT) 
      hot = Hot;
    else
      hot = Regular;
    loop = loop->getParentLoop();
  }
  return hot;
}

bool RiskEvaluator::runOnFunction(Function &F)
{
  if (!m_inst_map.count(&F)) {
    errs() << F.getName() << " not in the target\n";
    return false;
  }
  memset(FuncRiskStat, 0, sizeof(FuncRiskStat));
  LI = &getAnalysis<LoopInfo>(); 
  SE = &getAnalysis<ScalarEvolution>(); 
  INDENT = 4;
  InstVecTy &inst_vec = m_inst_map[&F];
  std::map<Loop *, unsigned> LoopDepthMap;
  DepGraph * graph = NULL;
  Hotness funcHot = calcCallerHotness(&F, depth);
  if (level > 1) {
    DepGraphBuilder & builder = getAnalysis<DepGraphBuilder>();
    graph = builder.getDepGraph();
  }
  for (InstVecIter I = inst_vec.begin(), E = inst_vec.end(); I != E; I++) {
    Instruction* inst = *I;
    RiskLevel risk = assess(inst, LoopDepthMap, funcHot);
    errind();
    eval_debug("%s\n", toRiskStr(risk));
    if (graph != NULL) {
      Slicer slicer(graph, Criterion(0, inst, true, AllDep));
      Instruction * propagate;
      eval_debug("Evaluating slice...\n");
      while ((propagate = slicer.next()) != NULL) {
        RiskLevel r = assess(propagate, LoopDepthMap, funcHot);
        errind();
        eval_debug("%s\n", toRiskStr(r));
      }
      eval_debug("Slice evaluation done.\n");
    }
    FuncRiskStat[risk]++;
    AllRiskStat[risk]++;
  }
  statFuncRisk(F.getName().data());
  return false;
}

inline void RiskEvaluator::statPrint(unsigned stat[RISKLEVELS+1])
{
  for (int i = NoRisk; i <= HighRisk; i++) {
    printf("%s:\t%u\n", toRiskStr((RiskLevel) i), stat[i]);
  }
}

void RiskEvaluator::statFuncRisk(const char * funcname)
{
  printf("===='%s' risk summary====\n", funcname);
  statPrint(FuncRiskStat);
}

void RiskEvaluator::statAllRisk()
{
  eval_debug("====Overall risk summary====\n");
  statPrint(AllRiskStat);
}


char RiskEvaluator::ID = 0;
const char * RiskEvaluator::PassName = "Risk evaluator pass";
} // End of llvm namespace

