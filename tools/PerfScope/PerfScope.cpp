#include <iostream>
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
#include "mapper/PgmDependenceGraph.h"
#include "mapper/DifferenceEngine.h"
#include "mapper/CallSiteFinder.h"
#include "mapper/Matcher.h"
#include "mapper/Slicer.h"
#include "analyzer/Evaluator.h"
#include "analyzer/X86CostModel.h"


using namespace std;
using namespace llvm;

#define PERFSCOPE_DEBUG false

#define STRIP_LEN 7 // define number of components(slashes) to strip of the full path in debug info 


gen_dbg(perf)

#ifdef PERFSCOPE_DEBUG
gen_dbg_impl(perf)
#else
gen_dbg_nop(perf)
#endif

static int module_strip_len = -1;
static int patch_strip_len = 0;

static int analysis_level = 1;

static char * program_name;

static char * bc_fname = NULL;
static char * id_fname = NULL;

static LLVMContext & Context = getGlobalContext();

struct ModuleArg {
  string name;
  Module *module;
  int strips;
  ModuleArg(string n, Module *m = NULL, int s = 0) : name(n), module(m), strips(s) {}
};

static vector<ModuleArg> newmods;
static vector<ModuleArg> oldmods;

static vector<string> syscalls;
static vector<string> expcalls;
static vector<string> lockcalls;

typedef RiskEvaluator::InstVecTy InstVecTy;
typedef RiskEvaluator::InstMapTy InstMapTy;

static int objlen = MAX_PATH;
static char objname[MAX_PATH];


void test_CallGraph()
{
  if (bc_fname == NULL) {
    cout << "NULL bc_fname" << endl;
  }
  LLVMContext ctx;
  Module * module = ReadModule(ctx, bc_fname);
  if (module != NULL) {
    Function* func = module->getFunction("_ZL23test_if_skip_sort_orderP13st_join_tableP8st_ordermbP6BitmapILj64EE");
    if (func == NULL) {
      cout << "Cannot find function!" << endl;
      return;
    }
    CallSiteFinder csf(func);
    CallSiteFinder::cs_iterator i = csf.begin(), e = csf.end();
    if(i == e) {
      cout << "No function";
    }
    else {
      cout << "The following functions";
    }
    const char *name = func->getName().data();
    cout << " called " << cpp_demangle(name) << endl;
    for (; i != e; i++) {
      name = (*i)->getName().data();
      cout << cpp_demangle(name) << endl;
    }
  }
  else {
    cout << "Cannot load " << bc_fname << endl;
  }
}

void assess(Instruction *I, MODTYPE type)
{
  if (isa<BranchInst>(I)) {
    /*
    BranchInst *bi = cast<BranchInst>(I);
    if (PERFSCOPE_DEBUG)
        errs() << "branch@" << ScopeInfoFinder::getInstLine(I) << "\n";
    if (bi->isConditional()) {
        unsigned n = bi->getNumSuccessors();
        for (unsigned i = 0; i < n; i++) {
            BasicBlock * BB = bi->getSuccessor(i);
        }
    }
    */
  } 
  if (isa<CmpInst>(I)) {

  } else if (isa<CallInst>(I) || isa<InvokeInst>(I)) {
    CallSite cs(I);
    //CallInst *ci = cast<CallInst>(I);
    Function *func = cs.getCalledFunction();
    if (func == NULL ) {
      #ifdef PERFSCOPE_DEBUG
      cout << "Callee unknown\n";
      #endif
      return;
    }
    else
      if(func->isIntrinsic())
        return;
    const char *dname = cpp_demangle(func->getName().data());
    if (binary_search(syscalls.begin(), syscalls.end(), dname))
      cout << "S%";
    else if (binary_search(expcalls.begin(), expcalls.end(), dname))
      cout << "E%";
    else if (binary_search(lockcalls.begin(), lockcalls.end(), dname))
      cout << "L%";
    else
      cout << "N%";
    cout << dname << "@" << ScopeInfoFinder::getInstLine(I) << ",";
    ////////////////
    //////////////////
  }
}
void assess(Module * module, CostModel * model, InstMapTy & instmap)
{
  if (instmap.size()) {
    OwningPtr<FunctionPassManager> FPasses;
    FPasses.reset(new FunctionPassManager(module));
    FPasses->add(new RiskEvaluator(instmap, model));
    FPasses->doInitialization();
    for (InstMapTy::iterator map_it = instmap.begin(), map_ie = instmap.end();
        map_it != map_ie; ++map_it) {
      FPasses->run(*(map_it->first));
    }
    FPasses->doFinalization();
  }
}

