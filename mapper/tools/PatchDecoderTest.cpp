#include "PatchDecoder.h"
#include "Handy.h"
#include<iostream>

using namespace std;


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
    cout << "Testing stripname finished: total " << total << ", " << failed << " failed." <<  endl;
}

void test_PatchDecoder(char *input)
{
    PatchDecoder * decoder = new PatchDecoder(input);
    assert(decoder);
    Patch *patch = NULL;
    Chapter *chap = NULL;
    Hunk * hunk = NULL;
    Mod * mod = NULL;
    while((patch = decoder->next_patch()) != NULL) {
        cout << "patch: " << patch->patchname << endl;
        while((chap = patch->next_chapter()) != NULL) {
            cout << "chapter: " << chap->filename << endl;
            while((hunk = chap->next_hunk()) != NULL) {
                cout << "hunk: " << hunk->start_line << endl;
                cout << hunk->ctrlseq << endl;
                assert(hunk->reduce());
                for (Hunk::iterator I = hunk->begin(), E = hunk->end();
                    I != E; I++) {
                    mod = *I;
                    assert(mod);
                    cout << "mod: " << *mod << endl;
                }
            }
        }
    }
}


int main(int argc, char *argv[])
{
    if (argc <= 1) {
        cerr << "Usage: " << argv[0] << " FILE" << endl;
        exit(1);
    }
    test_stripname();
    test_PatchDecoder(argv[1]);

    /**
    
    **/
    return 0;
    
}
