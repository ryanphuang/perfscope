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
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Type.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/IRReader.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Support/InstVisitor.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Target/TargetData.h"
#include "llvm/IntrinsicInst.h"

#include "commons/handy.h"
#include "parser/PatchDecoder.h"
#include "mapper/PgmDependenceGraph.h"
#include "mapper/DifferenceEngine.h"
#include "mapper/CallSiteFinder.h"
#include "mapper/Matcher.h"
#include "mapper/Slicer.h"

using namespace std;
using namespace llvm;

#define PERFSCOPE_DEBUG false

#define STRIP_LEN 7 // define number of components(slashes) to strip of the full path in debug info 

static string DefaultDataLayout;

static int module_strip_len = -1;
static int patch_strip_len = 0;

static int analysis_level = 1;

static char * program_name;

static char * bc_fname = NULL;
static char * id_fname = NULL;

static LLVMContext & Context = getGlobalContext();

static list<Module *> bmodules;
static list<IPModRef *> bmodrefs;
static list<PassManager *>bmanagers;
static list<string> bnames;
static list<int> bstrips;

static list<Module *> amodules;
static list<string> anames;
static list<int> astrips;

static vector<string> syscalls;
static vector<string> expcalls;
static vector<string> lockcalls;


static SmallVector<Scope, 4> *funcLoops;
typedef SmallVector<Scope, 4>::iterator loop_iterator;

// static SmallVector<Function *, 16> targetFuncs;

static DenseMap< Function *, SmallVector<Scope, 4> *> loopMap(16);
typedef DenseMap< Function *, SmallVector<Scope, 4> *>::iterator loop_map_iterator;

struct LoopInfoPrinter : public FunctionPass {
  static char ID;
  std::string PassName;

  LoopInfoPrinter() : FunctionPass(ID) {
    PassName = "LoopInfo Printer: ";
  }

  virtual bool runOnFunction(Function &F) {
    if (loopMap.count(&F))
      return false;
    LoopInfo &li = getAnalysis<LoopInfo>();
    funcLoops = new SmallVector<Scope, 4>();
    funcLoops->clear();
    Scope ls;
    for (LoopInfo::iterator LII = li.begin(),  LIE = li.end(); 
        LII != LIE; LII++) {
      if (ScopeInfoFinder::getLoopScope(ls, *LII)) { 
        funcLoops->push_back(ls);
      }
    }
    loopMap[&F] = funcLoops;
    return false;
  }

  virtual const char *getPassName() const { return PassName.c_str(); }

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    //AU.addRequired<DominatorTree>();
    AU.addRequired<LoopInfo>();
  }
};

struct CallSiteVisitor : public InstVisitor<CallSiteVisitor> {
  unsigned Count;

  CallSiteVisitor() : Count(0) {}
  void visitCallInst(CallInst &I)
  {
    cout << "Call Inst Encountered! " << endl;
    Count++;
  }
};

char LoopInfoPrinter::ID = 0;

struct stat sourcestat;

static int objlen = MAX_PATH;
static char objname[MAX_PATH];

/// Reads a module from a file.  On error, messages are written to stderr
/// and null is returned.
static Module *ReadModule(LLVMContext &Context, StringRef Name) {
  SMDiagnostic Diag;
  Module *M = ParseIRFile(Name, Diag, Context);
  if (!M)
    cerr << "IR file parsing failed: " << Diag.getMessage() << endl;
  return M;
}

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

