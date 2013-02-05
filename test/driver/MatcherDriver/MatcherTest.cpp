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

  static bool DEBUG = false;

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

      void processDomTree(Function *F)
      {
        /** Getting Dominator Tree **/
        DominatorTree & DT = getAnalysis<DominatorTree>(*F);
        DT.print(errs()); // DFS print
      }


      void testAnalyzer(Module &M)
      {

        matcher.process(M);
        matcher.setstrips(0, 7); // Set the strips of path in the debug info

        PatchDecoder * decoder = new PatchDecoder(ClScopeListFile);
        assert(decoder);
        Patch *patch = NULL;
        Chapter *chap = NULL;
        Hunk * hunk = NULL;
        Scope ls;
        while((patch = decoder->next_patch()) != NULL) {
          if (DEBUG)
            errs() << "patch: " << patch->patchname << "\n";
          while((chap = patch->next_chapter()) != NULL) {
            if (DEBUG)
              errs() << "chapter: " << chap->filename << "\n";
            while((hunk = chap->next_hunk()) != NULL) {
              if (DEBUG) {
                errs() << "hunk: " << hunk->start_line << "\n";
                errs() << hunk->ctrlseq << "\n";
              }
              assert(hunk->reduce());
              //match(chap->fullname, matcher, hunk->enclosing_scope);
              Function * f;
              int s = 0;
              Matcher::sp_iterator I = matcher.initMatch(chap->fullname);
              errs() << "[" << hunk->enclosing_scope << "] might touch ";
              Scope tmps = hunk->enclosing_scope; // match function will modify scope.
              Hunk::iterator HI = hunk->begin(), HE = hunk->end();
              while ((f = matcher.matchFunction(I, tmps)) != NULL ) {

                // The enclosing scope is a rough estimation:
                // We need to rely on the actual modification

                // Hunk: [  (..M1..)       (..M2..)  (..M3..) ]
                //                   {f1}

                // Skip the modifications didn't reach function's beginning
                while(HI != HE && (*HI)->scope.end < I->linenumber)
                  HI++;

                // no need to test the loop scope
                if (HI == HE || (*HI)->scope.begin > I->lastline)
                  continue;

                s++;
                const char *dname = cpp_demangle(I->name.c_str());
                errs() << "scope #" << s << ": " << dname;
                errs()  << " |=> " << hunk->enclosing_scope << "\n";
                LoopInfo &li = getAnalysis<LoopInfo>(*f);
                errs() << "\t";
                if (li.begin() == li.end()) {
                  errs() << "loop: none" << "\n";
                  continue;
                }

                // Only look into overlapping modifications when
                // there's loop inside this function.

                //TODO more elegant
                //TODO loop finder no need to restart
                Loop * loop = NULL;
                while(HI != HE && (*HI)->scope.begin <= I->lastline) {
                  // Will only match the *fist* in top level
                  // and innermost nested matching loop.
                  // FIXME may need to get all the loops.
                  loop = Matcher::matchLoop(li, (*HI)->scope);
                  if (loop != NULL) {
                    ScopeInfoFinder::getLoopScope(ls, loop);
                    errs() << "loop: " << ls;
                  }
                  HI++;
                }
                if (loop == NULL)
                  errs() << "loop: none" << "\n";
                else
                  errs() << "\n";
              }
              if (s == 0) {
                errs() << "insignificant scope";
              }
              errs() << "\n";
              /**
                for (Hunk::iterator I = hunk->begin(), E = hunk->end();
                I != E; I++) {
                mod = *I;
                assert(mod);
                if (DEBUG)
                cout << "mod: " << *mod << endl;
                }
               **/
            }
          }
        }

      }

      virtual bool runOnModule(Module &M) 
      {
        testMatching(M);
        return false;
      }

      virtual void getAnalysisUsage(AnalysisUsage &AU) const {
        AU.setPreservesAll();
        //AU.addRequired<DominatorTree>();
        AU.addRequired<LoopInfo>();
      }
      virtual void print(raw_ostream &O, const Module *M) const {
        O << "This is MatcherTest !\n";
      }
  };

}

char MatcherTest::ID = 0;
static RegisterPass<MatcherTest> X("matchtest", "Test Matcher Pass");
