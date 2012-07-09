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

void test_canonpath()
{
    cout << canonpath("a/./b//c", NULL) << endl;
    cout << canonpath("/a/./b//c/./d/", NULL) << endl;
    cout << canonpath("home/ryan/./Documents//../Projects/", NULL) << endl;
    cout << canonpath("home/../root", NULL) << endl;
}



void test_stripname()
{
    unsigned total = 0, failed = 0;
    bool fail = false;
    cout << "Testing stripname..." << endl;
    const char * path = "MYSQLPlus//MYSQLPlusTest/MYSQLPlusTest.cpp";
    const char * name = stripname(path, -1);
    fail = !(strcmp(name, "MYSQLPlusTest.cpp")==0);
    total++;
    failed += fail;
    cout << "Test case #1 " << (fail ? "failed" : "succeeded") << "." << endl;
    name = stripname(path, 1);
    fail = !(strcmp(name, "MYSQLPlusTest/MYSQLPlusTest.cpp")==0);
    total++;
    failed += fail;
    cout << "Test case #2 " << (fail ? "failed" : "succeeded") << "." << endl;
    path = "a//b/c///";
    name = stripname(path, -1);
    fail = !(strcmp(name, "")==0);
    total++;
    failed += fail;
    cout << "Test case #3 " << (fail ? "failed" : "succeeded") << "." << endl;
    fail = !(strcmp(stripname(realpath("/home/ryan/Projects/llvm-exp/mysql-5.0.15/sql/./sql_string.h", 
            NULL), 6), "sql/sql_string.h") == 0);
    total++;
    failed += fail;
    cout << "Testing stripname finished: total " << total << ", " << failed << " failed." <<  endl;
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
    //test_stripname();
    //test_PatchDecoder(argv[1]);
    //test_ScopeFinder();
    test_canonpath();
    return 0;
}
