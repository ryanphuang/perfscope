#define XTERN extern
#include<common.h>
#undef XTERN
#include<maketime.h>
#include<partime.h>
#include<dirname.h>
#include<handy.h>
#include<parser.h>
#include<backupfile.h>

static char const if_defined[] = "\n#ifdef %s\n";
static char const not_defined[] = "\n#ifndef %s\n";
static char const else_defined[] = "\n#else\n";
static char const end_defined[] = "\n#endif\n";

static FILE * create_output_file (char const *, int, mode_t);

PatchParser::PatchParser(const char * pname, const char *outname = NULL, enum difftype type = NO_DIFF ) :
    diff_type(type), hunkmax(INITHUNKMAX), p_efake(-1), p_bfake(-1), 
    p_end(-1), bufsize(8 * KB), skip_rest_of_patch(false), strippath(0), tifd(-1)
{
    patchname = dupstr(pname);
    outfile = dupstr(outname);
    inname = NULL;
    fullinname = NULL;
    pfp = NULL;
    p_Char = NULL;
    p_CtrlChar = NULL;
    p_len = NULL;
    p_line = NULL;
    p_c_function = NULL;
    i_buffer = NULL;
    i_ptr = NULL;
    init_array(p_name, 3);
    init_array(p_timestr, 2);
    buf = (char *) xmalloc(bufsize);

    tiline[0] = -1;
    tiline[1] = -1;
    
    tibuf[0] = NULL;
    tibuf[1] = NULL;

    //tmpoutname = make_temp ('o');
    //tmpinname = make_temp ('i');
    tmppatname = make_temp ('p');

    outstate.ofp = NULL;
    reverse_flag_specified = false;
    snap = false;
    patchprinted = false;
}

PatchParser::~PatchParser()
{
    if (outstate.ofp && (ferror (outstate.ofp) || fclose (outstate.ofp) != 0))
        write_fatal ();
    cleanup();
}

void PatchParser::re_patch()
{
    snap = false;
    p_first = 0;
    p_newfirst = 0;
    p_ptrn_lines = 0;
    p_repl_lines = 0;
    p_end = -1;
    p_max = 0;
    p_indent = 0;
    p_strip_trailing_cr = false;
}

void PatchParser::cleanup()
{
    //unlink(tmpinname);
    //unlink(tmpoutname);
    unlink(tmppatname);
    if (pfp)
        fclose(pfp);
}

/* Return a pointer to a particular patch line. */
char * PatchParser::pfetch (LINENUM line)
{
    return p_line[line];
}

/* Output a patch line.  */
bool PatchParser::pch_write_line (LINENUM line, FILE *file)
{
    bool after_newline = p_line[line][p_len[line] - 1] == '\n';
    if (! fwrite (p_line[line], sizeof (*p_line[line]), p_len[line], file))
        write_fatal ();
    return after_newline;
}

/* Return the length of a particular patch line. */
size_t PatchParser::pch_line_len (LINENUM line)
{
    return p_len[line];
}


/* Return the control character (+, -, *, !, etc) for a patch line.  A '\n'
   indicates an empty line in a hunk.  (The empty line is not part of the
   old or new context.  For some reson, the context format allows that.)  */
char PatchParser::pch_char (LINENUM line)
{
    return p_Char[line];
}

/* Return whether file WHICH (false = old, true = new) appears to nonexistent.
   Return 1 for empty, 2 for nonexistent.  */
int PatchParser::pch_says_nonexistent (bool which)
{
    return p_says_nonexistent[which];
}

/* Return timestamp of patch header for file WHICH (false = old, true = new),
   or -1 if there was no timestamp or an error in the timestamp.  */
time_t PatchParser::pch_timestamp (bool which)
{
    return p_timestamp[which];
}

const char* PatchParser::pch_timestr (bool which)
{
    return p_timestr[which];
}

const char * PatchParser::pch_name(enum nametype type)
{
    return type == NONE ? NULL : p_name[type];
}

void PatchParser::print_unidiff_range (FILE *fp, LINENUM start, LINENUM count)
{
    char numbuf0[LINENUM_LENGTH_BOUND + 1];
    char numbuf1[LINENUM_LENGTH_BOUND + 1];

    switch (count)
    {
        case 0:
            fprintf (fp, "%s,0", format_linenum (numbuf0, start - 1));
            break;

        case 1:
            fprintf (fp, "%s", format_linenum (numbuf0, start));
            break;

        default:
            fprintf (fp, "%s,%s",
                    format_linenum (numbuf0, start),
                    format_linenum (numbuf1, count));
            break;
    }
}

void PatchParser::print_header_line (FILE *fp, const char *tag, bool reverse)
{
    const char *name = pch_name ((nametype) reverse);
    const char *timestr = pch_timestr (reverse);

    /* FIXME: include timestamp as well. */
    fprintf (fp, "%s %s%s\n", tag, name ? name : "/dev/null",
            timestr ? timestr : "");
}
/* Open the new file. */
void PatchParser::init_output (char const *name, int open_flags, struct outstate *os)
{
    if (NULL == name)
        os->ofp = (FILE *) 0;
    else 
        if (strcmp (name, "-") != 0)
            os->ofp = create_output_file (name, open_flags, instat.st_mode);
        else
        {
            int stdout_dup = dup (fileno (stdout));
            os->ofp = fdopen (stdout_dup, "a");
            if (stdout_dup == -1 || ! os->ofp)
                diegrace("Failed to duplicate standard output");
            if (dup2 (fileno (stderr), fileno (stdout)) == -1)
                diegrace("Failed to redirect messages to standard error");
        }

    os->after_newline = true;
    os->zero_output = true;
}


/* Open the new file. */
void PatchParser::init_output(int open_flags)
{
    init_output(outfile, open_flags, &outstate);
}

/* Skip the current patch. */
void PatchParser::skippatch()
{
    skip_rest_of_patch = true;
}

void PatchParser::reinitialize()
{
    re_patch();
    re_input();

    input_lines = 0;
    last_frozen_line = 0;

    if (inname && ! explicit_inname) {
        free (inname);
        inname = 0;
    }

    in_offset = 0;
    out_offset = 0;

    diff_type = NO_DIFF;

    if (revision) {
        free(revision);
        revision = 0;
    }

    reverse = reverse_flag_specified;
    skip_rest_of_patch = false;
}


/* Remember where this patch ends so we know where to start up again. */
void PatchParser::next_intuit_at(file_offset file_pos, LINENUM file_line)
{
    p_base = file_pos;
    p_bline = file_line;
}


bool PatchParser::maybe_reverse (char const *name, bool nonexistent, bool empty)
{
    bool is_empty = nonexistent || empty;
    bool r;

    r = (! is_empty) < p_says_nonexistent[reverse ^ is_empty]
        && ok_to_reverse ("The next patch%s would %s the file %s,\nwhich %s!",
                reverse ? ", when reversed," : "",
                (nonexistent ? "delete"
                 : empty ? "empty out"
                 : "create"),
                quotearg (name),
                (nonexistent ? "does not exist"
                 : empty ? "is already empty"
                 : "already exists"));
    reverse ^= r;
    return r;
}

bool PatchParser::ok_to_reverse(char const *format, ...)
{
    //fprintf(stderr, "ok to reverse!\n");
    return false;
}