void parseList(vector<ModuleArg> & vec, char *arg, const char *delim)
{
  char *str = strtok(arg, delim);
  while (str != NULL) {
    vec.push_back(ModuleArg(string(str))); 
    str = strtok(NULL, delim);
  }
}

bool load(LLVMContext & context, ModuleArg & mod)
{
  mod.module = ReadModule(context, mod.name);
  if (mod.module == NULL)  {
    cout << "cannot load module " << mod.name << endl;
    return false;
  }
  int s = module_strip_len;
  if (s < 0) {
    s =  count_strips(mod.module);
    perf_debug("Calculated bstrips: %d for %s\n", s, mod.name.c_str());
  }
  mod.strips = s;
  return true;
}

void load(LLVMContext & context, vector<ModuleArg> & vec)
{
  vector<ModuleArg>::iterator it = vec.begin(), ie = vec.end();
  for (; it != ie; it++) {
    if (!load(context, *it))
      exit(1);
  }
}

void analyze(char *input)
{
  llvm_shutdown_obj Y;  // Call llvm_shutdown() on exit.
  PassRegistry &Registry = *PassRegistry::getPassRegistry();
  initPassRegistry(Registry);

  X86CostModel * XCM = new X86CostModel(getTargetMachine());
  PatchDecoder * decoder = new PatchDecoder(input);
  assert(decoder);
  Patch *patch = NULL;
  Chapter *chap = NULL;
  Hunk * hunk = NULL;
  while ((patch = decoder->next_patch()) != NULL) {
    perf_debug("patch: %s\n", patch->patchname.c_str());
    while ((chap = patch->next_chapter()) != NULL) {
      perf_debug("chapter: %s\n", chap->filename.c_str());
      if (src2obj(chap->fullname.c_str(), objname, &objlen) == NULL) { // skip header files for now
        chap->skip_rest_of_hunks();
        continue;
      }
      bool found = false;
      for (vector<ModuleArg>::iterator it = newmods.begin(), ie = newmods.end();
          it != ie; ++it) {
        if (it->module == NULL && load(Context, *it))
          continue;
        Matcher matcher(*(it->module), it->strips, patch_strip_len);
        // nasty hack here to avoid soft link handling inconsistency between diff and debug info
        if (chap->fullname == "src/backend/port/sysv_shmem.c") 
          chap->fullname = "src/backend/port/pg_shmem.c";
        Matcher::sp_iterator I  = matcher.resetTarget(chap->fullname);
        if (I != matcher.sp_end()) {
          found = true;
          inst_iterator  fi;
          Function *func = NULL;
          Function *prevfunc = NULL;
          InstMapTy instmap;
          while((hunk = chap->next_hunk())) {
            int s = 0;
            Scope scope = hunk->rep_enclosing_scope;
            perf_debug("hunk\n  begin: line %d\n  ctrl seq.: %s\n"
                        "  scope: [#%lu, #%lu]\n", hunk->start_line, hunk->ctrlseq,
                        scope.begin, scope.end);  
            Hunk::iterator HI = hunk->begin(), HE = hunk->end();
            bool multiple = true;
            for(; multiple; prevfunc = func) {
              func = matcher.matchFunction(I, scope, multiple);
              if (func == NULL)
                break;
              // The enclosing scope is the min, max range:
              //        [Mods[first].begin, Mods[last].end]
              // We should iterate the actual modification for intervals 

              // Hunk: [(..M1..)    (..M2..)  (..M3..)]
              //                {f1}

              // Skip DEL modifications and modifications that are 
              // before function's beginning
              while(HI != HE && ((*HI)->type == DEL || 
                    (*HI)->rep_scope.end < I->linenumber))
                HI++;

              // Run over modifications, break out to the next hunk
              if (HI == HE) {
                break;
              }

              // Modification cross function boundary, this
              // happens when the function lies in gaps.
              // But by definition, there's no gab between Mods.
              assert((*HI)->rep_scope.begin <= I->lastline);

              s++;
              const char *dname = cpp_demangle(I->name.c_str());
              if (dname == NULL)
                dname = I->name.c_str();
              perf_debug("scope #%d: %s |=> [#%lu, #%lu]\n  %s:", s,
                          dname, scope.begin, scope.end, dname);

              // Four situations(top mod, bottom func):
              // 1):   |_________|
              //            |________|
              // 2):   |_________|
              //         |____|
              // 3):   |_________|
              //     |_______|
              // 4):   |_________|
              //     |_______________| 
              //
              // TODO in case, the adjacent hunks are inside the same function, 
              // no need to restart search from beginning
              if (prevfunc != func)
                fi = inst_begin(func);
              
              // Find the instructions for Modifications within the range of the
              // function
              for (; HI != HE && (*HI)->rep_scope.begin <= I->lastline; ++HI) {
                if ((*HI)->type == DEL) { // skip delete
                  continue;
                }
                // need to modify rep_scope to reflect 
                // the processed lines
                Scope & rep_scope = (*HI)->rep_scope; 
                // reach the boundary
                if (rep_scope.begin > I->lastline) 
                  break;
                // adjust replacement mod scope
                if (rep_scope.begin < I->linenumber)
                  rep_scope.begin = I->linenumber;
                if (rep_scope.end > I->lastline)
                  rep_scope.end = I->lastline;
                ////////////////////////////////

                Instruction *inst;
                bool found_inst = false;
                while ( (inst = matcher.matchInstruction(fi, func, rep_scope)) != NULL) {
                  instmap[func].push_back(inst);
                  found_inst = true;
                } 
                if (!found_inst) 
                  perf_debug("Can't locate any instruction for mod @[#%lu, #%lu]\n",
                     rep_scope.begin, rep_scope.end); 
              }
              perf_debug("$$\n");
            }
            if (s == 0)
              perf_debug("insignificant scope\n");
          }
          assess(it->module, XCM, instmap);
          break; // already found in existing module, no need to try loading others
        }
      }
      if (!found) 
        chap->skip_rest_of_hunks();
    }
  }
}

