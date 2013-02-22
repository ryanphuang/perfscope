/**
 *  @file          CFGDAG.cpp
 *
 *  @version       1.0
 *  @created       02/11/2013 03:54:05 PM
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
 *  DAG for CFG implementation
 *
 */

#include "llvm/Support/raw_ostream.h"

#include "analyzer/CFGDAG.h"

//#define CFGDAG_DEBUG

using namespace llvm;

BBDAG::~BBDAG()
{
  for (NodeIterator I = _map.begin(), E = _map.end(); 
    I != E; ++I) {
    if (I->second)
      delete I->second;
  }
  if (_exit)
    delete _exit;
}

BBNode * BBDAG::getNode(const BasicBlock *BB)
{
  BBNode * node = _map[BB];
  if (node == NULL) {
    node = new BBNode(BB);
    _map[BB] = node;
  }
  return node;
}

void BBDAG::addEdge(BBNode * from, BBNode * to)
{
  from->addOutEdge(to);
  to->addInEdge(from);
}

void BBDAG::init()
{
  _root = getNode(_f.begin());
  _exit = new BBNode(NULL);
  std::stack<BBNode *> dfsStack;
  dfsStack.push(_root);
  while (!dfsStack.empty()) {
    BBNode * node = dfsStack.top();
    if (node->color != WHITE) {
      node->color = BLACK;
      dfsStack.pop();
      const TerminatorInst * terminator  = node->bb->getTerminator();
      if (isa<ReturnInst>(terminator) || isa<UnreachableInst>(terminator) ||
        isa<ResumeInst>(terminator)) 
        addEdge(node, _exit); // connect exit block to exit node
      continue;
    }
    node->color = GRAY;
    for (succ_const_iterator si = succ_begin(node->bb), se = succ_end(node->bb); 
      si != se; ++si) {
      BBNode * child = getNode(*si);
      switch (child->color) {
        case GRAY:
                  #ifdef CFGDAG_DEBUG
                  errs() << "Back edge <" << node->bb->getName() << ", " << 
                            (*si)->getName() << "> detected\n";
                  #endif
                  addEdge(node, _exit); // add dummy edge to exit node
                  break;
        case WHITE:
                  dfsStack.push(child);
        case BLACK:
                  addEdge(node, child); // already processed, only add edges
                  break;
      }
    }
  }
}

