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

#include "commons/handy.h"
#include "parser/PatchDecoder.h"
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


      static char ID; // Pass identification, replacement for typeid
      MatcherTest() : ModulePass(ID) {}


      BasicBlock * matchBlock(Function *func, Scope &scope)
      {
        return NULL;
      }

      void match(Matcher & matcher, unsigned long begin, unsigned long end)
      {
        Scope scope(begin, end);
        Function * f;
        int s = 0;
        errs() << "[" << begin << "," << end << "] ";
        // do not specify the CU name so that it will match 
        // the first CU
        Matcher::sp_iterator I = matcher.resetTarget("");
        if (I == matcher.sp_end())
          return;
        errs() << "touch {";
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
        Matcher matcher(M);
        matcher.process(M);
        match(matcher, 1, 3);
        match(matcher, 1, 38);
        match(matcher, 1, 48); 
        match(matcher, 7, 24);
        match(matcher, 7, 29);
        match(matcher, 18, 32);
        match(matcher, 24, 26);
        match(matcher, 31, 37);
        match(matcher, 32, 35);
        match(matcher, 33, 35);
        match(matcher, 37, 40);
        match(matcher, 43, 48); 
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