size_t count_strips(Module & M)
{
  if (NamedMDNode *CU_Nodes = M.getNamedMetadata("llvm.dbg.cu")) {
    size_t buf_len = 128;
    char *buf = (char *) malloc(buf_len);
    for (unsigned i = 0, e = CU_Nodes->getNumOperands(); i != e; ++i) {
      DICompileUnit CU1(CU_Nodes->getOperand(i));
      if (i == 0) {
        if (i + 1 != e) {
          i++;
          DICompileUnit CU2(CU_Nodes->getOperand(i));
          buf = common_prefix(buf, buf_len, CU1.getDirectory().data(), CU2.getDirectory().data());
        }
        else {
          size_t cu_len = CU1.getDirectory().size();
          if (cu_len > buf_len) {
            buf = (char *) realloc(buf, cu_len);
            buf_len = cu_len;
          }
          strcpy(buf, CU1.getDirectory().data()); 
        }
      }
      else {
        buf = common_prefix(buf, buf_len, buf, CU1.getDirectory().data());
      }
    }
    unsigned cnt = countnchr(buf, -1, '/');
    if (buf[strlen(buf) - 1] != '/')
      cnt++;
    //TODO dirty hacks
    // workaround problem where a single bc file inside the project's
    // subdirectory is provided. e.g. /home/ryan/Project/mysql/storage/innodb_plugin
    // will be the inferred root path.
    if (endswith(buf, "innodb_plugin"))
      cnt -= 2;
    else if (endswith(buf, "src/")) // hack for postgresql/src/
      cnt -= 1;

    return  cnt;
  }
  return 0;
}

inline bool passLoopEnd(const Scope &scope)
{
  if(funcLoops->empty())
    return false;
  return scope.begin > funcLoops->back().end;
}

