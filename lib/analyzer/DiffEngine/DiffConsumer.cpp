//===-- DiffConsumer.cpp - Difference Consumer ------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This files implements the the LLVM difference Consumer
//
//===----------------------------------------------------------------------===//

#include "llvm/Module.h"
#include "llvm/Instructions.h"
#include "llvm/Support/ErrorHandling.h"

#include "analyzer/DiffConsumer.h"

using namespace llvm;

static void ComputeNumbering(Function *F, DenseMap<Value*,unsigned> &Numbering){
  unsigned IN = 0;

  // Arguments get the first numbers.
  for (Function::arg_iterator
         AI = F->arg_begin(), AE = F->arg_end(); AI != AE; ++AI)
    if (!AI->hasName())
      Numbering[&*AI] = IN++;

  // Walk the basic blocks in order.
  for (Function::iterator FI = F->begin(), FE = F->end(); FI != FE; ++FI) {
    if (!FI->hasName())
      Numbering[&*FI] = IN++;

    // Walk the instructions in order.
    for (BasicBlock::iterator BI = FI->begin(), BE = FI->end(); BI != BE; ++BI)
      // void instructions don't get numbers.
      if (!BI->hasName() && !BI->getType()->isVoidTy())
        Numbering[&*BI] = IN++;
  }

  assert(!Numbering.empty() && "asked for numbering but numbering was no-op");
}


void DiffConsumer::printValue(Value *V, bool isL) {
  if (V->hasName()) {
    out << (isa<GlobalValue>(V) ? '@' : '%') << V->getName();
    return;
  }
  if (V->getType()->isVoidTy()) {
    if (isa<StoreInst>(V)) {
      out << "store to ";
      printValue(cast<StoreInst>(V)->getPointerOperand(), isL);
    } else if (isa<CallInst>(V)) {
      out << "call to ";
      printValue(cast<CallInst>(V)->getCalledValue(), isL);
    } else if (isa<InvokeInst>(V)) {
      out << "invoke to ";
      printValue(cast<InvokeInst>(V)->getCalledValue(), isL);
    } else {
      out << *V;
    }
    return;
  }

  unsigned N = contexts.size();
  while (N > 0) {
    --N;
    DiffContext &ctxt = contexts[N];
    if (!ctxt.IsFunction) continue;
    if (isL) {
      if (ctxt.LNumbering.empty())
        ComputeNumbering(cast<Function>(ctxt.L), ctxt.LNumbering);
      out << '%' << ctxt.LNumbering[V];
      return;
    } else {
      if (ctxt.RNumbering.empty())
        ComputeNumbering(cast<Function>(ctxt.R), ctxt.RNumbering);
      out << '%' << ctxt.RNumbering[V];
      return;
    }
  }

  out << "<anonymous>";
}

void DiffConsumer::header() {

}

void DiffConsumer::indent() {
  unsigned N = Indent;
  while (N--) out << ' ';
}

bool DiffConsumer::hadDifferences() const {
  return Differences;
}

void DiffConsumer::enterContext(Value *L, Value *R) {
  contexts.push_back(DiffContext(L, R));
  Indent += 2;
}

void DiffConsumer::exitContext() {
  Differences |= contexts.back().Differences;
  contexts.pop_back();
  Indent -= 2;
}

void DiffConsumer::log(StringRef text) {
}

void DiffConsumer::logf(const LogBuilder &Log) {
}

void DiffConsumer::logd(const DiffLogBuilder &Log) {
}
