//===- PgmDependenceGraph.h - Enumerate the PDG for a function --*- C++ -*-===//
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
// Key Classes:
// 
// enum PDGIteratorFlags -- Specify which dependences to enumerate.
// 
// class PDGIterator     -- The PDG iterator.  This is essentially like a
//                          pointer to class Dependence, but doesn't explicitly
//                          construct a Dependence object for each dependence.
//
// class PgmDependenceGraph -- Interface to obtain PDGIterators for each
//                          instruction.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_PGMDEPENDENCEGRAPH_H
#define LLVM_ANALYSIS_PGMDEPENDENCEGRAPH_H

#include "dsa/DataStructure.h"
#include "dsa/IPModRef.h"
#include "DependenceGraph.h"

/* #include "llvm/Analysis/PostDominators.h" -- see below */
//#include "MemoryDepAnalysis.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Instruction.h"
#include "llvm/Pass.h"
#include <iterator>
#include <stack>
#include <vector>

template<class Ty, class PtrDiffTy>
struct forward_iterator
  : public std::iterator<std::forward_iterator_tag, Ty, PtrDiffTy> {
};

namespace llvm {

extern class DependenceGraph *globalDPG;

class DependenceGraph;
class PgmDependenceGraph;
class FunctionModRefInfo;
class ModRefTable;

//---------------------------------------------------------------------------
/// enum PDGIteratorFlags - specify which dependences incident on a statement
/// are to be enumerated: Memory deps, SSA deps, Control deps, or any
/// combination thereof.
///
enum PDGIteratorFlags {
  MemoryDeps  = 0x1,                                 // load/store/call deps
  SSADeps     = 0x2,                                 // SSA deps (true)
  ControlDeps = /* 0x4*/ 0x0,                        // control dependences
  AllDataDeps = MemoryDeps | SSADeps,                // shorthand for data deps
  AllDeps     = MemoryDeps | SSADeps | ControlDeps   // shorthand for all three
};

//---------------------------------------------------------------------------
/// struct DepIterState - an internal implementation detail.
/// It are exposed here only to give inlinable access to field dep,
/// which is the representation for the current dependence pointed to by
/// a PgmDependenceGraph::iterator.
///
class DepIterState {
private:
  typedef char IterStateFlags;
  static const IterStateFlags NoFlag, MemDone, SSADone, AllDone, FirstTimeFlag;

public:
  DepGraphNode*              depNode;        // the node being enumerated
  DependenceGraph::iterator  memDepIter;     // pointer to current memory dep
  Instruction::op_iterator   ssaInEdgeIter;  // pointer to current SSA in-dep
  Value::use_iterator        ssaOutEdgeIter; // pointer to current SSA out-dep
  DependenceGraph*           funcDepGraph;    // the core dependence graph
  Dependence                 dep;            // the "current" dependence
  PDGIteratorFlags           depFlags:8;     // which deps are we enumerating?
  IterStateFlags             iterFlags:8;    // marking where the iter stands

  DepIterState(DependenceGraph* _funcDepGraph,
               Instruction&     I, 
               bool             incomingDeps,
               PDGIteratorFlags whichDeps);

  bool operator==(const DepIterState& S) {
    assert(funcDepGraph == S.funcDepGraph &&
           "Incompatible iterators! This is a probable sign of something BAD.");
    return (iterFlags == S.iterFlags &&
            dep == S.dep && depFlags == S.depFlags && depNode == S.depNode &&
            memDepIter == S.memDepIter && ssaInEdgeIter == S.ssaInEdgeIter &&
            ssaOutEdgeIter == S.ssaOutEdgeIter);
  }

  // Is the iteration completely done?
  // 
  bool done() const { return iterFlags & AllDone; }

  /// Next - Bump this iterator logically by 1 (to next dependence) and reset
  /// the dep field to represent the new dependence if there is one.
  /// Set done = true otherwise.
  /// 
  void Next();

  /// SetFirstMemoryDep - Find the first memory dependence for the current Mem
  /// In/Out iterators. Sets dep to that dependence and returns true if one is
  /// found. Returns false and leaves dep unchanged otherwise.
  /// 
  bool SetFirstMemoryDep();

  /// SetFirstSSADep - Find the next valid data dependence for the current SSA
  /// In/Out iterators. A valid data dependence is one that is to/from an
  /// Instruction. E.g., an SSA edge from a formal parameter is not a valid
  /// dependence. Sets dep to that dependence and returns true if a valid one is
  /// found. Returns false and leaves dep unchanged otherwise.
  ///
  bool SetFirstSSADep();
};


//---------------------------------------------------------------------------
/// PDGIterator Class - represents a pointer to a single dependence in the
/// program dependence graph.  It is essentially like a pointer to an object of
/// class Dependence but it is much more efficient to retrieve information about
/// the dependence directly rather than constructing the equivalent Dependence
/// object (since that object is normally not constructed for SSA def-use
/// dependences).
///
class PDGIterator: public forward_iterator<Dependence, ptrdiff_t> {
  DepIterState* istate;

#if 0
  /*copy*/     PDGIterator    (const PDGIterator& I);   // do not implement!
  PDGIterator& operator=      (const PDGIterator& I);   // do not implement!

