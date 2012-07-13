#include "Handy.h"
#include "PatchDecoder.h"
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
#include "llvm/Target/TargetData.h"

#include <iostream>
#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <vector>

using namespace std;

#define DEBUG true

#define STRIP_LEN 7 // define number of components(slashes) to strip of the full path in debug info 

static SmallVector<Scope, 4> funcLoops;

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
        /*
        int s = 0;
        Scope scope = hunk->enclosing_scope;
        errs() << hunk->enclosing_scope << " might touch ";
        ScopeInfoFinder::sp_iterator I = matcher.initMatch(chap->fullname);
        Hunk::iterator HI = hunk->begin(), HE = hunk->end();
        Scope ls;
        while ((f = matcher.matchFunction(I, scope)) != NULL ) {
            s++;
            errs() << "scope #" << s << ": " << cpp_demangle(f->getName().data())<< " |=> " << scope << ", ";
            FPasses->run(*f);
            errs() << "\t";
            if (li == NULL) {
                errs() << "NULL loopinfo" << "\n";
                continue;
            }
            if (li->begin() == li->end()) {
                errs() << "no loop in this function" << "\n";
            }
            else {
                //TODO more elegant
                //TODO get function scope
                //TODO loop finder no need to restart
                Loop * loop = NULL;
                while(HI != HE) {
                    mod = *HI;
                    // if there are multiple functions and this mod
                    // crossed the current function's scope, we break
                    // the loop
                    if (scope.end != 0  && mod->scope.begin > scope.begin)
                        break;
                    loop = Matcher::matchLoop(*li, scope);
                    if (loop != NULL) {
                        ScopeInfoFinder::getLoopScope(ls, loop);
                        errs() << "loop: " << ls;
                    }
                    HI++;
                }
                if (loop == NULL)
                    errs() << "loop: none" << "\n";
                else
                    errs() << "\n";
            }
        }
        if (s == 0) {
            errs() << "insignificant scope";
        }
        errs() << "\n";
        */
        return false;
    }

    virtual const char *getPassName() const { return PassName.c_str(); }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
        AU.setPreservesAll();
        AU.addRequired<DominatorTree>();
        AU.addRequired<LoopInfo>();
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
    int strips;
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
        result = stripname(t->name, t->strips);
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
    ScopeInfoFinder::sp_iterator I = matcher.initMatch(filename);
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


void moduleDriver()
{

}

void test_PatchDecoder(char *input)
{
    LLVMContext & Context = getGlobalContext();
    llvm_shutdown_obj Y;  // Call llvm_shutdown() on exit.
    PatchDecoder * decoder = new PatchDecoder(input);
    assert(decoder);
    Patch *patch = NULL;
    Chapter *chap = NULL;
    Hunk * hunk = NULL;
    Mod * mod = NULL;
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
    while((patch = decoder->next_patch()) != NULL) {
        if (DEBUG)
            cout << "patch: " << patch->patchname << endl;
        while((chap = patch->next_chapter()) != NULL) {
            if (DEBUG)
                cout << "chapter: " << chap->filename << endl;
            Module *module; 
            if (src2obj(chap->fullname.c_str(), objname, &objlen) == NULL) // skip header files for now
                module = NULL;
            else
                module = ReadModule(Context, objname);
            if (module == NULL) {
                cout << "cannot load module for this chapter " << endl;
                chap->skip_rest_of_hunks();
                continue;
            }
            FPasses.reset(new FunctionPassManager(module));
            FPasses->add(new LoopInfoPrinter());
            FPasses->doInitialization();
            Matcher matcher(*module, 0, STRIP_LEN);
            ScopeInfoFinder::sp_iterator I = matcher.initMatch(chap->fullname);
            while((hunk = chap->next_hunk()) != NULL) {
                if (DEBUG) {
                    cout << "hunk: " << hunk->start_line << endl;
                    cout << hunk->ctrlseq << endl;
                }
                assert(hunk->reduce());
                if (DEBUG)
                    cout << hunk->enclosing_scope << endl;
                Function *f;
                int s = 0;
                Scope scope = hunk->enclosing_scope;
                errs() << hunk->enclosing_scope << " might touch ";
                Scope ls;
                while ((f = matcher.matchFunction(I, scope)) != NULL ) {
                    s++;
                    errs() << "scope #" << s << ": " << cpp_demangle(f->getName().data())<< " |=> " << scope << ", ";
                    FPasses->run(*f);
                    errs() << "\t";
                    for (SmallVector<Scope, 4>::iterator I = funcLoops.begin(), E = funcLoops.end();
                            I != E; I++) {
                        errs() << *I << " ";
                    }
                    errs() << "\n";
                }
                if (s == 0) {
                    errs() << "insignificant scope";
                }
                errs() << "\n";
                /**
                for (Hunk::iterator I = hunk->begin(), E = hunk->end();
                    I != E; I++) {
                    mod = *I;
                    assert(mod);
                    if (DEBUG)
                        cout << "mod: " << *mod << endl;
                }
                **/
            }
            FPasses->doFinalization();
            delete module;
            module = NULL;
        }
    }
}

void test_ScopeFinder()
{
    LLVMContext context;
    Module *module = ReadModule(context, "sql_select.o");
    assert(module);
    ScopeInfoFinder finder;
    //finder.processSubprograms(*module);
}

int main(int argc, char *argv[])
{
    if (argc <= 1) {
        cerr << "Usage: " << argv[0] << " FILE" << endl;
        exit(1);
    }
    //test_src2obj();
    //test_canonpath();
    //test_stripname();
    test_PatchDecoder(argv[1]);
    //test_ScopeFinder();
    return 0;
}
