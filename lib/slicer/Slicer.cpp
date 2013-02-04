/**
 *  @file          lib/Slicer/Slicer.cpp
 *
 *  @version       1.0
 *  @created       01/27/2013 08:24:31 PM
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
 *  Slicer to get a list of instructions for a given slicing criteria
 *
 */

#include "graph/DFSIter.h"
#include "slicer/Slicer.h"


using namespace llvm;

Slicer::Slicer(DepGraph &graph, Criterion criterion) : m_criterion(criterion)
{ 
  m_iter = new DepIterator(graph, m_criterion.inst, 
    m_criterion.request, m_criterion.forward);
}

Slicer::~Slicer()
{
  if (m_iter != NULL) {
    delete m_iter;
    m_iter = NULL;
  }
}

Instruction * Slicer::next()
{
  if (m_iter->done())
    return NULL;
  // we increment regardless of whether it's first time,
  // because the first node in DFS is the node itself, thus should 
  // be skipped
  // we don't increment on first time
  ++*m_iter;
  return m_iter->getInst();
}

void Slicer::print(raw_ostream & OS)
{
  //OS << "|" << *(m_criterion.inst) << "\n";
  Instruction * inst;
  while ((inst = next()) != NULL) {
    OS << " " << *inst << "\n";
  }
}

