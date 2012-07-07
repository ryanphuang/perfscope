#include "PatchDecoder.h"
#include<iostream>

using namespace std;
int main()
{
    PatchDecoder * decoder = new PatchDecoder("out");
    assert(decoder);
    Patch *patch = decoder->next_patch();
    assert(patch);
    cout << "patch: " << patch->patchname << endl;
    Chapter *chap = patch->next_chapter();
    assert(chap);
    cout << "chapter: " << chap->filename << endl;
    return 0;
}
