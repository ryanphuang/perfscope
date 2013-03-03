/**
 *  @file          include/Slicer/Slicer.h
 *
 *  @version       1.0
 *  @created       01/27/2013 08:28:02 PM
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
 *
 */

#ifndef __SLICER_H_
#define __SLICER_H_

#include <iterator>
#include <stack>
#include <vector>

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Debug.h"
#include "llvm/Instruction.h"
#include "llvm/Support/raw_ostream.h"

#include "dependence/DepGraph.h"
#include "dependence/DepIter.h"

namespace llvm {

struct Criterion {
  unsigned int line;
  Instruction * inst;
  bool forward;
  DepType request;

  Criterion(unsigned int line, Instruction *inst, bool forward, DepType request) : 
          line(line), inst(inst), forward(forward), request(request) {}
};


class Slicer {
    private:
      Criterion m_criterion;
      DepIterator * m_iter;

    protected:
      bool validCriterion();

    public:
      Slicer(DepGraph * graph, Criterion criterion); 
      ~Slicer();

      Instruction * next(); 
      void print(raw_ostream & OS);
      void dump() { print(dbgs()); }

};


} // End llvm namespace

#endif /* __SLICER_H_ */