static char const * option_help[] =
{
  " -b FILE1,FILE2,...\tA comma separated list of bc files from before-revision source code.",
  " -a FILE1,FILE2,...\tA comma separated list of bc files from after-revision source code.",
  " -p LEN\tLevel of components to be striped of the path inside the patch file.",
  " -m LEN\tLevel of components to be striped of the path inside the module file.",
  " -s FILE\tA file containing the list of system call names",
  " -l FILE\tA file containing the list of synchronization call names",
  " -e FILE\tA file containing the list of expensive function calls",
  " -L LEVEL\tSpecify the level of analysis",
  " -h\tPrint this message.",
  0
};

void usage(FILE *fp = stderr)
{
  const char **p = option_help;
  fprintf(fp, "Usage: %s OPTIONS IDFILE", program_name);
  fprintf(fp, "\n");
  while (*p) {
    fprintf(fp, "%s\n\n", *p);
    p++;
  }
}

int main(int argc, char *argv[])
{
  program_name = argv[0];

  if (argc <= 1) {
    usage();
    exit(1);
  }

  int opt;
  int plen;
  while((opt = getopt(argc, argv, "a:b:e:hl:s:p:m:L:")) != -1) {
    switch(opt) {
      case 'a':
        parseList(newmods, optarg, ",");
        break;
      case 'b':
        parseList(oldmods, optarg, ",");
        break;
      case 'e':
        readlines2vector(optarg, expcalls);
        break;
      case 'l':
        readlines2vector(optarg, lockcalls);
        break;
      case 's':
        readlines2vector(optarg, syscalls);
        break;
      case 'p':
        plen = atoi(optarg);
        if (plen <= 0) {
          fprintf(stderr, "Strip len must be positive integer\n");
          exit(1);
        }
        patch_strip_len = plen;
        break;
      case 'm':
        plen = atoi(optarg);
        if (plen <= 0) {
          fprintf(stderr, "Strip len must be positive integer\n");
          exit(1);
        }
        module_strip_len = plen;
        break;
      case 'h':
        usage();
        exit(0);
      case 'L':
        analysis_level = atoi(optarg);
        if (analysis_level <= 0) {
          fprintf(stderr, "Level of analysis must be positive integer\n");
          exit(1);
        }
        break;
      case '?':
      default:
        usage();
        exit(1);
    }
  }
  if (newmods.begin() == newmods.end()) {
    fprintf(stderr, "Must specify before-revision bitcode file argument\n");
    exit(1);
  }
  if (optind != argc-1) {
    usage();
    exit(1);
  }
  id_fname = dupstr(argv[optind]);
  load(Context, oldmods);
  load(Context, newmods);

  struct timeval tim;
  gettimeofday(&tim, NULL);
  double t1 = tim.tv_sec * 1000.0 +(tim.tv_usec/1000.0);
  analyze(id_fname);
  gettimeofday(&tim, NULL);
  double t2 = tim.tv_sec * 1000.0 +(tim.tv_usec/1000.0);
  fprintf(stderr, "%.4f ms\n", t2-t1);
  return 0;
}
