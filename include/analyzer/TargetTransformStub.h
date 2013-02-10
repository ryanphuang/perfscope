/**
 *  @file          TargetTransformStub.h
 *
 *  @version       1.0
 *  @created       02/08/2013 01:48:35 PM
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
 *  Stub for target transform implementation from LLVM 3.2
 *
 */

#ifndef __TARGETTRANSFORMSTUB_H_
#define __TARGETTRANSFORMSTUB_H_

#include <utility>

#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetLowering.h"

#include "llvm/Support/Casting.h"

#include "llvm/CodeGen/ValueTypes.h"

namespace llvm {

class VectorTargetTransformStub {

protected:
  typedef std::pair<TargetLowering::LegalizeTypeAction, EVT> LegalizeKind;

  const TargetLowering *TLI;

public:

  /// Estimate the cost of type-legalization and the legalized type.
  std::pair<unsigned, MVT> getTypeLegalizationCost(Type *Ty) const;

  /// Estimate the overhead of scalarizing an instruction. Insert and Extract
  /// are set if the result needs to be inserted and/or extracted from vectors.
  unsigned getScalarizationOverhead(Type *Ty, bool Insert, bool Extract) const;

  ////////////////////////////////////////////////////////////////////////
  //
  // The following functions are scraped from various classes in LLVM 3.2 
  //
  ///////////////////////////////////////////////////////////////////////

  // Get the ISD node that corresponds to the Instruction class opcode.
  static int InstructionOpcodeToISD(unsigned Opcode);

  inline LegalizeKind getTypeConversion(LLVMContext &Context, EVT VT) const
  {
    return std::make_pair(TLI->getTypeAction(Context, VT), 
      TLI->getTypeToTransformTo(Context, VT));
  }

  inline unsigned getVectorNumElements(Type *Ty) const
  {
    return cast<VectorType>(Ty)->getNumElements();
  }

  inline bool isOperationExpand(unsigned Op, EVT VT) const 
  {
    return (!TLI->isTypeLegal(VT) || TLI->getOperationAction(Op, VT) == 
      TargetLowering::Expand);
  }

public:
  explicit VectorTargetTransformStub(const TargetLowering *TL) : TLI(TL) 
  {
    assert(TLI && "Target lowering cannot be NULL");
  }

  virtual ~VectorTargetTransformStub() {}

  virtual unsigned getInstrCost(unsigned Opcode, Type *Ty1, Type *Ty2) const;

  virtual unsigned getArithmeticInstrCost(unsigned Opcode, Type *Ty) const;

  virtual unsigned getBroadcastCost(Type *Tp) const;

  virtual unsigned getCastInstrCost(unsigned Opcode, Type *Dst,
                                    Type *Src) const;

  virtual unsigned getCFInstrCost(unsigned Opcode) const;

  virtual unsigned getCmpSelInstrCost(unsigned Opcode, Type *ValTy,
                                      Type *CondTy) const;

  virtual unsigned getVectorInstrCost(unsigned Opcode, Type *Val,
                                      unsigned Index) const;

  virtual unsigned getMemoryOpCost(unsigned Opcode, Type *Src,
                                   unsigned Alignment,
                                   unsigned AddressSpace) const;

  virtual unsigned getNumberOfParts(Type *Tp) const;
};

} // end llvm namespace


#endif /* __TARGETTRANSFORMSTUB_H_ */
