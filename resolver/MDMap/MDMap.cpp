//===- MDMap.cpp - A pass the extract all debug info for CU, Function, BB, etc. ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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
#include "llvm/Analysis/Dominators.h"
#include "llvm/Metadata.h"
#include "llvm/Module.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/Support/CFG.h"
#include "llvm/Constants.h"
#include "llvm/Instruction.h"
#include "llvm/InstrTypes.h"

#include <vector>
#include <deque>

using namespace llvm;

namespace {
    
    typedef DomTreeNodeBase<BasicBlock> BBNode;

    template <class T1, class T2> struct Pair
    {
      typedef T1 first_type;
      typedef T2 second_type;

      T1 first;
      T2 second;

      Pair() : first(T1()), second(T2()) {}
      Pair(const T1& x, const T2& y) : first(x), second(y) {}
      template <class U, class V> Pair (const Pair<U,V> &p) : first(p.first), second(p.second) { }
    };

    typedef struct Scope {
        unsigned long begin;
        unsigned long end;
        Scope(unsigned long b, unsigned long e) : begin(b), end(e) {}
        Scope(const Scope &another) : begin(another.begin), end(another.end) {}
    } Scope;

    typedef struct MetadataElement {
        unsigned key;
        DIDescriptor value;
    } MetadataElement;
   
    typedef struct MetadataNode {
        MetadataNode *parent;
        MetadataNode *left;
        MetadataNode *right;
        MetadataElement Element;
    } MetadataNode;
    
    class MetadataTree {
      public:
        MetadataNode *root;
        void insert(MetadataNode *);
        MetadataNode *search(unsigned key);
    };

    class MySP {
      public:
        DISubprogram SP;

        MySP(DISubprogram p) : SP(p) {}
        bool operator < (const MySP & another) const 
        { 
            return SP.getLineNumber() < another.SP.getLineNumber();
        }
    };

    class MDMap: public ModulePass {

        DebugInfoFinder Finder;
        std::vector<MySP> MySPs;

      public:
        static char ID; // Pass identification, replacement for typeid
        MDMap() : ModulePass(ID) {}
        
        /** A progressive method to match the function(s) in a given scope.
         *  When there are more than one function in the scope, the first function
         *  will be returned and scope's beginning is *modified* to the end of this 
         *  returned function so that the caller could perform a loop of call until
         *  matchFunction return NULL;
         *
         *
         *  Note: finder.processModule(M) should be called before the first call of matchFunction.
         *
         * **/
        Function * matchFunction(DebugInfoFinder::iterator I, Scope &scope)
        {
            // hit the boundary
            if (scope.begin == 0 || scope.end == 0 || scope.end < scope.begin)
                return NULL;
            /** Off-the-shelf SP finder **/
            unsigned long e;
            Function *f1 = NULL, *f2 = NULL;
            DebugInfoFinder::iterator E;
            
            if (I == NULL) {
                I = Finder.subprogram_begin();
            }
            for (E = Finder.subprogram_end(); I && I != E; I++) {
                DISubprogram DIS(*I);
                e = DIS.getLineNumber();
                f1 = f2;
                f2 = DIS.getFunction();
                if (scope.begin < e) {
                  if (f1 == NULL) { 
                    // boundary case, the modification begins before the first function
                    // we need to adjust the beginning to the first function and let the
                    // iteration continue
                    scope.begin = e; 
                  }
                  else {
                    break;
                  }
                }
            }

            /*****
             *
             * typical case #1:
             *
             * | foo1
             * | foo2   <- b, f1
             * | [scope.begin
             * | scope.end]
             * | foo3   <- e, f2
             * | foo4
             * # scope is modifyint foo2;
             *
             * typical case #2:
             *
             * | foo1
             * | foo2   <- b, f1
             * | [scope.begin
             * | foo3   <- e, f2
             * | scope.end]
             * | foo4
             * # scope is modifyint foo2 & foo3;
             *
             * boundary case 1:
             *
             * | 0      <- b, f1
             * | [scope.begin
             * | scope.end]
             * | foo1   <- e, f2 # advance scope.begin
             * | foo2
             *
             * boundary case 2:
             * | 0      <- b, f1
             * | [scope.begin
             * | foo1   <- e, f2 # advance scope.begin
             * | foo2
             * | scope.end]
             * | foo3
             * | foo4
             *
             * boundary case 3:
             * | 0      <- b, f1
             * | [scope.begin
             * | foo1   <- e, f2 # advance scope.begin, adjust f1 to f2
             * | scope.end]
             *
             * boundary case 4:
             * | 0
             * | foo1   <- b, f1   
             * | foo2   <- e, f2 # adjust f1 to f2
             * | [scope.begin
             * | scope.end]
             *
             *****/

            if (scope.end <= scope.begin) { // we over-advanced scope.begin, boundary case #1
                return NULL; 
            }

            if (I == E) { 
                // we've come to the end instead of jumping out from break, 
                // need to adjust f1 to f2, boundary case #3, #4
                // also, make sure we finish the matching
                scope.end = 0; 
                return f2;
            }

            if (scope.end > e) { // span multiple functions
                scope.begin = e; // boundary case #2
            }
            else { // span at most one function 
                scope.end = 0; // finish the matching
            }
            return f1;
        }

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
            DebugInfoFinder::iterator I = NULL;
            while ((f = matchFunction(I, scope)) != NULL ) {
                s++;
                errs() << "scope #" << s << ": " << f->getName() << " |=> [" << scope.begin << "," << scope.end << "], ";
            }
            if (s == 0) {
                errs() << "insignificant scope";
            }
            errs() << "\n";
        }