/* Make sure our dynamically realloced tables are malloced to begin with. */
void PatchParser::set_hunkmax (void)
{
    if (!p_line)
        p_line = (char **) malloc (hunkmax * sizeof *p_line);
    if (!p_len)
        p_len = (size_t *) malloc (hunkmax * sizeof *p_len);
    if (!p_Char)
        p_Char = (char *) malloc (hunkmax * sizeof *p_Char);
    if (!p_CtrlChar)
        p_CtrlChar = (char *) malloc (hunkmax * sizeof *p_CtrlChar);
}

void PatchParser::open_patch_file()
{
    file_offset file_pos = 0;
    struct stat st;
    if(isempty(patchname) || streq(patchname, "-")) {
        pfp = stdin;
        if (S_ISREG (st.st_mode) && (file_pos = ftell(stdin)) != -1)
        {
            pfp = stdin;
        }
        else {
            size_t charsread;
            int exclusive = tmppatname_needs_removal ? 0 : O_EXCL;
            tmppatname_needs_removal = 1;
            pfp = fdopen (create_file (tmppatname,
                        O_RDWR | O_BINARY | exclusive,
                        (mode_t) 0, true),
                    "w+b");
            if (!pfp)
                diegrace ("Can't open stream for file %s", quotearg (tmppatname));
            for (st.st_size = 0;
                    (charsread = fread (buf, 1, bufsize, stdin)) != 0;
                    st.st_size += charsread)
                if (fwrite (buf, 1, charsread, pfp) != charsread)
                    write_fatal ();
            if (ferror (stdin) || fclose (stdin) != 0)
                read_fatal ();
            if (fflush (pfp) != 0
                    || fseek (pfp, (file_offset) 0, SEEK_SET) != 0)
                write_fatal ();
        }
    }
    else {
        pfp = fopen(patchname, "r");
        if(NULL == pfp ) {
            diegrace("Unable to open %s", patchname);
        }
        if (fstat(fileno(pfp), &st) != 0) {
            diegrace("Cannot fstat %s", patchname);
        }
    }
    p_filesize = st.st_size;
    if (p_filesize != (file_offset) p_filesize)
        diegrace("patch file is too long");
    next_intuit_at (file_pos, (LINENUM) 1);
    set_hunkmax();
}

char * PatchParser::format_linenum (char numbuf[LINENUM_LENGTH_BOUND + 1], LINENUM n)
{
    char *p = numbuf + LINENUM_LENGTH_BOUND;
    *p = '\0';

    if (n < 0)
    {
        do
            *--p = '0' - (int) (n % 10);
        while ((n /= 10) != 0);

        *--p = '-';
    }
    else
    {
        do
            *--p = '0' + (int) (n % 10);
        while ((n /= 10) != 0);
    }

    return p;
}

void PatchParser::mangled_patch (LINENUM old, LINENUM newl)
{
    char numbuf0[LINENUM_LENGTH_BOUND + 1];
    char numbuf1[LINENUM_LENGTH_BOUND + 1];
    if (debug & 1)
        say ("oldchar = '%c', newchar = '%c'\n",
                pch_char (old), pch_char (newl));
    errgrace("Out-of-sync patch, lines %s,%s -- mangled text or line numbers, "
            "maybe?",
            format_linenum (numbuf0, p_hunk_beg + old),
            format_linenum (numbuf1, p_hunk_beg + newl));
}

/* Copy input file to output, up to wherever hunk is to be applied. */
bool PatchParser::copy_till (struct outstate *outstate, LINENUM lastline)
{
    LINENUM R_last_frozen_line = last_frozen_line;
    FILE *fp = outstate->ofp;
    char const *s;
    size_t size;

    if (R_last_frozen_line > lastline)
    {
        say ("misordered hunks! output would be garbled\n");
        return false;
    }
    while (R_last_frozen_line < lastline)
    {
        s = ifetch (++R_last_frozen_line, false, &size);
        if (size)
        {
            if ((! outstate->after_newline  &&  putc ('\n', fp) == EOF)
                    || ! fwrite (s, sizeof *s, size, fp))
                write_fatal ();
            outstate->after_newline = s[size - 1] == '\n';
            outstate->zero_output = false;
        }
    }
    last_frozen_line = R_last_frozen_line;
    return true;
}

/* We found where to apply it (we hope), so do it. */
bool PatchParser::apply_hunk (struct outstate *outstate, LINENUM where)
{
    return true;
}

/* Does the patch pattern match at line base+offset? */
bool PatchParser::patch_match (LINENUM base, LINENUM offset,
        LINENUM prefix_fuzz, LINENUM suffix_fuzz)
{
    LINENUM pline = 1 + prefix_fuzz;
    LINENUM iline;
    LINENUM pat_lines = p_ptrn_lines - suffix_fuzz;
    size_t size;
    char const *p;

    for (iline=base+offset+prefix_fuzz; pline <= pat_lines; pline++,iline++) {
        p = ifetch (iline, offset >= 0, &size);
        if (canonicalize) {
            if (!similar(p, size,
                        pfetch(pline),
                        pch_line_len(pline) ))
                return false;
        }
        else if (size != pch_line_len (pline)
                || memcmp (p, pfetch (pline), size) != 0)
            return false;
    }
    return true;
}

/* Output the rejected hunk.  */
void PatchParser::abort_hunk (bool header, bool reverse)
{
    FILE *fp = stdout;
    if (header)
    {
        if (pch_name (INDEX))
            fprintf(fp, "Index: %s\n", pch_name (INDEX));
        print_header_line (stdout, "---", reverse);
        print_header_line (stdout, "+++", ! reverse);
    }
}

/* analyze each patch in patch file */
skipreason PatchParser::gobble(FILE *_outfp)
{
    
    int hunk = 0;
    char *outname = NULL;
    enum skipreason reason = NO_REASON;

    char numbuf1[LINENUM_LENGTH_BOUND + 1];
    char numbuf2[LINENUM_LENGTH_BOUND + 1];

    FILE *outfp;
    if (_outfp == NULL)
        outfp = stdout;
    else
        outfp = _outfp;

    if (!issource(inname)) {
        DEBUG("skip non-source: %s\n", inname);
        reason = NON_SOURCE;
        skip_rest_of_patch = true;
    }

    if (!skip_rest_of_patch) {
        outname = outfile ? outfile : inname;
        get_input_file (inname, outname);
    }

    if (diff_type == ED_DIFF) {
        errgrace("ED type diff not supported\n");
    } else {
        int got_hunk;

        /* initialize the patched file */
        if (!skip_rest_of_patch)
        {
            /* find out where all the lines are */
            scan_input (inname);
        }

        /* from here on, open no standard i/o files, because malloc */
        /* might misfire and we can't catch it easily */

        /* apply each hunk of patch */
        while (0 < (got_hunk = another_hunk (diff_type)))
        {
            DEBUG("got a hunk\n");
           
            hunk++;
            if (hunk == 1) {
                if (p_first == 1 && p_ptrn_lines == 0) {
                    reason = NEW_FILE;
                    DEBUG("new file\n");
                    skip_rest_of_patch = true;
                }
                else
                    if (p_newfirst == 1 && p_repl_lines == 0) {
                        reason = DEL_FILE;
                        DEBUG("delete file\n");
                        skip_rest_of_patch = true;
                    }
                    else {
                            if (!skip_rest_of_patch) {
                                if (!patchprinted) {
                                    fprintf(outfp, "****\n%s\n", patchname);
                                    patchprinted = true;
                                }
                                fprintf(outfp, "====\n%s\n", inname) ;
                            }
                    }
            } 
            if (!skip_rest_of_patch) {
                if (p_nctrl > 0)
                    fprintf(outfp, "####%s,%s\n%s\n", format_linenum(numbuf1, p_first), 
                        format_linenum(numbuf2, p_newfirst), p_CtrlChar);
            }
            // We shouldn't break here, just let the parser gobble the rest of the patch
            // to move on to the next patch
        }
    }

    return reason;
}

