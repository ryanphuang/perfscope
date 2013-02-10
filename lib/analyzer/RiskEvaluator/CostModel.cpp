/**
 *  @file          CostModel.cpp
 *
 *  @version       1.0
 *  @created       02/07/2013 12:10:08 AM
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
 *  CostModel implementation
 *
 */

#include "llvm/Support/raw_ostream.h"
#include "llvm/IntrinsicInst.h"

#include "analyzer/CostModel.h"

using namespace llvm;

unsigned CostModel::getOperationCost(unsigned Opcode, Type *Ty, Type *OpTy) const 
{
  llvm_unreachable("Unimplemented!");
}

unsigned CostModel::getGEPCost(const Value *Ptr,
    ArrayRef<const Value *> Operands) const 
{
  // In the basic model, we just assume that all-constant GEPs will be folded
  // into their uses via addressing modes.
  for (unsigned Idx = 0, Size = Operands.size(); Idx != Size; ++Idx)
    if (!isa<Constant>(Operands[Idx]))
      return TCC_Basic;

  return TCC_Free;
}

unsigned CostModel::getCallCost(FunctionType *FTy, int NumArgs) const 
{
  assert(FTy && "FunctionType must be provided to this routine.");

  // The target-independent implementation just measures the size of the
  // function by approximating that each argument will take on average one
  // instruction to prepare.

  if (NumArgs < 0)
    // Set the argument number to the number of explicit arguments in the
    // function.
    NumArgs = FTy->getNumParams();

  return TCC_Basic * (NumArgs + 1);
}

unsigned CostModel::getCallCost(const Function *F, int NumArgs) const 
{
  llvm_unreachable("Unimplemented!");
}

unsigned CostModel::getCallCost(const Function *F,
    ArrayRef<const Value *> Arguments) const 
{
  llvm_unreachable("Unimplemented!");
}

unsigned CostModel::getIntrinsicCost(Intrinsic::ID IID, Type *RetTy,
    ArrayRef<Type *> ParamTys) const 
{
  switch (IID) {
    default:
      // Intrinsics rarely (if ever) have normal argument setup constraints.
      // Model them as having a basic instruction cost.
      // FIXME: This is wrong for libc intrinsics.
      return TCC_Basic;

    case Intrinsic::dbg_declare:
    case Intrinsic::dbg_value:
    case Intrinsic::invariant_start:
    case Intrinsic::invariant_end:
    case Intrinsic::lifetime_start:
    case Intrinsic::lifetime_end:
    case Intrinsic::objectsize:
    case Intrinsic::ptr_annotation:
    case Intrinsic::var_annotation:
      // These intrinsics don't actually represent code after lowering.
      return TCC_Free;
  }
}

unsigned CostModel::getIntrinsicCost(Intrinsic::ID IID, Type *RetTy,
    ArrayRef<const Value *> Arguments) const 
{
  llvm_unreachable("Unimplemented!");
}

unsigned CostModel::getUserCost(const User *U) const 
{
  llvm_unreachable("Unimplemented!");
}

unsigned CostModel::getArithmeticInstrCost(unsigned Opcode, Type *Ty) const 
{
  return TCC_Basic;
}

unsigned CostModel::getShuffleCost(ShuffleKind Kind, Type *Tp, int Index, 
  Type *SubTp ) const 
{
  return TCC_Basic;
}

unsigned CostModel::getCastInstrCost(unsigned Opcode, Type *Dst, Type *Src) const 
{
  return TCC_Basic;
}

unsigned CostModel::getCFInstrCost(unsigned Opcode) const 
{
  return TCC_Basic;
}

unsigned CostModel::getCmpSelInstrCost(unsigned Opcode, Type *ValTy, Type *CondTy) const 
{
  return TCC_Basic;
}

unsigned CostModel::getVectorInstrCost(unsigned Opcode, Type *Val, unsigned Index ) const 
{
  return TCC_Basic;
}

unsigned CostModel::getMemoryOpCost(unsigned Opcode, Type *Src, unsigned Alignment,
  unsigned AddressSpace) const 
{
  return TCC_Basic;
}

unsigned CostModel::getIntrinsicInstrCost(Intrinsic::ID ID, Type *RetTy, 
  ArrayRef<Type*> Tys) const 
{
  return TCC_Basic;
}

