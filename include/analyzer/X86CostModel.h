/**
 *  @file          X86CostModel.h
 *
 *  @version       1.0
 *  @created       02/07/2013 11:47:14 AM
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
 *  Cost model specifically for x86 architecture
 *
 */

#ifndef __X86COSTMODEL_H_
#define __X86COSTMODEL_H_

#include <string>
#include <utility>

#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetLowering.h"

#include "analyzer/TargetTransformStub.h"
#include "analyzer/X86SubtargetStub.h"
#include "analyzer/CostModel.h"

namespace llvm {

class X86CostModel : public CostModel {
  protected:
    X86SubtargetStub * ST;
    VectorTargetTransformStub * VTT;
    const TargetLowering * TLI;

  public:
    X86CostModel(const std::string TripleStr = "x86_64-unknown-linux-gnu", const std::string FeatureStr = "");

    virtual unsigned getNumberOfRegisters(bool Vector);
    virtual unsigned getRegisterBitWidth(bool Vector);
    virtual unsigned getMaximumUnrollFactor();
    virtual unsigned getArithmeticInstrCost(unsigned Opcode, Type *Ty);
    virtual unsigned getShuffleCost(ShuffleKind Kind, Type *Tp, int Index, Type *SubTp);
    virtual unsigned getCastInstrCost(unsigned Opcode, Type *Dst, Type *Src);
    virtual unsigned getCmpSelInstrCost(unsigned Opcode, Type *ValTy, Type *CondTy);
    virtual unsigned getVectorInstrCost(unsigned Opcode, Type *Val, unsigned Index);
    virtual unsigned getMemoryOpCost(unsigned Opcode, Type *Src, unsigned Alignment, 
        unsigned AddressSpace);
};

} // End of llvm namespace
#endif /* __X86COSTMODEL_H_ */
