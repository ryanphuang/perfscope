#include <iostream>
#include <limits.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <list>

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

#include "mapper/Handy.h"
#include "mapper/PatchDecoder.h"
#include "mapper/CallSiteFinder.h"
#include "mapper/Matcher.h"

using namespace std;

#define LOCAL_DEBUG true

#define STRIP_LEN 7 // define number of components(slashes) to strip of the full path in debug info 

static char * program_name;

static char * bc_fname = NULL;
static char * id_fname = NULL;

static list<Module *> lmodules;
static list<int> lstrips;

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

inline static void begin_test(const char * name)
{
    cout << "Testing " << name << "..." << endl;
}

inline static void one_test(int & total, int & failed, bool fail, string expect = "", 
        string actual = "")
{
    cout << "Test case #" << total << " ";
    if (fail) {
        cout << "failed";
        if (!expect.empty())
            cout << ": expected(" << expect << "), got(" << actual << ")";
        cout << "." << endl;
    }
    else
        cout << "succeeded." << endl;
    total++;
    failed += fail;
}

inline static void end_test(const char *name, int & total, int & failed)
{
    cout << "Testing " << name << " finished: total " << total << ", " << failed << " failed." <<  endl;
}

static const char * canonpath_test[] = {
    "a/./b//c",
    "/a/./b//c/./d/",
    "home/ryan/./Documents//../Projects/",
    "home/../root",
    0
};

static const char * canonpath_expect[] = {
    "a/b/c",
    "/a/b/c/d",
    "home/ryan/Projects",
    "root",
    0
};

void test_canonpath()
{
    int total = 1, failed = 0;
    bool fail = false;
    begin_test("canonpath");
    const char **t = canonpath_test;
    const char **e = canonpath_expect;
    const char *result, *expect;
    while (*t && *e) {
        result = canonpath(*t, NULL);
        expect = *e;
        if((fail = (strcmp(result, expect) != 0))) {
            one_test(total, failed, fail, expect, result);
        }
        else
            one_test(total, failed, fail);
        t++;
        e++;
    }
    end_test("canonpath", total, failed);
}


typedef struct stripname_T {
    const char * name;
    int lstrips;
} stripname_T;

static const stripname_T stripname_test [] = {
    {"MYSQLPlus//MYSQLPlusTest/MYSQLPlusTest.cpp", -1},
    {"MYSQLPlus//MYSQLPlusTest/MYSQLPlusTest.cpp", 1},
    {"a//b/c///", -1},
    {"/home/ryan/Projects/llvm-exp/mysql-5.0.15/sql/./sql_string.h", 6},
    {0, 0}
};

static const char * stripname_expect [] = {
    "MYSQLPlusTest.cpp",
    "MYSQLPlusTest/MYSQLPlusTest.cpp",
    "",
    "sql/sql_string.h",
    0
};

void test_stripname()
{
    int total = 1, failed = 0;
    bool fail = false;
    begin_test("stripname");
    const stripname_T *t = stripname_test;
    const char **e = stripname_expect;
    const char *result, *expect;
    while (t->name && *e) {
        result = stripname(t->name, t->lstrips);
        expect = *e;
        if((fail = (strcmp(result, expect) != 0))) {
            one_test(total, failed, fail, expect, result);
        }
        else
            one_test(total, failed, fail);
        t++;
        e++;
    }
    end_test("stripname", total, failed);
}

void test_src2obj()
{
    char *p;
    p = src2obj("storage/innobase/fil/fil0fil.c", objname, &objlen);
    assert(streq(p, "storage/innobase/fil/fil0fil.o"));
    p = src2obj("client/mysql_plugin.c", objname, &objlen);
    assert(streq(p, "client/mysql_plugin.o"));
    p = src2obj("include/m_ctype.h", objname, &objlen);
    assert(p == NULL);
}

void testMatching(string & filename, Matcher & matcher, Scope & scope)
{
    Function * f;
    int s = 0;
    errs() << scope << " might touch ";
    Matcher::sp_iterator I = matcher.initMatch(filename);
    while ((f = matcher.matchFunction(I, scope)) != NULL ) {
        s++;
        errs() << "scope #" << s << ": " << f->getName() << " |=> " << scope << ", ";
    }
    if (s == 0) {
        errs() << "insignificant scope";
    }
    errs() << "\n";
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

void test_PatchDecoder(char *input)
{
    llvm_shutdown_obj Y;  // Call llvm_shutdown() on exit.
    PatchDecoder * decoder = new PatchDecoder(input);
    assert(decoder);
    Patch *patch = NULL;
    Chapter *chap = NULL;
    Hunk * hunk = NULL;
    // Create a PassManager to hold and optimize the collection of passes we are
    // about to build.
    //
    //PassManager Passes;

    OwningPtr<FunctionPassManager> FPasses;
    // Initialize passes
    PassRegistry &Registry = *PassRegistry::getPassRegistry();
    initializeCore(Registry);
    initializeScalarOpts(Registry);
    initializeIPO(Registry);
    initializeAnalysis(Registry);
    initializeIPA(Registry);
    initializeTransformUtils(Registry);
    initializeInstCombine(Registry);
    initializeInstrumentation(Registry);
    initializeTarget(Registry);
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
            list<int>::iterator ii = lstrips.begin(), ie = lstrips.end();
            for(list<Module *>::iterator mi = lmodules.begin(), me = lmodules.end(); mi != me && ii != ie; mi++, ii++) {
                module = *mi;
                if (module == NULL)
                    continue;
                Matcher matcher(*module, 0, *ii);
                Matcher::sp_iterator I = matcher.initMatch(chap->fullname);
                if (I == matcher.sp_end()) {
                    continue;
                }
                else {
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
//  " -f BCFILE A single BC file to be used. If this option is not 
//    specified. The BC file will be infered from the source name.",
    " -d Intermediate diff file. Required.",
    " -p STRIPLEN Level of components to be striped of the path inside the debug symbol.",
    " -h Print this message.",
     0
};

void usage(FILE *fp = stderr)
{
    const char **p = option_help;
    fprintf(fp, "Usage: %s BCFILE...", program_name);
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
    int strip_len = -1;
    while((opt = getopt(argc, argv, "d:p:h")) != -1) {
        switch(opt) {
            case 'd':
                id_fname = dupstr(optarg);
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
                exit(1);
            case '?':
                if (optopt == 'f')
                    fprintf(stderr, "Must specify the bitcode file argument\n");
                else if (optopt == 'p')
                    fprintf(stderr, "Must specify the bitcode file argument\n");
                else
                    usage();
                return 1;
            default:
                usage();
                exit(1);
        }
    }
    if (id_fname == NULL || optind >= argc) {
        usage();
        exit(1);
    }
    int i;
    LLVMContext & Context = getGlobalContext();
    for (i = optind ; i < argc; i++) {
        Module *module = ReadModule(Context, argv[i]);
        if (module == NULL)  {
            cout << "cannot load module " << argv[i] << endl;
            return 1;
        }
        lmodules.push_back(module);
        if (strip_len < 0) {
            size_t s =  count_strips(*module);
            lstrips.push_back(s);
            if (LOCAL_DEBUG)
                cout << "Calculated lstrips: " << s << " for " << argv[i] << endl;
        }
        else
            lstrips.push_back(strip_len);
    }
    test_PatchDecoder(id_fname);
    //test_src2obj();
    //test_canonpath();
    //test_stripname();
    return 0;
}