/* True if the remainder of the patch file contains a diff of some sort. */
bool PatchParser::there_is_another_patch()
{
    bool need_header = (!(inname || posixly_correct));
    if (p_base != 0 && p_base >= p_filesize) {
        DEBUG("done\n");
        return false;
    }
    diff_type = intuit_diff_type (need_header);
    if (diff_type == NO_DIFF) {
        if (verbosity == VERBOSE)
            say (p_base
                    ? "  Ignoring the trailing garbage.\ndone\n"
                    : "  I can't seem to find a patch in there anywhere.\n");
        if (! p_base && p_filesize)
            diegrace("Only garbage was found in the patch input.");
        return false;
    }
    if (skip_rest_of_patch)
    {
        Fseek (pfp, p_start, SEEK_SET);
        p_input_line = p_sline - 1;
        return true;
    }
    if (verbosity == VERBOSE)
        say ("  %sooks like %s to me...\n",
                (p_base == 0 ? "L" : "The next patch l"),
                diff_type == UNI_DIFF ? "a unified diff" :
                diff_type == CONTEXT_DIFF ? "a context diff" :
                diff_type == NEW_CONTEXT_DIFF ? "a new-style context diff" :
                diff_type == NORMAL_DIFF ? "a normal diff" :
                "an ed script" );

    if (verbosity != SILENT)
    {
        if (p_indent)
            say ("(Patch is indented %lu space%s.)\n",
                    (unsigned long int) p_indent, p_indent==1?"":"s");
        if (no_strip_trailing_cr)
            p_strip_trailing_cr = false;
        else if (p_strip_trailing_cr)
            say ("(Stripping trailing CRs from patch.)\n");
        if (! inname)
        {
            char numbuf[LINENUM_LENGTH_BOUND + 1];
            say ("can't find file to patch at input line %s\n",
                    format_linenum (numbuf, p_sline));
            if (diff_type != ED_DIFF && diff_type != NORMAL_DIFF)
                say (strippath == -1
                        ? "Perhaps you should have used the -p or --strip option?\n"
                        : "Perhaps you used the wrong -p or --strip option?\n");
        }
    }

    skip_to(p_start,p_sline);
    while (!inname) {
        char *t;
        if (force | batch) {
            say ("No file to patch.  Skipping patch.\n");
            skip_rest_of_patch = true;
            return true;
        }
        ask ("File to patch: ");
        t = buf + strlen (buf);
        if (t > buf + 1 && *(t - 1) == '\n')
        {
            inname = dupbuf (buf, t - buf);
            inname[t - buf - 1] = 0;
            if (stat (inname, &instat) == 0)
            {
                inerrno = 0;
                invc = -1;
            }
            else
            {
                perror (inname);
                fflush (stderr);
                free (inname);
                inname = 0;
            }
        }
        if (!inname) {
            ask ("Skip this patch? [y] ");
            if (*buf != 'n') {
                if (verbosity != SILENT)
                    say ("Skipping patch.\n");
                skip_rest_of_patch = true;
                return true;
            }
        }
    }
    return true;
}

/* Basically a verbose fseek() to the actual diff listing. */

void PatchParser::skip_to (file_offset file_pos, LINENUM file_line)
{
    FILE *i = pfp;
    FILE *o = stdout;
    int c;

    assert(p_base <= file_pos);
    if ((verbosity == VERBOSE || !inname) && p_base < file_pos) {
        Fseek (i, p_base, SEEK_SET);
        say ("The text leading up to this was:\n--------------------------\n");

        while (ftell(i) < file_pos)
        {
            putc ('|', o);
            do
            {
                if ((c = getc (i)) == EOF)
                    read_fatal ();
                putc (c, o);
            }
            while (c != '\n');
        }

        say ("--------------------------\n");
    }
    else
        Fseek (i, file_pos, SEEK_SET);
    p_input_line = file_line - 1;
}

size_t PatchParser::get_line(void)
{
    return pget_line (p_indent, p_rfc934_nesting, p_strip_trailing_cr,
            p_pass_comments_through);
}

/* Input a line from the patch file, worrying about indentation.
   Strip up to INDENT characters' worth of leading indentation.
   Then remove up to RFC934_NESTING instances of leading "- ".
   If STRIP_TRAILING_CR is true, remove any trailing carriage-return.
   Unless PASS_COMMENTS_THROUGH is true, ignore any resulting lines
   that begin with '#'; they're comments.
   Ignore any partial lines at end of input, but warn about them.
   Succeed if a line was read; it is terminated by "\n\0" for convenience.
   Return the number of characters read, including '\n' but not '\0'.
   Return -1 if we ran out of memory.  */

size_t PatchParser::pget_line (size_t indent, int rfc934_nesting, bool strip_trailing_cr,
        bool pass_comments_through)
{
    FILE *fp = pfp;
    int c;
    size_t i;
    char *b;
    size_t s;

    do
    {
        i = 0;
        for (;;)
        {
            c = getc (fp);
            if (c == EOF)
            {
                if (ferror (fp))
                    read_fatal ();
                return 0;
            }
            if (indent <= i)
                break;
            if (c == ' ' || c == 'X')
                i++;
            else if (c == '\t')
                i = (i + 8) & ~7;
            else
                break;
        }

        i = 0;
        b = buf;

        while (c == '-' && 0 <= --rfc934_nesting)
        {
            c = getc (fp);
            if (c == EOF)
                goto patch_ends_in_middle_of_line;
            if (c != ' ')
            {
                i = 1;
                b[0] = '-';
                break;
            }
            c = getc (fp);
            if (c == EOF)
                goto patch_ends_in_middle_of_line;
        }

        s = bufsize;

        for (;;)
        {
            if (i == s - 1)
            {
                s *= 2;
                b = (char *) realloc (b, s);
                if (!b)
                {
                    if (!using_plan_a)
                        xalloc_die ();
                    return (size_t) -1;
                }
                buf = b;
                bufsize = s;
            }
            b[i++] = c;
            if (c == '\n')
                break;
            c = getc (fp);
            if (c == EOF)
                goto patch_ends_in_middle_of_line;
        }

        p_input_line++;
    }
    while (*b == '#' && !pass_comments_through);

    if (strip_trailing_cr && 2 <= i && b[i - 2] == '\r')
        b[i-- - 2] = '\n';
    b[i] = '\0';
    return i;

patch_ends_in_middle_of_line:
    if (ferror (fp))
        read_fatal ();
    say ("patch unexpectedly ends in middle of line\n");
    return 0;
}


