// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

#ifndef LANGUAGES_LLVMSUPPORT_H
#define LANGUAGES_LLVMSUPPORT_H

#include <llvm/Function.h>
#include <llvm/Instructions.h>

#include "PointsTo.h"

namespace llvm {

  template<typename OutIterator>
    void getCalledFunctions(const CallInst *CI, const ptr::PointsToSets &PS,
        OutIterator out) {
      const Value *stripped = CI->getCalledValue()->stripPointerCasts();

      if (const Function *F = dyn_cast<Function>(stripped)) {
        *out++ = F;
      } else {
        typedef ptr::PointsToSets::PointsToSet PTSet;
        const PTSet &S = getPointsToSet(stripped, PS);
        // errs() << "Possible targets for";
        // CI->dump();
        for (PTSet::const_iterator I = S.begin(), E = S.end(); I != E; ++I) {
          if (const Function *F = dyn_cast<Function>(*I)) {
            // errs() << "\t" << F->getName() << "\n";
            *out++ = F;
          }
        }
      }
    }

}

#endif
