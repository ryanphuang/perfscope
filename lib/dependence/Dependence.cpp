/**
 *  @file          lib/Dependence/Dependence.cpp
 *
 *  @version       1.0
 *  @created       01/28/2013 01:17:34 PM
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

#include "dependence/Dependence.h"

using namespace llvm;

raw_ostream & operator<< (raw_ostream & O, MemDepType type)
{
  O << '#';
  switch (type) {
    case NoneMemDep:
      O << "None";
      break;
    case TrueMemDep:
      O << "True Memory Dependence";
      break;
    case AntiMemDep:
      O << "Anti Memory Dependence";
      break;
    case OutMemDep:
      O << "Out Memory Dependence";
      break;
    default:
      O << "UNKNOWN";
      break;
  }
  O << '#';
  return O;
}

