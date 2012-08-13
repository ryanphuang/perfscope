//===- Slicer.cpp - Simple Slicing Tool ----*- C++ -*-===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// This file implements a simple slicer based on DependenceGraph 
//===----------------------------------------------------------------------===//

#include "Slicer.h"

namespace llvm {
void Slicer::addToSliceStack(DepGraphNode *dgNode)
{
  DepGraphNode::iterator DI = dgNode->outDepBegin();
  DepGraphNode::iterator DE = dgNode->outDepEnd();
  for (; DI != DE; DI++) {
    Instruction & inst = DI->getSink()->getInstr();
    if (visitedInstr.count(&inst)) {
      // errs() << "Warning duplicate dependency detected!\n";
      continue;
    }
    sliceStack.push(&inst);
    visitedInstr.insert(&inst);
  }
}

bool Slicer::sliceInit(Instruction &I, PDGIteratorFlags whichDeps)
{
  DepGraphNode* dgNode = funcDepGraph->getNode(I, false);
  if (dgNode == NULL) {
    return false;
  }
  while(!sliceStack.empty())
    sliceStack.pop();
  visitedInstr.clear();
  addToSliceStack(dgNode);
  sliceInited = true;
  return true;
}

Instruction * Slicer::sliceNext()
{
  if (!sliceInited) {
    errs() << "Must call sliceInit first!\n";
    return NULL;
  }
  if (sliceStack.empty()) { // stack is exhausted
    sliceInited = false;
    return NULL;
  }
  Instruction * inst = sliceStack.top();
  sliceStack.pop();
  if (inst == NULL) {
    sliceInited = false;
    return NULL;
  }
  DepGraphNode* dgNode = funcDepGraph->getNode(*inst, false);
  if (dgNode != NULL)
    addToSliceStack(dgNode);
  return inst;
}

} // End llvm namespace
