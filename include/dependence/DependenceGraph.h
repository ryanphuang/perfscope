/**
 *  @file          include/Dependence/DependenceGraph.h
 *
 *  @version       1.0
 *  @created       01/24/2013 01:20:59 PM
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
 *  Dependence graph definition.
 *
 *  The dependence only contains memory dependency, but not SSA dependency, which 
 *  is already provided from Instruction *.
 *
 *
 */

#ifndef __DEPENDENCEGRAPH_H_
#define __DEPENDENCEGRAPH_H_

#include "llvm/Instruction.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Casting.h"

#include "graph/Graph.h"
#include "graph/Container.h"
#include "graph/DFSIter.h"
#include "dependence/Dependence.h"

namespace llvm {

/// Dependence graph node, essentially Instruction, with MemDepType as edge type
typedef GraphNode<Instruction *, MemDepType> DepNode;
typedef Graph<Instruction *, MemDepType> DependenceGraph;
typedef DFSIterator<Instruction *, MemDepType> DepDFSIter;

typedef Node<Instruction *> InstNode;

typedef MapGraphBase<Instruction *> MemDepGraph;


template<>
struct NodeConcept<InstNode> {
  typedef Instruction * NodeValTy;
  inline static NodeValTy getNodeVal(InstNode * node)
  {
    return node->getNodeVal();
  }

};

template<>
struct GraphConcept<MemDepGraph>
{
  typedef MemDepGraph::NodeType NodeType;
  typedef MemDepGraph::node_iterator node_iterator;
  typedef MemDepGraph::in_iterator in_iterator;
  typedef MemDepGraph::out_iterator out_iterator;

  static node_iterator node_begin(MemDepGraph *graph)
  {
    return graph->begin();
  }
  static node_iterator node_end(MemDepGraph *graph)
  {
    return graph->end();
  }
  static in_iterator in_begin(MemDepGraph *graph, NodeType *node) 
  {
    return node->in_begin();
  }
  static in_iterator in_end(MemDepGraph *graph, NodeType *node)
  {
    return node->in_end();
  }
  static out_iterator out_begin(MemDepGraph *graph, NodeType *node)
  {
    return node->out_begin();
  }
  static out_iterator out_end(MemDepGraph *grap, NodeType *node)
  {
    return node->out_end();
  }
};

template<>
struct NodeConcept<Instruction> {
  typedef Instruction NodeType;
  typedef Instruction * NodeValTy;
  // this is weird, but we simply return node ptr as value here
  inline static NodeValTy getNodeVal(NodeType * node) 
  {
    return node; 
  }
  inline static void print(raw_ostream &OS, NodeType * node)
  {
    if (node == NULL)
      return;
    OS << *node << "\n";
  }

  static void print_full(raw_ostream &OS, NodeType * node)
  {
    if (node == NULL)
      return;
    OS << "|";
    print(OS, node);
    Instruction * inst;
    OS << "In:\n";
    Instruction::op_iterator OI = node->op_begin();
    Instruction::op_iterator OE = node->op_end();
    while(OI != OE) {
      if ((inst = dyn_cast<Instruction>(*OI)) != NULL) {
        OS << " <";
        print(OS, inst);
      }
      ++OI;
    }
    OS << "Out:\n";
    Instruction::use_iterator UI = node->use_begin();
    Instruction::use_iterator UE = node->use_end();
    while(UI != UE) {
      OS << " >";
      if ((inst = dyn_cast<Instruction>(*UI)) != NULL)
        print(OS, inst);
      ++UI;
    }
  }
};

// We don't really need a graph to store SSA dependence
class SSADepGraph 
{
  public:
    class in_iterator: public iterator_adapter<Instruction::op_iterator>  {
      typedef iterator_adapter<Instruction::op_iterator> super;
      public:
        in_iterator() : super() {}
        in_iterator(const Instruction::op_iterator & oi) : super(oi) {}
        Instruction *operator*() const
        {
          return dyn_cast<Instruction>(*this->_current);
        }
    };
    class out_iterator: public iterator_adapter<Instruction::use_iterator>  {
      typedef iterator_adapter<Instruction::use_iterator> super;
      public:
        out_iterator() : super() {}
        out_iterator(const Instruction::use_iterator & ui) : super(ui) {}
        Instruction *operator*() const
        {
          return dyn_cast<Instruction>(*this->_current);
        }
    };

    // Since SSADepGraph is a dummy graph with no nodes
    // We rely on other concrete instruction graph to pass 
    // the nodes for it to print
    template<typename InstGraph>
    void print(raw_ostream & OS, InstGraph * graph, bool full = true)
    {
      // no need to do print by ourselves if it's not full print
      if (!full) {
        graph->print(OS, full);
        return;
      }
      typedef typename GraphConcept<InstGraph>::node_iterator node_iterator;
      node_iterator I = GraphConcept<InstGraph>::node_begin(graph);
      node_iterator E = GraphConcept<InstGraph>::node_end(graph);
      while (I != E) {
        Instruction * inst= NodeConcept<InstNode>::getNodeVal(*I);
        NodeConcept<Instruction>::print_full(OS, inst);
        ++I;
      }
    }
};

template<>
struct GraphConcept<SSADepGraph>
{
  typedef Instruction NodeType;
  typedef SSADepGraph::in_iterator in_iterator;
  typedef SSADepGraph::out_iterator out_iterator;

  // DON'T SUPPORT THE FOLLOWING
  //
  // static node_iterator node_begin(SSADepGraph *graph);
  // static node_iterator node_end(SSADepGraph *graph);

  static in_iterator in_begin(SSADepGraph *graph, NodeType *node) 
  {
    return in_iterator(node->op_begin());
  }
  static in_iterator in_end(SSADepGraph *graph, NodeType *node)
  {
    return in_iterator(node->op_end());
  }
  static out_iterator out_begin(SSADepGraph *graph, NodeType *node)
  {
    return out_iterator(node->use_begin());
  }
  static out_iterator out_end(SSADepGraph *grap, NodeType *node)
  {
    return out_iterator(node->use_end());
  }
};

class DepGraph {
  public:
    MemDepGraph * mem_graph;
    SSADepGraph * ssa_graph;
    DepGraph(MemDepGraph * mem_graph = NULL, SSADepGraph * ssa_graph = NULL)
      : mem_graph(mem_graph), ssa_graph(ssa_graph) {}
    ~DepGraph()
    {
      if (mem_graph != NULL)
        delete mem_graph;
      if (ssa_graph != NULL)
        delete ssa_graph;
    }
    
    void print(raw_ostream &OS)
    {
      if (mem_graph != NULL) {
        OS << "===========\n";
        OS << "Memory Dependence Graph\n";
        OS << "===========\n";
        mem_graph->print(OS);
        OS << "\n\n";
        // Only print SSA graph when there's memory dependence graph
        if (ssa_graph != NULL) { 
          OS << "===========\n";
          OS << "SSA Dependence Graph\n";
          OS << "===========\n";
          ssa_graph->print(OS, mem_graph);
          OS << "===========\n";
        }
      }
    }
};

} //End of llvm namespace


#endif /* __DEPENDENCEGRAPH_H_ */
