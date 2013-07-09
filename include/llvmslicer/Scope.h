/**
 *  @file          Scope.h
 *
 *  @version       1.0
 *  @created       07/13/2012 05:23:23 PM
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
 *  Scope Data Structure
 *
 */

#ifndef ___PERF_SCOPE__H_ 
#define ___PERF_SCOPE__H_ 

#include <iostream>
#include "llvm/Support/raw_ostream.h"

typedef struct Scope {
    unsigned long begin;
    unsigned long end;
    
    Scope() : begin(0), end(0) {}
    Scope(unsigned long b) : begin(b), end(b) {}
    Scope(unsigned long b, unsigned long e) : begin(b), end(e) {}
    Scope(const Scope &another) : begin(another.begin), end(another.end) {}

    bool includes(const Scope &another) { return (another.begin >= begin) && (another.end <= end); }
    bool intersects(const Scope &another) { return (another.end >= begin && another.begin <= end); }

    bool operator==(const Scope & another) const { return begin == another.begin && end == another.end; }

} Scope;

llvm::raw_ostream & operator<<(llvm::raw_ostream& , const Scope & );
std::ostream & operator<<(std::ostream& , const Scope & );

#endif
