/* @(#) main.cc -- the entry point of parser */

#define XTERN
#include<common.h>
#undef XTERN
#include<parser.h>
#include<handy.h>
#include<getopt.h>
#include<backupfile.h>
#include<string>

static const char* program_name;

static const char* options[] = {
 "-d DIRECTORY --directory DIRECTORY\tTreat every file in DIRECTORY as input patch to parse.",
 "-v --version Version information.",
 "-h --help Print help information.",
 0 // explicit sentinel
};

static bool from_directory = false;
static std::string directory("./");

static const char* shortopts = "d:vh";

static PatchParser* current_parser = NULL;

static const struct option longopts[] =
{
  {"directory", required_argument, NULL, 'd'},
  {"version", no_argument, NULL, 'v'},
  {"help", no_argument, NULL, 'h'},
  {NULL, no_argument, NULL, 0}
};

static void usage(FILE *stream)
{
    fprintf(stream, "Usage: %s  [OPTION]... [PATCHFILE]\n\n", program_name);
    const char **p;
    for (p = options; *p; p++) {
        fprintf(stream, "%s\n\n", *p);
    }
}

static void version()
{
    printf("%s Version 1.0 \n", program_name);
}

void cleanup()
{
    if (debug) {
        fprintf(stdout, "cleaning up...\n");
    }
    if (current_parser != NULL) {
        current_parser->cleanup();
    }
}

int main(int argc, char *argv[])
{
    program_name = dupstr(argv[0]);

    atexit(cleanup); // clean up work
    
    int optc;

    while ((optc = getopt_long (argc, argv, shortopts, longopts, (int *) 0)) != -1) {
        switch(optc) {
            case 'd': {
                    from_directory = true;
                    if (optarg == NULL) {
                        diegrace("NULL directory");
                    }
                    directory.assign(optarg);
                    if (directory.at(directory.length() - 1) != DIRECTORY_SEPARATOR) {
                        directory += DIRECTORY_SEPARATOR;
                    }
                }
                break;
            case 'v':
                version();
                break;
            case 'h':
                usage(stdout);
                break;
            default:
                usage(stderr);
        }
    }
    if (from_directory && (argc == optind + 1)) {
        fprintf(stderr, "Invalid: input files already specified from the directory\n");
        exit(1);
    }
    if (argc > optind + 1) {
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
    debug = 0;
    time(&initial_time);

    gbufsize = 8 * KB;
    gbuf = (char *) xmalloc(gbufsize);

    init_backup_hash_table(); 

    filenode *lst;
    if (from_directory) {
        lst = listdir(directory.c_str());
        if (NULL == lst) {
            diegrace("Cannot get input files from %s\n", directory.c_str());
        }
    }
    else{
        lst = (filenode *) malloc(sizeof(filenode));
        lst->file = argv[optind];
        lst->next = NULL;
    }
    filenode *p;
    int files = 0;
    int nonsources = 0;
    int newfile = 0;
    int delfile = 0;
    std::string fullname;
    FILE *fnonsource = fopen("non.txt", "w");
    FILE *fnew = fopen("new.txt", "w");
    FILE *fdel = fopen("del.txt", "w");
    if (fnonsource == NULL || fnew == NULL || fdel == NULL) {
        diegrace("Faile to open file\n");
    }
    
    errstay = true;
    for (p = lst; p; p = p->next){
        PatchParser *parser; 
        FILE *gobblefp = NULL;
        if (!p->file) {
            parser = new PatchParser(NULL, NULL, UNI_DIFF); 
        }
        else {
            fullname.clear();
            if (from_directory) {
                fullname += directory;
            }
            fullname += p->file;
            DEBUG("scan patch: %s\n", fullname.c_str());
            parser = new PatchParser(fullname.c_str(), NULL, UNI_DIFF); 
            //TODO less brutal
            std::string gobblef = fullname + ".id";
            gobblefp = fopen(gobblef.c_str(), "w");
            if (gobblefp == NULL) {
                WARN("Cannot setup gobble output file. Using stdout instead.");
            }
        }
        if (NULL == parser) {
            errgrace("out of memory");
        }

        current_parser == parser;

        parser->init_output(0);
        parser->open_patch_file();

        bool apply_empty_patch = false;
        
        /* for each patch in patch file */
        while(parser->there_is_another_patch() || apply_empty_patch) {
            files++;
            DEBUG("got a patch\n");
            skipreason reason = parser->gobble(gobblefp);
            switch (reason) {
                case NO_REASON:
                    break;
                case NEW_FILE:
                    newfile++;
                    fprintf(fnew, "%s\n", parser->inname);
                    break;
                case DEL_FILE:
                    delfile++;
                    fprintf(fdel, "%s\n", parser->inname);
                    break;
                case NON_SOURCE:
                    nonsources++;
                    fprintf(fnonsource, "%s\n", parser->inname);
                    break;
                default:
                    WARN("Unknown reason %d\n", reason);
            }
            parser->reinitialize();
            apply_empty_patch = false;
        }
        if(parser->snap) { // something was wrong
            fprintf(stderr, "Oooops when parsing %s\n", fullname.c_str());
        }
        delete parser;
        current_parser = NULL;
        if (gobblefp)
            fclose(gobblefp);
    }
    if (gbuf) {
        free(gbuf);
    }
    fclose(fnonsource);
    fprintf(stderr, "Scanned total: %d; Non-source: %d; Delete: %d; Create: %d\n", files, nonsources,
    delfile, newfile);
    // save the effort to free file list
    return 0;
}