        virtual bool runOnModule(Module &M) 
        {
            Finder.processModule(M);
            testMatching(1, 3);
            testMatching(7, 24);
            testMatching(7, 29);
            testMatching(24, 26);
            testMatching(1, 38);
            testMatching(37, 40);

            // actually we don't know the end of last function, so the result will be last function
            testMatching(40, 45); 

            /** DIY SP finder **/
            /**
            if (NamedMDNode *CU_Nodes = M.getNamedMetadata("llvm.dbg.cu"))
                for (unsigned i = 0, e = CU_Nodes->getNumOperands(); i != e; ++i) {
                    DICompileUnit DICU(CU_Nodes->getOperand(i));
                    errs() << "CU: " << DICU.getDirectory() << "/" << DICU.getFilename() << "\n";
                    if (DICU.getVersion() > LLVMDebugVersion10) {
                        DIArray SPs = DICU.getSubprograms();
                        for (unsigned i = 0, e = SPs.getNumElements(); i != e; i++) {
                            DISubprogram DIS(SPs.getElement(i));
                            MySPs.push_back(MySP(DIS));
                            //errs() << "SP@" << DIS.getLineNumber() << ": " << 
                            //    DIS.getLinkageName() << "(" << DIS.getName() << ") \n";
                        }
                    }
                }

            if (NamedMDNode *NMD = M.getNamedMetadata("llvm.dbg.sp"))
                for (unsigned i = 0, e = NMD->getNumOperands(); i != e; ++i) {
                    DISubprogram DIS(NMD->getOperand(i));
                    errs() << "DIS: " << DIS.getName() << ", " << DIS.getDisplayName() << "\n";
                }
            **/

            /**
            for (Module::iterator I = M.begin(), E = M.end(); I != E; I++) {
                if (I->getName().startswith("llvm.dbg")) { // Skip intrinsic functions
                    continue;
                }
                errs() << "Function: " << I->getName() << "\n";
            **/

                /** Getting Dominator Tree **/
                /**
                if (!I->isDeclaration()) { // Dominator Tree only works with function definition, not declaration.
                  DominatorTree & DT = getAnalysis<DominatorTree>(*I);
                  //DT.print(errs()); // DFS print

                **/

                  /** BFS Traversal  **/
                  /**
                  BBNode *Node = DT.getRootNode();
                  std::deque< Pair<BBNode *, unsigned> > ques;
                  ques.push_back(Pair<BBNode *, unsigned>(Node, 1));
                  while (!ques.empty()) {
                      Pair<BBNode *, unsigned> pair = ques.front();
                      Node = pair.first;
                      ques.pop_front();
                      if (Node) {
                          BasicBlock * BB = Node->getBlock();
                          errs() << "[" << pair.second << "] " << BB->getName() << "\n";
                          for (BBNode::iterator I = Node->begin(), E = Node->end(); I != E; I++) {
                            BBNode * N = *I;
                            ques.push_back(Pair<BBNode *, unsigned>(N, pair.second + 1));
                          }
                      }
                  }
                }
                **/
               
                //for (Function::iterator FI = I->begin(), FE = I->end(); FI != FE; FI++) {
                    
                    /** Traverse the successors
                    errs() << FI->getName() << "\n";
                    for (succ_iterator SI = succ_begin(FI), SE = succ_end(FI); SI != SE; SI++) {
                        errs() << "\t" << (*SI)->getName() << "\n";
                    }
                    **/


                    /** Use first and last instruction to get the scope information
                    Instruction *first = FI->getFirstNonPHI();
                    Instruction *last = FI->getTerminator();
                    if (first == NULL || last == NULL) {
                        errs() << "NULL scope instructions " << "\n";
                        continue;
                    }
                    DebugLoc Loc = first->getDebugLoc();
                    if (Loc.isUnknown()) {
                        errs() << "Unknown LOC information" << "\n";
                        continue;
                    }
                    errs() << "Block :" << Loc.getLine();
                    Loc = last->getDebugLoc();
                    if (Loc.isUnknown()) {
                        errs() << "Unknown LOC information" << "\n";
                        continue;
                    }
                    errs() << ", " << Loc.getLine() << "\n";
                    **/
                    
                    /** Get each instruction's scope information
                    for (BasicBlock::iterator BI = FI->begin(), BE = FI->end(); BI != BE; BI++) {
                        DebugLoc Loc = BI->getDebugLoc();
                        if (Loc.isUnknown())
                            continue;
                        LLVMContext & Ctx = BI->getContext();

                        DIDescriptor Scope(Loc.getScope(Ctx));
                        if (Scope.isLexicalBlock()) {
                           DILexicalBlock DILB(Scope);
                           errs() << "Block :" << DILB.getLineNumber() << ", " << DILB.getColumnNumber() << "\n";
                        }
                    }
                    **/
                //}
            //}


            /** Off-the-shelf SP finder
            Finder.processModule(M);
            for (DebugInfoFinder::iterator I = Finder.subprogram_begin(), E = 
                  Finder.subprogram_end(); I != E; I++) {
                DISubprogram DIS(*I);
                errs() << "@" << DIS.getLineNumber() << ": " << 
                DIS.getLinkageName() << "(" << DIS.getName() << ") \n";
            }
            **/

            /** Sort based on line number
            std::sort(MySPs.begin(), MySPs.end());
            std::vector<MySP>::iterator I, E;
            for (I = MySPs.begin(), E = MySPs.end(); I != E; I++) {
                errs() << "@" << I->SP.getLineNumber() << ": " << 
                  I->SP.getLinkageName() << "(" << I->SP.getName() << ") \n";
            }
            **/
            return false;
        }

        virtual void getAnalysisUsage(AnalysisUsage &AU) const {
            AU.setPreservesAll();
            AU.addRequired<DominatorTree>();
        }
    };

    void MetadataTree::insert(MetadataNode * node) {

    }
}

char MDMap::ID = 0;
static RegisterPass<MDMap> X("mdmap", "Metadata Map Pass");
