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

#include <map>
#include <iostream>

#include "llvm/Analysis/PostDominators.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/InstVisitor.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/Function.h"
#include "llvm/IntrinsicInst.h"

#include "slicer/PDGSlicer.h"
#include "dependence/PgmDependenceGraph.h"
#include "mapper/dsa/DSGraph.h"

namespace llvm {

class DependenceGraph *globalDPG = NULL;

///--------------------------------------------------------------------------
/// class ModRefInfoBuilder:
/// 
/// A simple InstVisitor<> class that retrieves the Mod/Ref info for
/// Load/Store/Call instructions and inserts this information in
/// a ModRefTable.  It also records all instructions that Mod any node
/// and all that use any node.
///--------------------------------------------------------------------------

///--------------------------------------------------------------------------
/// struct ModRefTable:
/// 
/// A data structure that tracks ModRefInfo for instructions:
///   -- modRefMap is a map of Instruction* -> ModRefInfo for the instr.
///   -- definers  is a vector of instructions that define    any node
///   -- users     is a vector of instructions that reference any node
///   -- numUsersBeforeDef is a vector indicating that the number of users
///                seen before definers[i] is numUsersBeforeDef[i].
/// 
/// numUsersBeforeDef[] effectively tells us the exact interleaving of
/// definers and users within the ModRefTable.
/// This is only maintained when constructing the table for one SCC, and
/// not copied over from one table to another since it is no longer useful.
///--------------------------------------------------------------------------

class ModRefTable {
public:
  typedef std::map<Instruction*, ModRefInfo> ModRefMap;
  typedef ModRefMap::const_iterator                 const_map_iterator;
  typedef ModRefMap::iterator                       map_iterator;
  typedef std::vector<Instruction*>::const_iterator const_ref_iterator;
  typedef std::vector<Instruction*>::iterator       ref_iterator;

  ModRefMap                 modRefMap;
  std::vector<Instruction*> definers;
  std::vector<Instruction*> users;
  std::vector<unsigned>     numUsersBeforeDef;

  // Iterators to enumerate all the defining instructions
  const_ref_iterator defsBegin()  const {  return definers.begin(); }
        ref_iterator defsBegin()        {  return definers.begin(); }
  const_ref_iterator defsEnd()    const {  return definers.end(); }
        ref_iterator defsEnd()          {  return definers.end(); }

  // Iterators to enumerate all the user instructions
  const_ref_iterator usersBegin() const {  return users.begin(); }
        ref_iterator usersBegin()       {  return users.begin(); }
  const_ref_iterator usersEnd()   const {  return users.end(); }
        ref_iterator usersEnd()         {  return users.end(); }

  // Iterator identifying the last user that was seen *before* a
  // specified def.  In particular, all users in the half-closed range
  //    [ usersBegin(), usersBeforeDef_End(defPtr) )
  // were seen *before* the specified def.  All users in the half-closed range
  //    [ usersBeforeDef_End(defPtr), usersEnd() )
  // were seen *after* the specified def.
  // 
  ref_iterator usersBeforeDef_End(const_ref_iterator defPtr) {
    unsigned defIndex = (unsigned) (defPtr - defsBegin());
    assert(defIndex < numUsersBeforeDef.size());
    assert(usersBegin() + numUsersBeforeDef[defIndex] <= usersEnd()); 
    return usersBegin() + numUsersBeforeDef[defIndex]; 
  }
  const_ref_iterator usersBeforeDef_End(const_ref_iterator defPtr) const {
    return const_cast<ModRefTable*>(this)->usersBeforeDef_End(defPtr);
  }

