//===- DependenceGraph.cpp - Dependence graph for a function ----*- C++ -*-===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// This file implements an explicit representation for the dependence graph
// of a function, with one node per instruction and one edge per dependence.
// Dependences include both data and control dependences.
// 
// Each dep. graph node (class DepGraphNode) keeps lists of incoming and
// outgoing dependence edges.
// 
// Each dep. graph edge (class Dependence) keeps a pointer to one end-point
// of the dependence.  This saves space and is important because dep. graphs
// can grow quickly.  It works just fine because the standard idiom is to
// start with a known node and enumerate the dependences to or from that node.
//===----------------------------------------------------------------------===//


#include "DependenceGraph.h"
#include "llvm/Function.h"
#include "llvm/Analysis/DebugInfo.h"

namespace llvm {

unsigned getInstLine(const Instruction &I)
{
  DebugLoc Loc = I.getDebugLoc();
  if (Loc.isUnknown())
    return 0;
  return Loc.getLine();
}

//----------------------------------------------------------------------------
// class Dependence:
// 
// A representation of a simple (non-loop-related) dependence
//----------------------------------------------------------------------------

bool Dependence::operator<(const Dependence& D) const {
    if (toOrFromNode == NULL || D.toOrFromNode == NULL)
      return false;
    const Instruction &i1 = toOrFromNode->getInstr();
    const Instruction &i2 = D.toOrFromNode->getInstr();
    unsigned l1 = getInstLine(i1);
    unsigned l2 = getInstLine(i2);
    return l1 < l2;
}
void Dependence::print(raw_ostream &O) const
{
  assert(depType != NoDependence && "This dependence should never be created!");
  switch (depType) {
  case TrueDependence:    O << "TRUE"; break;
  case AntiDependence:    O << "ANTI"; break;
  case OutputDependence:  O << "OUTPUT"; break;
  case ControlDependence: O << "CONTROL"; break;
  default: assert(0 && "Invalid dependence type"); break;
  }
}


//----------------------------------------------------------------------------
// class DepGraphNode
//----------------------------------------------------------------------------

void DepGraphNode::print(raw_ostream &O) const
{
  const_iterator DI = outDepBegin(), DE = outDepEnd();

  //O << "\nfrom instr:" << getInstLine(getInstr());
  O << "\nfrom " << getInstr();

  for ( ; DI != DE; ++DI)
    {
      O << "\t";
      DI->print(O);
      O << " to ";
      //O << getInstLine(DI->getSink()->getInstr());
      O << DI->getSink()->getInstr();
    }
}

//----------------------------------------------------------------------------
// class DependenceGraph
//----------------------------------------------------------------------------

DependenceGraph::~DependenceGraph()
{
  // Free all DepGraphNode objects created for this graph
  for (map_iterator I = depNodeMap.begin(), E = depNodeMap.end(); I != E; ++I)
    delete I->second;
}

void DependenceGraph::print(const Function& func, raw_ostream &O) const
{
  O << "DEPENDENCE GRAPH FOR FUNCTION " << func.getName() << ":\n";
  for (Function::const_iterator BB=func.begin(), FE=func.end(); BB != FE; ++BB)
    for (BasicBlock::const_iterator II=BB->begin(), IE=BB->end(); II !=IE; ++II)
      if (const DepGraphNode* dgNode = this->getNode(*II))
        dgNode->print(O);
}

} // End llvm namespace
