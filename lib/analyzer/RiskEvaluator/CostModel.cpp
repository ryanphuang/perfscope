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

#include "analyzer/CostModel.h"

using namespace llvm;

unsigned CostModel::getOperationCost(unsigned Opcode, Type *Ty, Type *OpTy) const 
{
  switch (Opcode) {
    default:
      // By default, just classify everything as 'basic'.
      return TCC_Basic;

    case Instruction::GetElementPtr:
      llvm_unreachable("Use getGEPCost for GEP operations!");

    case Instruction::BitCast:
      assert(OpTy && "Cast instructions must provide the operand type");
      if (Ty == OpTy || (Ty->isPointerTy() && OpTy->isPointerTy()))
        // Identity and pointer-to-pointer casts are free.
        return TCC_Free;

      // Otherwise, the default basic cost is used.
      return TCC_Basic;

    case Instruction::IntToPtr:
      // An inttoptr cast is free so long as the input is a legal integer type
      // which doesn't contain values outside the range of a pointer.
      llvm_unreachable("Unimplemented!");
      /*
      if (DL && DL->isLegalInteger(OpTy->getScalarSizeInBits()) &&
          OpTy->getScalarSizeInBits() <= DL->getPointerSizeInBits())
        return TCC_Free;

      // Otherwise it's not a no-op.
      return TCC_Basic;
      */

    case Instruction::PtrToInt:
      llvm_unreachable("Unimplemented!");
      // A ptrtoint cast is free so long as the result is large enough to store
      // the pointer, and a legal integer type.
      /*
      if (DL && DL->isLegalInteger(OpTy->getScalarSizeInBits()) &&
          OpTy->getScalarSizeInBits() >= DL->getPointerSizeInBits())
        return TCC_Free;

      // Otherwise it's not a no-op.
      return TCC_Basic;
      */

    case Instruction::Trunc:
      llvm_unreachable("Unimplemented!");
      /*
      // trunc to a native type is free (assuming the target has compare and
      // shift-right of the same width).
      if (DL && DL->isLegalInteger(DL->getTypeSizeInBits(Ty)))
        return TCC_Free;

      return TCC_Basic;
      */
  }
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
  assert(F && "A concrete function must be provided to this routine.");

  if (NumArgs < 0)
    // Set the argument number to the number of explicit arguments in the
    // function.
    NumArgs = F->arg_size();
  
  llvm_unreachable("Unimplemented!");

  /*
  if (Intrinsic::ID IID = (Intrinsic::ID)F->getIntrinsicID()) {
    FunctionType *FTy = F->getFunctionType();
    SmallVector<Type *, 8> ParamTys(FTy->param_begin(), FTy->param_end());
    return TopTTI->getIntrinsicCost(IID, FTy->getReturnType(), ParamTys);
  }

  if (!TopTTI->isLoweredToCall(F))
    return TCC_Basic; // Give a basic cost if it will be lowered directly.

  return TopTTI->getCallCost(F->getFunctionType(), NumArgs);
  */
}

unsigned CostModel::getCallCost(const Function *F,
    ArrayRef<const Value *> Arguments) const 
{
  // Simply delegate to generic handling of the call.
  // FIXME: We should use instsimplify or something else to catch calls which
  // will constant fold with these arguments.
  llvm_unreachable("Unimplemented!");
  // return TopTTI->getCallCost(F, Arguments.size());
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
  // Delegate to the generic intrinsic handling code. This mostly provides an
  // opportunity for targets to (for example) special case the cost of
  // certain intrinsics based on constants used as arguments.
  llvm_unreachable("Unimplemented!");
  /*
  SmallVector<Type *, 8> ParamTys;
  ParamTys.reserve(Arguments.size());
  for (unsigned Idx = 0, Size = Arguments.size(); Idx != Size; ++Idx)
    ParamTys.push_back(Arguments[Idx]->getType());
  return TopTTI->getIntrinsicCost(IID, RetTy, ParamTys);
  */
}

unsigned CostModel::getUserCost(const User *U) const 
{
  if (isa<PHINode>(U))
    return TCC_Free; // Model all PHI nodes as free.

  if (const GEPOperator *GEP = dyn_cast<GEPOperator>(U))
    // In the basic model we just assume that all-constant GEPs will be
    // folded into their uses via addressing modes.
    return GEP->hasAllConstantIndices() ? TCC_Free : TCC_Basic;

  if (ImmutableCallSite CS = U) {
    llvm_unreachable("Unimplemented!");
    /*
    const Function *F = CS.getCalledFunction();
    if (!F) {
      // Just use the called value type.
      Type *FTy = CS.getCalledValue()->getType()->getPointerElementType();
      return TopTTI->getCallCost(cast<FunctionType>(FTy), CS.arg_size());
    }

    SmallVector<const Value *, 8> Arguments;
    for (ImmutableCallSite::arg_iterator AI = CS.arg_begin(),
        AE = CS.arg_end();
        AI != AE; ++AI)
      Arguments.push_back(*AI);

    return TopTTI->getCallCost(F, Arguments);
    */
  }

  if (const CastInst *CI = dyn_cast<CastInst>(U)) {
    // Result of a cmp instruction is often extended (to be used by other
    // cmp instructions, logical or return instructions). These are usually
    // nop on most sane targets.
    if (isa<CmpInst>(CI->getOperand(0)))
      return TCC_Free;
  }

  // Otherwise delegate to the fully generic implementations.
  return getOperationCost(Operator::getOpcode(U), U->getType(),
      U->getNumOperands() == 1 ?
      U->getOperand(0)->getType() : 0);
}

unsigned CostModel::getArithmeticInstrCost(unsigned Opcode, Type *Ty) const 
{
  return 1;
}

unsigned CostModel::getShuffleCost(ShuffleKind Kind, Type *Tp, int Index, 
  Type *SubTp ) const 
{
  return 1;
}

unsigned CostModel::getCastInstrCost(unsigned Opcode, Type *Dst, Type *Src) const 
{
  return 1;
}

unsigned CostModel::getCFInstrCost(unsigned Opcode) const 
{
  return 1;
}

unsigned CostModel::getCmpSelInstrCost(unsigned Opcode, Type *ValTy, Type *CondTy) const 
{
  return 1;
}

unsigned CostModel::getVectorInstrCost(unsigned Opcode, Type *Val, unsigned Index ) const 
{
  return 1;
}

unsigned CostModel::getMemoryOpCost(unsigned Opcode, Type *Src, unsigned Alignment,
  unsigned AddressSpace) const 
{
  return 1;
}

unsigned CostModel::getIntrinsicInstrCost(Intrinsic::ID ID, Type *RetTy, 
  ArrayRef<Type*> Tys) const 
{
  return 1;
}

unsigned CostModel::getInstructionCost(Instruction *I) const
{
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
    default:
      // We don't have any information on this instruction.
      return -1;
  }
}

