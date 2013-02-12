/**
 *  @file          TestRiskEval.cpp
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
#include "llvm/Module.h"
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
#include "analyzer/Evaluator.h"

using namespace llvm;

X86CostModel * XCM = NULL;

void initPassRegistry(PassRegistry & Registry)
{
    initializeCore(Registry);
    initializeScalarOpts(Registry);
    initializeIPO(Registry);
    initializeAnalysis(Registry);
    initializeIPA(Registry);
    initializeTransformUtils(Registry);
    initializeInstCombine(Registry);
    initializeInstrumentation(Registry);
    initializeTarget(Registry);
}

int main(int argc, char **argv)
{
  if (argc <= 1) {
    errs() << "Usage: riskeval INPUT\n";
    exit(1);
  }

  const std::string TripleStr = "x86_64-unknown-linux-gnu";
  const std::string FeatureStr = "";
  const std::string CPUStr = llvm::sys::getHostCPUName();
  errs() << "CPU String is " << CPUStr << "\n";
  std::string Err;
  const Target* T;

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

  LocalRiskEvaluator::InstMapTy map;
  //TODO nasty hard code, use file for test
  for (Module::const_iterator MI = M->begin(), ME = M->end(); MI != ME; ++MI) {
    const Function * F = MI;
    if (F->getName() != "foo")
      continue;
    map[F] = LocalRiskEvaluator::InstVecTy();
    int i = 0;
    for (const_inst_iterator II = inst_begin(MI), IE = inst_end(MI); II != IE; ++II, ++i) {
      if (i < 4)
        continue;
      if (i > 10)
        break;
      const Instruction * inst = &*II;
      map[F].push_back(inst);
    }
  }
  
  PassRegistry &Registry = *PassRegistry::getPassRegistry();
  initPassRegistry(Registry);

  // Build up all of the passes that we want to do to the module.
  PassManager PM;

  // Add the target data from the target machine, if it exists, or the module.
  if (const TargetData *TD = TM->getTargetData())
    PM.add(new TargetData(*TD));
  PM.add(new LocalRiskEvaluator(map)); 
  PM.run(*M);
  return 0;
}
