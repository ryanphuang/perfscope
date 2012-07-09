//===- MDMap.cpp - A pass the extract all debug info for CU, Function, BB, etc. ---------------===//
//
//                     PerfScope Mapper
//
//      Author: Ryan Huang <ryanhuang@cs.ucsd.edu>
//
//
//===----------------------------------------------------------------------===//
//
//      This file implements a pass that intends to extract all metadata debug information
//      in a .bc file: CU, function, BB, etc.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "hello"
#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"

#include "llvm/Analysis/DebugInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Metadata.h"
#include "llvm/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/Support/CFG.h"
#include "llvm/Constants.h"
#include "llvm/Instruction.h"
#include "llvm/InstrTypes.h"

#include "PatchDecoder.h"
#include "Matcher.h"

#include <vector>
#include <deque>

using namespace llvm;

static cl::opt<std::string>  ClScopeListFile("md-scopelist",
       cl::desc("File containing the list of scopes to analyzed"
                ), cl::Hidden);

namespace {
    
    class MDMap: public ModulePass {

      public:

        Matcher matcher;

        static char ID; // Pass identification, replacement for typeid
        MDMap() : ModulePass(ID) {}
        

        BasicBlock * matchBlock(Function *func, Scope &scope)
        {
            return NULL;
        }

        void testMatching(unsigned long begin, unsigned long end)
        {
            Scope scope(begin, end);
            Function * f;
            int s = 0;
            errs() << "[" << begin << "," << end << "] might touch ";
            ScopeInfoFinder::sp_iterator I = matcher.initMatch("");
            while ((f = matcher.matchFunction(I, scope)) != NULL ) {
                s++;
                errs() << "scope #" << s << ": " << f->getName() << " |=> " << scope << ", ";
            }
            if (s == 0) {
                errs() << "insignificant scope";
            }
            errs() << "\n";
        }

        void testDriver()
        {
            testMatching(1, 3);
            testMatching(7, 24);
            testMatching(7, 29);
            testMatching(24, 26);
            testMatching(1, 38);
            testMatching(37, 40);

            // actually we don't know the end of last function, so the result will be last function
            testMatching(40, 45); 
        }


        void processLoops(Function *F)
        {
            LoopInfo &li = getAnalysis<LoopInfo>(*F);
            matcher.getFinder().processLoops(li);
        }

        void processDomTree(Function *F)
        {
            /** Getting Dominator Tree **/
            DominatorTree & DT = getAnalysis<DominatorTree>(*F);
            DT.print(errs()); // DFS print
        }

        virtual bool runOnModule(Module &M) 
        {
            matcher.init(M);

            //Finder.processModule(M);
            //processSubprograms(M);

            errs() << ClScopeListFile << "\n";
            for (Module::iterator I = M.begin(), E = M.end(); I != E; I++) {
                if (skipFunction(I))
                    continue;
                //processLoops(I);
                //succTraversal(I);
                //processInst(I);
                errs() << "Function: " << I->getName() << "\n";
                //if (I->getName().equals("_ZL23test_if_skip_sort_orderP13st_join_tableP8st_ordermb") || 
                //    I->getName().equals("_ZL23remove_dup_with_compareP3THDP8st_tablePP5FieldmP4Item")) {
                    //processBasicBlock(I);
                    //processDomTree(I);
                    //preTraversal(I);
                    processLoops(I);
                //}
            }
            return false;
        }

        virtual void getAnalysisUsage(AnalysisUsage &AU) const {
            AU.setPreservesAll();
            AU.addRequired<DominatorTree>();
            AU.addRequired<LoopInfo>();
        }
    };

}

char MDMap::ID = 0;
static RegisterPass<MDMap> X("mdmap", "Metadata Map Pass");
