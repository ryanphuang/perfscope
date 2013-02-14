#include "commons/handy.h"
#include "parser/PatchDecoder.h"

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

static char * program_name;

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

static char const * option_help[] =
{
  " -h Print this message.",
  0
};

void usage(FILE *fp = stderr)
{
  const char **p = option_help;
  fprintf(fp, "Usage: %s FILE(patch ir file, an output from patch-c)", program_name);
  fprintf(fp, "\n");
  while (*p) {
    fprintf(fp, "%s\n\n", *p);
    p++;
  }
}

int main(int argc, char *argv[])
{
  program_name = argv[0];
  if (argc == 1) {
    usage();
    exit(1);
  }
  int opt;
  while((opt = getopt(argc, argv, "h")) != -1) {
    switch(opt) {
      case 'h':
        usage();
        exit(0);
      default:
        usage();
        exit(1);
    }
  }
  if (optind + 1 < argc) {
    usage();
    exit(1);
  }
  test_PatchDecoder(argv[optind]);
  return 0;
}
