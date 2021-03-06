/**
 *  @file          CFGDAG.h
 *
 *  @version       1.0
 *  @created       02/11/2013 03:47:45 PM
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
 *  DAG for processing CFG
 *
 */

#ifndef __CFGDAG_H_
#define __CFGDAG_H_

#include <map>
#include <stack>

#include "llvm/Instructions.h"
#include "llvm/Function.h"

#include "llvm/Support/CFG.h"

namespace llvm {

enum BBNodeColor {
  WHITE, 
  GRAY,
  BLACK 
};

struct BBNode {
  const BasicBlock * bb;
  BBNodeColor color;
  std::vector<BBNode *> pred;
  std::vector<BBNode *> succ;
  unsigned cost;
  unsigned in_cnt;

  typedef std::vector<BBNode *>::iterator edgeIter;

  BBNode(const BasicBlock * bb, BBNodeColor color = WHITE, unsigned cost = 0) : 
    bb(bb), color(color), cost(cost), in_cnt(0) {}

  inline unsigned dec_in_count()
  {
    return in_cnt == 0 ? 0 : --in_cnt;
  }

  inline unsigned get_in_count()
  {
    return in_cnt;
  }

  inline edgeIter in_begin()
  {
    return pred.begin();
  }

  inline edgeIter in_end()
  {
    return pred.end();
  }

  inline edgeIter out_begin()
  {
    return succ.begin();
  }

  inline edgeIter out_end()
  {
    return succ.end();
  }

  inline void addOutEdge(BBNode *to)
  {
    succ.push_back(to);
  }

  inline void addInEdge(BBNode *from)
  {
    in_cnt++;
    pred.push_back(from);
  }
};


class BBDAG {
  public:
    typedef std::map<const BasicBlock *, BBNode *> BBNodeMapTy;
    typedef BBNodeMapTy::iterator NodeIterator;

  protected:
    Function & _f;
    BBNode * _root;
    BBNode * _exit;
    BBNodeMapTy _map;

  public:

    BBDAG (Function &F) : _f(F), _root(NULL), _exit(NULL) { init(); }
    ~BBDAG();
    void init();
    inline BBNode * getEntryNode() {return _root;}
    inline BBNode * getExitNode() {return _exit;}
    BBNode * getNode(const BasicBlock *BB);
    inline bool isExitNode(const BBNode * node) const {return node->bb == NULL;}
    void addEdge(BBNode * from, BBNode * to);
};

} // End of llvm namespace

#endif /* __CFGDAG_H_ */
