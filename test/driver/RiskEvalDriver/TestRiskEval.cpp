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

#include "llvm/Transforms/Scalar.h"

#include "llvm/Support/CFG.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/IRReader.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"

#include "commons/LLVMHelper.h"
#include "analyzer/X86CostModel.h"
#include "analyzer/Evaluator.h"

using namespace llvm;

X86CostModel * XCM = NULL;

struct FooPass : public FunctionPass {
    static char ID;
    std::string PassName;

    FooPass() : FunctionPass(ID) {
      PassName = "Foo Function Pass: ";
    }

    virtual bool runOnFunction(Function &F) {
      errs() << "Func: " << F.getName() << "\n";
      return false;
    }

    virtual const char *getPassName() const { return PassName.c_str(); }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
    }
};

char FooPass::ID = 0;

int main(int argc, char **argv)
{
  if (argc <= 2) {
    errs() << "Usage: riskeval INPUT FUNCTION\n";
    exit(1);
  }

  TargetMachine* TM = getTargetMachine();
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

  PassRegistry &Registry = *PassRegistry::getPassRegistry();
  initPassRegistry(Registry);

  Function *func = M->getFunction(argv[2]);
  if (func == NULL) {
    errs() << "Unable to find function '" << argv[2] << "'\n";
    exit(1);
  }

  OwningPtr<FunctionPassManager> FPasses;
  FPasses.reset(new FunctionPassManager(M));
  FPasses->add(createPromoteMemoryToRegisterPass());
  FPasses->doInitialization();
  FPasses->run(*func);
  FPasses->doFinalization();

  RiskEvaluator::InstMapTy map;
  for (inst_iterator II = inst_begin(func), IE = inst_end(func); II != IE; ++II) {
    Instruction * inst = &*II;
    map[func].push_back(inst);
  }
  FPasses.reset(new FunctionPassManager(M));
  FPasses->add(new RiskEvaluator(map));
  FPasses->doInitialization();
  FPasses->run(*func);
  FPasses->doFinalization();
  return 0;
}
