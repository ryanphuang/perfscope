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
#include "llvm/Metadata.h"
#include "llvm/Module.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/Constants.h"

using namespace llvm;

namespace {
    struct MDMap: public ModulePass {
        static char ID; // Pass identification, replacement for typeid
        MDMap() : ModulePass(ID) {}

        virtual bool runOnModule(Module &M) {
            if (NamedMDNode *CU_Nodes = M.getNamedMetadata("llvm.dbg.cu"))
                for (unsigned i = 0, e = CU_Nodes->getNumOperands(); i != e; ++i) {
                    DICompileUnit DICU(CU_Nodes->getOperand(i));
                    errs() << "CU: " << DICU.getDirectory() << "/" << DICU.getFilename() << "\n";
                    if (DICU.getVersion() > LLVMDebugVersion10) {
                        DIArray SPs = DICU.getSubprograms();
                        for (unsigned i = 0, e = SPs.getNumElements(); i != e; i++) {
                            DISubprogram DIS(SPs.getElement(i));
                            errs() << "SP@" << DIS.getLineNumber() << ": " << 
                                DIS.getLinkageName() << "(" << DIS.getName() << ") \n";
                        }
                    }
                }

            if (NamedMDNode *NMD = M.getNamedMetadata("llvm.dbg.sp"))
                for (unsigned i = 0, e = NMD->getNumOperands(); i != e; ++i) {
                    DISubprogram DIS(NMD->getOperand(i));
                    errs() << "DIS: " << DIS.getName() << ", " << DIS.getDisplayName() << "\n";
                }
            return false;
        }
    };
}

char MDMap::ID = 0;
static RegisterPass<MDMap> X("mdmap", "Metadata Map Pass");
