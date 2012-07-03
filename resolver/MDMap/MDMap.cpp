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

    class MetadataElement {
      public:
        unsigned key;
        DIDescriptor value;
    };

    class MetadataNode {
      public:
        MetadataNode *parent;
        MetadataNode *left;
        MetadataNode *right;
        MetadataElement Element;
    };
    
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

        virtual bool runOnModule(Module &M) {
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
            **/
            if (NamedMDNode *NMD = M.getNamedMetadata("llvm.dbg.sp"))
                for (unsigned i = 0, e = NMD->getNumOperands(); i != e; ++i) {
                    DISubprogram DIS(NMD->getOperand(i));
                    errs() << "DIS: " << DIS.getName() << ", " << DIS.getDisplayName() << "\n";
                }

            for (Module::iterator I = M.begin(), E = M.end(); I != E; I++) {
                if (I->getName().startswith("llvm.dbg")) { // Skip intrinsic functions
                    continue;
                }
                errs() << "Function: " << I->getName() << "\n";
                //** Getting Dominator Tree
                if (!I->isDeclaration()) { // Dominator Tree only works with function definition, not declaration.
                  DominatorTree & DT = getAnalysis<DominatorTree>(*I);
                  //DT.print(errs()); // DFS print


                  /** BFS Traversal **/
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
                //**/
               
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
            }


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
