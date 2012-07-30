#include "Handy.h"
#include "PatchDecoder.h"
#include "CallSiteFinder.h"
#include "Matcher.h"

#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Type.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/IRReader.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Support/InstVisitor.h"
#include "llvm/Target/TargetData.h"

#include <iostream>
#include <limits.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <vector>
#include <list>

using namespace std;

#define LOCAL_DEBUG true

#define STRIP_LEN 7 // define number of components(slashes) to strip of the full path in debug info 

static int strip_len = -1;

static char * program_name;

static char * bc_fname = NULL;
static char * id_fname = NULL;

static list<Module *> bmodules;
static list<int> bstrips;

static list<Module *> amodules;
static list<int> astrips;

static SmallVector<Scope, 4> funcLoops;
typedef SmallVector<Scope, 4>::iterator loop_iterator;

static SmallVector<Function *, 16> targetFuncs;

struct LoopInfoPrinter : public FunctionPass {
    static char ID;
    std::string PassName;

    LoopInfoPrinter() : FunctionPass(ID) {
            PassName = "LoopInfo Printer: ";
    }

    virtual bool runOnFunction(Function &F) {
        LoopInfo &li = getAnalysis<LoopInfo>();
        funcLoops.clear();
        Scope ls;
        for (LoopInfo::iterator LII = li.begin(),  LIE = li.end(); LII != LIE; LII++) {
            if (ScopeInfoFinder::getLoopScope(ls, *LII)) { 
                funcLoops.push_back(ls);
            }
        }
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
        cerr << "IR file parsing failed" << endl;
    return M;
}

Module *loadModule(const char *sourcename)
{
    if (src2obj(sourcename, objname, &objlen) == NULL) // skip header files for now
        return NULL;
    LLVMContext Context;
    Module *module = ReadModule(Context, objname);
    return module;
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
        return  cnt;
    }
    return 0;
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


void assess(char *input)
{
    llvm_shutdown_obj Y;  // Call llvm_shutdown() on exit.
    PatchDecoder * decoder = new PatchDecoder(input);
    assert(decoder);
    Patch *patch = NULL;
    Chapter *chap = NULL;
    Hunk * hunk = NULL;

    OwningPtr<FunctionPassManager> FPasses;
    // Initialize passes
    PassRegistry &Registry = *PassRegistry::getPassRegistry();
    initPassRegistry(Registry);

    Module *module = NULL; 
    while((patch = decoder->next_patch()) != NULL) {
        if (LOCAL_DEBUG)
            cout << "patch: " << patch->patchname << endl;
        while((chap = patch->next_chapter()) != NULL) {
            if (LOCAL_DEBUG)
                cout << "chapter: " << chap->filename << endl;
            if (src2obj(chap->fullname.c_str(), objname, &objlen) == NULL) { // skip header files for now
                chap->skip_rest_of_hunks();
                continue;
            }
            bool found = false;
            list<int>::iterator ii = bstrips.begin(), ie = bstrips.end();
            for(list<Module *>::iterator mi = bmodules.begin(), me = bmodules.end(); mi != me && ii != ie; mi++, ii++) {
                module = *mi;
                if (module == NULL)
                    continue;
                Matcher matcher(*module, 0, *ii);
                Matcher::cu_iterator ci  = matcher.matchCompileUnit(chap->fullname);
                if (ci == matcher.cu_end()) {
                    continue;
                }
                else {
                    Matcher::sp_iterator I = matcher.initMatch(ci);
                    found = true;
                    FPasses.reset(new FunctionPassManager(module));
                    FPasses->add(new LoopInfoPrinter());
                    FPasses->doInitialization();
                    while((hunk = chap->next_hunk()) != NULL) {
                        if (LOCAL_DEBUG) {
                            cout << "hunk: " << hunk->start_line << endl;
                            cout << hunk->ctrlseq << endl;
                        }
                        assert(hunk->reduce());
                        if (LOCAL_DEBUG)
                            cout << hunk->enclosing_scope << endl;
                        Function *f;
                        int s = 0;
                        Scope scope = hunk->enclosing_scope;
                        if (LOCAL_DEBUG) 
                            cout << hunk->enclosing_scope << " might touch ";
                        Scope ls;
                        Hunk::iterator HI = hunk->begin(), HE = hunk->end();
                        while ((f = matcher.matchFunction(I, scope)) != NULL ) {
                            // The enclosing scope is a rough estimation:
                            // We need to rely on the actual modification

                            // Hunk: [  (..M1..)       (..M2..)  (..M3..) ]
                            //                   {f1}

                            // Skip the modifications didn't reach function's beginning
                            while(HI != HE && (*HI)->scope.end < I->linenumber)
                                HI++;

                            // no need to test the loop scope
                            if (HI == HE || (*HI)->scope.begin > I->lastline)
                                continue;

                            s++;
                            const char *dname = cpp_demangle(I->name.c_str());
                            if (dname == NULL)
                                dname = I->name.c_str();
                            if (LOCAL_DEBUG) {
                                cout << "scope #" << s << ": " << dname;
                                cout << " |=> " << scope << "\n";
                                cout << "\t";
                            }
                            else 
                                cout << dname << ":"; // Structued output
                            FPasses->run(*f);
                            if (funcLoops.begin() == funcLoops.end()) {
                                if (LOCAL_DEBUG)
                                    cout << "loop: none";
                                cout << "\n";
                                continue;
                            }

                            // Only look into overlapping modifications when
                            // there's loop inside this function.

                            //TODO more elegant
                            //TODO loop finder no need to restart
                            bool match_loop = false;
                            while(HI != HE && (*HI)->scope.begin <= I->lastline) {
                                // Will only match the *fist* in top level matching loop.
                                // FIXME may need to get all the loops.
                                for (loop_iterator I = funcLoops.begin(), E = funcLoops.end();
                                        I != E; I++) {
                                    if (I->intersects((*HI)->scope)) {
                                        match_loop = true;
                                        if (LOCAL_DEBUG)
                                            cout << "loop: " << *I << " ";
                                        else
                                            cout << *I << " ";
                                    }
                                }
                                HI++;
                            }
                            if (!match_loop) {
                                if (LOCAL_DEBUG)
                                    cout << "loop: none";
                            }
                            cout << "\n";
                        }
                        if (s == 0) {
                            if (LOCAL_DEBUG)
                                cout << "insignificant scope\n";
                        }
                    }
                    FPasses->doFinalization();
                    break;
                }
            }
            if (!found) 
                chap->skip_rest_of_hunks();
        }
    }
}

static char const * option_help[] =
{
    " -b BCFILE1,BCFILE2,...\tA comma separated list of bc files from before-revision source code.",
    " -a BCFILE1,BCFILE2,...\tA comma separated list of bc files from after-revision source code.",
    " -p STRIPLEN\tLevel of components to be striped of the path inside the debug symbol.",
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

void parseList(vector<string> & vec, char *arg, const char *delim)
{
    char *str = strtok(arg, delim);
    while (str != NULL) {
        vec.push_back(string(str)); 
        str = strtok(NULL, delim);
    }
}

void load(LLVMContext & Context, vector<string> & vec, bool before)
{
    vector<string>::iterator it = vec.begin(), et = vec.end();
    for (; it != et; it++) {
        Module *module = ReadModule(Context, *it);
        if (module == NULL)  {
            cout << "cannot load module " << *it << endl;
            exit(1);
        }
        if (before)
            bmodules.push_back(module);
        else
            amodules.push_back(module);
        int s = strip_len;
        if (s < 0) {
            s =  count_strips(*module);
            if (LOCAL_DEBUG)
                cout << "Calculated bstrips: " << s << " for " << *it << endl;
        }
        if (before)
            bstrips.push_back(s);
        else
            astrips.push_back(s);
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
    vector<string> bs, as;
    while((opt = getopt(argc, argv, "a:b:p:h")) != -1) {
        switch(opt) {
            case 'a':
                parseList(as, optarg, ",");
                break;
            case 'b':
                parseList(bs, optarg, ",");
                break;
            case 'p':
                plen = atoi(optarg);
                if (plen <= 0) {
                    fprintf(stderr, "Strip len must be positive integer\n");
                    exit(1);
                }
                strip_len = plen;
                break;
            case 'h':
                usage();
                exit(0);
            case '?':
            default:
                usage();
                exit(1);
        }
    }
    if (bs.begin() == bs.end()) {
        fprintf(stderr, "Must specify before-revision bitcode file argument\n");
        exit(1);
    }
    if (optind != argc-1) {
        usage();
        exit(1);
    }
    id_fname = dupstr(argv[optind]);
    LLVMContext & Context = getGlobalContext();
    load(Context, as, false);
    load(Context, bs, true);
    assess(id_fname);
    return 0;
}
