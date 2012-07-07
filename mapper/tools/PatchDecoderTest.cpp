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
    Patch *patch = decoder->next_patch();
    assert(patch);
    cout << "patch: " << patch->patchname << endl;
    Chapter *chap = patch->next_chapter();
    assert(chap);
    cout << "chapter: " << chap->filename << endl;
    return 0;
    
}
