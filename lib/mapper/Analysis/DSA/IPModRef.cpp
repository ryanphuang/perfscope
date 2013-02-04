//===- IPModRef.cpp - Compute IP Mod/Ref information ------------*- C++ -*-===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// See high-level comments in IPModRef.h
// 
//===----------------------------------------------------------------------===//

#include <vector>

#include "llvm/Module.h"
#include "llvm/Instructions.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringExtras.h"

#include "mapper/dsa/IPModRef.h"
#include "mapper/dsa/DataStructure.h"
#include "mapper/dsa/DSGraph.h"

namespace llvm {
Function *gFunc = NULL;
//----------------------------------------------------------------------------
// Private constants and data
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// class ModRefInfo
//----------------------------------------------------------------------------

void ModRefInfo::print(raw_ostream &O,
                       const std::string& sprefix) const
{
  O << sprefix << "Modified   nodes = " << modNodeSet;
  O << sprefix << "Referenced nodes = " << refNodeSet;
}

void ModRefInfo::dump() const
{
  print(errs());
}

//----------------------------------------------------------------------------
// class FunctionModRefInfo
//----------------------------------------------------------------------------


// This constructor computes a node numbering for the TD graph.
// 
FunctionModRefInfo::FunctionModRefInfo(const Function& func,
                                       IPModRef& ipmro,
                                       DSGraph* tdgClone)
  : F(func), IPModRefObj(ipmro), 
    funcTDGraph(tdgClone),
    funcModRefInfo(tdgClone->getGraphSize())
{
  unsigned i = 0;
  for (DSGraph::node_const_iterator NI = funcTDGraph->node_begin(),
         E = funcTDGraph->node_end(); NI != E; ++NI)
    NodeIds[NI] = i++;
}


FunctionModRefInfo::~FunctionModRefInfo()
{
  for(std::map<const Instruction*, ModRefInfo*>::iterator
        I=callSiteModRefInfo.begin(), E=callSiteModRefInfo.end(); I != E; ++I)
    delete(I->second);

  // Empty map just to make problems easier to track down
  callSiteModRefInfo.clear();

  delete funcTDGraph;
}

unsigned FunctionModRefInfo::getNodeId(const Value* value) const {
  return getNodeId(funcTDGraph->getNodeForValue(const_cast<Value*>(value)).getNode());
}



// Compute Mod/Ref bit vectors for the entire function.
// These are simply copies of the Read/Write flags from the nodes of
// the top-down DS graph.
// 
void FunctionModRefInfo::computeModRef(const Function &func)
{
  // Mark all nodes in the graph that are marked MOD as being mod
  // and all those marked REF as being ref.
  unsigned i = 0;
  for (DSGraph::node_iterator NI = funcTDGraph->node_begin(),
         E = funcTDGraph->node_end(); NI != E; ++NI, ++i) {
    if (NI->isModifiedNode()) 
        funcModRefInfo.setNodeIsMod(i);
    if (NI->isReadNode())
        funcModRefInfo.setNodeIsRef(i);
  }

  // Compute the Mod/Ref info for all call sites within the function.
  // The call sites are recorded in the TD graph.
  const DSGraph::FunctionListTy & callSites = funcTDGraph->getFunctionCalls();
  for (DSGraph::FunctionListTy::const_iterator csi = callSites.begin(), cse = callSites.end();
      csi != cse; csi++) 
    computeModRef(csi->getCallSite());
}


// ResolveCallSiteModRefInfo - This method performs the following actions:
//
//  1. It clones the top-down graph for the current function
//  2. It clears all of the mod/ref bits in the cloned graph
//  3. It then merges the bottom-up graph(s) for the specified call-site into
//     the clone (bringing new mod/ref bits).
//  4. It returns the clone, and a mapping of nodes from the original TDGraph to
//     the cloned graph with Mod/Ref info for the callsite.
//
// NOTE: Because this clones a dsgraph and returns it, the caller is responsible
//       for deleting the returned graph!
// NOTE: This method may return a null pointer if it is unable to determine the
//       requested information (because the call site calls an external
//       function or we cannot determine the complete set of functions invoked).
//
DSGraph* FunctionModRefInfo::ResolveCallSiteModRefInfo(CallSite CS,
                               std::map<const DSNode*, DSNodeHandle> &NodeMap)
{
  // Step #0: Quick check if we are going to fail anyway: avoid
  // all the graph cloning and map copying in steps #1 and #2.
  // 
  if (const Function *F = CS.getCalledFunction()) {
    if (F->isDeclaration())
      return 0;   // We cannot compute Mod/Ref info for this callsite...
  } else {
    // Eventually, should check here if any callee is external.
    // For now we are not handling this case anyway.
    errs() << "IP Mod/Ref indirect call not implemented yet: "
              << "Being conservative\n";
    return 0;   // We cannot compute Mod/Ref info for this callsite...
  }
  // Step #1: Clone the top-down graph...
  DSGraph *Result = new DSGraph(funcTDGraph, IPModRefObj.getGlobalECs(), 
    IPModRefObj.getTypeSS(), NodeMap);

  // Step #2: Clear Mod/Ref information...
  Result->maskNodeTypes(~(DSNode::ModifiedNode | DSNode::ReadNode));

  // Step #3: clone the bottom up graphs for the callees into the caller graph
  if (Function *F = CS.getCalledFunction())
    {
      if (F->isIntrinsic() || F->isDeclaration())
      {
        errs() << "Skip intrinsic or declaration function\n";

      }
      else {
        // Build up a DSCallSite for our invocation point here...

        // If the call returns a value, make sure to merge the nodes...
        DSNodeHandle RetVal;
        if (isa<PointerType>(CS.getInstruction()->getType()))
          RetVal = Result->getNodeForValue(CS.getInstruction());

                                                                                         
        // Calculate the arguments vector...
        // Add all fixed pointer arguments, then merge the rest together
        // Populate the arguments list...
        std::vector<DSNodeHandle> Args;
        Args.reserve(CS.arg_end()-CS.arg_begin());
        DSNodeHandle VarArgNH;
        const FunctionType *CalleeFuncType = DSCallSite::FunctionTypeOfCallSite(CS);       
        int NumFixedArgs = CalleeFuncType->getNumParams();
        for (CallSite::arg_iterator I = CS.arg_begin(), E = CS.arg_end();
            I != E; ++I)
          if (isa<PointerType>((*I)->getType())) {
            DSNodeHandle ArgNode = Result->getNodeForValue(*I);
            if (I - CS.arg_begin() < NumFixedArgs) {
                Args.push_back(ArgNode);
            } else {                                                                           
                VarArgNH.mergeWith(ArgNode);
            }
          }

        // Build the call site...
        DSCallSite NCS(CS, RetVal, VarArgNH,  F, Args);

        // Perform the merging now of the graph for the callee, which will
        // come with mod/ref bits set...
        Result->mergeInGraph(NCS, *F, IPModRefObj.getBUDSGraph(*F),
            DSGraph::StripAllocaBit
            | DSGraph::DontCloneCallNodes
            | DSGraph::DontCloneAuxCallNodes);
      }
    }
  else
    assert(0 && "See error message");

  // Remove dead nodes aggressively to match the caller's original graph.
  Result->removeDeadNodes(DSGraph::KeepUnreachableGlobals);

  // Step #4: Return the clone + the mapping (by ref)
  return Result;
}

// Compute Mod/Ref bit vectors for a single call site.
// These are copies of the Read/Write flags from the nodes of
// the graph produced by clearing all flags in the caller's TD graph
// and then inlining the callee's BU graph into the caller's TD graph.
// 
void
FunctionModRefInfo::computeModRef(CallSite CS)
{
  // Allocate the mod/ref info for the call site.  Bits automatically cleared.
  ModRefInfo* callModRefInfo = new ModRefInfo(funcTDGraph->getGraphSize());
  callSiteModRefInfo[CS.getInstruction()] = callModRefInfo;

  // Get a copy of the graph for the callee with the callee inlined
  std::map<const DSNode*, DSNodeHandle> NodeMap;
  DSGraph* csgp = ResolveCallSiteModRefInfo(CS, NodeMap);
  if (!csgp)
    { // Callee's side effects are unknown: mark all nodes Mod and Ref.
      // Eventually this should only mark nodes visible to the callee, i.e.,
      // exclude stack variables not reachable from any outgoing argument
      // or any global.
      callModRefInfo->getModSet().set();
      callModRefInfo->getRefSet().set();
      return;
    }

  // For all nodes in the graph, extract the mod/ref information
  for (DSGraph::node_iterator NI = funcTDGraph->node_begin(),
         E = funcTDGraph->node_end(); NI != E; ++NI) { 
    DSNode* csgNode = NodeMap[NI].getNode();
    assert(csgNode && "Inlined and original graphs do not correspond!");
    if (csgNode->isModifiedNode())
      callModRefInfo->setNodeIsMod(getNodeId(NI));
    if (csgNode->isReadNode())
      callModRefInfo->setNodeIsRef(getNodeId(NI));
  }

  // Drop nodemap before we delete the graph...
  NodeMap.clear();
  delete csgp;
}


class DSGraphPrintHelper {
  const DSGraph& tdGraph;
  std::vector<std::vector<const Value*> > knownValues; // identifiable objects

public:
  /*ctor*/ DSGraphPrintHelper(const FunctionModRefInfo& fmrInfo)
    : tdGraph(fmrInfo.getFuncGraph())
  {
    knownValues.resize(tdGraph.getGraphSize());

    // For every identifiable value, save Value pointer in knownValues[i]
    const DSGraph::ScalarMapTy &SM = tdGraph.getScalarMap();
    for (DSGraph::ScalarMapTy::const_iterator I = SM.begin(), E = SM.end(); I != E; ++I)
      if (isa<GlobalValue>(I->first) ||
          isa<Argument>(I->first) ||
          isa<LoadInst>(I->first) ||
         // isa<MallocInst>(I->first) || 
          isa<AllocaInst>(I->first)) 
        {
          unsigned nodeId = fmrInfo.getNodeId(I->second.getNode());
          knownValues[nodeId].push_back(I->first);
        }
  }

