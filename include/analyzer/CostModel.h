/**
 *  @file          CostModel.h
 *
 *  @version       1.0
 *  @created       02/07/2013 12:04:54 AM
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
 *  A simple Cost Model for different types of instructions.
 *
 *  In the latest LLVM, there's a provided interface to do this and 
 *  it's target-specific. But unfortunately, that interface is unstable
 *  now and the latest LLVM fails to compile big project like MySQL.
 *  So we mimic the behavior
 *
 */

#ifndef __COSTMODEL_H_
#define __COSTMODEL_H_

#include "llvm/Instructions.h"
#include "llvm/Function.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/Intrinsics.h"
#include "llvm/Operator.h"
#include "llvm/Support/CallSite.h"


namespace llvm {

class CostModel {

  public:
    /// \brief Underlying constants for 'cost' values in this interface.
    ///
    /// Many APIs in this interface return a cost. This enum defines the
    /// fundamental values that should be used to interpret (and produce) those
    /// costs. The costs are returned as an unsigned rather than a member of this
    /// enumeration because it is expected that the cost of one IR instruction
    /// may have a multiplicative factor to it or otherwise won't fit dircetly
    /// into the enum. Moreover, it is common to sum or average costs which works
    /// better as simple integral values. Thus this enum only provides constants.
    ///
    /// Note that these costs should usually reflect the intersection of code-size
    /// cost and execution cost. A free instruction is typically one that folds
    /// into another instruction. For example, reg-to-reg moves can often be
    /// skipped by renaming the registers in the CPU, but they still are encoded
    /// and thus wouldn't be considered 'free' here.

    enum TargetCostConstants {
      TCC_Free = 0,       ///< Expected to fold away in lowering.
      TCC_Basic = 1,      ///< The cost of a typical 'add' instruction.
      TCC_Expensive = 4   ///< The cost of a 'div' instruction on x86.
    };

    /// \brief Estimate the cost of a specific operation when lowered.
    ///
    /// Note that this is designed to work on an arbitrary synthetic opcode, and
    /// thus work for hypothetical queries before an instruction has even been
    /// formed. However, this does *not* work for GEPs, and must not be called
    /// for a GEP instruction. Instead, use the dedicated getGEPCost interface as
    /// analyzing a GEP's cost required more information.
    ///
    /// Typically only the result type is required, and the operand type can be
    /// omitted. However, if the opcode is one of the cast instructions, the
    /// operand type is required.
    ///
    /// The returned cost is defined in terms of \c TargetCostConstants, see its
    /// comments for a detailed explanation of the cost values.
    virtual unsigned getOperationCost(unsigned Opcode, Type *Ty,
                                      Type *OpTy = 0);

    /// \brief Estimate the cost of a GEP operation when lowered.
    ///
    /// The contract for this function is the same as \c getOperationCost except
    /// that it supports an interface that provides extra information specific to
    /// the GEP operation.
    virtual unsigned getGEPCost(const Value *Ptr,
                                ArrayRef<const Value *> Operands);

    /// \brief Estimate the cost of a function call when lowered.
    ///
    /// The contract for this is the same as \c getOperationCost except that it
    /// supports an interface that provides extra information specific to call
    /// instructions.
    ///
    /// This is the most basic query for estimating call cost: it only knows the
    /// function type and (potentially) the number of arguments at the call site.
    /// The latter is only interesting for varargs function types.
    virtual unsigned getCallCost(FunctionType *FTy, int NumArgs = -1);

    /// \brief Estimate the cost of calling a specific function when lowered.
    ///
    /// This overload adds the ability to reason about the particular function
    /// being called in the event it is a library call with special lowering.
    virtual unsigned getCallCost(const Function *F, int NumArgs = -1);

    /// \brief Estimate the cost of calling a specific function when lowered.
    ///
    /// This overload allows specifying a set of candidate argument values.
    virtual unsigned getCallCost(const Function *F,
                                 ArrayRef<const Value *> Arguments);

    /// \brief Estimate the cost of an intrinsic when lowered.
    ///
    /// Mirrors the \c getCallCost method but uses an intrinsic identifier.
    virtual unsigned getIntrinsicCost(Intrinsic::ID IID, Type *RetTy,
                                      ArrayRef<Type *> ParamTys);

    /// \brief Estimate the cost of an intrinsic when lowered.
    ///
    /// Mirrors the \c getCallCost method but uses an intrinsic identifier.
    virtual unsigned getIntrinsicCost(Intrinsic::ID IID, Type *RetTy,
                                      ArrayRef<const Value *> Arguments);

    /// \brief Estimate the cost of a given IR user when lowered.
    ///
    /// This can estimate the cost of either a ConstantExpr or Instruction when
    /// lowered. It has two primary advantages over the \c getOperationCost and
    /// \c getGEPCost above, and one significant disadvantage: it can only be
    /// used when the IR construct has already been formed.
    ///
    /// The advantages are that it can inspect the SSA use graph to reason more
    /// accurately about the cost. For example, all-constant-GEPs can often be
    /// folded into a load or other instruction, but if they are used in some
    /// other context they may not be folded. This routine can distinguish such
    /// cases.
    ///
    /// The returned cost is defined in terms of \c TargetCostConstants, see its
    /// comments for a detailed explanation of the cost values.
    virtual unsigned getUserCost(const User *U);

    /// \brief The various kinds of shuffle patterns for vector queries.
    enum ShuffleKind {
      SK_Broadcast,       ///< Broadcast element 0 to all other elements.
      SK_Reverse,         ///< Reverse the order of the vector.
      SK_InsertSubvector, ///< InsertSubvector. Index indicates start offset.
      SK_ExtractSubvector ///< ExtractSubvector Index indicates start offset.
    };

    /// \return The expected cost of arithmetic ops, such as mul, xor, fsub, etc.
    virtual unsigned getArithmeticInstrCost(unsigned Opcode, Type *Ty);

    /// \return The cost of a shuffle instruction of kind Kind and of type Tp.
    /// The index and subtype parameters are used by the subvector insertion and
    /// extraction shuffle kinds.
    virtual unsigned getShuffleCost(ShuffleKind Kind, Type *Tp, int Index = 0,
                                    Type *SubTp = 0);

    /// \return The expected cost of cast instructions, such as bitcast, trunc,
    /// zext, etc.
    virtual unsigned getCastInstrCost(unsigned Opcode, Type *Dst,
                                      Type *Src);

    /// \return The expected cost of control-flow related instrutctions such as
    /// Phi, Ret, Br.
    virtual unsigned getCFInstrCost(unsigned Opcode);

    /// \returns The expected cost of compare and select instructions.
    virtual unsigned getCmpSelInstrCost(unsigned Opcode, Type *ValTy,
                                        Type *CondTy = 0);

    /// \return The expected cost of vector Insert and Extract.
    /// Use -1 to indicate that there is no information on the index value.
    virtual unsigned getVectorInstrCost(unsigned Opcode, Type *Val,
                                        unsigned Index = -1);

    /// \return The cost of Load and Store instructions.
    virtual unsigned getMemoryOpCost(unsigned Opcode, Type *Src,
                                     unsigned Alignment,
                                     unsigned AddressSpace);

    /// \returns The cost of Intrinsic instructions.
    virtual unsigned getIntrinsicInstrCost(Intrinsic::ID ID, Type *RetTy,
                                           ArrayRef<Type *> Tys);

  public:
    //Currently only need a virtual interface
    virtual unsigned getInstructionCost(Instruction *I);
};


} // End of llvm

#endif /* __COSTMODEL_H_ */