  // 
  // Modifier methods
  // 
  void AddDef(Instruction* D) {
    definers.push_back(D);
    numUsersBeforeDef.push_back(users.size());
  }
  void AddUse(Instruction* U) {
    users.push_back(U);
  }
  void Insert(const ModRefTable& fromTable) {
    modRefMap.insert(fromTable.modRefMap.begin(), fromTable.modRefMap.end());
    definers.insert(definers.end(),
                    fromTable.definers.begin(), fromTable.definers.end());
    users.insert(users.end(),
                 fromTable.users.begin(), fromTable.users.end());
    numUsersBeforeDef.clear(); 
  }
};


class ModRefInfoBuilder: public InstVisitor<ModRefInfoBuilder> {
  const DSGraph& funcGraph;
  const FunctionModRefInfo& funcModRef;
  class ModRefTable&  modRefTable;

  ModRefInfoBuilder();                         // DO NOT IMPLEMENT
  ModRefInfoBuilder(const ModRefInfoBuilder&); // DO NOT IMPLEMENT
  void operator=(const ModRefInfoBuilder&);    // DO NOT IMPLEMENT

public:
  ModRefInfoBuilder(const DSGraph& _funcGraph, const FunctionModRefInfo& _funcModRef,
    ModRefTable& _modRefTable) : funcGraph(_funcGraph), funcModRef(_funcModRef), modRefTable(_modRefTable)
  {
  }

  // At a call instruction, retrieve the ModRefInfo using IPModRef results.
  // Add the call to the defs list if it modifies any nodes and to the uses
  // list if it refs any nodes.
  // 
  void visitCallInst(CallInst& callInst) {
    ModRefInfo safeModRef(funcGraph.getGraphSize());
    const ModRefInfo* callModRef = funcModRef.getModRefInfo(callInst);
    if (callModRef == NULL) {
      // call to external/unknown function: mark all nodes as Mod and Ref
      safeModRef.getModSet().set();
      safeModRef.getRefSet().set();
      callModRef = &safeModRef;
    }

    modRefTable.modRefMap.insert(std::make_pair(&callInst,
                                                ModRefInfo(*callModRef)));
    if (callModRef->getModSet().any())
      modRefTable.AddDef(&callInst);
    if (callModRef->getRefSet().any())
      modRefTable.AddUse(&callInst);
  }

  // At a store instruction, add to the mod set the single node pointed to
  // by the pointer argument of the store.  Interestingly, if there is no
  // such node, that would be a null pointer reference!
  void visitStoreInst(StoreInst& storeInst) {
    const DSNodeHandle& ptrNode =
      funcGraph.getNodeForValue(storeInst.getPointerOperand());
    if (const DSNode* target = ptrNode.getNode()) {
      unsigned nodeId = funcModRef.getNodeId(target);
      ModRefInfo& minfo =
        modRefTable.modRefMap.insert(
          std::make_pair(&storeInst,
                         ModRefInfo(funcGraph.getGraphSize()))).first->second;
      minfo.setNodeIsMod(nodeId);
      modRefTable.AddDef(&storeInst);
    } else
      errs() << "Warning: Uninitialized pointer reference!\n";
  }