/* Determine what kind of diff is in the remaining part of the patch file. */
enum difftype PatchParser::intuit_diff_type (bool need_header)
{
    file_offset this_line = 0;
    file_offset first_command_line = -1;
    char first_ed_command_letter = 0;
    LINENUM fcl_line = 0; /* Pacify `gcc -W'.  */
    bool this_is_a_command = false;
    bool stars_this_line = false;
    //    enum nametype i;
    int i;
    struct stat st[3];
    int stat_errno[3];
    int version_controlled[3];
    enum difftype retval;

    for (i = OLD;  i <= INDEX;  i++)
        if (p_name[i]) {
            free (p_name[i]);
            p_name[i] = 0;
        }
    for (i = OLD; i <= NEW; i++)
        if (p_timestr[i])
        {
            free(p_timestr[i]);
            p_timestr[i] = 0;
        }

    /* Ed and normal format patches don't have filename headers.  */
    if (diff_type == ED_DIFF || diff_type == NORMAL_DIFF)
        need_header = false;

    version_controlled[OLD] = -1;
    version_controlled[NEW] = -1;
    version_controlled[INDEX] = -1;
    p_rfc934_nesting = 0;
    p_timestamp[OLD] = p_timestamp[NEW] = (time_t) -1;
    p_says_nonexistent[OLD] = p_says_nonexistent[NEW] = 0;
    Fseek (pfp, p_base, SEEK_SET);
    p_input_line = p_bline - 1;
    for (;;) {
        char *s;
        char *t;
        file_offset previous_line = this_line;
        bool last_line_was_command = this_is_a_command;
        bool stars_last_line = stars_this_line;
        size_t indent = 0;
        char ed_command_letter;
        bool strip_trailing_cr;
        size_t chars_read;

        this_line = ftell (pfp);
        chars_read = pget_line (0, 0, false, false);
        if (chars_read == (size_t) -1)
            xalloc_die();
        if (! chars_read) {
            if (first_ed_command_letter) {
                /* nothing but deletes!? */
                p_start = first_command_line;
                p_sline = fcl_line;
                retval = ED_DIFF;
                goto scan_exit;
            }
            else {
                p_start = this_line;
                p_sline = p_input_line;
                return NO_DIFF;
            }
        }
        strip_trailing_cr = 2 <= chars_read && buf[chars_read - 2] == '\r';
        for (s = buf; *s == ' ' || *s == '\t' || *s == 'X'; s++) {
            if (*s == '\t')
                indent = (indent + 8) & ~7;
            else
                indent++;
        }
        if (ISDIGIT (*s))
        {
            for (t = s + 1; ISDIGIT (*t) || *t == ',';  t++)
                continue;
            if (*t == 'd' || *t == 'c' || *t == 'a')
            {
                for (t++;  ISDIGIT (*t) || *t == ',';  t++)
                    continue;
                for (; *t == ' ' || *t == '\t'; t++)
                    continue;
                if (*t == '\r')
                    t++;
                this_is_a_command = (*t == '\n');
            }
        }
        if (! need_header
                && first_command_line < 0
                && ((ed_command_letter = get_ed_command_letter (s))
                    || this_is_a_command)) {
            first_command_line = this_line;
            first_ed_command_letter = ed_command_letter;
            fcl_line = p_input_line;
            p_indent = indent;		/* assume this for now */
            p_strip_trailing_cr = strip_trailing_cr;
        }
        if (!stars_last_line && strneq(s, "*** ", 4))
        {
            p_name[OLD] = fetchname (s+4, strippath, &p_timestr[OLD],
                    &p_timestamp[OLD]);
            need_header = false;
        }
        else if (strneq(s, "+++ ", 4))
        {
            /* Swap with NEW below.  */
            p_name[OLD] = fetchname (s+4, strippath, &p_timestr[OLD],
                    &p_timestamp[OLD]);
            need_header = false;
            p_strip_trailing_cr = strip_trailing_cr;
        }
        else if (strneq(s, "Index:", 6))
        {
            p_name[INDEX] = fetchname (s+6, strippath, (char **) 0,
                    (time_t *) 0);
            need_header = false;
            p_strip_trailing_cr = strip_trailing_cr;
        }
        else if (strneq(s, "Prereq:", 7)) {
            for (t = s + 7;  ISSPACE ((unsigned char) *t);  t++)
                continue;
            revision = t;
            for (t = revision;  *t;  t++)
                if (ISSPACE ((unsigned char) *t))
                {
                    char const *u;
                    for (u = t + 1;  ISSPACE ((unsigned char) *u);  u++)
                        continue;
                    if (*u)
                    {
                        char numbuf[LINENUM_LENGTH_BOUND + 1];
                        say ("Prereq: with multiple words at line %s of patch\n",
                                format_linenum (numbuf, this_line));
                    }
                    break;
                }
            if (t == revision)
                revision = 0;
            else {
                char oldc = *t;
                *t = '\0';
                revision = dupstr(revision);
                *t = oldc;
            }
        } 
        else
        {
            for (t = s;  t[0] == '-' && t[1] == ' ';  t += 2)
                continue;
            if (strneq(t, "--- ", 4))
            {
                time_t timestamp = (time_t) -1;
                p_name[NEW] = fetchname (t+4, strippath, &p_timestr[NEW],
                        &timestamp);
                
                need_header = false;
                if (timestamp != (time_t) -1)
                {
                    p_timestamp[NEW] = timestamp;
                    p_rfc934_nesting = (t - s) >> 1;
                }
                p_strip_trailing_cr = strip_trailing_cr;
            }
        }
            if (need_header)
                continue;
            if ((diff_type == NO_DIFF || diff_type == ED_DIFF) &&
                    first_command_line >= 0 &&
                    streq(s, ".\n") ) {
                p_start = first_command_line;
                p_sline = fcl_line;
                retval = ED_DIFF;
                goto scan_exit;
            }
            if ((diff_type == NO_DIFF || diff_type == UNI_DIFF)
                    && strneq(s, "@@ -", 4)) {

                /* `p_name', `p_timestr', and `p_timestamp' are backwards;
                   swap them.  */
                time_t ti = p_timestamp[OLD];
                p_timestamp[OLD] = p_timestamp[NEW];
                p_timestamp[NEW] = ti;
                t = p_name[OLD];
                p_name[OLD] = p_name[NEW];
                p_name[NEW] = t;
                t = p_timestr[OLD];
                p_timestr[OLD] = p_timestr[NEW];
                p_timestr[NEW] = t;

                s += 4;
                if (s[0] == '0' && !ISDIGIT (s[1]))
                    p_says_nonexistent[OLD] = 1 + ! p_timestamp[OLD];
                while (*s != ' ' && *s != '\n')
                    s++;
                while (*s == ' ')
                    s++;
                if (s[0] == '+' && s[1] == '0' && !ISDIGIT (s[2]))
                    p_says_nonexistent[NEW] = 1 + ! p_timestamp[NEW];
                p_indent = indent;
                p_start = this_line;
                p_sline = p_input_line;
                retval = UNI_DIFF;
                if (! ((p_name[OLD] || ! p_timestamp[OLD])
                            && (p_name[NEW] || ! p_timestamp[NEW]))
                        && ! p_name[INDEX] && need_header)
                {
                    char numbuf[LINENUM_LENGTH_BOUND + 1];
                    say ("missing header for unified diff at line %s of patch\n",
                            format_linenum (numbuf, p_sline));
                }
                goto scan_exit;
            }
            stars_this_line = strneq(s, "********", 8);
            if ((diff_type == NO_DIFF
                        || diff_type == CONTEXT_DIFF
                        || diff_type == NEW_CONTEXT_DIFF)
                    && stars_last_line && strneq (s, "*** ", 4)) {
                s += 4;
                if (s[0] == '0' && !ISDIGIT (s[1]))
                    p_says_nonexistent[OLD] = 1 + ! p_timestamp[OLD];
                /* if this is a new context diff the character just before */
                /* the newline is a '*'. */
                while (*s != '\n')
                    s++;
                p_indent = indent;
                p_strip_trailing_cr = strip_trailing_cr;
                p_start = previous_line;
                p_sline = p_input_line - 1;
                retval = (*(s-1) == '*' ? NEW_CONTEXT_DIFF : CONTEXT_DIFF);

                {
                    /* Scan the first hunk to see whether the file contents
                       appear to have been deleted.  */
                    file_offset saved_p_base = p_base;
                    LINENUM saved_p_bline = p_bline;
                    Fseek (pfp, previous_line, SEEK_SET);
                    p_input_line -= 2;
                    if (another_hunk (retval)
                            && ! p_repl_lines && p_newfirst == 1)
                        p_says_nonexistent[NEW] = 1 + ! p_timestamp[NEW];
                    next_intuit_at (saved_p_base, saved_p_bline);
                }

                if (! ((p_name[OLD] || ! p_timestamp[OLD])
                            && (p_name[NEW] || ! p_timestamp[NEW]))
                        && ! p_name[INDEX] && need_header)
                {
                    char numbuf[LINENUM_LENGTH_BOUND + 1];
                    say ("missing header for context diff at line %s of patch\n",
                            format_linenum (numbuf, p_sline));
                }
                goto scan_exit;
            }
            if ((diff_type == NO_DIFF || diff_type == NORMAL_DIFF) &&
                    last_line_was_command &&
                    (strneq(s, "< ", 2) || strneq(s, "> ", 2)) ) {
                p_start = previous_line;
                p_sline = p_input_line - 1;
                p_indent = indent;
                p_strip_trailing_cr = strip_trailing_cr;
                retval = NORMAL_DIFF;
                goto scan_exit;
            }
    }

scan_exit:

