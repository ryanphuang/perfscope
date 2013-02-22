#include <iostream>
#include <memory>
#include <limits.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <vector>
#include <list>

#include "llvm/LLVMContext.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Type.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Triple.h"

#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include "llvm/Target/TargetLowering.h"

#include "llvm/Support/Host.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/IRReader.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Support/InstVisitor.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/TargetRegistry.h"

#include "commons/handy.h"
#include "commons/LLVMHelper.h"
#include "parser/PatchDecoder.h"
#include "mapper/DifferenceEngine.h"
#include "mapper/Matcher.h"
#include "analyzer/Evaluator.h"
#include "analyzer/X86CostModel.h"


using namespace std;
using namespace llvm;

static char * program_name;

#define PROFILE_DEBUG

gen_dbg(profile)

#ifdef PROFILE_DEBUG
gen_dbg_impl(profile)
#else
gen_dbg_nop(profile)
#endif

static LLVMContext & Context = getGlobalContext();

void usage(FILE *fp = stderr)
{
  fprintf(fp, "Usage: %s MODULE", program_name);
  fprintf(fp, "\n");
}

typedef struct FuncCost {
  string name;
  unsigned cost;
  FuncCost() : cost(0) {}
  FuncCost(const char *name, unsigned cost) : name(name), cost(cost) {}
} FuncCost;

int compareFuncCost(const void *a, const void *b)
{
  FuncCost * fa = (FuncCost *) a;
  FuncCost * fb = (FuncCost *) b;
  return (fb->cost - fa->cost); // reverse order
}

void static_profile(Module * module, CostModel * model)
{
  size_t size = module->size();
  if (size == 0) {
    profile_debug("Module has no functions\n");
    return;
  }
  FuncCost * func_cost = new FuncCost[size];
  if (func_cost == NULL) {
    fprintf(stderr, "Cannot allocate memory for function cost\n");
    return;
  }
  unsigned i = 0;
  for (Module::iterator MI = module->begin(), ME = module->end(); 
      MI != ME; ++MI, ++i) {
    Function * F = MI;
    unsigned cost = model->getFunctionCost(F);
    if (cost == (unsigned) -1)
      cost = 0;
    func_cost[i] = FuncCost(cpp_demangle(F->getName().data()), cost);
  } 
  qsort(func_cost, size, sizeof(FuncCost), compareFuncCost);
  for (i = 0; i < size; i++) {
    printf("%s: %u\n", func_cost[i].name.c_str(), func_cost[i].cost);
  }
  delete [] func_cost;
}

int main(int argc, char *argv[])
{
  program_name = argv[0];

  if (argc <= 1) {
    usage();
    exit(1);
  }

  LLVMContext & Context = getGlobalContext();

  Module * module = ReadModule(Context, argv[1]);
  if (module == NULL)  {
    cout << "cannot load module " << argv[1] << endl;
    return false;
  }

  llvm_shutdown_obj Y;  // Call llvm_shutdown() on exit.
  PassRegistry &Registry = *PassRegistry::getPassRegistry();
  initPassRegistry(Registry);

  X86CostModel * XCM = new X86CostModel(getTargetMachine());
  static_profile(module, XCM);
  return 0;
}
