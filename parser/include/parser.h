#ifndef __DIFF_PARSER_H__
#define __DIFF_PARSER_H__

enum nametype { OLD, NEW, INDEX, NONE };
enum difftype { NO_DIFF, CONTEXT_DIFF, NORMAL_DIFF, ED_DIFF, NEW_CONTEXT_DIFF, UNI_DIFF};

#define INITHUNKMAX 125			/* initial dynamic allocation size */

#define MAXFUZZ 2 /* fuzz factor for inexact matching, should parse from config */

/* An upper bound on the print length of a signed decimal line number.
   Add one for the sign.  */
#define LINENUM_LENGTH_BOUND (sizeof (LINENUM) * CHAR_BIT / 3 + 1) 

#ifndef TIBUFSIZE_MINIMUM
#define TIBUFSIZE_MINIMUM (8 * 1024)	/* minimum value for tibufsize */
#endif


#define merge_hunk(hunk, outstate, where, somefailed) false

struct ParserConfig {
    bool set_time;
    bool set_utc;
    bool reverse_flag_specified;
    bool backup;
    bool dry_run;
};

/* Output stream state.  */
struct outstate
{
    FILE *ofp;
    bool after_newline;
    bool zero_output;
};


class PatchParser {
public:


    char *patchname;
    FILE *pfp;

    char *inname;
    char *outfile;

    bool snap;

    difftype diff_type;

    char *p_c_function;		/* the C function a hunk is in */

    char *p_name[3];			/* filenames in patch headers */
    time_t p_timestamp[2];		/* timestamps in patch headers */
    char *p_timestr[2];		/* timestamps as strings */

    off_t p_filesize;		/* size of the patch file */


    LINENUM hunkmax;	/* size of above arrays */

    /* how long is input file in lines */
    LINENUM input_lines; 
    /* offset in the input and output at which the previous hunk matched */
    LINENUM in_offset;
    LINENUM out_offset;
    /* how many input lines have been irretractably output */

    LINENUM p_bline;			/* line # of p_base */
    LINENUM p_sline;			/* and the line number for it */
    LINENUM p_hunk_beg;		/* line number of current hunk */
    LINENUM p_efake;		/* end of faked up lines--don't free */
    LINENUM p_bfake;		/* beg of faked up lines */
    LINENUM p_first;			/* 1st line number */
    LINENUM p_newfirst;		/* 1st line number of replacement */
    LINENUM p_ptrn_lines;		/* # lines in pattern */
    LINENUM p_repl_lines;		/* # lines in replacement text */
    LINENUM p_end;		/* last line in hunk */
    LINENUM p_max;			/* max allowed value of p_end */
    LINENUM p_prefix_context;	/* # of prefix context lines */
    LINENUM p_suffix_context;	/* # of suffix context lines */
    LINENUM p_input_line;		/* current line # from patch file */
    LINENUM last_frozen_line;


    PatchParser(const char *, const char *, enum difftype);
    ~PatchParser();

    void gobble(void);

    void skippatch(void);

    char *fetchname (char *, int, char **, time_t *);

    void print_header_line (FILE *, const char *, bool);
    void print_unidiff_range (FILE *, LINENUM, LINENUM);

    LINENUM locate_hunk (LINENUM);
    bool patch_match (LINENUM, LINENUM, LINENUM, LINENUM);
    bool apply_hunk (struct outstate *, LINENUM);
    void abort_hunk (bool, bool);
    bool copy_till (struct outstate *outstate, LINENUM lastline);
    void mangled_patch (LINENUM, LINENUM);
    bool spew_output (struct outstate *, struct stat *);