    /* To intuit `inname', the name of the file to patch,
       use the algorithm specified by POSIX 1003.1-2001 XCU lines 25680-26599
       (with some modifications if posixly_correct is zero):

       - Take the old and new names from the context header if present,
       and take the index name from the `Index:' line if present and
       if either the old and new names are both absent
       or posixly_correct is nonzero.
       Consider the file names to be in the order (old, new, index).
       - If some named files exist, use the first one if posixly_correct
       is nonzero, the best one otherwise.
       - If patch_get is nonzero, and no named files exist,
       but an RCS or SCCS master file exists,
       use the first named file with an RCS or SCCS master.
       - If no named files exist, no RCS or SCCS master was found,
       some names are given, posixly_correct is zero,
       and the patch appears to create a file, then use the best name
       requiring the creation of the fewest directories.
       - Otherwise, report failure by setting `inname' to 0;
       this causes our invoker to ask the user for a file name.  */
    

    i = NONE;
    
    if (!inname)
    {
        enum nametype i0 = NONE;

        if (! posixly_correct && (p_name[OLD] || p_name[NEW]) && p_name[INDEX])
        {
            free (p_name[INDEX]);
            p_name[INDEX] = 0;
        }

        for (i = OLD;  i <= INDEX;  i++)
            if (p_name[i])
            {
                if (i0 != NONE && strcmp (p_name[i0], p_name[i]) == 0)
                {
                    // It's the same name as before; reuse stat results.
                    stat_errno[i] = stat_errno[i0];
                    if (! stat_errno[i])
                        st[i] = st[i0];
                }
                else if (stat (p_name[i], &st[i]) != 0)
                    stat_errno[i] = errno;
                else
                {
                    stat_errno[i] = 0;
                    if (posixly_correct)
                        break;
                }
                i0 = (nametype) i;
            }

        /***
        if (! posixly_correct)
        {
            i = best_name (p_name, stat_errno);

            if (i == NONE && patch_get)
            {
                enum nametype nope = NONE;

                for (i = OLD;  i <= INDEX;  i++)
                    if (p_name[i])
                    {
                        char const *cs;
                        char *getbuf;
                        char *diffbuf;
                        bool readonly = (outfile
                                && strcmp (outfile, p_name[i]) != 0);

                        if (nope == NONE || strcmp (p_name[nope], p_name[i]) != 0)
                        {
                            cs = (version_controller
                                    (p_name[i], readonly, (struct stat *) 0,
                                     &getbuf, &diffbuf));
                            version_controlled[i] = !! cs;
                            if (cs)
                            {
                                WARN_UNIMPL;
                            }
                        }

                        nope = (nametype) i;
                    }
            }

            if (i != NONE && st[i].st_size > 0)
                i0 = (nametype) i;
            if (i0 != NONE
                    && ! maybe_reverse (p_name[i0], i == NONE,
                        i == NONE || st[i].st_size == 0))
                i = i0;

            if (i == NONE && p_says_nonexistent[reverse])
            {
                int newdirs[3];
                int newdirs_min = INT_MAX;
                int distance_from_minimum[3];

                for (i = OLD;  i <= INDEX;  i++)
                    if (p_name[i])
                    {
                        newdirs[i] = (prefix_components (p_name[i], false)
                                - prefix_components (p_name[i], true));
                        if (newdirs[i] < newdirs_min)
                            newdirs_min = newdirs[i];
                    }

                for (i = OLD;  i <= INDEX;  i++)
                    if (p_name[i])
                        distance_from_minimum[i] = newdirs[i] - newdirs_min;

                i = best_name (p_name, distance_from_minimum);
            }
        }
        ***/
    }


    /**
    if (i == NONE)
    {
        if (inname)
        {
            inerrno = stat (inname, &instat) == 0 ? 0 : errno;
            maybe_reverse (inname, inerrno, inerrno || instat.st_size == 0);
        }
        else
            inerrno = -1;
    }
    
    else
    {
        inname = dupstr(p_name[i]);
        inerrno = stat_errno[i];
        invc = version_controlled[i];
        instat = st[i];
    }
    **/
    for (i = OLD; i <= INDEX; i++) {
        if (p_name[i]) {
            inname = dupstr(p_name[i]);
            inerrno = stat_errno[i];
            instat = st[i];
            break;
        }
    }
    return retval;
}

/* 1 if there is more of the current diff listing to process;
   0 if not; -1 if ran out of memory. */
