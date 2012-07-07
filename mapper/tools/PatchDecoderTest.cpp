#include "PatchDecoder.h"
#include "Handy.h"
#include<iostream>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc <= 1) {
        cerr << "Usage: " << argv[0] << " FILE" << endl;
        exit(1);
    }
    PatchDecoder * decoder = new PatchDecoder(argv[1]);
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
    
    return 0;
    
}
