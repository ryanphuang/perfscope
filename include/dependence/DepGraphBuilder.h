/**
 *  @file          include/Dependence/DepGraphBuilder.h
 *
 *  @version       1.0
 *  @created       01/28/2013 12:04:08 AM
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

#ifndef __DEPGRAPHBUILDER_H_
#define __DEPGRAPHBUILDER_H_

#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Instruction.h"

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"

#include "dependence/Dependence.h"
#include "dependence/DependenceGraph.h"

namespace llvm {

class DepGraphBuilder : public FunctionPass {
  private:
    Function * F;
    DepGraph * m_graph;
    DepType m_request;
    bool m_must; // Is it must-dependence analysis
  
  protected:
    bool buildMemDepGraph();

  public:
    static char ID; // Pass identification, replacement for typeid
    DepGraphBuilder (DepType request = DataDep, bool must = false) : 
      FunctionPass(ID), m_graph(NULL), m_request(request), m_must(must) 
    {
    }
    ~DepGraphBuilder ()  
    {
      if (m_graph != NULL) 
        delete m_graph;
    }

    bool buildDepGraph();

    inline DepGraph * getDepGraph() {return m_graph;}

    virtual bool runOnFunction(Function &F) {this->F = &F; buildDepGraph(); return false;}

    // We don't modify the program, so we preserve all analyses
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequiredTransitive<AliasAnalysis>();
      AU.addRequiredTransitive<MemoryDependenceAnalysis>();
      AU.setPreservesAll();
    }

  private:
    void AddDepResultToGraph(Instruction * inst, const MemDepResult & Res, MemDepType type);
};


} // End of llvm namespace

#endif /* __DEPGRAPHBUILDER_H_ */