int PatchParser::another_hunk (enum difftype diff)
{
    char *s;
    LINENUM context = 0;
    size_t chars_read;
    char numbuf0[LINENUM_LENGTH_BOUND + 1];
    char numbuf1[LINENUM_LENGTH_BOUND + 1];

    while (p_end >= 0) {
        if (p_end == p_efake)
            p_end = p_bfake;		/* don't free twice */
        else
            free(p_line[p_end]);
        p_end--;
    }
    assert(p_end == -1);
    p_efake = -1;

    if (p_c_function)
    {
        free (p_c_function);
        p_c_function = NULL;
    }

    p_max = hunkmax;			/* gets reduced when --- found */
    if (diff == CONTEXT_DIFF || diff == NEW_CONTEXT_DIFF) {
        errgrace("CONTEXT type diff not supported\n");
    }
    else if (diff == UNI_DIFF) {
        file_offset line_beginning = ftell (pfp);
        /* file pos of the current line */
        LINENUM fillsrc;	/* index of old lines */
        LINENUM filldst;	/* index of new lines */

        LINENUM orig;

        char ch = '\0';

        chars_read = get_line ();
        if (chars_read == (size_t) -1
                || chars_read <= 4
                || strncmp (buf, "@@ -", 4) != 0) {
            next_intuit_at(line_beginning,p_input_line);
            return chars_read == (size_t) -1 ? -1 : 0;
        }
        s = scan_linenum (buf + 4, &p_first);
        if (*s == ',')
            s = scan_linenum (s + 1, &p_ptrn_lines);
        else
            p_ptrn_lines = 1;
        if (*s == ' ') s++;
        if (*s != '+')
            malformed ();
        s = scan_linenum (s + 1, &p_newfirst);
        if (*s == ',')
            s = scan_linenum (s + 1, &p_repl_lines);
        else
            p_repl_lines = 1;
        if (*s == ' ') s++;
        if (*s++ != '@')
            malformed ();
        if (*s++ == '@' && *s == ' ' && *s != '\0')
        {
            p_c_function = s;
            while (*s != '\n')
                s++;
            *s = '\0';
            p_c_function = dupstr (p_c_function);
        }
        if (!p_ptrn_lines)
            p_first++;			/* do append rather than insert */
        if (!p_repl_lines)
            p_newfirst++;
        p_max = p_ptrn_lines + p_repl_lines + 1;
        while (p_max + 1 >= hunkmax)
            if (! grow_hunkmax ())
                return -1;
        fillsrc = 1;
        orig = 0;
        filldst = fillsrc + p_ptrn_lines;
        p_end = filldst + p_repl_lines;
        sprintf (buf, "*** %s,%s ****\n",
                format_linenum (numbuf0, p_first),
                format_linenum (numbuf1, p_first + p_ptrn_lines - 1));
        p_len[0] = strlen (buf);
        if (! (p_line[0] = dupstr (buf))) {
            p_end = -1;
            return -1;
        }
        p_Char[0] = '*';
        sprintf (buf, "--- %s,%s ----\n",
                format_linenum (numbuf0, p_newfirst),
                format_linenum (numbuf1, p_newfirst + p_repl_lines - 1));
        p_len[filldst] = strlen (buf);
        if (! (p_line[filldst] = dupstr (buf))) {
            p_end = 0;
            return -1;
        }
        p_Char[filldst++] = '=';
        p_prefix_context = -1;
        p_hunk_beg = p_input_line + 1;
        while (fillsrc <= p_ptrn_lines || filldst <= p_end) {
            chars_read = get_line ();
            if (!chars_read) {
                if (p_max - filldst < 3) {
                    strcpy (buf, " \n");  /* assume blank lines got chopped */
                    chars_read = 2;
                } else {
//                    errgrace ("unexpected end of file in patch");
                    p_base = p_filesize; // hack to force parsing stop
                    errreturn(-1, "unexpected end of file in patch '%s' at hunk @@ %s, %s", patchname, format_linenum(numbuf0, p_first), format_linenum(numbuf1, p_newfirst));
                }
            }
            if (chars_read == (size_t) -1)
                s = 0;
            else if (*buf == '\t' || *buf == '\n') {
                ch = ' ';		/* assume the space got eaten */
                s = dupbuf (buf, chars_read);
            }
            else {
                ch = *buf;
                s = dupbuf (buf+1, --chars_read);
            }
            if (chars_read && ! s)
            {
                while (--filldst > p_ptrn_lines)
                    free(p_line[filldst]);
                p_end = fillsrc-1;
                return -1;
            }
            switch (ch) {
                case '-':
                    if (fillsrc > p_ptrn_lines) {
                        free(s);
                        p_end = filldst-1;
                        malformed ();
                    }
                    chars_read -= fillsrc == p_ptrn_lines && incomplete_line ();
                    p_CtrlChar[orig++] = ch;
                    p_Char[fillsrc] = ch;
                    p_line[fillsrc] = s;
                    p_len[fillsrc++] = chars_read;
                    break;
                case '=':
                    ch = ' ';
                    /* FALL THROUGH */
                case ' ':
                    if (fillsrc > p_ptrn_lines) {
                        free(s);
                        while (--filldst > p_ptrn_lines)
                            free(p_line[filldst]);
                        p_end = fillsrc-1;
                        malformed ();
                    }
                    context++;
                    chars_read -= fillsrc == p_ptrn_lines && incomplete_line ();
                    p_Char[fillsrc] = ch;
                    p_CtrlChar[orig++] = '~';
                    p_line[fillsrc] = s;
                    p_len[fillsrc++] = chars_read;
                    s = dupbuf (s, chars_read);
                    if (chars_read && ! s) {
                        while (--filldst > p_ptrn_lines)
                            free(p_line[filldst]);
                        p_end = fillsrc-1;
                        return -1;
                    }
                    /* FALL THROUGH */
                case '+':
                    if (filldst > p_end) {
                        free(s);
                        while (--filldst > p_ptrn_lines)
                            free(p_line[filldst]);
                        p_end = fillsrc-1;
                        malformed ();
                    }
                    chars_read -= filldst == p_end && incomplete_line ();
                    if (ch == '+') { // because of fall through
                        p_CtrlChar[orig++] = ch;
                    }
                    p_Char[filldst] = ch;
                    p_line[filldst] = s;
                    p_len[filldst++] = chars_read;
                    break;
                default:
                    p_end = filldst;
                    malformed ();
            }
            if (ch != ' ') {
                if (p_prefix_context == -1)
                    p_prefix_context = context;
                context = 0;
            }
        }/* while */
        if (p_prefix_context == -1)
            malformed ();
        p_suffix_context = context;
        p_nctrl = orig;
    }
    else { /* normal diff--fake it up */
        errgrace("normal type diff not supported\n");
    }
    
    assert (p_end + 1 < hunkmax);
    p_Char[p_end + 1] = '^';  /* add a stopper for apply_hunk */
    p_CtrlChar[p_nctrl] = '\0';
    if (debug & 2) {
        LINENUM i;
        for (i = 0; i <= p_end + 1; i++) {
            fprintf (stderr, "%s %c",
                    format_linenum (numbuf0, i),
                    p_Char[i]);
            if (p_Char[i] == '*')
                fprintf (stderr, " %s,%s\n",
                        format_linenum (numbuf0, p_first),
                        format_linenum (numbuf1, p_ptrn_lines));
            else if (p_Char[i] == '=')
                fprintf (stderr, " %s,%s\n",
                        format_linenum (numbuf0, p_newfirst),
                        format_linenum (numbuf1, p_repl_lines));
            else if (p_Char[i] != '^')
            {
                fputs(" |", stderr);
                pch_write_line (i, stderr);
            }
            else
                fputc('\n', stderr);
        }
        fflush (stderr);
    }
    return 1;
}

