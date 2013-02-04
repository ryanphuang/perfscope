/**
 *  @file          lib/Dependence/DepGraphBuilder.cpp
 *
 *  @version       1.0
 *  @created       01/28/2013 12:16:36 AM
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
 *  Constructing dependence graph using memory dependence analysis
 *
 */

#include "dependence/DepGraphBuilder.h"

//#define DEPBUILDER_DEBUG

using namespace llvm;

bool DepGraphBuilder::buildMemDepGraph()
{
  errs().write_escaped(F->getName()) << "\n";
  MemoryDependenceAnalysis & MDA = getAnalysis<MemoryDependenceAnalysis>();
  AliasAnalysis &AA = getAnalysis<AliasAnalysis>();
  for (inst_iterator I = inst_begin(*F), E = inst_end(*F); I != E; I++) {
    Instruction * inst = &*I;
    // Skip non-memory access instruction
    if (!inst->mayReadOrWriteMemory())
      continue;
    MemDepResult Res = MDA.getDependency(inst);
    if (!Res.isNonLocal()) {
      AddDepResultToGraph(inst, Res, TrueMemDep);
    } else if (CallSite CS = cast<Value>(inst)) {
      const MemoryDependenceAnalysis::NonLocalDepInfo &NLDI =
        MDA.getNonLocalCallDependency(CS);
      for (MemoryDependenceAnalysis::NonLocalDepInfo::const_iterator
          NLI = NLDI.begin(), NLE = NLDI.end(); NLI != NLE; ++NLI) {
        AddDepResultToGraph(inst, NLI->getResult(), AntiMemDep);
      }
    }
    else {
      SmallVector<NonLocalDepResult, 4> NLDI;
      if (LoadInst *load = dyn_cast<LoadInst>(inst)) {
        if (!load->isUnordered())
          continue;
        //AliasAnalysis::Location Loc(load->getPointerOperand());
        AliasAnalysis::Location Loc = AA.getLocation(load);
        MDA.getNonLocalPointerDependency(Loc, true, load->getParent(), NLDI);
      } else if (StoreInst *store = dyn_cast<StoreInst>(inst)) {
        if (!store->isUnordered())
          continue;
        //AliasAnalysis::Location Loc(store->getPointerOperand());
        AliasAnalysis::Location Loc = AA.getLocation(store);
        MDA.getNonLocalPointerDependency(Loc, false, store->getParent(), NLDI);
      } else if (VAArgInst *VI = dyn_cast<VAArgInst>(inst)) {
        AliasAnalysis::Location Loc = AA.getLocation(VI);
        MDA.getNonLocalPointerDependency(Loc, false, VI->getParent(), NLDI);
      } else {
        llvm_unreachable("Unknown memory instruction!");
      }
      for (SmallVectorImpl<NonLocalDepResult>::const_iterator
          I = NLDI.begin(), E = NLDI.end(); I != E; ++I) {
        AddDepResultToGraph(inst, I->getResult(), OutMemDep);
      }
    }
    #ifdef DEPBUILDER_DEBUG
    errs() << "|";
    inst->print(errs());
    errs() << "\n";
    #endif
  }
  //m_graph->mem_graph->print(errs());
  return true;
}

bool DepGraphBuilder::buildDepGraph()
{
  assert(F != NULL);
  if (m_graph!= NULL) {
    delete m_graph;
    m_graph = NULL;
  }
  m_graph = new DepGraph();
  if (m_request & MemDep) {
    m_graph->mem_graph = new MemDepGraph(); 
    buildMemDepGraph();
  }
  if (m_request & SSADep)
    m_graph->ssa_graph = new SSADepGraph(); 
  return true;
}

inline void DepGraphBuilder::AddDepResultToGraph(Instruction * inst, 
      const MemDepResult & Res, MemDepType type)
{
  Instruction * r = Res.getInst();
  if (inst == r) // Avoid loop node
    return;
  if (r && (!m_must || Res.isDef())) {
    //TODO, we ignore the type for now
    m_graph->mem_graph->addEdge(r, inst);
    #ifdef DEPBUILDER_DEBUG
    errs() << " >";
    r->print(errs());
    errs() << "\n";
    #endif
  }
}

char DepGraphBuilder::ID = 0;
static RegisterPass<DepGraphBuilder> X("depass", "Memory dependence pass");

// INITIALIZE_PASS_BEGIN(DepGraphBuilder, "print-memdeps",
//                       "Print MemDeps of function", false, true)
// INITIALIZE_PASS_DEPENDENCY(DepGraphBuilder)
// INITIALIZE_PASS_END(DepGraphBuilder, "print-memdeps",
//                      "Print MemDeps of function", false, true)