bool isInLoop(const Scope &scope)
{
  // Will only match the *first* in top level matching loop.
  // FIXME may need to get all the loops.
  for (loop_iterator LI = funcLoops->begin(), LE = funcLoops->end();
      LI != LE; LI++) {
    if (LI->intersects(scope)) {
      #ifdef PERFSCOPE_DEBUG
        cout << "loop: "; 
      #endif
      cout << *LI << " ";
      return true;
    }
  }
  return false;
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

bool followCS(Function *func, FunctionPassManager & FPass)
{
  Value::use_iterator i = func->use_begin(), e = func->use_end();
  if(i == e) {
    cout << "<- ->";
    return false;
  }
  else {
    cout << "<- ";
  }
  const char *name; 
  for (; i != e; ++i) {
    if (Instruction* use = dyn_cast<Instruction>(*i)) {
      CallSite call(use);
      Function *callee = call.getCalledFunction();
      Function *caller = call.getCaller(); 
      if (caller != NULL) {
        if (callee != NULL && callee == func) {
          name = caller->getName().data();
          SmallVector<Scope, 4> * old = funcLoops;
          FPass.run(*caller);
          unsigned line = ScopeInfoFinder::getInstLine(use); 
          cout << cpp_demangle(name) << "@" << line; 
          Scope scope(line);
          if (isInLoop(scope)) {
            funcLoops = old;
            cout << "->";
            return true;
          }
          funcLoops = old;
          cout << "->";
        }
      }
    }
  }
  return false;
}

void slice(DependenceGraph * depGraph, Instruction *I, MODTYPE type)
{
  if (depGraph == NULL) {
    // errs() << "NULL dependence graph, not slicing\n";
    return;
  }
  if (isa<DbgInfoIntrinsic>(I)) { // Skip intrinsics 
    return;
  }
  Slicer slicer(depGraph);
  if (slicer.sliceInit(*I, MemoryDeps)) {
    #ifdef PERFSCOPE_DEBUG
    errs() << "Slice for " << *I << "\n\t";
    #endif
    Instruction *N;
    cout << "<";
    while((N = slicer.sliceNext()) != NULL) {
      #ifdef PERFSCOPE_DEBUG
      errs() << *N << "||";
      #endif
      assess(N, type);
    }
    cout << ">";
    #ifdef PERFSCOPE_DEBUG
    errs() << "\n";
    #endif
  }
}

void initPassRegistry(PassRegistry & Registry)
{
  initializeCore(Registry);
  initializeScalarOpts(Registry);
  initializeIPO(Registry);
  initializeAnalysis(Registry);
  initializeIPA(Registry);
  initializeTransformUtils(Registry);
  initializeInstCombine(Registry);
  initializeInstrumentation(Registry);
  initializeTarget(Registry);
}


TargetData * getTargetData(PassManager & Passes, Module *M)
{
  // Add an appropriate TargetLibraryInfo pass for the module's triple.
  TargetLibraryInfo *TLI = new TargetLibraryInfo(Triple(M->getTargetTriple()));
  Passes.add(TLI);
  // Add an appropriate TargetData instance for this module.
  TargetData *TD = 0;
  const std::string &ModuleDataLayout = M->getDataLayout();
  if (!ModuleDataLayout.empty())
    TD = new TargetData(ModuleDataLayout);
  else if (!DefaultDataLayout.empty())
    TD = new TargetData(DefaultDataLayout);
  return TD;
}


void analyze(char *input)
{
  PatchDecoder * decoder = new PatchDecoder(input);
  assert(decoder);
  Patch *patch = NULL;
  Chapter *chap = NULL;
  Hunk * hunk = NULL;
  Module *module = NULL; 
  while((patch = decoder->next_patch()) != NULL) {
    #ifdef PERFSCOPE_DEBUG
    cout << "patch: " << patch->patchname << endl;
    #endif
    while((chap = patch->next_chapter()) != NULL) {
      #ifdef PERFSCOPE_DEBUG
      cout << "chapter: " << chap->filename << endl;
      #endif
      if (src2obj(chap->fullname.c_str(), objname, &objlen) == NULL) { // skip header files for now
        chap->skip_rest_of_hunks();
        continue;
      }
      bool found = false;
      list<int>::iterator ii = bstrips.begin(), ie = bstrips.end();
      list<string>::iterator si = bnames.begin(), se = bnames.end();
      for(list<Module *>::iterator mi = bmodules.begin(), me = bmodules.end(); 
          mi != me && ii != ie && si != se; mi++, ii++, si++) {
        module = *mi;
        if (module == NULL) {
          module = ReadModule(Context, *si);
          if (module == NULL)  {
            cout << "cannot load module " << *si<< endl;
            continue;
          }
          *mi = module;
          int tmps = module_strip_len;
          if (tmps < 0) {
            tmps =  count_strips(*module);
            #ifdef PERFSCOPE_DEBUG
            cout << "Calculated bstrips: " << tmps << " for " << *si << endl;
            #endif
          }
          *ii = tmps;
        }
        Matcher matcher(*module, *ii, patch_strip_len);
        // nasty hack here to avoid soft link handling inconsistency between diff and debug info
        if (chap->fullname == "src/backend/port/sysv_shmem.c") 
          chap->fullname = "src/backend/port/pg_shmem.c";
        Matcher::sp_iterator I  = matcher.resetTarget(chap->fullname);
        if (I == matcher.sp_end())
          continue;
        else {
          found = true;
          while((hunk = chap->next_hunk()) != NULL) {
            #ifdef PERFSCOPE_DEBUG
            cout << "hunk: " << hunk->start_line << endl;
            cout << hunk->ctrlseq << endl;
            #endif
            assert(hunk->reduce());
            #ifdef PERFSCOPE_DEBUG
            cout << hunk->rep_enclosing_scope << endl;
            #endif
            Function *f;
            int s = 0;
            Scope scope = hunk->rep_enclosing_scope;
            #ifdef PERFSCOPE_DEBUG
            cout << hunk->rep_enclosing_scope << " might touch ";
            #endif
            Scope ls;
            Hunk::iterator HI = hunk->begin(), HE = hunk->end();
            for(; (f = matcher.matchFunction(I, scope)) != NULL;) {
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
              #ifdef PERFSCOPE_DEBUG
              cout << "scope #" << s << ": " << dname;
              cout << " |=> " << scope << "\n";
              cout << "\t";
              #endif
              cout << dname << ":"; // Structued output

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
              inst_iterator fi = inst_begin(f);
              inst_iterator fe = inst_end(f);
              
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
                while ( (inst = matcher.matchInstruction(fi, f, rep_scope)) != NULL) {
                  assess(inst, (*HI)->type);
                  found_inst = true;
                } 
                #ifdef PERFSCOPE_DEBUG
                if (!found_inst) {
                  errs() << "Can't locate any instruction for mod @" << rep_scope << "\n";
                }
                #endif
              }
              cout << "$$\n";
            }
            #ifdef PERFSCOPE_DEBUG
            if (s == 0) {
              cout << "insignificant scope\n";
            }
            #endif
          }
        }
        break; // already found in existing module, no need to try loading others
      }
      if (!found) 
        chap->skip_rest_of_hunks();
    }
  }
}

