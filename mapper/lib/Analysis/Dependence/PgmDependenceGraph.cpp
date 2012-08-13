//===- PgmDependenceGraph.cpp - Enumerate PDG for a function ----*- C++ -*-===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
// 
// The Program Dependence Graph (PDG) for a single function represents all
// data and control dependences for the function.  This file provides an
// iterator to enumerate all these dependences.  In particular, it enumerates:
// 
// -- Data dependences on memory locations, computed using the
//    MemoryDepAnalysis pass;
// -- Data dependences on SSA registers, directly from Def-Use edges of Values;
// -- Control dependences, computed using postdominance frontiers
//    (NOT YET IMPLEMENTED).
// 
// Note that this file does not create an explicit dependence graph --
// it only provides an iterator to traverse the PDG conceptually.
// The MemoryDepAnalysis does build an explicit graph, which is used internally
// here.  That graph could be augmented with the other dependences above if
// desired, but for most uses there will be little need to do that.
//
//===----------------------------------------------------------------------===//

#include "PgmDependenceGraph.h"
#include "Slicer.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Function.h"
#include "llvm/IntrinsicInst.h"

#include <map>
#include <iostream>

namespace llvm {

class DependenceGraph *globalDPG = NULL;

//----------------------------------------------------------------------------
// class DepIterState
//----------------------------------------------------------------------------

const DepIterState::IterStateFlags DepIterState::NoFlag  = 0x0;
const DepIterState::IterStateFlags DepIterState::MemDone = 0x1;
const DepIterState::IterStateFlags DepIterState::SSADone = 0x2;
const DepIterState::IterStateFlags DepIterState::AllDone = 0x4;
const DepIterState::IterStateFlags DepIterState::FirstTimeFlag= 0x8;

// Find the first memory dependence for the current Mem In/Out iterators.
// Find the first memory dependence for the current Mem In/Out iterators.
// Sets dep to that dependence and returns true if one is found.
// 
bool DepIterState::SetFirstMemoryDep()
{
  if (! (depFlags & MemoryDeps))
    return false;

  bool doIncomingDeps = dep.getDepType() & IncomingFlag;

  if (( doIncomingDeps && memDepIter == funcDepGraph->inDepEnd( *depNode)) ||
      (!doIncomingDeps && memDepIter == funcDepGraph->outDepEnd(*depNode)))
    {
      iterFlags |= MemDone;
      return false;
    }

  dep = *memDepIter;     // simple copy from dependence in memory DepGraph

  return true;
}


// Find the first valid data dependence for the current SSA In/Out iterators.
// A valid data dependence is one that is to/from an Instruction.
// E.g., an SSA edge from a formal parameter is not a valid dependence.
// Sets dep to that dependence and returns true if a valid one is found.
// Returns false and leaves dep unchanged otherwise.
// 
bool DepIterState::SetFirstSSADep()
{
  if (! (depFlags & SSADeps))
    return false;

  bool doIncomingDeps = dep.getDepType() & IncomingFlag;
  Instruction* firstTarget = NULL;

  // Increment the In or Out iterator till it runs out or we find a valid dep
  if (doIncomingDeps)
    for (Instruction::op_iterator E = depNode->getInstr().op_end();
         ssaInEdgeIter != E &&
           (firstTarget = dyn_cast<Instruction>(ssaInEdgeIter))== NULL; )
      ++ssaInEdgeIter;
  else
    for (Value::use_iterator E = depNode->getInstr().use_end();
         ssaOutEdgeIter != E &&
           (firstTarget = dyn_cast<Instruction>(*ssaOutEdgeIter)) == NULL; )
      ++ssaOutEdgeIter;

  // If the iterator ran out before we found a valid dep, there isn't one.
  if (!firstTarget)
    {
      iterFlags |= SSADone;
      return false;
    }

  // Create a simple dependence object to represent this SSA dependence.
  dep = Dependence(funcDepGraph->getNode(*firstTarget, /*create*/ true),
                   TrueDependence, doIncomingDeps);

  return true;
}


DepIterState::DepIterState(DependenceGraph* _funcDepGraph,
                           Instruction&     I, 
                           bool             incomingDeps,
                           PDGIteratorFlags whichDeps)
  : funcDepGraph(_funcDepGraph),
    depFlags(whichDeps),
    iterFlags(NoFlag)
{
  depNode = funcDepGraph->getNode(I, /*create*/ true);

  if (incomingDeps)
    {
      if (whichDeps & MemoryDeps) memDepIter= funcDepGraph->inDepBegin(*depNode);
      if (whichDeps & SSADeps)    ssaInEdgeIter = I.op_begin();
      /* Initialize control dependence iterator here. */
    }
  else
    {
      if (whichDeps & MemoryDeps) memDepIter=funcDepGraph->outDepBegin(*depNode);
      if (whichDeps & SSADeps)    ssaOutEdgeIter = I.use_begin();
      /* Initialize control dependence iterator here. */
    }

  // Set the dependence to the first of a memory dep or an SSA dep
  // and set the done flag if either is found.  Otherwise, set the
  // init flag to indicate that the iterators have just been initialized.
  // 
  if (!SetFirstMemoryDep() && !SetFirstSSADep())
    iterFlags |= AllDone;
  else
    iterFlags |= FirstTimeFlag;
}


// Helper function for ++ operator that bumps iterator by 1 (to next
// dependence) and resets the dep field to represent the new dependence.
// 
void DepIterState::Next()
{
  // firstMemDone and firstSsaDone are used to indicate when the memory or
  // SSA iterators just ran out, or when this is the very first increment.
  // In either case, the next iterator (if any) should not be incremented.
  // 
  bool firstMemDone = iterFlags & FirstTimeFlag;
  bool firstSsaDone = iterFlags & FirstTimeFlag;
  bool doIncomingDeps = dep.getDepType() & IncomingFlag;

  if (depFlags & MemoryDeps && ! (iterFlags & MemDone))
    {
      iterFlags &= ~FirstTimeFlag;           // clear "firstTime" flag
      ++memDepIter;
      if (SetFirstMemoryDep())
        return;
      firstMemDone = true;              // flags that we _just_ rolled over
    }

  if (depFlags & SSADeps && ! (iterFlags & SSADone))
    {
      // Don't increment the SSA iterator if we either just rolled over from
      // the memory dep iterator, or if the SSA iterator is already done.
      iterFlags &= ~FirstTimeFlag;           // clear "firstTime" flag
      if (! firstMemDone) {
        if (doIncomingDeps) ++ssaInEdgeIter;
        else ++ssaOutEdgeIter;
      }
      if (SetFirstSSADep())
        return;
      firstSsaDone = true;                   // flags if we just rolled over
    } 

  if ((depFlags & ControlDeps) != 0)
    {
      assert(0 && "Cannot handle control deps");
      // iterFlags &= ~FirstTimeFlag;           // clear "firstTime" flag
    }

  // This iterator is now complete.
  iterFlags |= AllDone;
}


//----------------------------------------------------------------------------
// class PgmDependenceGraph
//----------------------------------------------------------------------------


// MakeIterator -- Create and initialize an iterator as specified.
// 
PDGIterator PgmDependenceGraph::MakeIterator(Instruction& I,
                                             bool incomingDeps,
                                             PDGIteratorFlags whichDeps)
{
  assert(funcDepGraph && "Function not initialized!");
  return PDGIterator(new DepIterState(funcDepGraph, I, incomingDeps, whichDeps));
}

bool PgmDependenceGraph::buildPDG(Function& func)
{
	MemoryDependenceAnalysis &mda = getAnalysis<MemoryDependenceAnalysis>();
  for (Function::iterator FI = func.begin(), FE = func.end(); FI != FE; FI++) {
    for (BasicBlock::iterator BI = FI->begin(), BE = FI->end(); BI != BE; BI++) {
      Instruction *I = &(*BI);

      if (isa<DbgInfoIntrinsic>(I)) { // Skip building dependency for intrinsics 
        continue;
      }

      // Register dependency
      // though it's already handled in the iterator part, we still
      // explicitly insert them to a set associated with a graph node 
      // so later we can went through all unique data dependency instructions

      for (Value::use_iterator UI = I->use_begin(), UE= I->use_end(); UI != UE; UI++) {
        Instruction *use = dyn_cast<Instruction>(*UI);
        if (use != NULL) {
          funcDepGraph->AddSimpleDependence(*I, *use, OutputDependence);
        }
      }
      
      // Memory dependency
      MemDepResult mdr = mda.getDependency(I); 
      Instruction *dep = mdr.getInst();
      if (dep != NULL) {
        if (isa<DbgInfoIntrinsic>(dep)) { // Skip building dependency for intrinsics 
          continue;
        }
        if (mdr.isDef()) {
          funcDepGraph->AddSimpleDependence(*dep, *I, OutputDependence);
        }
        else
            if (mdr.isClobber()) {
              if (isa<LoadInst>(I)) { // RAW
                funcDepGraph->AddSimpleDependence(*dep, *I, TrueDependence);
              }
              else
                if (isa<StoreInst>(I)) { // WAW
                  // A lot of false dependency here
                  // funcDepGraph->AddSimpleDependence(*dep, *I, AntiDependence);
                }
                else {
                  // outs() << "Clobber: " << *dep << "; " << *I << "\n";
                  funcDepGraph->AddSimpleDependence(*dep, *I, OutputDependence);
                }
            }
      }
    }
  } 
  return true;
}

void PgmDependenceGraph::printOutgoingSSADeps(Instruction& I,
                                              raw_ostream &O)
{
  iterator SI = this->outDepBegin(I, SSADeps);
  iterator SE = this->outDepEnd(I, SSADeps);
  if (SI == SE)
    return;

  O << "\n [All] for :" << I;
  //O << "\n [O-SSA] for :" << I;
  //O << "\n [O-SSA] for :" << getInstLine(I);
  for ( ; SI != SE; ++SI)
    {
      O << "\t(";
      SI->print(O);
      O << ") to :";
      O << SI->getSink()->getInstr();
      //O << getInstLine(SI->getSink()->getInstr());
    }
}

void PgmDependenceGraph::testSlicing(Function &func)
{
  // Slicer slicer(funcDepGraph);
  // for (Function::iterator BB=func.begin(), FE=func.end(); BB != FE; ++BB)
  //   for (BasicBlock::iterator II=BB->begin(), IE=BB->end(); II !=IE; ++II)
  //   {
  //     Instruction *I = II;
  //     if (isa<DbgInfoIntrinsic>(I)) { // Skip intrinsics 
  //       continue;
  //     }
  //     if (slicer.sliceInit(*I, MemoryDeps)) {
  //       errs() << "Slicing " << *I << "\n\t";
  //       Instruction *N;
  //       while((N = slicer.sliceNext()) != NULL) {
  //         errs() << *N << "||";
  //       }
  //       errs() << "\n";
  //     }
  //   }
}

bool PgmDependenceGraph::runOnFunction(Function& func) 
{
  if (globalDPG != NULL) {
      delete globalDPG;
      globalDPG = NULL;
  }
  funcDepGraph = new DependenceGraph();
  buildPDG(func);
  globalDPG = funcDepGraph;
  //printFuncDeps(&func, errs());
  //testSlicing(func);
  return true;
}

void PgmDependenceGraph::printFuncDeps(Function*func, raw_ostream &O)
{
  // funcDepGraph->print(*func, O);
  O << "DEPENDENCE GRAPH FOR FUNCTION " << func->getName() << ":\n";
  for (Function::iterator BB=func->begin(), FE=func->end(); BB != FE; ++BB)
    for (BasicBlock::iterator II=BB->begin(), IE=BB->end(); II !=IE; ++II)
      {
        DepGraphNode* dgNode = funcDepGraph->getNode(*II, true);
        dgNode->print(O);
        // const_cast<PgmDependenceGraph*>(this)->printOutgoingSSADeps(*II, O);
      }
}


void PgmDependenceGraph::dump() const
{
}

char PgmDependenceGraph::ID = 0;
PgmDependenceGraph::iterator PgmDependenceGraph::NULLIterator = PgmDependenceGraph::MakeIterator();
static RegisterPass<PgmDependenceGraph> X("pgmdep", "Enumerate Program Dependence Graph (data and control)");

} // End llvm namespace
