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

#include "Handy.h"
#include "PatchDecoder.h"
#include "Matcher.h"

#include <vector>
#include <deque>

using namespace llvm;

static cl::opt<std::string>  ClScopeListFile("diffout",
       cl::desc("File containing the parsed result of diff"
                ), cl::Hidden);

static cl::opt<int>  StripLen("strips",
       cl::desc("Levels to strips of path in debug info "
                ));

namespace {
    
    static bool DEBUG = false;

    class MDMap: public ModulePass {

      public:

        Matcher matcher;

        static char ID; // Pass identification, replacement for typeid
        MDMap() : ModulePass(ID) {}
        

        BasicBlock * matchBlock(Function *func, Scope &scope)
        {
            return NULL;
        }

        void match(unsigned long begin, unsigned long end)
        {
            Scope scope(begin, end);
            Function * f;
            int s = 0;
            errs() << "[" << begin << "," << end << "] might touch ";
            ScopeInfoFinder::sp_iterator I = matcher.initMatch("a.c");
            while ((f = matcher.matchFunction(I, scope)) != NULL ) {
                s++;
                errs() << "scope #" << s << ": " << f->getName() << " |=> " << scope << ", ";
            }
            if (s == 0) {
                errs() << "insignificant scope";
            }
            errs() << "\n";
        }

        void testMatching(Module &M)
        {
            matcher.process(M);
            matcher.setstrips(0, 6); // Set the strips of path in the debug info
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
            matcher.getFinder().processLoops(li);
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
            matcher.setstrips(0, 6); // Set the strips of path in the debug info

            //Finder.processModule(M);
            //processSubprograms(M);
            //errs() << ClScopeListFile << "\n";

            PatchDecoder * decoder = new PatchDecoder(ClScopeListFile);
            assert(decoder);
            Patch *patch = NULL;
            Chapter *chap = NULL;
            Hunk * hunk = NULL;
            Mod * mod = NULL;
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
                        ScopeInfoFinder::sp_iterator I = matcher.initMatch(chap->fullname);
                        errs() << "[" << hunk->enclosing_scope << "] might touch ";
                        Scope tmps = hunk->enclosing_scope; // match function will modify scope.
                        Hunk::iterator HI = hunk->begin(), HE = hunk->end();
                        while ((f = matcher.matchFunction(I, tmps)) != NULL ) {
                            s++;
                            errs() << "scope #" << s << ": " << cpp_demangle(f->getName().data()) << " |=> " << 
                                    hunk->enclosing_scope << "\n";
                            /**
                            LoopInfo &li = getAnalysis<LoopInfo>(*f);
                            errs() << "\t";
                            if (li.begin() == li.end()) {
                                errs() << "loop: none" << "\n";
                            }
                            else {
                                //TODO more elegant
                                //TODO get function scope
                                //TODO loop finder no need to restart
                                Loop * loop = NULL;
                                while(HI != HE) {
                                    mod = *HI;
                                    // if there are multiple functions and this mod
                                    // crossed the current function's scope, we break
                                    // the loop
                                    if (tmps.end != 0  && mod->scope.begin > tmps.begin)
                                        break;
                                    loop = Matcher::matchLoop(li, tmps);
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
                            **/
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

            /**
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
            **/


        }

        void testScopeFinder(Module &M)
        {
            ScopeInfoFinder finder;
            finder.processSubprograms(M);
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
                O << "This is MDMap !\n";
        }
    };

}

char MDMap::ID = 0;
static RegisterPass<MDMap> X("mdmap", "Metadata Map Pass");