  /*copy*/     PDGIterator    (PDGIterator& I) : istate(I.istate) {
    I.istate = NULL;                  // ensure this is not deleted twice.
  }
#endif

  friend class PgmDependenceGraph;

public:
  typedef PDGIterator _Self;

  PDGIterator(DepIterState* _istate) : istate(_istate) {}
  ~PDGIterator() { delete istate; }

  PDGIterator(const PDGIterator& I) :istate(new DepIterState(*I.istate)) {}

  PDGIterator& operator=(const PDGIterator& I) {
    if (istate) delete istate;
    istate = new DepIterState(*I.istate);
    return *this;
  }

  /// fini - check if the iteration is complete
  /// 
  bool fini() const { return !istate || istate->done(); }

  // Retrieve the underlying Dependence.  Returns NULL if fini().
  // 
  Dependence* operator*()  const { return fini() ?  NULL : &istate->dep; }
  Dependence* operator->() const { assert(!fini()); return &istate->dep; }

  // Increment the iterator
  // 
  _Self& operator++() { if (!fini()) istate->Next(); return *this;}
  _Self& operator++(int);   // do not implement!

  // Equality comparison: a "null" state should compare equal to done
  // This is efficient for comparing with "end" or with itself, but could
  // be quite inefficient for other cases.
  // 
  bool operator==(const PDGIterator& I) const {
    if (I.istate == NULL)               // most common case: iter == end()
      return (istate == NULL || istate->done());
    if (istate == NULL)
      return (I.istate == NULL || I.istate->done());
    return (*istate == *I.istate);
  }
  bool operator!=(const PDGIterator& I) const {
    return ! (*this == I);
  }
};


class PgmDependenceGraph: public FunctionPass {

public:
  static char ID; // Pass identification, replacement for typeid
  typedef PDGIterator iterator;
  /* typedef PDGIterator<const Dependence> const iterator; */


private:
  /// Information about the function being analyzed.
  /// 
  static PDGIterator NULLIterator;
  DependenceGraph* funcDepGraph;
  /// Information about one function being analyzed.
  const DSGraph*  funcGraph;
  const FunctionModRefInfo* funcModRef;

  friend class PDGIterator;
  friend class DepIterState;


public:
  PgmDependenceGraph() : FunctionPass(ID), funcDepGraph(NULL) {}
  ~PgmDependenceGraph() {}

  /// Iterators to enumerate the program dependence graph for a function.
  /// Note that this does not provide "end" iterators to check for completion.
  /// Instead, just use iterator::fini() or iterator::operator*() == NULL
  ///
  iterator  inDepBegin(Instruction& I, PDGIteratorFlags whichDeps = AllDeps) {
    return MakeIterator(I, /*inDeps*/ true, whichDeps);
  }
  iterator  inDepEnd  (Instruction& I, PDGIteratorFlags whichDeps = AllDeps) {
    return NULLIterator;
    //return MakeIterator();
  }
  iterator  outDepBegin(Instruction& I, PDGIteratorFlags whichDeps = AllDeps) {
    return MakeIterator(I, /*inDeps*/ false, whichDeps);
  }
  iterator  outDepEnd  (Instruction& I, PDGIteratorFlags whichDeps = AllDeps) {
    return NULLIterator;
    //return MakeIterator();
  }

  public:
  ///----END TEMPORARY FUNCTIONS---------------------------------------------

  bool buildPDG(Function& F);
  bool buildPDG2(Function& F);
  
  /// This initializes the program dependence graph iterator for a function.
  /// 
  bool runOnFunction(Function& F);

  /// MakeIterator - creates a null iterator representing end-of-iteration.
  /// 
  static PDGIterator  MakeIterator() { return PDGIterator(NULL); }

  /// getAnalysisUsage - This does not modify anything.
  /// It uses the Memory Dependence Analysis pass.
  /// It needs to use the PostDominanceFrontier pass, but cannot because
  /// that is a FunctionPass.  This means control dependence are not emumerated.
  ///
  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    //AU.addRequired<PostDominatorTree>();
    //AU.addRequired<AliasAnalysis>();
    //AU.addRequired<MemoryDependenceAnalysis>();
    AU.addRequired<IPModRef>();
    //AU.addRequired<LocalDataStructures>();
    //AU.addRequired<MemoryDepAnalysis>();
  }

  /// Debugging support methods
  /// 
  void printFuncDeps(Function* func, raw_ostream &O);
  void dump() const;

  private:
  // print helper function.
  void printOutgoingSSADeps(Instruction& I, raw_ostream &O);

  /// MakeIterator - creates and initializes an iterator as specified.
  ///
  PDGIterator   MakeIterator(Instruction& I, bool incomingDeps, PDGIteratorFlags whichDeps);

  void testSlicing(Function &F);
  /// Internal routine that processes each SCC of the CFG.                           
  void ProcessSCC(std::vector<BasicBlock*> &S, ModRefTable& ModRefAfter, bool hasLoop);
};

} // End llvm namespace

#endif
