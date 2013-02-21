/**
 *  @file          Evaluator.h
 *
 *  @version       1.0
 *  @created       02/06/2013 12:05:25 AM
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
 *  Local risk evaluator
 *
 */

#ifndef __EVALUATOR_H_
#define __EVALUATOR_H_

#include "llvm/Pass.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Instructions.h"

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/InstIterator.h"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"

#include "analyzer/CostModel.h"
#include "commons/LLVMHelper.h"
#include "commons/CallSiteFinder.h"
#include "dependence/DepGraphBuilder.h"
#include "slicer/Slicer.h"

#include <map>

namespace llvm {

enum RiskLevel {
  NoRisk = 0,       // e.g., renaming, formatting
  LowRisk,      // e.g., a few number of arithmetic operations in cold path
  ModerateRisk, // e.g., expensive operations in cold path
  HighRisk,     // e.g., expensive operations in tight loop
  ExtremeRisk   // e.g., expensive operations in tight loop
};
const char * toRiskStr(RiskLevel risk);
#define RISKLEVELS 5

enum Hotness {
  Cold = 0,
  Regular,
  Hot
};
#define HOTNESSES 3
const char * toHotStr(Hotness hot);

enum Expensiveness {
  Minor = 0,
  Normal,
  Expensive
};

#define EXPNESSES 3
const char * toExpStr(Expensiveness exp);

#define LOOPCOUNTTIGHT 10 // the threshold of a tight loop

#define INSTEXP 10 // threshold of an expensive instruction

#define CALLERHOT 10 // threshold of how many callers is a function defined hot

class RiskEvaluator: public FunctionPass {
  public:
    typedef SmallVector<Instruction *, 8> InstVecTy;
    typedef std::map<Function *, InstVecTy> InstMapTy;

  private:
    typedef SmallVector<Instruction *, 8>::iterator InstVecIter;
    InstMapTy m_inst_map;
    CostModel * cost_model;
    Profile * profile;
    LoopInfo * LI;
    ScalarEvolution *SE;
    unsigned AllRiskStat[RISKLEVELS];
    unsigned FuncRiskStat[RISKLEVELS];
    unsigned level; // denote the level of the analysis
    unsigned depth; // denote the depth of tracing up

  public:
    static char ID;
    static const char * PassName; 

    RiskEvaluator(InstMapTy & inst_map, CostModel * model = NULL, 
        Profile * profile = NULL, unsigned level = 1, unsigned depth = 2) : FunctionPass(ID), 
        m_inst_map(inst_map), cost_model(model), profile(profile), LI(NULL), SE(NULL), 
        level(level), depth(depth)
    {
      memset(AllRiskStat, 0, sizeof(AllRiskStat));
      memset(FuncRiskStat, 0, sizeof(FuncRiskStat));
    }

    virtual const char *getPassName() const { return PassName;}

    virtual bool runOnFunction(Function &F); 

    RiskLevel assess(Instruction *I, std::map<Loop *, unsigned> & LoopDepthMap, Hotness FuncHotness);

    unsigned getLoopDepth(Loop *L, std::map<Loop *, unsigned> & LoopDepthMap);

    Hotness calcInstHotness(Instruction *I, std::map<Loop *, unsigned> & LoopDepthMap);

    Hotness calcFuncHotness(Function * func);
    Hotness calcFuncHotness(const char * funcName);
    Hotness calcCallerHotness(Function * func, int level = 3);

    Expensiveness calcInstExp(Instruction *I);

    void clearFuncStat();
    void statFuncRisk(const char * funcname);
    void statAllRisk();

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
      AU.addRequired<LoopInfo>();
      AU.addRequired<ScalarEvolution>(); 
      AU.addRequired<DepGraphBuilder>();
      AU.addRequired<MemoryDependenceAnalysis>();
    }
  
  private:
    inline void statPrint(unsigned stat[RISKLEVELS]);

};

} // End of llvm namespace


#endif /* __EVALUATOR_H_ */