/* Reverse the old and new portions of the current hunk. */
bool PatchParser::pch_swap (void)
{
    //NOT IMPLEMENTED
    return true;
}

/* Enlarge the arrays containing the current hunk of patch. */

bool PatchParser::grow_hunkmax (void)
{
    hunkmax *= 2;
    assert (p_line && p_len && p_Char && p_CtrlChar);
    if ((p_line = (char **) realloc (p_line, hunkmax * sizeof (*p_line)))
            && (p_len = (size_t *) realloc (p_len, hunkmax * sizeof (*p_len)))
            && (p_Char = (char *) realloc (p_Char, hunkmax * sizeof (*p_Char))) && (p_CtrlChar = (char *) realloc (p_CtrlChar, hunkmax * sizeof (*p_CtrlChar))))
        return true;
    if (!using_plan_a)
        xalloc_die ();
    /* Don't free previous values of p_line etc.,
       since some broken implementations free them for us.
       Whatever is null will be allocated again from within plan_a (),
       of all places.  */
    return false;
}

bool PatchParser::incomplete_line (void)
{
    FILE *fp = pfp;
    int c;
    file_offset line_beginning = ftell (fp);

    if (getc (fp) == '\\')
    {
        while ((c = getc (fp)) != '\n'  &&  c != EOF)
            continue;
        return true;
    }
    else
    {
        /* We don't trust ungetc.  */
        Fseek (pfp, line_beginning, SEEK_SET);
        return false;
    }
}

/* Parse a line number from a string.
   Return the address of the first char after the number.  */
char * PatchParser::scan_linenum (char *s0, LINENUM *linenum)
{
    char *s;
    LINENUM n = 0;
    bool overflow = false;
    char numbuf[LINENUM_LENGTH_BOUND + 1];

    for (s = s0;  ISDIGIT (*s);  s++)
    {
        LINENUM new_n = 10 * n + (*s - '0');
        overflow |= new_n / 10 != n;
        n = new_n;
    }

    if (s == s0)
        errgrace ("missing line number at line %s: %s",
                format_linenum (numbuf, p_input_line), buf);

    if (overflow)
        errgrace ("line number %.*s is too large at line %s: %s",
                (int) (s - s0), s0, format_linenum (numbuf, p_input_line), buf);

    *linenum = n;
    return s;
}

/* Make this a function for better debugging.  */
void PatchParser::malformed (void)
{
    char numbuf[LINENUM_LENGTH_BOUND + 1];
    errgrace("malformed patch at line %s: %s", format_linenum (numbuf, p_input_line), buf);
    /* about as informative as "Syntax error" in C */
}

char * PatchParser::fetchname (char *at, int strip_leading, char **ptimestr, time_t *pstamp)
{
    char *name;
    char *timestr = NULL;
    char *t;
    int sleading = strip_leading;
    time_t stamp = (time_t) -1;

    while (ISSPACE ((unsigned char) *at))
        at++;

    name = at;
    /* Strip up to `strip_leading' leading slashes and null terminate.
       If `strip_leading' is negative, strip all leading slashes.  */
    for (t = at;  *t;  t++)
    {
        if (ISSLASH (*t))
        {
            while (ISSLASH (t[1]))
                t++;
            if (strip_leading < 0 || --sleading >= 0)
                name = t+1;
        }
        else if (ISSPACE ((unsigned char) *t))
        {
            /* Allow file names with internal spaces,
               but only if a tab separates the file name from the date.  */
            char const *u = t;
            while (*u != '\t' && ISSPACE ((unsigned char) u[1]))
                u++;
            if (*u != '\t' && strchr (u + 1, '\t'))
                continue;

            if (*u == '\n')
                stamp = (time_t) -1;
            else
            {
                if (ptimestr)
                {
                    char const *t = u + strlen (u);
                    if (t != u && *(t-1) == '\n')
                        t--;
                    if (t != u && *(t-1) == '\r')
                        t--;
                    timestr = dupbuf (u, t - u + 1);
                    timestr[t - u] = 0;
                }

                if (set_time | set_utc)
                    stamp = str2time (&u, initial_time,
                            set_utc ? 0L : TM_LOCAL_ZONE);
                else
                {
                    /* The head says the file is nonexistent if the
                       timestamp is the epoch; but the listed time is
                       local time, not UTC, and POSIX.1 allows local
                       time offset anywhere in the range -25:00 <
                       offset < +26:00.  Match any time in that range
                       by assuming local time is -25:00 and then
                       matching any ``local'' time T in the range 0 <
                       T < 25+26 hours.  */
                    stamp = str2time (&u, initial_time, -25L * 60 * 60);
                    if (0 < stamp && stamp < (25 + 26) * 60L * 60)
                        stamp = 0;
                }

                if (*u && ! ISSPACE ((unsigned char) *u))
                    stamp = (time_t) -1;
            }

            *t = '\0';
            break;
        }
    }

    if (!*name)
    {
        if (timestr)
            free (timestr);
        return 0;
    }

    /* If the name is "/dev/null", ignore the name and mark the file
       as being nonexistent.  The name "/dev/null" appears in patches
       regardless of how NULL_DEVICE is spelled.  */
    if (strcmp (at, "/dev/null") == 0)
    {
        if (pstamp)
            *pstamp = 0;
        if (timestr)
            free (timestr);
        return 0;
    }

    /* Ignore the name if it doesn't have enough slashes to strip off.  */
    if (0 < sleading)
    {
        if (timestr)
            free (timestr);
        return 0;
    }

    if (pstamp)
        *pstamp = stamp;
    if (timestr)
        *ptimestr = timestr;
    return dupstr (name);
}


/* Fetch a line from the input file.
   WHICHBUF is ignored when the file is in memory.  */

const char * PatchParser::ifetch (LINENUM line, bool whichbuf, size_t *psize)
{
    char const *p;

    if (line < 1 || line > input_lines) {
        *psize = 0;
        return "";
    }
    if (using_plan_a) {
        p = i_ptr[line];
        *psize = i_ptr[line + 1] - p;
        return p;
    } else {
        DIE_UNIMPL;
    }
}

void PatchParser::get_input_file (char const *filename, char const *outname)
{
    bool elsewhere = strcmp (filename, outname) != 0;
    char const *cs;
    char *diffbuf;
    char *getbuf;

    if (inerrno == -1)
        inerrno = stat (filename, &instat) == 0 ? 0 : errno;

    /* Perhaps look for RCS or SCCS versions.  */
    if (patch_get
            && invc != 0
            && (inerrno
                || (! elsewhere
                    && (/* No one can write to it.  */
                        (instat.st_mode & (S_IWUSR|S_IWGRP|S_IWOTH)) == 0
                        /* Only the owner (who's not me) can write to it.  */
                        || ((instat.st_mode & (S_IWGRP|S_IWOTH)) == 0
                            && instat.st_uid != geteuid ()))))
            && (invc = !! (cs = (version_controller
                        (filename, elsewhere,
                         inerrno ? (struct stat *) 0 : &instat,
                         &getbuf, &diffbuf))))) {
        WARN_UNIMPL;
    }

    if (inerrno)
    {
        instat.st_mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;
        instat.st_size = 0;
    }
    else if (! S_ISREG (instat.st_mode))
        errgrace ("File %s is not a regular file -- can't patch",
                quotearg (filename));
}


