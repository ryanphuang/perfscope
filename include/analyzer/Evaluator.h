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

#include <map>

namespace llvm {

class RiskEvaluator: public FunctionPass {
  public:
    typedef SmallVector<Instruction *, 8> InstVecTy;
    typedef std::map<Function *, InstVecTy> InstMapTy;

    enum RiskLevel {
      NoRisk,     // e.g., renaming, formatting
      LowRisk,    // e.g., a few number of arithmetic operations in regular places
      MediumRisk, // e.g., arithmetic operations in tight loop
      HighRisk    // e.g., expensive operations in tight loop
    };

    #define RISKLEVELS 4

  private:
    typedef SmallVector<Instruction *, 8>::iterator InstVecIter;
    InstMapTy m_inst_map;
    CostModel * cost_model;
    Profile * profile;
    LoopInfo * LI;
    ScalarEvolution *SE;
    unsigned AllRiskStat[RISKLEVELS + 1];
    unsigned FuncRiskStat[RISKLEVELS + 1];

  public:
    static char ID;
    static const char * PassName; 

    RiskEvaluator(InstMapTy & inst_map, CostModel * model = NULL, 
        Profile * profile = NULL) : FunctionPass(ID), m_inst_map(inst_map), 
        cost_model(model), profile(profile), LI(NULL), SE(NULL) 
    {
      memset(AllRiskStat, 0, sizeof(AllRiskStat));
      memset(FuncRiskStat, 0, sizeof(FuncRiskStat));
    }

    virtual const char *getPassName() const { return PassName;}

    virtual bool runOnFunction(Function &F); 

    RiskLevel assess(Instruction *I, std::map<Loop *, unsigned> & LoopDepthMap);

    unsigned getLoopDepth(Loop *L, std::map<Loop *, unsigned> & LoopDepthMap);

    bool inhot(Instruction *I, std::map<Loop *, unsigned> & LoopDepthMap);

    bool expensive(Instruction *I);

    const char * toRiskStr(RiskLevel risk);
  
    void clearFuncStat();
    void statFuncRisk(const char * funcname);
    void statAllRisk();

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
      AU.addRequired<LoopInfo>();
      AU.addRequired<ScalarEvolution>(); 
    }
};

} // End of llvm namespace


#endif /* __EVALUATOR_H_ */
