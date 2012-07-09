#include "Handy.h"
#include "PatchDecoder.h"
#include "Matcher.h"

#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Type.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/IRReader.h"

#include <iostream>
#include <limits.h>
#include <stdlib.h>

#include <vector>

using namespace std;

static bool DEBUG = false;

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

void test_PatchDecoder(char *input)
{
    LLVMContext Context;
    Module *module = ReadModule(Context, "sql_select.o");
    assert(module);
    Matcher matcher(*module, 0, 6);


    PatchDecoder * decoder = new PatchDecoder(input);
    assert(decoder);
    Patch *patch = NULL;
    Chapter *chap = NULL;
    Hunk * hunk = NULL;
    Mod * mod = NULL;
    while((patch = decoder->next_patch()) != NULL) {
        if (DEBUG)
            cout << "patch: " << patch->patchname << endl;
        while((chap = patch->next_chapter()) != NULL) {
            if (DEBUG)
                cout << "chapter: " << chap->filename << endl;
            while((hunk = chap->next_hunk()) != NULL) {
                if (DEBUG) {
                    cout << "hunk: " << hunk->start_line << endl;
                    cout << hunk->ctrlseq << endl;
                }
                assert(hunk->reduce());
                //if (DEBUG)
                    cout << hunk->enclosing_scope << endl;
                testMatching(chap->fullname, matcher, hunk->enclosing_scope);
                for (Hunk::iterator I = hunk->begin(), E = hunk->end();
                    I != E; I++) {
                    mod = *I;
                    assert(mod);
                    if (DEBUG)
                        cout << "mod: " << *mod << endl;
                }
            }
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
    test_stripname();
    //test_PatchDecoder(argv[1]);
    //test_ScopeFinder();
    test_canonpath();
    return 0;
}
