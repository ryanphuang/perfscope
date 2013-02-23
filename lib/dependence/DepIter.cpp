/**
 *  @file          lib/Dependence/DepIter.cpp
 *
 *  @version       1.0
 *  @created       01/25/2013 10:31:52 PM
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
 *  Dependence iterator implementation
 *
 */

#include "llvm/Support/raw_ostream.h"

#include "dependence/DepIter.h"

using namespace llvm;

bool DepIterator::validate()
{
  if (m_inst == NULL)
    return false;
  if ((m_request & MemDep) && m_graph->mem_graph == NULL)
    return false;
  if ((m_request & SSADep) && m_graph->ssa_graph == NULL) 
    return false;
  return true;
}
DepIterator::DepIterator(DepGraph * graph, Instruction * inst, DepType request, bool forward)
    : m_state(Ready), m_graph(graph), m_inst(inst), m_request(request), m_forward(forward)
{
  assert(validate() && "Invalid request parameter");
  bool end = true;
  if (m_request & MemDep)
    end = end && !setMemDepIter();
  if (m_request & SSADep)
    end = end && !setSSADepIter();
  if (end)
    m_state |= AllDone;
}

bool DepIterator::setSSADepIter()
{
  m_depinst = NULL;
  if (!(m_request & SSADep) || (m_state & SSADone)) {
    m_state |= SSADone; // mark SSADone so the test of done() can succeed
    return false;
  }
  // we don't increment on first time
  if (m_state & Ready) {
    m_state &= ~Ready; // clear ready flag
    m_ssa_iter = SSADepIter::begin(m_graph->ssa_graph, m_inst, !m_forward);
  }
  else
    ++m_ssa_iter;
  for(; m_ssa_iter != SSADepIter::end() && (m_depinst = dyn_cast<Instruction>(
      *m_ssa_iter)) == NULL; ++m_ssa_iter);
  if (!m_depinst) {
    m_state |= SSADone;
    return false;
  }
  m_state |= Start;
  return true;
}

bool DepIterator::setMemDepIter()
{
  m_depinst = NULL; // invalidate depinst
  if (!(m_request & MemDep) || (m_state & MemDone) ||
        !m_inst->mayReadOrWriteMemory()) { // ignore all non-memory instructions
    m_state |= MemDone; // mark MemDone so the test of done() can succeed
    return false;
  }
  // we don't increment on first time
  if (m_state & Ready) {
    m_state &= ~Ready; // clear ready flag
    m_mem_iter = MemDepIter::begin(m_graph->mem_graph, m_graph->mem_graph->get(m_inst), !m_forward);
  }
  else
    ++m_mem_iter; 
  for(; m_mem_iter != MemDepIter::end() && (m_depinst = dyn_cast<Instruction>(
    (*m_mem_iter)->getNodeVal())) == NULL; ++m_mem_iter);
  if (!m_depinst) {
    m_state |= MemDone;
    return false;
  }
  m_state |= Start;
  return true;
}

bool DepIterator::next()
{
  // When the quest is AllDep, we first exhaust memory dependence and then SSA
  if (m_request & MemDep && !(m_state & MemDone)) {
    if (setMemDepIter())
      return true;
  }
  if (m_request & SSADep && !(m_state & SSADone)) {
    if (setSSADepIter())
      return true;
  }
  //TODO: control dependency
  return false;
}

Instruction * DepIterator::getInst() const
{
  if (done())
    return NULL;
  return m_depinst;
}

// } // End of llvm namespace
