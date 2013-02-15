/**
 *  @file          TestDummy.cpp
 *
 *  @version       1.0
 *  @created       02/14/2013 04:40:04 PM
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
 *  A scratch pad for temporary testing
 *
 */
#include "llvm/Pass.h"
#include "llvm/DefaultPasses.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

struct LoopCountPass : public FunctionPass {
    static char ID;
    std::string PassName;

    LoopCountPass() : FunctionPass(ID) {
      PassName = "Loop Count Function Pass: ";
    }

    virtual bool runOnFunction(Function &F) {
      errs() << "Func: " << F.getName() << "\n";
      LoopInfo & li = getAnalysis<LoopInfo>(); 
      ScalarEvolution & se = getAnalysis<ScalarEvolution>(); 
      for (LoopInfo::iterator LII = li.begin(),  LIE = li.end(); LII != LIE; ++LII) {
        Loop * loop = *LII;
        errs() << "Top level loop '" << loop->getHeader()->getName() << "', count: ";
        SmallVector<BasicBlock *, 4> exits;
        loop->getExitingBlocks(exits);
        unsigned count = 0;
        for (SmallVector<BasicBlock *, 4>::iterator ei = exits.begin(), ee = exits.end();
          ei != ee; ++ei) {
          if (*ei) {
            unsigned c = se.getSmallConstantTripCount(loop, *ei); 
            if (c > count)
              count = c;
          }
        }
        errs() << count << "\n";
      }
      return false;
    }

    virtual const char *getPassName() const { return PassName.c_str(); }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
      AU.addRequired<LoopInfo>();
      AU.addRequired<ScalarEvolution>(); 
    }
};

}

char LoopCountPass::ID = 0;
static RegisterPass<LoopCountPass> X("tripcount", "Print trip count of loops in function");