unsigned CostModel::getInstructionCost(Instruction *I) const
{
  if (isa<IntrinsicInst>(I)) {
    return TCC_Free; // simply ignore intrinsic instructions
  }
  switch (I->getOpcode()) {
    case Instruction::Ret:
    case Instruction::PHI:
    case Instruction::Br: {
      return getCFInstrCost(I->getOpcode());
    }
    case Instruction::Add:
    case Instruction::FAdd:
    case Instruction::Sub:
    case Instruction::FSub:
    case Instruction::Mul:
    case Instruction::FMul:
    case Instruction::UDiv:
    case Instruction::SDiv:
    case Instruction::FDiv:
    case Instruction::URem:
    case Instruction::SRem:
    case Instruction::FRem:
    case Instruction::Shl:
    case Instruction::LShr:
    case Instruction::AShr:
    case Instruction::And:
    case Instruction::Or:
    case Instruction::Xor: {
      return getArithmeticInstrCost(I->getOpcode(), I->getType());
    }
    case Instruction::Select: {
      SelectInst *SI = cast<SelectInst>(I);
      Type *CondTy = SI->getCondition()->getType();
      return getCmpSelInstrCost(I->getOpcode(), I->getType(), CondTy);
    }
    case Instruction::ICmp:
    case Instruction::FCmp: {
      Type *ValTy = I->getOperand(0)->getType();
      return getCmpSelInstrCost(I->getOpcode(), ValTy);
    }
    case Instruction::Alloca: {
      // For now, we assume alloca is free
      return 0;
    }
    case Instruction::Store: {
      StoreInst *SI = cast<StoreInst>(I);
      Type *ValTy = SI->getValueOperand()->getType();
      return getMemoryOpCost(I->getOpcode(), ValTy,
                                   SI->getAlignment(),
                                   SI->getPointerAddressSpace());
    }
    case Instruction::Load: {
      LoadInst *LI = cast<LoadInst>(I);
      return getMemoryOpCost(I->getOpcode(), I->getType(),
                                  LI->getAlignment(),
                                  LI->getPointerAddressSpace());
    }
    case Instruction::ZExt:
    case Instruction::SExt:
    case Instruction::FPToUI:
    case Instruction::FPToSI:
    case Instruction::FPExt:
    case Instruction::PtrToInt:
    case Instruction::IntToPtr:
    case Instruction::SIToFP:
    case Instruction::UIToFP:
    case Instruction::Trunc:
    case Instruction::FPTrunc:
    case Instruction::BitCast: {
      Type *SrcTy = I->getOperand(0)->getType();
      return getCastInstrCost(I->getOpcode(), I->getType(), SrcTy);
    }
    case Instruction::ExtractElement: {
      ExtractElementInst * EEI = cast<ExtractElementInst>(I);
      ConstantInt *CI = dyn_cast<ConstantInt>(I->getOperand(1));
      unsigned Idx = -1;
      if (CI)
        Idx = CI->getZExtValue();
      return getVectorInstrCost(I->getOpcode(),
      EEI->getOperand(0)->getType(), Idx);
    }
    case Instruction::InsertElement: {
      InsertElementInst * IE = cast<InsertElementInst>(I);
      ConstantInt *CI = dyn_cast<ConstantInt>(IE->getOperand(2));
      unsigned Idx = -1;
      if (CI)
        Idx = CI->getZExtValue();
      return getVectorInstrCost(I->getOpcode(),
                                      IE->getType(), Idx);
    }
    case Instruction::Call: {
      // The target-independent implementation just measures the size of the
      // function by approximating that each argument will take on average one
      // instruction to prepare.
      CallInst * CI = dyn_cast<CallInst>(I);
      return TCC_Basic * (CI->getNumArgOperands() + 1);
    }
    default:
      errs() << "Warning: unknown cost for instruction " << I->getOpcode() << "\n";
      return -1;
  }
}

unsigned CostModel::getBasicBlockCost(const BasicBlock *BB) const
{
  return TCC_Basic;
}

unsigned CostModel::getFunctionCost(const Function *F) const
{
  return TCC_Basic;

}