void parseList(list<string> & vec, char *arg, const char *delim)
{
  char *str = strtok(arg, delim);
  while (str != NULL) {
    vec.push_back(string(str)); 
    str = strtok(NULL, delim);
  }
}

Module *loadFromSource(const char *sourcename)
{
  if (src2obj(sourcename, objname, &objlen) == NULL) // skip header files for now
    return NULL;
  LLVMContext Context;
  Module *module = ReadModule(Context, objname);
  return module;
}

void delayedLoad(LLVMContext & Context, list<string> & lst, bool before)
{
  list<string>::iterator it = lst.begin(), et = lst.end();
  // just initialize
  for (; it != et; it++) {
    if (before) {
      bmodules.push_back(NULL);
      bstrips.push_back(0);
    }
    else{
      amodules.push_back(NULL);
      astrips.push_back(0);
    }
  }
}

void load(LLVMContext & Context, list<string> & lst, bool before)
{
  list<string>::iterator it = lst.begin(), et = lst.end();
  for (; it != et; it++) {
    Module *module = ReadModule(Context, *it);
    if (module == NULL)  {
      cout << "cannot load module " << *it << endl;
      exit(1);
    }
    int s = module_strip_len;
    if (s < 0) {
      s =  count_strips(*module);
      #ifdef PERFSCOPE_DEBUG
      cout << "Calculated bstrips: " << s << " for " << *it << endl;
      #endif
    }
    if (before) {
      bmodules.push_back(module);
      bstrips.push_back(s);
    }
    else {
      amodules.push_back(module);
      astrips.push_back(s);
    }
  }
}

void readToVector(char *fname, vector<string> &vec)
{
  FILE *fp = fopen(fname,"r");
  if (fp == NULL) {
    perror("Read file");
    return;
  }
  char buf[256];
  while (fgets(buf, 256, fp) != NULL) {
    buf[strcspn(buf, "\n")] = '\0';
    if (buf[0] == 0)
      continue;
    vec.push_back(buf);
  }
  if (vec.begin() != vec.end())
    sort(vec.begin(), vec.end());
  #ifdef PERFSCOPE_DEBUG
    vector<string>::iterator it = vec.begin(), ie = vec.end();
    for (; it != ie; it++) {
      cout << *it << endl;
    }
  #endif
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
        parseList(anames, optarg, ",");
        break;
      case 'b':
        parseList(bnames, optarg, ",");
        break;
      case 'e':
        readToVector(optarg, expcalls);
        break;
      case 'l':
        readToVector(optarg, lockcalls);
        break;
      case 's':
        readToVector(optarg, syscalls);
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
  if (bnames.begin() == bnames.end()) {
    fprintf(stderr, "Must specify before-revision bitcode file argument\n");
    exit(1);
  }
  if (optind != argc-1) {
    usage();
    exit(1);
  }
  id_fname = dupstr(argv[optind]);
  //delayedLoad(Context, anames, false);
  //delayedLoad(Context, anames, false);
  load(Context, anames, false);
  load(Context, bnames, true);

  struct timeval tim;
  gettimeofday(&tim, NULL);
  double t1 = tim.tv_sec * 1000.0 +(tim.tv_usec/1000.0);
  analyze(id_fname);
  gettimeofday(&tim, NULL);
  double t2 = tim.tv_sec * 1000.0 +(tim.tv_usec/1000.0);
  fprintf(stderr, "%.4f ms\n", t2-t1);
  return 0;
}
