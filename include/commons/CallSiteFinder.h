/**
 *  @file          CallSiteFinder.h
 *
 *  @version       1.0
 *  @created       07/23/2012 05:46:54 PM
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
 *  CallSite Analysis  
 *
 */

#ifndef __CALL_SITE_FINDER__H_
#define __CALL_SITE_FINDER__H_

#include "llvm/Function.h"
#include "llvm/ADT/SmallVector.h"

#include <utility>

namespace llvm {

class CallSiteFinder {
    public:
        // The caller and callsite instruction
        typedef std::pair<const Function *, const Instruction *> CallInfo;

    protected:
        SmallVector<CallInfo, 4> callsites;

    public:
        typedef SmallVector<CallInfo, 4>::iterator cs_iterator;
        typedef SmallVector<CallInfo, 4>::const_iterator const_cs_iterator;

    public:
        CallSiteFinder(const llvm::Function * func);

        inline cs_iterator begin() { return callsites.begin(); }
        inline const_cs_iterator begin() const { return callsites.begin(); }
        inline cs_iterator end() { return callsites.end(); }
        inline const_cs_iterator end() const { return callsites.end(); }

        inline size_t size() { return callsites.size(); }
};

} // End of llvm namespace

#endif
