/**
 *  @file          TestCostModel.cpp
 *
 *  @version       1.0
 *  @created       02/07/2013 02:35:16 PM
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
 *  Test driver for CostModel
 *
 */

#include <string>

#include "llvm/LLVMContext.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"

#include "llvm/ADT/DepthFirstIterator.h"

#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include "llvm/Target/TargetLowering.h"

#include "llvm/Support/CFG.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/IRReader.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"

#include "analyzer/X86CostModel.h"

using namespace llvm;

X86CostModel * XCM = NULL;

struct CostModelDriver : public FunctionPass {
  static char ID;
  std::string PassName;

  CostModelDriver () : FunctionPass(ID) {
    PassName = "Cost Model Driver: ";
  }

  virtual bool runOnFunction(Function &F) {
    assert(XCM && "Cost model cannot be NULL");
    errs() << F.getNameStr() << "\n";
    unsigned cost = XCM->getFunctionCost(&F);
    errs() << "Cost: " << cost << "\n";
    return false;
  }

  virtual const char *getPassName() const { return PassName.c_str(); }

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
  }
};

char CostModelDriver::ID = 0;

int main(int argc, char **argv)
{
  if (argc <= 1) {
    errs() << "Usage: costmodel INPUT\n";
    exit(1);
  }

  const std::string TripleStr = "x86_64-unknown-linux-gnu";
  const std::string FeatureStr = "";
  const std::string CPUStr = llvm::sys::getHostCPUName();
  errs() << "CPU String is " << CPUStr << "\n";
  std::string Err;
  const Target* T;
  // specially call for X86 target for general target, use:
  // InitializeAllTargets();
  // InitializeAllTargetMCs();
  // InitializeAllAsmPrinters();
  // InitializeAllAsmParsers();

  LLVMInitializeX86TargetInfo();
  LLVMInitializeX86Target();
  LLVMInitializeX86TargetMC();
  LLVMInitializeX86AsmPrinter();
  LLVMInitializeX86AsmParser();

  T = TargetRegistry::lookupTarget(TripleStr, Err);
  if(!Err.empty()) {
    errs() << "Cannot find target: " << Err << "\n";
    exit(1);
  }
  // Create TargetMachine
  TargetMachine* TM = T->createTargetMachine(TripleStr, CPUStr, FeatureStr);
  if(TM == NULL) {
    errs() << "Cannot create target machine\n";
    exit(1);
  }

  XCM = new X86CostModel(TM);

  llvm_shutdown_obj Y;  // Call llvm_shutdown() on exit.
  LLVMContext &Context = getGlobalContext();

  // Load the input module...
  const std::string input(argv[1]);
  SMDiagnostic diag;
  Module * M = ParseIRFile(input, diag, Context);
  if (M == NULL) {
    errs() << "Invalid module format\n";
    exit(1);
  }

  // Build up all of the passes that we want to do to the module.
  PassManager PM;

  // Add the target data from the target machine, if it exists, or the module.
  if (const TargetData *TD = TM->getTargetData())
    PM.add(new TargetData(*TD));
  PM.add(new CostModelDriver()); 
  PM.run(*M);
  return 0;
}


// static RegisterPass<CostModelDriver> X("costmodel", "cost model driver");
