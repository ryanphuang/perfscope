/**
 *  @file          include/Graph/Container.h
 *
 *  @version       1.0
 *  @created       01/31/2013 08:26:47 PM
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
 *  Container type and tag definitions for node and edges
 *
 */

#ifndef __CONTAINER_H_
#define __CONTAINER_H_

#include <map>
#include <list>
#include <algorithm>
#include <set>
#include <vector>
#include <utility>
#include <iostream>

namespace llvm {

// #define CONTAINER_DEBUG 1

struct vec_storage_tag {};
struct list_storage_tag {};
struct set_storage_tag {};
struct map_storage_tag {};


template <typename NodeValTy, typename StorageTag = vec_storage_tag>
class Node;

template <typename StorageTag, typename NodeValTy>
struct node_container_type {
  // typedef .. type
};

template <typename NodeValTy>
struct node_container_type<map_storage_tag, NodeValTy> {
  typedef std::map<NodeValTy, Node<NodeValTy> *> type;
};

template <typename NodeValTy>
struct node_container_type<vec_storage_tag, NodeValTy> {
  typedef std::vector< Node<NodeValTy> *> type;
};

template <typename NodeValTy>
struct node_container_type<list_storage_tag, NodeValTy> {
  typedef std::list< Node<NodeValTy> *> type;
};

template <typename NodeValTy>
struct node_container_type<set_storage_tag, NodeValTy> {
  typedef std::set< Node<NodeValTy> *> type;
};


template <typename _Iterator>
class iterator_adapter {
  protected:
    typedef typename std::iterator_traits<_Iterator>::value_type value_type;
    typedef iterator_adapter<_Iterator> _Self;
    _Iterator _current;

  public:
    iterator_adapter() : _current(_Iterator()) {}
    iterator_adapter(const _Iterator current) : _current(current) {}

    _Self & operator++()
    {
      ++_current;
      return (*this);
    }
    bool operator==(const _Self & rhs) const
    {
      return _current == rhs.base();
    }
    bool operator!=(const _Self & rhs) const 
    {
      return !operator==(rhs);
    }
    const _Iterator & base() const
    {
      return _current;
    }
};

template <typename NodeValTy>
class map_iterator_adapter : public iterator_adapter< 
      typename node_container_type<map_storage_tag, NodeValTy>::type::iterator > {
  public:
      typedef typename node_container_type<map_storage_tag, NodeValTy>::type type;
      typedef typename type::iterator iterator;
      typedef iterator_adapter<iterator> super;
      typedef typename super::value_type value_type;
      typedef typename value_type::second_type second_type;
  
      map_iterator_adapter() : super() {} 
      map_iterator_adapter(const iterator it) : super(it) {} 
      second_type operator*() const
      {
        return this->_current->second;
      }
};

// The nodes that are stored in the container stores could be either created by the 
// container itself, or supplied from other places. 
// When the container is destructed, it's questionable as to whether to destroy the
// nodes also. We let the user of container to decide. 
// But user may wrong destructive, e.g., asking for destruction when all the nodes 
// are only added through add(NodeType * node), i.e., not created by the container and 
// may still referred elsewhere. Or the other way around.
// In these cases, issue warnings when doing such destruction.

#ifdef CONTAINER_DEBUG 
enum CleanupWarning {
  NoWarning  = 0,
  FineNow    = 0x1,
  Leakage    = 0x2,  // should destroy
  Corruption = 0x4   // should not destroy
};

inline void issue_cleanup_warning(CleanupWarning warning)
{
  switch(warning) {
    case NoWarning: 
    case FineNow: break;
    case Leakage: 
         std::cout << "Warning: there are nodes created inside the container \
but won't be destroyed upon destruction, potential leakage!" << std::endl; 
         break;
    case Corruption: 
         std::cout << "Warning: there nodes not created inside the container \
but will be destroyed upon destruction, potential corruption! " << std::endl; 
         break;
  }
}
#endif  // CONTAINER_DEBUG

template <typename StorageTag, typename NodeValTy>
struct NodeContainer {
  // type storage;
  // virtual NodeType * addNode(NodeValTy & val);
};

template <typename NodeValTy>
class NodeContainer<map_storage_tag, NodeValTy> {
  public:
    typedef Node<NodeValTy> NodeType;
    typedef typename node_container_type<map_storage_tag, NodeValTy>::type type;
    typedef map_iterator_adapter<NodeValTy> iterator;