  // At a load instruction, add to the ref set the single node pointed to
  // by the pointer argument of the load.  Interestingly, if there is no
  // such node, that would be a null pointer reference!
  void visitLoadInst(LoadInst& loadInst) {
    const DSNodeHandle& ptrNode =
      funcGraph.getNodeForValue(loadInst.getPointerOperand());
    if (const DSNode* target = ptrNode.getNode()) {
      unsigned nodeId = funcModRef.getNodeId(target);
      ModRefInfo& minfo =
        modRefTable.modRefMap.insert(
          std::make_pair(&loadInst,
                         ModRefInfo(funcGraph.getGraphSize()))).first->second;
      minfo.setNodeIsRef(nodeId);
      modRefTable.AddUse(&loadInst);
    } else
      errs() << "Warning: Uninitialized pointer reference!\n";
  }
};


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

/// Basic dependence gathering algorithm, using scc_iterator on CFG:
/// 
/// for every SCC S in the CFG in PostOrder on the SCC DAG
///     {
///       for every basic block BB in S in *postorder*
///         for every instruction I in BB in reverse
///           Add (I, ModRef[I]) to ModRefCurrent
///           if (Mod[I] != NULL)
///               Add I to DefSetCurrent:  { I \in S : Mod[I] != NULL }
///           if (Ref[I] != NULL)
///               Add I to UseSetCurrent:  { I       : Ref[I] != NULL }
/// 
///       for every def D in DefSetCurrent
/// 
///           // NOTE: D comes after itself iff S contains a loop
///           if (HasLoop(S) && D & D)
///               Add output-dep: D -> D2
/// 
///           for every def D2 *after* D in DefSetCurrent
///               // NOTE: D2 comes before D in execution order
///               if (D & D2)
///                   Add output-dep: D2 -> D
///                   if (HasLoop(S))
///                       Add output-dep: D -> D2
/// 
///           for every use U in UseSetCurrent that was seen *before* D
///               // NOTE: U comes after D in execution order
///               if (U & D)
///                   if (U != D || HasLoop(S))
///                       Add true-dep: D -> U
///                   if (HasLoop(S))
///                       Add anti-dep: U -> D
/// 
///           for every use U in UseSetCurrent that was seen *after* D
///               // NOTE: U comes before D in execution order
///               if (U & D)
///                   if (U != D || HasLoop(S))
///                       Add anti-dep: U -> D
///                   if (HasLoop(S))
///                       Add true-dep: D -> U
/// 
///           for every def Dnext in DefSetAfter
///               // NOTE: Dnext comes after D in execution order
///               if (Dnext & D)
///                   Add output-dep: D -> Dnext
/// 
///           for every use Unext in UseSetAfter
///               // NOTE: Unext comes after D in execution order
///               if (Unext & D)
///                   Add true-dep: D -> Unext
/// 
///       for every use U in UseSetCurrent
///           for every def Dnext in DefSetAfter
///               // NOTE: Dnext comes after U in execution order
///               if (Dnext & D)
///                   Add anti-dep: U -> Dnext
/// 
///       Add ModRefCurrent to ModRefAfter: { (I, ModRef[I] ) }
///       Add DefSetCurrent to DefSetAfter: { I : Mod[I] != NULL }
///       Add UseSetCurrent to UseSetAfter: { I : Ref[I] != NULL }
///     }
///         
///
void PgmDependenceGraph::ProcessSCC(std::vector<BasicBlock*> &S, ModRefTable& ModRefAfter, bool hasLoop)
{

  ModRefTable ModRefCurrent;
  ModRefTable::ModRefMap& mapCurrent = ModRefCurrent.modRefMap;
  ModRefTable::ModRefMap& mapAfter   = ModRefAfter.modRefMap;

  // Builder class fills out a ModRefTable one instruction at a time.
  // To use it, we just invoke it's visit function for each basic block:
  // 
  //   for each basic block BB in the SCC in *postorder*
  //       for each instruction  I in BB in *reverse*
  //           ModRefInfoBuilder::visit(I)
  //           : Add (I, ModRef[I]) to ModRefCurrent.modRefMap
  //           : Add I  to ModRefCurrent.definers if it defines any node
  //           : Add I  to ModRefCurrent.users    if it uses any node
  // 
  ModRefInfoBuilder builder(*funcGraph, *funcModRef, ModRefCurrent);

  for (std::vector<BasicBlock*>::iterator BI = S.begin(), BE = S.end();
      BI != BE; ++BI) {
    // outs() << (*BI)->getName() << "\t";
    // Note: BBs in the SCC<> created by scc_iterator are in postorder.
    BasicBlock::InstListType & instList = (*BI)->getInstList();
    for (BasicBlock::InstListType::reverse_iterator II=instList.rbegin(), IE=instList.rend(); 
        II !=IE; ++II) {
      Instruction *I = &(*II);
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

      // outs() << *II << "||";
      builder.visit(*II);
    }
    // outs() << "\n";
  }

  ///       for every def D in DefSetCurrent
  /// 
  for (ModRefTable::ref_iterator II=ModRefCurrent.defsBegin(),
      IE=ModRefCurrent.defsEnd(); II != IE; ++II)
  {
    ///           // NOTE: D comes after itself iff S contains a loop
    ///           if (HasLoop(S))
    ///               Add output-dep: D -> D2
    if (hasLoop)
      funcDepGraph->AddSimpleDependence(**II, **II, OutputDependence);

    ///           for every def D2 *after* D in DefSetCurrent
    ///               // NOTE: D2 comes before D in execution order
    ///               if (D2 & D)
    ///                   Add output-dep: D2 -> D
    ///                   if (HasLoop(S))
    ///                       Add output-dep: D -> D2
    for (ModRefTable::ref_iterator JI=II+1; JI != IE; ++JI)
      if (!Disjoint(mapCurrent.find(*II)->second.getModSet(),
            mapCurrent.find(*JI)->second.getModSet()))
      {
        funcDepGraph->AddSimpleDependence(**JI, **II, OutputDependence);
        if (hasLoop)
          funcDepGraph->AddSimpleDependence(**II, **JI, OutputDependence);
      }

    ///           for every use U in UseSetCurrent that was seen *before* D
    ///               // NOTE: U comes after D in execution order
    ///               if (U & D)
    ///                   if (U != D || HasLoop(S))
    ///                       Add true-dep: U -> D
    ///                   if (HasLoop(S))
    ///                       Add anti-dep: D -> U
    {
      ModRefTable::ref_iterator JI=ModRefCurrent.usersBegin();
      ModRefTable::ref_iterator JE = ModRefCurrent.usersBeforeDef_End(II);
      for ( ; JI != JE; ++JI)
        if (!Disjoint(mapCurrent.find(*II)->second.getModSet(),
              mapCurrent.find(*JI)->second.getRefSet()))
        {
          if (*II != *JI || hasLoop)
            funcDepGraph->AddSimpleDependence(**II, **JI, TrueDependence);
          if (hasLoop)
            funcDepGraph->AddSimpleDependence(**JI, **II, AntiDependence);
        }

      ///           for every use U in UseSetCurrent that was seen *after* D
      ///               // NOTE: U comes before D in execution order
      ///               if (U & D)
      ///                   if (U != D || HasLoop(S))
      ///                       Add anti-dep: U -> D
      ///                   if (HasLoop(S))
      ///                       Add true-dep: D -> U
      for (/*continue JI*/ JE = ModRefCurrent.usersEnd(); JI != JE; ++JI)
        if (!Disjoint(mapCurrent.find(*II)->second.getModSet(),
              mapCurrent.find(*JI)->second.getRefSet()))
        {
          if (*II != *JI || hasLoop)
            funcDepGraph->AddSimpleDependence(**JI, **II, AntiDependence);
          if (hasLoop)
            funcDepGraph->AddSimpleDependence(**II, **JI, TrueDependence);
        }
    }

    ///           for every def Dnext in DefSetPrev
    ///               // NOTE: Dnext comes after D in execution order
    ///               if (Dnext & D)
    ///                   Add output-dep: D -> Dnext
    for (ModRefTable::ref_iterator JI=ModRefAfter.defsBegin(),
        JE=ModRefAfter.defsEnd(); JI != JE; ++JI)
      if (!Disjoint(mapCurrent.find(*II)->second.getModSet(),
            mapAfter.find(*JI)->second.getModSet()))
        funcDepGraph->AddSimpleDependence(**II, **JI, OutputDependence);

    ///           for every use Unext in UseSetAfter
    ///               // NOTE: Unext comes after D in execution order
    ///               if (Unext & D)
    ///                   Add true-dep: D -> Unext
    for (ModRefTable::ref_iterator JI=ModRefAfter.usersBegin(),
        JE=ModRefAfter.usersEnd(); JI != JE; ++JI)
      if (!Disjoint(mapCurrent.find(*II)->second.getModSet(),
            mapAfter.find(*JI)->second.getRefSet()))
        funcDepGraph->AddSimpleDependence(**II, **JI, TrueDependence);
  }

  /// 
  ///       for every use U in UseSetCurrent
  ///           for every def Dnext in DefSetAfter
  ///               // NOTE: Dnext comes after U in execution order
  ///               if (Dnext & D)
  ///                   Add anti-dep: U -> Dnext
  for (ModRefTable::ref_iterator II=ModRefCurrent.usersBegin(),
      IE=ModRefCurrent.usersEnd(); II != IE; ++II)
    for (ModRefTable::ref_iterator JI=ModRefAfter.defsBegin(),
        JE=ModRefAfter.defsEnd(); JI != JE; ++JI)
      if (!Disjoint(mapCurrent.find(*II)->second.getRefSet(),
            mapAfter.find(*JI)->second.getModSet()))
        funcDepGraph->AddSimpleDependence(**II, **JI, AntiDependence);

  ///       Add ModRefCurrent to ModRefAfter: { (I, ModRef[I] ) }
  ///       Add DefSetCurrent to DefSetAfter: { I : Mod[I] != NULL }
  ///       Add UseSetCurrent to UseSetAfter: { I : Ref[I] != NULL }
  ModRefAfter.Insert(ModRefCurrent);
}

bool PgmDependenceGraph::buildPDG2(Function& F)
{

  assert(funcModRef != NULL);
  //funcModRef =  &getAnalysis<IPModRef>().getFunctionModRefInfo(F);
  funcGraph  = &funcModRef->getFuncGraph();

  ModRefTable ModRefAfter;
  // unsigned sccNum = 0;
  // outs() << "SCCs for Function " << F.getName() << " in PostOrder:";
  for (scc_iterator<Function*> I = scc_begin(&F), E = scc_end(&F); I != E; ++I) {
    // outs() << "\nSCC #" << ++sccNum << " : ";
    ProcessSCC(*I, ModRefAfter, I.hasLoop());
  }
  return true;
}

// The following implementation didn't fully capture all the memory dependencies.
// TODO investigate why
//     hint: the MemeoryDependenceyAnalysis only reply one result for a given 
//           instruction while we need a set of dependencies.
//
bool PgmDependenceGraph::buildPDG(Function& F)
{
  MemoryDependenceAnalysis &mda = getAnalysis<MemoryDependenceAnalysis>();
  for (Function::iterator FI = F.begin(), FE = F.end(); FI != FE; FI++) {
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
                funcDepGraph->AddSimpleDependence(*dep, *I, AntiDependence);
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

void PgmDependenceGraph::testSlicing(Function &F)
{
  PDGSlicer slicer(funcDepGraph);
  for (Function::iterator BB=F.begin(), FE=F.end(); BB != FE; ++BB)
    for (BasicBlock::iterator II=BB->begin(), IE=BB->end(); II !=IE; ++II)
    {
      Instruction *I = II;
      if (isa<DbgInfoIntrinsic>(I)) { // Skip intrinsics 
        continue;
      }
      if (slicer.sliceInit(*I, AllDeps)) {
        errs() << "Slicing " << *I << "\n\t";
        Instruction *N;
        while((N = slicer.sliceNext()) != NULL) {
          errs() << *N << "||";
        }
        errs() << "\n";
      }
    }
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

bool PgmDependenceGraph::runOnFunction(Function& F) 
{
  funcDepGraph = new DependenceGraph();
  buildPDG(F);
  //buildPDG2(F);
  //printFuncDeps(&F, errs());
  //testSlicing(F);
  //delete funcDepGraph;
  //funcDepGraph = NULL;
  return true;
}

char PgmDependenceGraph::ID = 0;
PgmDependenceGraph::iterator PgmDependenceGraph::NULLIterator = PgmDependenceGraph::MakeIterator();
static RegisterPass<PgmDependenceGraph> X("pgmdep", "Enumerate Program Dependence Graph (data and control)");

} // End llvm namespace
