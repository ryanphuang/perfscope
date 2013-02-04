/**
 *  @file          include/Graph/DFSIter.h
 *
 *  @version       1.0
 *  @created       01/28/2013 09:01:32 PM
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
 *  Generic graph DFS iterator, based on llvm/ADT/DepthFirstIterator.h
 *
 */

#ifndef __DFSITER_H_
#define __DFSITER_H_

#include <iterator>
#include <stack>

#include "llvm/ADT/PointerIntPair.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/Casting.h"

#include "graph/Graph.h"

namespace llvm {

template<typename IncidenceGraph>
class DFSIter : public std::iterator< std::forward_iterator_tag, 
          typename GraphConcept<IncidenceGraph>::NodeType > {
  private:
    typedef typename GraphConcept<IncidenceGraph>::NodeType NodeType;
    typedef std::iterator<std::forward_iterator_tag, NodeType> super;
    typedef typename GraphConcept<IncidenceGraph>::in_iterator in_iterator;
    typedef typename GraphConcept<IncidenceGraph>::out_iterator out_iterator;
    typedef PointerIntPair<NodeType *, 1> PointerIntTy;
    typedef IncidenceGraph G;

    IncidenceGraph * m_graph;
    NodeType * m_node;
    typedef std::stack< std::pair<PointerIntTy, in_iterator> > in_list_type;
    typedef std::stack< std::pair<PointerIntTy, out_iterator> > out_list_type;
    in_list_type m_in_worklist;
    out_list_type m_out_worklist;

    SmallPtrSet<NodeType *, 8> m_visited;
    bool m_in_visit;   // whether to visit in edges

  private:
    DFSIter(IncidenceGraph * graph, NodeType * node, bool in = false) : m_graph(graph), 
        m_in_visit(in)
    {
      if (node != NULL) {
        if (m_in_visit)
          m_in_worklist.push(std::make_pair(PointerIntTy(node, 0), 
            GraphConcept<IncidenceGraph>::in_begin(m_graph, node)));
        else
          m_out_worklist.push(std::make_pair(PointerIntTy(node, 0), 
            GraphConcept<IncidenceGraph>::out_begin(m_graph, node)));
      }
    }

  protected:
    void in_next()
    {
      while (!m_in_worklist.empty()) {
        std::pair<PointerIntTy, in_iterator> & top = m_in_worklist.top();
        in_iterator &I = top.second;
        if (!top.first.getInt()) {
          top.first.setInt(1);
          I = GraphConcept<IncidenceGraph>::in_begin(m_graph, top.first.getPointer());
        }
        in_iterator E;
        E = GraphConcept<IncidenceGraph>::in_end(m_graph, top.first.getPointer());
        for (; I != E; ++I) {
          //is_node<IncidenceGraph, std::iterator_traits<
          NodeType * node = *I; 
          if (node && !m_visited.count(node)) { // node not visited before
            m_visited.insert(node);
            // no need to fill in the true iterator now 
            m_in_worklist.push(std::make_pair(PointerIntTy(node, 0), 
                  GraphConcept<IncidenceGraph>::in_begin(m_graph, node)));
            return;
          }
        }
        m_in_worklist.pop(); // pop it now
      }
    }

    void out_next()
    {
      while (!m_out_worklist.empty()) {
        std::pair<PointerIntTy, out_iterator> & top = m_out_worklist.top();
        out_iterator &I = top.second;
        if (!top.first.getInt()) {
          top.first.setInt(1);
          I = GraphConcept<IncidenceGraph>::out_begin(m_graph, top.first.getPointer());
        }
        out_iterator E;
        E = GraphConcept<IncidenceGraph>::out_end(m_graph, top.first.getPointer());
        for (; I != E; ++I) {
          NodeType * node = *I; 
          if (node && !m_visited.count(node)) { // node not visited before
            m_visited.insert(node);
            // no need to fill in the true iterator now 
            m_out_worklist.push(std::make_pair(PointerIntTy(node, 0), 
                  GraphConcept<IncidenceGraph>::out_begin(m_graph, node)));
            return;
          }
        }
        m_out_worklist.pop(); // pop it now
      }
    }

  public:
    DFSIter() // Default constructor
    {
    }

    typedef typename super::pointer pointer;
    typedef DFSIter<IncidenceGraph> _Self;

    static inline _Self begin(IncidenceGraph * graph, NodeType * node, bool in = false)
    {
      return _Self(graph, node, in);
    }

    static inline _Self end()
    {
      return END;
    }
  
    _Self & operator++()
    {
      if (m_in_visit)
        in_next(); 
      else
        out_next(); 
      return (*this);
    }

    bool operator==(const _Self & rhs) const
    {
      // only test work list
      return m_in_worklist == rhs.m_in_worklist && m_out_worklist == rhs.m_out_worklist;
    }

    bool operator!=(const _Self & rhs) const 
    {
      return !operator==(rhs);
    }

    NodeType *operator*() const
    {
      if (m_in_visit) {
        if (m_in_worklist.empty())
          return NULL;
        return m_in_worklist.top().first.getPointer();
      }
      else {
        if (m_out_worklist.empty())
          return NULL;
        return m_out_worklist.top().first.getPointer();
      }
    }

  private:
    static _Self END;
};

template<typename G>
DFSIter<G> DFSIter<G>::END = DFSIter<G>();


template<class NodeValTy, typename EdgeValTy>
class DFSIterator : public std::iterator< std::forward_iterator_tag, 
                                        GraphNode<NodeValTy, EdgeValTy> > {
  private:
    typedef Graph<NodeValTy, EdgeValTy> GraphType;
    typedef typename GraphType::NodeType NodeType;
    typedef std::iterator<std::forward_iterator_tag, NodeType> super;
    typedef typename NodeType::iterator n_iterator;
    typedef PointerIntPair<NodeType *, 1> PointerIntTy;

    NodeType * m_node;
    std::stack< std::pair<PointerIntTy, n_iterator> > m_worklist;
    SmallPtrSet<NodeType *, 8> m_visited;
    bool m_in_visit;   // whether to visit in edges

  private:
    DFSIterator(NodeType * node, bool in = false) : m_in_visit(in)
    {
      m_worklist.push(std::make_pair(PointerIntTy(node, 0), node->in_begin()));
    }

  protected:

    void visitNext()
    {
      while (!m_worklist.empty()) {
        std::pair<PointerIntTy, n_iterator> & top = m_worklist.top();
        n_iterator &I = top.second;
        if (!top.first.getInt()) {
          top.first.setInt(1);
          if (m_in_visit)
            I = top.first.getPointer()->in_begin();
          else
            I = top.first.getPointer()->out_begin();
        }
        n_iterator E;
        if (m_in_visit)
          E = top.first.getPointer()->in_end();
        else
          E = top.first.getPointer()->out_end();
        for (; I != E; I++) {
          NodeType * node = I->first; 
          if (node && !m_visited.count(node)) { // node not visited before
            m_visited.insert(node);
            // no need to fill in the true iterator now 
            m_worklist.push(std::make_pair(PointerIntTy(node, 0), node->in_begin()));
            return;
          }
        }
        m_worklist.pop(); // pop it now
      }
    }
  

  public:
    DFSIterator() // Default constructor
    {
    }

    typedef typename super::pointer pointer;
    typedef DFSIterator<NodeValTy, EdgeValTy> _Self;

    static inline _Self begin(NodeType * node, bool in = false)
    {
      return _Self(node, in);
    }

    static inline _Self end()
    {
      return END;
    }
  
    _Self & operator++()
    {
      visitNext();
      return (*this);
    }

    bool operator==(const _Self & rhs) const
    {
      // only test work list
      return m_worklist == rhs.m_worklist;
    }

    bool operator!=(const _Self & rhs) const 
    {
      return !operator==(rhs);
    }

    NodeType *operator*() const
    {
      if (m_worklist.empty())
        return NULL;
      return m_worklist.top().first.getPointer();
      
    }

  private:
    static _Self END;

};

template<class N, class E>
DFSIterator<N, E> DFSIterator<N, E>::END = DFSIterator<N, E>();

}

#endif /* __DFSITER_H_ */
