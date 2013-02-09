/**
 *  @file          x86SubtargetStub.h
 *
 *  @version       1.0
 *  @created       02/08/2013 12:05:41 AM
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
 *  Manually created from X86GenSubtarget.inc
 *
 */

#ifndef __X86SUBTARGETSTUB_H_
#define __X86SUBTARGETSTUB_H_

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {

class X86SubtargetStub {
protected:

  enum FeatureEncoding {
    Feature3DNow =  1ULL << 0,
    Feature3DNowA =  1ULL << 1,
    Feature64Bit =  1ULL << 2,
    FeatureAES =  1ULL << 3,
    FeatureAVX =  1ULL << 4,
    FeatureBMI =  1ULL << 5,
    FeatureCLMUL =  1ULL << 6,
    FeatureCMOV =  1ULL << 7,
    FeatureCMPXCHG16B =  1ULL << 8,
    FeatureF16C =  1ULL << 9,
    FeatureFMA3 =  1ULL << 10,
    FeatureFMA4 =  1ULL << 11,
    FeatureFastUAMem =  1ULL << 12,
    FeatureLZCNT =  1ULL << 13,
    FeatureMMX =  1ULL << 14,
    FeatureMOVBE =  1ULL << 15,
    FeaturePOPCNT =  1ULL << 16,
    FeatureRDRAND =  1ULL << 17,
    FeatureSSE1 =  1ULL << 18,
    FeatureSSE2 =  1ULL << 19,
    FeatureSSE3 =  1ULL << 20,
    FeatureSSE4A =  1ULL << 21,
    FeatureSSE41 =  1ULL << 22,
    FeatureSSE42 =  1ULL << 23,
    FeatureSSSE3 =  1ULL << 24,
    FeatureSlowBTMem =  1ULL << 25,
    FeatureVectorUAMem =  1ULL << 26,
    Mode64Bit =  1ULL << 27,
    ModeNaCl =  1ULL << 28
  };

  enum X86SSEEnum {
    NoMMXSSE, MMX, SSE1, SSE2, SSE3, SSSE3, SSE41, SSE42
  };

  enum X863DNowEnum {
    NoThreeDNow, ThreeDNow, ThreeDNowA
  };


  /// X86SSELevel - MMX, SSE1, SSE2, SSE3, SSSE3, SSE41, SSE42, or
  /// none supported.
  X86SSEEnum X86SSELevel;

  /// X863DNowLevel - 3DNow or 3DNow Athlon, or none supported.
  ///
  X863DNowEnum X863DNowLevel;

  /// HasCMov - True if this processor has conditional move instructions
  /// (generally pentium pro+).
  bool HasCMov;

  /// HasX86_64 - True if the processor supports X86-64 instructions.
  ///
  bool HasX86_64;

  /// HasPOPCNT - True if the processor supports POPCNT.
  bool HasPOPCNT;

  /// HasSSE4A - True if the processor supports SSE4A instructions.
  bool HasSSE4A;

  /// HasAVX - Target has AVX instructions
  bool HasAVX;

  /// HasAES - Target has AES instructions
  bool HasAES;

  /// HasCLMUL - Target has carry-less multiplication
  bool HasCLMUL;

  /// HasFMA3 - Target has 3-operand fused multiply-add
  bool HasFMA3;

  /// HasFMA4 - Target has 4-operand fused multiply-add
  bool HasFMA4;

  /// HasMOVBE - True if the processor has the MOVBE instruction.
  bool HasMOVBE;

  /// HasRDRAND - True if the processor has the RDRAND instruction.
  bool HasRDRAND;

  /// HasF16C - Processor has 16-bit floating point conversion instructions.
  bool HasF16C;

  /// HasLZCNT - Processor has LZCNT instruction.
  bool HasLZCNT;

  /// HasBMI - Processor has BMI1 instructions.
  bool HasBMI;

  /// IsBTMemSlow - True if BT (bit test) of memory instructions are slow.
  bool IsBTMemSlow;

  /// IsUAMemFast - True if unaligned memory access is fast.
  bool IsUAMemFast;

  /// HasVectorUAMem - True if SIMD operations can have unaligned memory
  /// operands. This may require setting a feature bit in the processor.
  bool HasVectorUAMem;

  /// HasCmpxchg16b - True if this processor has the CMPXCHG16B instruction;
  /// this is true for most x86-64 chips, but not the first AMD chips.
  bool HasCmpxchg16b;

private:
  /// In64BitMode - True if compiling for 64-bit, false for 32-bit.
  bool In64BitMode;

  /// InNaClMode - True if compiling for Native Client target.
  bool InNaClMode;

public:
  X86SubtargetStub(uint64_t bits) 
  : X86SSELevel(NoMMXSSE)
  , X863DNowLevel(NoThreeDNow)
  , HasCMov(false)
  , HasX86_64(false)
  , HasPOPCNT(false)
  , HasSSE4A(false)
  , HasAVX(false)
  , HasAES(false)
  , HasCLMUL(false)
  , HasFMA3(false)
  , HasFMA4(false)
  , HasMOVBE(false)
  , HasRDRAND(false)
  , HasF16C(false)
  , HasLZCNT(false)
  , HasBMI(false)
  , IsBTMemSlow(false)
  , IsUAMemFast(false)
  , HasVectorUAMem(false)
  , HasCmpxchg16b(false)
  , In64BitMode(false) 
  , InNaClMode(false)
  {
    ParseSubtargetFeatures(bits);
  }

  //TODO stub
  bool isAtom() const { return false; }

  bool is64Bit() const { return In64BitMode; }

  bool hasCMov() const { return HasCMov; }
  bool hasMMX() const { return X86SSELevel >= MMX; }
  bool hasSSE1() const { return X86SSELevel >= SSE1; }
  bool hasSSE2() const { return X86SSELevel >= SSE2; }
  bool hasSSE3() const { return X86SSELevel >= SSE3; }
  bool hasSSSE3() const { return X86SSELevel >= SSSE3; }
  bool hasSSE41() const { return X86SSELevel >= SSE41; }
  bool hasSSE42() const { return X86SSELevel >= SSE42; }
  bool hasSSE4A() const { return HasSSE4A; }
  bool has3DNow() const { return X863DNowLevel >= ThreeDNow; }
  bool has3DNowA() const { return X863DNowLevel >= ThreeDNowA; }
  bool hasPOPCNT() const { return HasPOPCNT; }
  bool hasAVX() const { return HasAVX; }
  bool hasXMM() const { return hasSSE1() || hasAVX(); }
  bool hasXMMInt() const { return hasSSE2() || hasAVX(); }
  bool hasAES() const { return HasAES; }
  bool hasCLMUL() const { return HasCLMUL; }
  bool hasFMA3() const { return HasFMA3; }
  bool hasFMA4() const { return HasFMA4; }
  bool hasMOVBE() const { return HasMOVBE; }
  bool hasRDRAND() const { return HasRDRAND; }
  bool hasF16C() const { return HasF16C; }
  bool hasLZCNT() const { return HasLZCNT; }
  bool hasBMI() const { return HasBMI; }
  bool isBTMemSlow() const { return IsBTMemSlow; }
  bool isUnalignedMemAccessFast() const { return IsUAMemFast; }
  bool hasVectorUAMem() const { return HasVectorUAMem; }
  bool hasCmpxchg16b() const { return HasCmpxchg16b; }

  void ParseSubtargetFeatures(uint64_t Bits) {
    if ((Bits & Feature3DNow) != 0 && X863DNowLevel < ThreeDNow) X863DNowLevel = ThreeDNow;
    if ((Bits & Feature3DNowA) != 0 && X863DNowLevel < ThreeDNowA) X863DNowLevel = ThreeDNowA;
    if ((Bits & Feature64Bit) != 0) HasX86_64 = true;
    if ((Bits & FeatureAES) != 0) HasAES = true;
    if ((Bits & FeatureAVX) != 0) HasAVX = true;
    if ((Bits & FeatureBMI) != 0) HasBMI = true;
    if ((Bits & FeatureCLMUL) != 0) HasCLMUL = true;
    if ((Bits & FeatureCMOV) != 0) HasCMov = true;
    if ((Bits & FeatureCMPXCHG16B) != 0) HasCmpxchg16b = true;
    if ((Bits & FeatureF16C) != 0) HasF16C = true;
    if ((Bits & FeatureFMA3) != 0) HasFMA3 = true;
    if ((Bits & FeatureFMA4) != 0) HasFMA4 = true;
    if ((Bits & FeatureFastUAMem) != 0) IsUAMemFast = true;
    if ((Bits & FeatureLZCNT) != 0) HasLZCNT = true;
    if ((Bits & FeatureMMX) != 0 && X86SSELevel < MMX) X86SSELevel = MMX;
    if ((Bits & FeatureMOVBE) != 0) HasMOVBE = true;
    if ((Bits & FeaturePOPCNT) != 0) HasPOPCNT = true;
    if ((Bits & FeatureRDRAND) != 0) HasRDRAND = true;
    if ((Bits & FeatureSSE1) != 0 && X86SSELevel < SSE1) X86SSELevel = SSE1;
    if ((Bits & FeatureSSE2) != 0 && X86SSELevel < SSE2) X86SSELevel = SSE2;
    if ((Bits & FeatureSSE3) != 0 && X86SSELevel < SSE3) X86SSELevel = SSE3;
    if ((Bits & FeatureSSE4A) != 0) HasSSE4A = true;
    if ((Bits & FeatureSSE41) != 0 && X86SSELevel < SSE41) X86SSELevel = SSE41;
    if ((Bits & FeatureSSE42) != 0 && X86SSELevel < SSE42) X86SSELevel = SSE42;
    if ((Bits & FeatureSSSE3) != 0 && X86SSELevel < SSSE3) X86SSELevel = SSSE3;
    if ((Bits & FeatureSlowBTMem) != 0) IsBTMemSlow = true;
    if ((Bits & FeatureVectorUAMem) != 0) HasVectorUAMem = true;
    if ((Bits & Mode64Bit) != 0) In64BitMode = true;
    if ((Bits & ModeNaCl) != 0) InNaClMode = true;
  }
};


} // End llvm namespace 



#endif /* __X86SUBTARGETSTUB_H_ */
