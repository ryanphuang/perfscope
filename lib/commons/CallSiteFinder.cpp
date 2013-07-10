#include "llvm/Instruction.h"
#include "llvm/Support/CallSite.h"

#include "commons/CallSiteFinder.h"

using namespace llvm;

CallSiteFinder::CallSiteFinder(const Function * func)
{
  if (func == NULL)
    return;
  for (Value::const_use_iterator i = func->use_begin(), e = func->use_end(); i != e; ++i) {
    if (const CallInst * CI = dyn_cast<CallInst>(*i)) {
        const Function * caller = CI->getParent()->getParent();
        callsites.push_back(std::make_pair(caller, CI));
    }
  }
}
