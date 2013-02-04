/**
 *  @file          Dependence.h
 *
 *  @version       1.0
 *  @created       01/28/2013 01:16:50 PM
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
 *  Dependence type definitions
 *
 */

#ifndef __DEPENDENCE_H_
#define __DEPENDENCE_H_

#include "llvm/Support/raw_ostream.h"

namespace llvm {

enum DepType {
  SSADep  = 1 << 0,
  MemDep  = 1 << 1,
  DataDep = SSADep | MemDep,
  CtrlDep = 1 << 2,
  AllDep  = CtrlDep | DataDep
};

enum MemDepType {
  NoneMemDep  = 0,
  TrueMemDep  = 1 << 0,
  AntiMemDep  = 1 << 1,
  OutMemDep   = 1 << 2
};

raw_ostream & operator<< (raw_ostream & O, MemDepType type);

} // End of llvm namespace

#endif /* __DEPENDENCE_H_ */
