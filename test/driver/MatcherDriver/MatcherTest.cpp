//===- MatcherTest.cpp - A pass the extract all debug info for CU, Function, BB, etc. ---------------===//
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

#include <vector>
#include <deque>

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

#include "mapper/Handy.h"
#include "mapper/PatchDecoder.h"
#include "mapper/Matcher.h"

using namespace llvm;

static cl::opt<std::string>  ClScopeListFile("diffout",
       cl::desc("File containing the parsed result of diff"
                ), cl::Hidden);

static cl::opt<int>  StripLen("strips",
       cl::desc("Levels to strips of path in debug info "
                ));

namespace {

  class MatcherTest: public ModulePass {

    public:

      Matcher matcher;

      static char ID; // Pass identification, replacement for typeid
      MatcherTest() : ModulePass(ID) {}


      BasicBlock * matchBlock(Function *func, Scope &scope)
      {
        return NULL;
      }

      void match(unsigned long begin, unsigned long end)
      {
        Scope scope(begin, end);
        Function * f;
        int s = 0;
        errs() << "[" << begin << "," << end << "] touch {";
        // do not specify the CU name so that it will match 
        // the first CU
        Matcher::sp_iterator I = matcher.initMatch("");
        while ((f = matcher.matchFunction(I, scope)) != NULL ) {
          s++;
          errs() << "scope #" << s << ": " << f->getName() << " |=> " << scope << ", ";
        }
        if (s == 0) {
          errs() << "insignificant scope";
        }
        errs() << "}\n";
      }

      void testMatching(Module &M)
      {
        matcher.process(M);
        match(1, 3);
        match(1, 38);
        match(1, 48); 
        match(7, 24);
        match(7, 29);
        match(18, 32);
        match(24, 26);
        match(31, 37);
        match(32, 35);
        match(33, 35);
        match(37, 40);
        match(43, 48); 
      }


      void processLoops(Function *F)
      {
        LoopInfo &li = getAnalysis<LoopInfo>(*F);
        matcher.processLoops(li);
      }

      virtual bool runOnModule(Module &M) 
      {
        testMatching(M);
        return false;
      }

      virtual void getAnalysisUsage(AnalysisUsage &AU) const {
        AU.setPreservesAll();
      }
      virtual void print(raw_ostream &O, const Module *M) const {
        O << "This is MatcherTest !\n";
      }
  };

}

char MatcherTest::ID = 0;
static RegisterPass<MatcherTest> X("matchtest", "Test Matcher Pass");