void PatchParser::re_input (void)
{
    if (using_plan_a) {
        if (i_buffer)
        {
            free (i_buffer);
            i_buffer = 0;
            free (i_ptr);
        }
    }
    else {
        close (tifd);
        tifd = -1;
        if (tibuf[0])
        {
            free (tibuf[0]);
            tibuf[0] = 0;
        }
        tiline[0] = tiline[1] = -1;
        tireclen = 0;
    }
}



/* Construct the line index, somehow or other. */

void PatchParser::scan_input (char *filename)
{
    using_plan_a = plan_a (filename);
    if (!using_plan_a)
        plan_b(filename);

    if (verbosity != SILENT)
    {
        filename = quotearg (filename);

        if (verbosity == VERBOSE)
            say ("Patching file %s using Plan %s...\n",
                    filename, using_plan_a ? "A" : "B");
        else
            say ("patching file %s\n", filename);
    }
}

/* Try keeping everything in memory. */

bool PatchParser::plan_a(char const *filename)
{
    char const *s;
    char const *lim;
    char const **ptr;
    char *buffer;
    LINENUM iline;
    size_t size = instat.st_size;

    /* Fail if the file size doesn't fit in a size_t,
       or if storage isn't available.  */
    if (! (size == instat.st_size
                && (buffer = (char *) malloc (size ? size : (size_t) 1))))
        return false;

    /* Read the input file, but don't bother reading it if it's empty.
       When creating files, the files do not actually exist.  */
    if (size)
    {
        int ifd = open (filename, O_RDONLY|binary_transput);
        size_t buffered = 0, n;
        if (ifd < 0)
            diegrace ("can't open file %s", quotearg (filename));

        while (size - buffered != 0)
        {
            n = read (ifd, buffer + buffered, size - buffered);
            if (n == 0)
            {
                /* Some non-POSIX hosts exaggerate st_size in text mode;
                   or the file may have shrunk!  */
                size = buffered;
                break;
            }
            if (n == (size_t) -1)
            {
                /* Perhaps size is too large for this host.  */
                close (ifd);
                free (buffer);
                return false;
            }
            buffered += n;
        }

        if (close (ifd) != 0)
            read_fatal ();
    }

    /* Scan the buffer and build array of pointers to lines.  */
    lim = buffer + size;
    iline = 3; /* 1 unused, 1 for SOF, 1 for EOF if last line is incomplete */
    for (s = buffer;  (s = (char *) memchr (s, '\n', lim - s));  s++)
        if (++iline < 0)
            too_many_lines (filename);
    if (! (iline == (size_t) iline
                && (size_t) iline * sizeof *ptr / sizeof *ptr == (size_t) iline
                && (ptr = (char const **) malloc ((size_t) iline * sizeof *ptr))))
    {
        free (buffer);
        return false;
    }
    iline = 0;
    for (s = buffer;  ;  s++)
    {
        ptr[++iline] = s;
        if (! (s = (char *) memchr (s, '\n', lim - s)))
            break;
    }
    if (size && lim[-1] != '\n')
        ptr[++iline] = lim;
    input_lines = iline - 1;

    if (revision)
    {
        WARN_UNIMPL;
    }

    /* Plan A will work.  */
    i_buffer = buffer;
    i_ptr = ptr;
    return true;
}

/* Keep (virtually) nothing in memory. */
void PatchParser::plan_b(char const *filename)
{
    errgrace("Ooops, plan B not implmeneted");
}

/* Return the index of the best of NAME[OLD], NAME[NEW], and NAME[INDEX].
   Ignore null names, and ignore NAME[i] if IGNORE[i] is nonzero.
   Return NONE if all names are ignored.  */
enum nametype PatchParser::best_name (char *const *name, int const *ignore)
{
    //enum nametype i;
    int i;
    int components[3];
    int components_min = INT_MAX;
    size_t basename_len[3];
    size_t basename_len_min = SIZE_MAX;
    size_t len[3];
    size_t len_min = SIZE_MAX;

    for (i = OLD;  i <= INDEX;  i++)
        if (name[i] && !ignore[i])
        {
            /* Take the names with the fewest prefix components.  */
            components[i] = prefix_components (name[i], false);
            if (components_min < components[i])
                continue;
            components_min = components[i];

            /* Of those, take the names with the shortest basename.  */
            basename_len[i] = base_len (name[i]);
            if (basename_len_min < basename_len[i])
                continue;
            basename_len_min = basename_len[i];

            /* Of those, take the shortest names.  */
            len[i] = strlen (name[i]);
            if (len_min < len[i])
                continue;
            len_min = len[i];
        }

    /* Of those, take the first name.  */
    for (i = OLD;  i <= INDEX;  i++)
        if (name[i] && !ignore[i]
                && components[i] == components_min
                && basename_len[i] == basename_len_min
                && len[i] == len_min)
            break;

    return (nametype) i;
}

/* Count the path name components in FILENAME's prefix.
   If CHECKDIRS is true, count only existing directories.  */
int PatchParser::prefix_components (char *filename, bool checkdirs)
{
    int count = 0;
    struct stat stat_buf;
    int stat_result;
    char *f = filename + FILE_SYSTEM_PREFIX_LEN (filename);

    if (*f)
        while (*++f)
            if (ISSLASH (f[0]) && ! ISSLASH (f[-1]))
            {
                if (checkdirs)
                {
                    *f = '\0';
                    stat_result = stat (filename, &stat_buf);
                    *f = '/';
                    if (! (stat_result == 0 && S_ISDIR (stat_buf.st_mode)))
                        break;
                }

                count++;
            }

    return count;
}

/* Is the newline-terminated line a valid `ed' command for patch
   input?  If so, return the command character; if not, return 0.
   This accepts just a subset of the valid commands, but it's
   good enough in practice.  */

char PatchParser::get_ed_command_letter (char const *line)
{
    char const *p = line;
    char letter;
    bool pair = false;

    if (ISDIGIT (*p))
    {
        while (ISDIGIT (*++p))
            continue;
        if (*p == ',')
        {
            if (! ISDIGIT (*++p))
                return 0;
            while (ISDIGIT (*++p))
                continue;
            pair = true;
        }
    }

    letter = *p++;

    switch (letter)
    {
        case 'a':
        case 'i':
            if (pair)
                return 0;
            break;

        case 'c':
        case 'd':
            break;

        case 's':
            if (strncmp (p, "/.//", 4) != 0)
                return 0;
            p += 4;
            break;

        default:
            return 0;
    }

    while (*p == ' ' || *p == '\t')
        p++;
    if (*p == '\n')
        return letter;
    return 0;
}


/* Create an output file.  */
static FILE * create_output_file (char const *name, int open_flags, mode_t mode)
{
    int fd = create_file (name, O_WRONLY | open_flags,
            mode, true);
    //FILE *f = fdopen (fd, binary_transput ? "wb" : "w");
    FILE *f = fdopen (fd, "w");
    if (! f)
        diegrace("Can't create file %s", quotearg (name));
    return f;
}
