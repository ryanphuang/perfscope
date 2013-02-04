/**
 *  @file          include/Graph/Graph.h
 *
 *  @version       1.0
 *  @created       01/24/2013 02:51:10 PM
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
 *  Generic graph definition
 *
 */

#ifndef __GRAPH_H_
#define __GRAPH_H_

#include <map>
#include <list>
#include <set>
#include <vector>
#include <utility>


#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/ilist_node.h"

#include "graph/Container.h"
#include "graph/Node.h"

namespace llvm {

template <typename G>
struct GraphConcept {
  /// The following is only a template, not every graph needs to define them
  
  typedef typename G::NodeType NodeType;
  typedef typename G::EdgeType EdgeType;
  typedef typename G::in_iterator in_iterator;
  typedef typename G::out_iterator out_iterator;
  typedef typename G::node_iterator node_iterator;
  typedef typename G::edge_iterator edge_iterator;

  static node_iterator node_begin(G * graph);
  static node_iterator node_end(G * graph);

  static in_iterator in_begin(G *graph, NodeType *node); 
  static in_iterator in_end(G *graph, NodeType *node);
  static out_iterator out_begin(G *graph, NodeType *node);
  static out_iterator out_end(G *graph, NodeType *node);
};

template <typename EdgeValTy>
class Edge;

struct NoneNoneType {}; // a special mark for none type

//TODO template parameter is currently unused for the base graph
template <typename NodeValTy>
class GraphBase {
  private:
    size_t V; // number of nodes
    size_t E; // number of edges

  public:
    GraphBase(size_t v = 0, size_t e = 0) : V (v), E(e) {}
    virtual void print(raw_ostream &OS, bool full = true)
    {
      OS << "Graph size: |V|=" << V << ", |E|=" << E << "\n";
    }
    virtual inline void inc_node() { ++V; }
    virtual inline void inc_edge() { ++E; }
    virtual inline size_t num_nodes() { return V; }
    virtual inline size_t num_edges() { return E; }
};


template <typename NodeValTy>
class MapGraphBase: public GraphBase<NodeValTy>, 
                    public NodeContainer<map_storage_tag, NodeValTy> {
  public:
    typedef GraphBase<NodeValTy> super1;
    typedef NodeContainer<map_storage_tag, NodeValTy> super2;

    typedef Node<NodeValTy> NodeType;
    typedef typename super2::iterator node_iterator;
    typedef typename NodeType::in_iterator in_iterator;
    typedef typename NodeType::out_iterator out_iterator;

  public:
    ~MapGraphBase()
    {
      node_iterator I = super2::begin(), E = super2::end();
      while (I != E) {
        NodeType * node = *I;
        if (node)
          delete node;
        ++I;
      }
    }
    virtual inline NodeType * add(NodeValTy & val)
    {
      super1::inc_node();
      return super2::add(val);
    }
    virtual inline void add(NodeType * node)
    {
      super2::add(node);
      super1::inc_node();
    }
    virtual bool addEdge(NodeValTy &fval, NodeValTy &tval)
    {
      NodeType * from = get(fval, true);
      NodeType * to = get(tval, true);
      return addEdge(from, to);
    }
    virtual bool addEdge(NodeType *from, NodeType *to)
    {
      super1::inc_edge();
      from->addOutNeighbor(to);
      to->addInNeighbor(from);
      return true;
    }

    virtual void print(raw_ostream &OS, bool full = true)
    {
      super1::print(OS);
      OS << "\n";
      node_iterator I = super2::begin(), E = super2::end();
      while (I != E) {
        NodeType * node = *I;
        if (full)
          node->print_full(OS);
        else
          node->print(OS);
        ++I;
      }
    }
};

//-=================================================================================-
// Class: Graph - Generic graph definition
//
//
//-----------------------------------------------------------------------------------

template <typename NodeValTy, typename EdgeValTy>
class Graph {

  private:
    size_t V; // number of nodes
    size_t E; // number of edges

    typedef std::map<NodeValTy, GraphNode<NodeValTy, EdgeValTy> *> GraphNodeMapTy;
    typedef typename GraphNodeMapTy::iterator nmap_iterator;
    
    GraphNodeMapTy graphNodeMap;

  public:
    typedef GraphNode<NodeValTy, EdgeValTy>  NodeType;
    typedef typename NodeType::iterator iterator;
    typedef typename NodeType::const_iterator const_iterator;

    Graph() : V(0), E(0) {}
    ~Graph()
    {
      nmap_iterator I = graphNodeMap.begin();
      nmap_iterator E = graphNodeMap.end();
      for (; I != E ; I++)
        delete I->second;
      graphNodeMap.clear();
    }

    virtual inline NodeType * addNode(NodeValTy & val)
    {
      NodeType * node = new NodeType(val);
      graphNodeMap.insert(std::make_pair(val, node));
      V++;
      return node;
    }

    virtual NodeType * getNode(NodeValTy & val, bool create = false)
    {
      nmap_iterator I = graphNodeMap.find(val);
      if (I == graphNodeMap.end()) 
        return create ? addNode(val) : NULL;
      else
        return I->second;
    }

    virtual bool addEdge(NodeValTy &fval, NodeValTy &tval, EdgeValTy edge)
    {
      NodeType * from = getNode(fval, true);
      NodeType * to = getNode(tval, true);
      return addEdge(from, to, edge);
    }

    virtual bool addEdge(NodeType *from, NodeType *to, EdgeValTy edge)
    {
      E++;
      from->addOutEdge(std::make_pair(to, edge));
      to->addInEdge(std::make_pair(from, edge));
      return true;
    }

    inline iterator in_begin(NodeType & node) { return node.in_begin(); }
    inline const_iterator in_begin(const NodeType & node) const { return node.in_begin(); }
    inline iterator in_end(NodeType & node) { return node.in_end(); }
    inline const_iterator in_end(const NodeType & node) const { return node.in_end(); }

    inline iterator out_begin(NodeType &node) { return node.out_begin(); }
    inline const_iterator out_begin(const NodeType &node) const { return node.out_begin(); }
    inline iterator out_end(NodeType &node) { return node.out_end(); }
    inline const_iterator out_end(const NodeType &node) const { return node.out_end(); }
  
    virtual void print(raw_ostream &O)
    {
      nmap_iterator I = graphNodeMap.begin();
      nmap_iterator E = graphNodeMap.end();
      for (; I != E ; I++) {
        I->second->print_full(O);
      }
    }

    virtual void summary(raw_ostream &O)
    {
      O << "Graph size: |V|=" << V << ", |E|=" << E << "\n";
    }
};

} // End of llvm namespace

#endif /* __GRAPH_H_ */