    char const *pch_c_function (void);
    char const * pch_timestr (bool which);
    LINENUM pch_newfirst (void);
    LINENUM pch_prefix_context (void);
    LINENUM pch_suffix_context (void);
    bool pch_swap (void);
    bool pch_write_line (LINENUM, FILE *);
    bool there_is_another_patch ();
    char *pfetch (LINENUM);
    char pch_char (LINENUM);
    int another_hunk (enum difftype, bool);
    int pch_says_nonexistent (bool);
    size_t pch_line_len (LINENUM);
    const char *pch_name(enum nametype);
    time_t pch_timestamp (bool);
    void do_ed_script (FILE *);
    void open_patch_file (void);
    void re_patch (void);
    void set_hunkmax (void);
    void pch_normalize (enum difftype);
    void next_intuit_at (file_offset, LINENUM);
    void skip_to (file_offset, LINENUM);
    size_t pget_line (size_t, int, bool, bool);
    size_t get_line (void);
    bool maybe_reverse (const char *, bool, bool);
    bool ok_to_reverse (char const *, ...) __attribute__ ((format (printf, 2, 3)));

    bool version_get (char const *, char const *, bool, bool, char const *, struct stat *);

    void malformed(void) __attribute__ ((noreturn));
    char * format_linenum (char [], LINENUM);
    bool grow_hunkmax (void);

    static int prefix_components (char *, bool);
    static char get_ed_command_letter (const char *);

    static enum nametype best_name (char * const *, int const *);
    enum difftype intuit_diff_type (bool);

    void cleanup(void);
    void reinitialize(void);

    const char *ifetch (LINENUM, bool, size_t *);
    void get_input_file (char const *, char const *);
    void re_input (void);
    void scan_input (char *);

    bool plan_a (char const *);	/* yield false if memory runs out */
    void plan_b (char const *);
    void report_revision (bool);
    char * scan_linenum (char *, LINENUM *);

    bool incomplete_line (void);

    void init_output (int);

protected:
    void init_output (const char *, int , struct outstate *);


    char *buf;			/* general purpose buffer */
    size_t bufsize;			/* allocated size of buf */
    static LINENUM maxfuzz;

    char linenumbuf[LINENUM_LENGTH_BOUND + 1];

    struct outstate outstate;
    struct stat instat;
    struct stat outst;

    const char * volatile tmpinname;
    const char * volatile tmpoutname;
    const char * volatile tmppatname;

    int volatile tmpinname_needs_removal;
    int volatile tmpoutname_needs_removal;
    int volatile tmppatname_needs_removal;

    int p_says_nonexistent[2];	/* [0] for old file, [1] for new:
		0 for existent and nonempty,
		1 for existent and probably (but not necessarily) empty,
		2 for nonexistent */

    int p_rfc934_nesting;		/* RFC 934 nesting level */

    char **p_line;			/* the text of the hunk */
    size_t *p_len;			/* line length including \n if any */
    char *p_Char;			/* +, -, and ! */
    size_t p_indent;			/* indent to patch */

    bool p_strip_trailing_cr;	/* true if stripping trailing \r */
    bool p_pass_comments_through;	/* true if not ignoring # lines */
    bool skip_rest_of_patch;

    file_offset p_base;		/* where to intuit this time */
    file_offset p_start;		/* where intuit found a patch */

    int inerrno;
    int invc;

    int strippath;
    int patch_get;

    bool dry_run;
    bool using_plan_a;
    bool canonicalize;
    bool set_time;
    bool set_utc;
    bool force;
    bool batch;
    bool noreverse;
    bool reverse;
    /* true if -R was specified on command line.  */
    bool reverse_flag_specified;


    char *i_buffer;			/* plan A buffer */
    char const **i_ptr;		/* pointers to lines in plan A buffer */

    size_t tibufsize;		/* size of plan b buffers */
    int tifd;			/* plan b virtual string array */
    char *tibuf[2];			/* plan b buffers */

    LINENUM tiline[2];	/* 1st line in each buffer */
    LINENUM lines_per_buf;		/* how many lines per buffer */

    size_t tireclen;			/* length of records in tmp file */
    size_t last_line_size;		/* size of last input line */
};

typedef PatchParser * PPParser;

#endif
