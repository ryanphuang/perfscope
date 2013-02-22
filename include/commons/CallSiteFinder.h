//===-- CallSiteFinder.h - CallSite Analysis --------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef __CALL_SITE_FINDER__H_
#define __CALL_SITE_FINDER__H_

#include "llvm/Function.h"
#include "llvm/ADT/SmallVector.h"

#include <utility>

namespace llvm {

class CallSiteFinder {
    public:
        // The caller and callsite instruction
        typedef std::pair<Function *, Instruction *> CallInfo;

    protected:
        SmallVector<CallInfo, 4> callsites;

    public:
        typedef SmallVector<CallInfo, 4>::iterator cs_iterator;
        typedef SmallVector<CallInfo, 4>::const_iterator const_cs_iterator;

    public:
        CallSiteFinder(llvm::Function * func);

        inline cs_iterator begin() { return callsites.begin(); }
        inline const_cs_iterator begin() const { return callsites.begin(); }
        inline cs_iterator end() { return callsites.end(); }
        inline const_cs_iterator end() const { return callsites.end(); }

        inline size_t size() { return callsites.size(); }
};

} // End of llvm namespace

#endif
