#define XTERN
#include<common.h>
#undef XTERN
#include<parser.h>
#include<handy.h>

int main(int argc, const char **argv)
{
    verbosity = SILENT;
    posixly_correct = false;
    batch = false;
    initial_time = 0;
    no_strip_trailing_cr = false;
    explicit_inname = false;
    revision = NULL;
    debug = false;
    time(&initial_time);
    gbufsize = 8 * KB;
    gbuf = (char *) xmalloc(gbufsize);
    init_backup_hash_table(); 

    free(gbuf);
}
