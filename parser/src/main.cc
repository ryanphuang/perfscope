/* @(#) main.cc -- the entry point of parser */

#define XTERN
#include<common.h>
#undef XTERN
#include<parser.h>
#include<handy.h>
#include<backupfile.h>

static const char* program_name;

static const char* options[] = {
 "-v --version Version information.",
 "-h --help Print help information.",
 0 // explicit sentinel
};

static void usage(FILE *stream)
{
    fprintf(stream, "Usage: %s  [OPTION]... [PATCHFILE]\n\n", program_name);
    const char **p;
    for (p = options; *p; p++) {
        fprintf(stream, "%s\n\n", *p);
    }
}

int main(int argc, const char **argv)
{
    program_name = dupstr(argv[0]);
    if (argc > 2) {
        usage(stdout);
        exit(1);
    }


    simple_backup_suffix = ".orig";

    verbosity = SILENT;
    posixly_correct = false;
    batch = false;
    initial_time = 0;
    no_strip_trailing_cr = false;
    explicit_inname = false;
    revision = NULL;
    debug = 8;
    time(&initial_time);

    gbufsize = 8 * KB;
    gbuf = (char *) xmalloc(gbufsize);

    init_backup_hash_table(); 

    PatchParser *parser = new PatchParser(argv[1], UNI_DIFF); 
    parser->init_output(0);
    parser->open_patch_file();

    bool apply_empty_patch = false;
    
    /* for each patch in patch file */
    while(parser->there_is_another_patch() || apply_empty_patch) {
        if (debug) {
            printf("got a patch\n");
        }
        if (!issource(parser->inname)) {
            printf("skip non-source patch\n");
            parser->skippatch();
        }
        parser->gobble();
        parser->reinitialize();
        apply_empty_patch = false;
    }
    free(gbuf);
    if(parser->snap) { // something was wrong
        exit(1);
    }
    return 0;
}
