/**
 *  @file          include/Dependence/DepIter.h
 *
 *  @version       1.0
 *  @created       01/25/2013 03:07:04 PM
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
 *  Dependence graph iterator
 *
 *  It iterates both the dependence graph itself but also SSA dependence.
 *
 */

#ifndef __DEPITER_H_
#define __DEPITER_H_

#include <iterator>
#include <stack>

#include "graph/DFSIter.h"
#include "dependence/Dependence.h"
#include "dependence/DepGraph.h"

namespace llvm {


enum IterState {
  Ready   = 1 << 0,
  Start   = 1 << 1,
  SSADone = 1 << 2,
  MemDone = 1 << 3,
  AllDone = 1 << 4
};

typedef DFSIter<MemDepGraph> MemDepIter;
typedef DFSIter<SSADepGraph> SSADepIter;
/*
template<typename DepGraph>
class DepIter : public DFSIter<DepGraph> {
  private:
    typedef DepIter<DepGraph> _Self;
    typedef DFSIter<DepGraph> super;
    typedef typename GraphConcept<DepGraph>::NodeType NodeType;
    typedef typename NodeConcept<NodeType>::NodeValType NodeValTy;
    super _current;
  protected:
    void next()
    {
      NodeValTy val;
      do{
        ++_current; 
        val = **this;
      } while(val == NULL);
    }
  public:
    DepIter(DepGraph * graph, NodeType * node, bool in = false) :
    {
      _current = supper::begin(graph, node, in);
      if (**this != NULL)
        ++*this;
    }
    _Self & operator++() 
    { 
      next();
      return *this;
    }
    inline NodeValTy *operator*() const 
    {
      return NodeConcept<NodeType>::getNodeVal(*_current);
    }
};
*/

class DepIterator: public std::iterator<std::forward_iterator_tag, DepNode::HalfEdge> {
  public:
    typedef DepIterator _Self;

  public:
    unsigned char              m_state;        // state of the iterator
    DepGraph    *              m_graph;        // dependence graph
    Instruction *              m_inst;         // instruction to be analyzed
    DepType                    m_request;      // dependence request type
    bool                       m_forward;      // forward iterator or backward
                                               // instruction to be analyzed
    Instruction *              m_depinst;      // instruction that has dependency with 
                                               // the target instruction 

    MemDepIter                 m_mem_iter;     // DFS iterator for memory dependence graph
    SSADepIter                 m_ssa_iter;     // DFS iterator for ssa dependence graph

  protected:
    bool validate();
    bool setSSADepIter();
    bool setMemDepIter();

  public:
    DepIterator(DepGraph * graph, Instruction * inst, DepType request, bool forward);

    bool next();
    _Self & operator++() { next(); return *this;}
    Instruction *operator*() const {return getInst();}

    bool done() const { return m_state & AllDone; }
    Instruction * getInst() const;
};


}

#endif /* __DEPITER_H_ */
