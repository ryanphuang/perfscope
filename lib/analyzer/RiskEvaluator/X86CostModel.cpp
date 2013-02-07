/**
 *  @file          X86CostModel.cpp
 *
 *  @version       1.0
 *  @created       02/07/2013 11:53:02 AM
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
 *  x86 cost model implementation
 *
 */

#include "analyzer/X86CostModel.h"

using namespace llvm;

unsigned X86CostModel::getNumberOfRegisters(bool Vector)
{
  return 1;
}
unsigned X86CostModel::getRegisterBitWidth(bool Vector)
{
  return 1;
}
unsigned X86CostModel::getMaximumUnrollFactor()
{
  return 1;
}
unsigned X86CostModel::getArithmeticInstrCost(unsigned Opcode, Type *Ty)
{
  return 1;
}
unsigned X86CostModel::getShuffleCost(ShuffleKind Kind, Type *Tp,
   int Index, Type *SubTp)
{
  return 1;
}
unsigned X86CostModel::getCastInstrCost(unsigned Opcode, Type *Dst,
   Type *Src)
{
  return 1;
}
unsigned X86CostModel::getCmpSelInstrCost(unsigned Opcode, Type *ValTy,
   Type *CondTy)
{
  return 1;
}
unsigned X86CostModel::getVectorInstrCost(unsigned Opcode, Type *Val,
   unsigned Index)
{
  return 1;
}
unsigned X86CostModel::getMemoryOpCost(unsigned Opcode, Type *Src,
   unsigned Alignment, unsigned AddressSpace)
{
  return 1;
}


