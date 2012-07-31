#include "Handy.h"
#include "PatchDecoder.h"
#include "Matcher.h"

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

void test_PatchDecoder(char *input)
{
    PatchDecoder * decoder = new PatchDecoder(input);
    assert(decoder);
    Patch *patch = NULL;
    Chapter *chap = NULL;
    Hunk * hunk = NULL;
    while((patch = decoder->next_patch()) != NULL) {
        cout << "patch: " << patch->patchname << endl;
        while((chap = patch->next_chapter()) != NULL) {
            cout << "chapter: " << chap->filename << endl;
            while((hunk = chap->next_hunk()) != NULL) {
                cout << "hunk: " << hunk->start_line << "," << hunk->rep_start_line << endl;
                cout << hunk->ctrlseq << endl;
                assert(hunk->reduce());
                Hunk::iterator HI = hunk->begin(), HE = hunk->end();
                while (HI != HE) {
                    cout << "orig scope :" << (*HI)->scope << "# rep scope :" << (*HI)->rep_scope << endl;
                    HI++;
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc == 1)
        return 1;
    test_PatchDecoder(argv[1]);
    return 0;
}
