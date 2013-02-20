/**
 *  @file          include/Graph/Node.h
 *
 *  @version       1.0
 *  @created       01/24/2013 04:41:38 PM
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
 *  Generic graph node definition
 *
 */

#ifndef __GRAPHNODE_H_
#define __GRAPHNODE_H_

#include <utility>
#include <vector>


#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/ilist_node.h"

namespace llvm {

//-=================================================================================-
// Wrapper to get around situations when the argument could be both reference and 
// pointer type.
//
//-----------------------------------------------------------------------------------
template <typename T>
T * ptr(T & obj) { return &obj; }

template <typename T>
T * ptr(T * obj) { return obj; }

template <typename Node>
struct NodeConcept {
  typedef typename Node::NodeValTy NodeValTy;
  static NodeValTy getNodeVal(Node * node);
};

template <typename NodeValTy, typename StorageTag>
class Node {
  private:
    NodeValTy val; 
    typedef NodeContainer<StorageTag, NodeValTy> container_type;
    container_type in_neighbor;
    container_type out_neighbor;
    
  public:
    //TODO:
    // For now, we use the same container for IN and OUT neighbor
    // should support different container types
    //
    typedef typename container_type::iterator in_iterator; 
    typedef typename container_type::iterator out_iterator;

    Node(NodeValTy & v) : val (v) {}

    inline in_iterator in_begin()
    {
      return in_neighbor.begin();
    }
    inline in_iterator in_end()
    {
      return in_neighbor.end();
    }
    inline out_iterator out_begin()
    {
      return out_neighbor.begin();
    }
    inline out_iterator out_end()
    {
      return out_neighbor.end();
    }
    inline bool addOutNeighbor(NodeValTy & val) { out_neighbor.add(val); return true; }
    inline bool addInNeighbor(NodeValTy & val) { in_neighbor.add(val); return true; }
    inline bool addOutNeighbor(Node *node) { out_neighbor.add(node); return true; }
    inline bool addInNeighbor(Node *node) { in_neighbor.add(node); return true; }

    inline const NodeValTy & getNodeVal() const { return val; }
    inline const NodeValTy * getNodeValPtr() const { return &val; }

    virtual inline void print(raw_ostream &OS)
    {
      OS << *ptr(val) << "\n"; // use ptr to support both pointer and reference val
    }

    virtual void print_full(raw_ostream &OS)
    { 
      OS << "|";
      print(OS);
      OS << "In:\n";
      for (in_iterator I = in_begin(), E = in_end(); I != E; I++) {
        OS << " <";
        (*I)->print(OS);
      }
      OS << "Out:\n";
      for (out_iterator I = out_begin(), E = out_end(); I != E; I++) {
        OS << " >";
        (*I)->print(OS);
      }
    }
};


//-=================================================================================-
// Class: GraphNode - Generic graph node definition
//
// Ideally, the graph node should just store node information without be tied to a 
// particular edge type, and it's beneficial that there can be multiple types of edge
// between one type of nodes.
//
// But if edge type is not coupled, the adjacency list for graph node can't be defined
// here, delaying that to Graph definition, say maintaining a map from node to edge 
// vector will incur big overhead.
//
// 
// Note: The NodeValTy in the template could be reference like int, or pointer like
//       Instruction *. Most times, it doesn't matter. But when it do, such as print
//       the value, if it's pointer we want to print the actual obj's value instead
//       of the pointer itself. That's why in these cases, we wrap val with ptr
//-----------------------------------------------------------------------------------


template <typename NodeValTy, typename EdgeValTy>
class GraphNode {
  public:
    typedef std::pair<GraphNode *, EdgeValTy> HalfEdge;
    template <typename N, typename E> friend class Graph;

  private:
    NodeValTy val; 
    std::vector<HalfEdge> out_neighbors;
    std::vector<HalfEdge> in_neighbors;

  public:
    GraphNode(NodeValTy & v) : val (v) {}

    typedef typename std::vector<HalfEdge>::iterator iterator;
    typedef typename std::vector<HalfEdge>::const_iterator const_iterator;

    inline iterator in_begin() { return in_neighbors.begin(); }
    inline const_iterator in_begin() const { return in_neighbors.begin(); }
    inline iterator in_end() { return in_neighbors.end(); }
    inline const_iterator in_end() const { return in_neighbors.end(); }

    inline iterator out_begin() { return out_neighbors.begin(); }
    inline const_iterator out_begin() const { return out_neighbors.begin(); }
    inline iterator out_end() { return out_neighbors.end(); }
    inline const_iterator out_end() const { return out_neighbors.end(); }

    inline bool addOutEdge(HalfEdge edge) { out_neighbors.push_back(edge); return true; }
    inline bool addInEdge(HalfEdge edge) { in_neighbors.push_back(edge); return true; }

    inline NodeValTy & getNodeVal() { return val; }
    inline NodeValTy * getNodeValPtr() { return &val; }
    inline const NodeValTy & getNodeVal() const { return val; }
    inline const NodeValTy * getNodeValPtr() const { return &val; }

    virtual inline void print(raw_ostream &O, bool newline = true)
    {
      O << *ptr(val); // use ptr to support both pointer and reference val
      if (newline) 
        O << "\n";
    }

  protected:
    virtual void print_neighbor(raw_ostream &O, iterator begin, iterator end)
    {
      for (iterator I = begin, E = end; I != E; I++) {
        I->first->print(O, false);
        O << "," << I->second; 
      }
      O << "}\n";
    }

    virtual void print_full(raw_ostream &O)
    { 
      print(O);
      O << "\t In neighbors: {";
      print_neighbor(O, in_neighbors.begin(), in_neighbors.end());
      O << "\t Out neighbors: {";
      print_neighbor(O, out_neighbors.begin(), out_neighbors.end());
    }

};

} // End of llvm namespace



#endif /* __GRAPHNODE_H_ */
