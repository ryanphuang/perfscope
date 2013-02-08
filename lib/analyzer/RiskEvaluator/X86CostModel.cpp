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

#include<string>

#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetSubtargetInfo.h"

#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Host.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/CodeGen/ISDOpcodes.h"
#include "llvm/MC/MCSubtargetInfo.h"

#include "analyzer/CostTable.h"
#include "analyzer/X86SubtargetStub.h"
#include "analyzer/X86CostModel.h"

using namespace llvm;
X86CostModel::X86CostModel()
{
  const std::string TripleStr = "x86_64-unknown-linux-gnu"; // hard coded for example
  const std::string FeatureStr = ""; // hard coded for example
  const std::string CPUStr = llvm::sys::getHostCPUName();
  errs() << "CPU String is " << CPUStr << "\n";

  std::string Err;
  const Target* T;
  TargetMachine* TM = NULL;
  const TargetData* TD;

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
    return;
  }

  // Create TargetMachine
  TM = T->createTargetMachine(TripleStr, CPUStr, FeatureStr);
  if(TM == NULL) {
    errs() << "Cannot create target machine\n";
    return;
  }
  // TD is what you want.
  TD = TM->getTargetData();
  const MCSubtargetInfo &TSI = TM->getSubtarget<MCSubtargetInfo>();
  uint64_t Bits = TSI.getFeatureBits();
  //TM.getSubtarget<X86Subtarget>().is64Bit()
  errs() << "The feature bits are: " << Bits << "\n";

  X86SubtargetStub * stub = new X86SubtargetStub(Bits);
  errs() << "has3DNow " << stub->has3DNow() << "\n";
  errs() << "hasAVX " << stub->hasAVX() << "\n";
  errs() << "is64Bit " << stub->is64Bit() << "\n";
  errs() << "hasBMI " << stub->hasBMI() << "\n";
  errs() << "isBTMemSlow " << stub->isBTMemSlow() << "\n";
  errs() << "hasCmpxchg16b " << stub->hasCmpxchg16b() << "\n";

  static const CostTblEntry<MVT> AVX1CostTable[] = {
    // We don't have to scalarize unsupported ops. We can issue two half-sized
    // operations and we only need to extract the upper YMM half.
    // Two ops + 1 extract + 1 insert = 4.
    { ISD::MUL,     MVT::v8i32,    4 },
    { ISD::SUB,     MVT::v8i32,    4 },
    { ISD::ADD,     MVT::v8i32,    4 },
    { ISD::MUL,     MVT::v4i64,    4 },
    { ISD::SUB,     MVT::v4i64,    4 },
    { ISD::ADD,     MVT::v4i64,    4 },
  };

  //const X86Subtarget &Subtarget = TM->getSubtarget<X86Subtarget>();
}

int X86CostModel::InstructionOpcodeToISD(unsigned Opcode)
{
  enum InstructionOpcodes {
#define HANDLE_INST(NUM, OPCODE, CLASS) OPCODE = NUM,
#define LAST_OTHER_INST(NUM) InstructionOpcodesCount = NUM
#include "llvm/Instruction.def"
  };
  switch (static_cast<InstructionOpcodes>(Opcode)) {
  case Ret:            return 0;
  case Br:             return 0;
  case Switch:         return 0;
  case IndirectBr:     return 0;
  case Invoke:         return 0;
  case Resume:         return 0;
  case Unreachable:    return 0;
  case Add:            return ISD::ADD;
  case FAdd:           return ISD::FADD;
  case Sub:            return ISD::SUB;
  case FSub:           return ISD::FSUB;
  case Mul:            return ISD::MUL;
  case FMul:           return ISD::FMUL;
  case UDiv:           return ISD::UDIV;
  case SDiv:           return ISD::UDIV;
  case FDiv:           return ISD::FDIV;
  case URem:           return ISD::UREM;
  case SRem:           return ISD::SREM;
  case FRem:           return ISD::FREM;
  case Shl:            return ISD::SHL;
  case LShr:           return ISD::SRL;
  case AShr:           return ISD::SRA;
  case And:            return ISD::AND;
  case Or:             return ISD::OR;
  case Xor:            return ISD::XOR;
  case Alloca:         return 0;
  case Load:           return ISD::LOAD;
  case Store:          return ISD::STORE;
  case GetElementPtr:  return 0;
  case Fence:          return 0;
  case AtomicCmpXchg:  return 0;
  case AtomicRMW:      return 0;
  case Trunc:          return ISD::TRUNCATE;
  case ZExt:           return ISD::ZERO_EXTEND;
  case SExt:           return ISD::SIGN_EXTEND;
  case FPToUI:         return ISD::FP_TO_UINT;
  case FPToSI:         return ISD::FP_TO_SINT;
  case UIToFP:         return ISD::UINT_TO_FP;
  case SIToFP:         return ISD::SINT_TO_FP;
  case FPTrunc:        return ISD::FP_ROUND;
  case FPExt:          return ISD::FP_EXTEND;
  case PtrToInt:       return ISD::BITCAST;
  case IntToPtr:       return ISD::BITCAST;
  case BitCast:        return ISD::BITCAST;
  case ICmp:           return ISD::SETCC;
  case FCmp:           return ISD::SETCC;
  case PHI:            return 0;
  case Call:           return 0;
  case Select:         return ISD::SELECT;
  case UserOp1:        return 0;
  case UserOp2:        return 0;
  case VAArg:          return 0;
  case ExtractElement: return ISD::EXTRACT_VECTOR_ELT;
  case InsertElement:  return ISD::INSERT_VECTOR_ELT;
  case ShuffleVector:  return ISD::VECTOR_SHUFFLE;
  case ExtractValue:   return ISD::MERGE_VALUES;
  case InsertValue:    return ISD::MERGE_VALUES;
  case LandingPad:     return 0;
  }
}

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