  private:
    typedef typename type::iterator map_iterator;
    type storage;

    // destructive indicates whether the container should 
    // destroy all its elements upon exit
    bool destructive;

    #ifdef CONTAINER_DEBUG 
    CleanupWarning warning; 
    #endif

  public:
    NodeContainer(bool destructive = true) : destructive(destructive)
    {
      #ifdef CONTAINER_DEBUG 
      warning = NoWarning; 
      #endif
    }

    ~NodeContainer()
    {
      #ifdef CONTAINER_DEBUG
      issue_cleanup_warning(warning);
      #endif
      /* 
      if (destructive) {
        iterator I = begin();
        iterator E = end();
        NodeType * node;
        while (I != E) {
          node = *I;
          if (node != NULL)
            delete node;
          ++I;
        }
      }
      */
    }

    virtual inline iterator begin()
    {
      map_iterator b = storage.begin();
      return iterator(b);
    }

    virtual inline iterator end()
    {
      map_iterator e = storage.end();
      return iterator(e);
    }

    virtual inline void add(NodeType * node)
    {
      #ifdef CONTAINER_DEBUG
      if (destructive) {
        if (warning == FineNow) // called from add
          warning = NoWarning;  // clear flag
        else
          warning = Corruption; 
      }
      #endif
      storage.insert(std::make_pair(node->getNodeVal(), node));
    }

    virtual inline NodeType * add(NodeValTy & val)
    {
      #ifdef CONTAINER_DEBUG
      if (!destructive)
        warning = Leakage; 
      else
        warning = FineNow;
      #endif
      NodeType * node = new NodeType(val);
      add(node);
      return node;
    }

    virtual NodeType * get(NodeValTy & val, bool create = false)
    {
      map_iterator I = storage.find(val);
      if (I == storage.end()) 
        return create ? add(val) : NULL;
      else
        return I->second;
    }
};

template <typename NodeValTy>
class NodeContainer<vec_storage_tag, NodeValTy> {
  public:
    typedef Node<NodeValTy> NodeType;
    typedef typename node_container_type<vec_storage_tag, NodeValTy>::type type;
    typedef typename type::iterator iterator;

  private:
    type storage;
    // destructive indicates whether the container should 
    // destroy all its elements upon exit
    bool destructive;

    #ifdef CONTAINER_DEBUG
    CleanupWarning warning; 
    #endif

  public:
    // destructive indicates whether the container should destroy all its elements
    // upon exit
    NodeContainer(bool destructive = true) : destructive(destructive)
    {
      #ifdef CONTAINER_DEBUG
      warning = NoWarning;
      #endif
    }

    ~NodeContainer()
    {
      #ifdef CONTAINER_DEBUG
      issue_cleanup_warning(warning);
      #endif
      /*
      if (destructive) {
        iterator I = begin();
        iterator E = end();
        NodeType * node;
        while (I != E) {
          node = *I;
          if (node != NULL)
            delete node;
          ++I;
        }
      }
      */
    }

    virtual inline iterator begin()
    {
      return storage.begin();
    }

    virtual inline iterator end()
    {
      return storage.end();
    }

    virtual inline void add(NodeType * node)
    {
      #ifdef CONTAINER_DEBUG
      if (destructive) {
        if (warning == FineNow) // called from add
          warning = NoWarning;  // clear flag
        else
          warning = Corruption; 
      }
      #endif
      storage.push_back(node);
    }

    virtual inline NodeType * add(NodeValTy & val)
    {
      #ifdef CONTAINER_DEBUG
      if (!destructive)
        warning = Leakage;
      else
        warning = FineNow;
      #endif
      NodeType * node = new NodeType(val);
      add(node);
      return node;
    }

    virtual NodeType * get(NodeValTy & val, bool create = false)
    {
      iterator I, E;
      for (I = storage.begin(), E = storage.end(); I != E; I++) {
        if ((*I)->getNodeVal() == val)
          return *I;
      }
      return create ? add(val) : NULL;
    }
};

} // End of llvm namespace
#endif /* __CONTAINER_H_ */