  void printValuesInBitVec(raw_ostream &O, const BitSetVector& bv) const
  {
    assert(bv.size() == knownValues.size());

    if (bv.none())
      { // No bits are set: just say so and return
        O << "\tNONE.\n";
        return;
      }

    if (bv.all())
      { // All bits are set: just say so and return
        O << "\tALL GRAPH NODES.\n";
        return;
      }

    for (unsigned i=0, N=bv.size(); i < N; ++i)
      if (bv.test(i))
        {
          O << "\tNode# " << i << " : ";
          if (! knownValues[i].empty())
            for (unsigned j=0, NV=knownValues[i].size(); j < NV; j++)
              {
                const Value* V = knownValues[i][j];

                if      (isa<GlobalValue>(V))  O << "(Global) ";
                else if (isa<Argument>(V))     O << "(Target of FormalParm) ";
                else if (isa<LoadInst>(V))     O << "(Target of LoadInst  ) ";
                else if (isa<AllocaInst>(V))   O << "(Target of AllocaInst) ";

                if (V->hasName())             O << V->getName();
                else if (isa<Instruction>(V)) O << *V;
                else                          O << "(Value*) 0x" << (void*) V;

                O << std::string((j < NV-1)? "; " : "\n");
              }
#if 0
          else
            tdGraph.getNodes()[i]->print(O, /*graph*/ NULL);
#endif
        }
  }
};


// Print the results of the pass.
// Currently this just prints bit-vectors and is not very readable.
// 
void FunctionModRefInfo::print(raw_ostream &O) const
{
  DSGraphPrintHelper DPH(*this);

  O << "========== Mod/ref information for function "
    << F.getName() << "========== \n\n";

  // First: Print Globals and Locals modified anywhere in the function.
  // 
  O << "  -----Mod/Ref in the body of function " << F.getName()<< ":\n";

  O << "    --Objects modified in the function body:\n";
  DPH.printValuesInBitVec(O, funcModRefInfo.getModSet());

  O << "    --Objects referenced in the function body:\n";
  DPH.printValuesInBitVec(O, funcModRefInfo.getRefSet());

  O << "    --Mod and Ref vectors for the nodes listed above:\n";
  funcModRefInfo.print(O, "\t");

  O << "\n";

  // Second: Print Globals and Locals modified at each call site in function
  // 
  for (std::map<const Instruction *, ModRefInfo*>::const_iterator
         CI = callSiteModRefInfo.begin(), CE = callSiteModRefInfo.end();
       CI != CE; ++CI)
    {
      O << "  ----Mod/Ref information for call site\n" << *CI->first;

      O << "    --Objects modified at call site:\n";
      DPH.printValuesInBitVec(O, CI->second->getModSet());

      O << "    --Objects referenced at call site:\n";
      DPH.printValuesInBitVec(O, CI->second->getRefSet());

      O << "    --Mod and Ref vectors for the nodes listed above:\n";
      CI->second->print(O, "\t");

      O << "\n";
    }

  O << "\n";
}

void FunctionModRefInfo::dump() const
{
  print(errs());
}


//----------------------------------------------------------------------------
// class IPModRef: An interprocedural pass that computes IP Mod/Ref info.
//----------------------------------------------------------------------------

// Run the "interprocedural" pass on each function.  This needs to do
// NO real interprocedural work because all that has been done the
// data structure analysis.
// 
bool IPModRef::runOnModule(Module &theModule)
{
  M = &theModule;

  if (gFunc == NULL) {
    errs() << "GFunc is NULL!!\n";
    return true;
  }
  for (Module::iterator FI = M->begin(), FE = M->end(); FI != FE; ++FI) {
    Function *f = FI;
    if (!f->isDeclaration() && f == gFunc) {
      getFuncInfo(*FI, true);
      break;
    }
  }
  return true;
}

/*
bool IPModRef::runOnFunction(Function &F)
{
  if (!F.isDeclaration())
    getFuncInfo(F, true);
  return true;
}
*/

FunctionModRefInfo& IPModRef::getFuncInfo(const Function& func, bool computeIfMissing)
{
  FunctionModRefInfo* &funcInfo = funcToModRefInfoMap[&func];
  assert (funcInfo != NULL || computeIfMissing);
  if (funcInfo == NULL)
  { // Create a new FunctionModRefInfo object.
      // Clone the top-down graph and remove any dead nodes first, because
      // otherwise original and merged graphs will not match.
      // The memory for this graph clone will be freed by FunctionModRefInfo.
      TDDataStructures & TDDS = getAnalysis<TDDataStructures>();
      GlobalECs = &TDDS.getGlobalECs();
      TypeSS = &TDDS.getTypeSS();
      assert(GlobalECs != NULL && TypeSS != NULL && "Cannot have NULL ECs");
      DSGraph* funcTDGraph = new DSGraph(TDDS.getDSGraph(func), *GlobalECs, *TypeSS);
      funcTDGraph->removeDeadNodes(DSGraph::KeepUnreachableGlobals);

      funcInfo = new FunctionModRefInfo(func, *this, funcTDGraph); //auto-insert
      funcInfo->computeModRef(func);  // computes the mod/ref info
  }
  return *funcInfo;
}

/// getBUDSGraph - This method returns the BU data structure graph for F through
/// the use of the BUDataStructures object.
///
const DSGraph &IPModRef::getBUDSGraph(const Function &F) {
  return *(getAnalysis<BUDataStructures>().getDSGraph(F));
}


// getAnalysisUsage - This pass requires top-down data structure graphs.
// It modifies nothing.
// 
void IPModRef::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<LocalDataStructures>();
  AU.addRequired<BUDataStructures>();
  AU.addRequired<TDDataStructures>();
}


void IPModRef::print(raw_ostream &O, const Module*) const
{
  O << "\nRESULTS OF INTERPROCEDURAL MOD/REF ANALYSIS:\n\n";
  
  for (std::map<const Function*, FunctionModRefInfo*>::const_iterator
         mapI = funcToModRefInfoMap.begin(), mapE = funcToModRefInfoMap.end();
       mapI != mapE; ++mapI)
    mapI->second->print(O);

  O << "\n";
}


void IPModRef::dump() const
{
  print(errs());
}

void IPModRef::releaseMemory()
{
  // Don't release here, we will refer it later!
  /*
  for(std::map<const Function*, FunctionModRefInfo*>::iterator
        I=funcToModRefInfoMap.begin(), E=funcToModRefInfoMap.end(); I != E; ++I)
    delete(I->second);
  // Clear map so memory is not re-released if we are called again
  funcToModRefInfoMap.clear();
  */
}

char IPModRef::ID = 0;
static RegisterPass<IPModRef> X("ipmodref", "Interprocedural mod/ref analysis");
} // End llvm namespace
