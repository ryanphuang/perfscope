#define XTERN extern
#include<common.h>
#undef XTERN
#include<maketime.h>
#include<partime.h>
#include<dirname.h>
#include<handy.h>
#include<parser.h>

LINENUM PatchParser::hunkmax = INITHUNKMAX;
size_t PatchParser::bufsize = 8 * 1024;


PatchParser::PatchParser(const char * pname, enum difftype type = NO_DIFF ) :
    diff_type(type), skip_rest_of_patch(false), p_efake(-1), p_bfake(-1), 
    p_end(-1), strippath(-1), tifd(-1) 
{
    patchname = dupstr(pname);
    buf = (char *) xmalloc(bufsize);
    tiline[0] = -1;
    tiline[1] = -1;
    tmpoutname = make_temp ('o');
    tmpinname = make_temp ('i');
    tmppatname = make_temp ('p');
}

PatchParser::~PatchParser()
{
    cleanup();
}

void PatchParser::re_patch()
{
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
    unlink(tmpinname);
    unlink(tmpoutname);
    unlink(tmppatname);
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

/* Open the new file. */

void PatchParser::init_output(char const *name, int open_flags, struct outstate *outstate)
{
    if (! name)
        outstate->ofp = (FILE *) 0;
    else if (strcmp (name, "-") != 0)
        outstate->ofp = create_output_file (name, open_flags, instat.st_mode);
    else
    {
        int stdout_dup = dup (fileno (stdout));
        outstate->ofp = fdopen (stdout_dup, "a");
        if (stdout_dup == -1 || ! outstate->ofp)
            diegrace("Failed to duplicate standard output");
        if (dup2 (fileno (stderr), fileno (stdout)) == -1)
            diegrace("Failed to redirect messages to standard error");
    }

    outstate->after_newline = true;
    outstate->zero_output = true;
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
  bool r = false;

  if (noreverse || ! (force && verbosity == SILENT))
    {
      va_list args;
      va_start (args, format);
      vsay (format, args);
      va_end (args);
    }

  if (noreverse)
    {
      say ("  Skipping patch.\n");
      skip_rest_of_patch = true;
    }
  else if (force)
    {
      if (verbosity != SILENT)
	say ("  Applying it anyway.\n");
    }
  else if (batch)
    {
      say (reverse ? "  Ignoring -R.\n" : "  Assuming -R.\n");
      r = true;
    }
  else
    {
      ask (reverse ? "  Ignore -R? [n] " : "  Assume -R? [n] ");
      r = *buf == 'y';
      if (! r)
	{
	  ask ("Apply anyway? [n] ");
	  if (*buf != 'y')
	    {
	      if (verbosity != SILENT)
		say ("Skipping patch.\n");
	      skip_rest_of_patch = true;
	    }
	}
    }

  return r;
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
}


void PatchParser::open_patch_file(const char * filename)
{
    file_offset file_pos = 0;
    struct stat st;
    if(isempty(filename) || streq(filename, "-")) {
        pfp = stdin;
	    if (S_ISREG (st.st_mode) && (file_pos = ftell(stdin)) != -1)
	    {
	        pfp = stdin;
	    }
        else {
            diegrace("Unsupported special input type");
        }
    }
    else {
        pfp = fopen(filename, "r");
        if(NULL == pfp ) {
            diegrace("Unable to open %s", filename);
        }
        if (fstat(fileno(pfp), &st) != 0) {
            diegrace("Cannot fstat %s", filename);
        }
    }
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

/* True if the remainder of the patch file contains a diff of some sort. */

bool PatchParser::there_is_another_patch (bool need_header)
{
    if (p_base != 0 && p_base >= p_filesize) {
        say ("done\n");
        return false;
    }
    if (verbosity == VERBOSE)
        say ("Hmm...");
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
    register FILE *i = pfp;
    register FILE *o = stdout;
    register int c;

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
    register FILE *fp = pfp;
    register int c;
    register size_t i;
    register char *b;
    register size_t s;

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
    register file_offset this_line = 0;
    register file_offset first_command_line = -1;
    char first_ed_command_letter = 0;
    LINENUM fcl_line = 0; /* Pacify `gcc -W'.  */
    register bool this_is_a_command = false;
    register bool stars_this_line = false;
//    enum nametype i;
    int i;
    struct stat st[3];
    int stat_errno[3];
    int version_controlled[3];
    register enum difftype retval;

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
        register char *s;
        register char *t;
        register file_offset previous_line = this_line;
        register bool last_line_was_command = this_is_a_command;
        register bool stars_last_line = stars_this_line;
        register size_t indent = 0;
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
        } else
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
                    if (another_hunk (retval, false)
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
                    /* It's the same name as before; reuse stat results.  */
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
                                if (version_get (p_name[i], cs, false, readonly,
                                            getbuf, &st[i]))
                                    stat_errno[i] = 0;
                                else
                                    version_controlled[i] = 0;

                                free (getbuf);
                                if (diffbuf)
                                    free (diffbuf);

                                if (! stat_errno[i])
                                    break;
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
    }

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

    return retval;
}

bool PatchParser::pch_write_line (LINENUM line, FILE *file)
{
  bool after_newline = p_line[line][p_len[line] - 1] == '\n';
  if (! fwrite (p_line[line], sizeof (*p_line[line]), p_len[line], file))
    write_fatal ();
  return after_newline;
}

/* 1 if there is more of the current diff listing to process;
   0 if not; -1 if ran out of memory. */
int PatchParser::another_hunk (enum difftype diff, bool rev)
{
    register char *s;
    register LINENUM context = 0;
    register size_t chars_read;
    char numbuf0[LINENUM_LENGTH_BOUND + 1];
    char numbuf1[LINENUM_LENGTH_BOUND + 1];
    char numbuf2[LINENUM_LENGTH_BOUND + 1];
    char numbuf3[LINENUM_LENGTH_BOUND + 1];

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
        file_offset line_beginning = ftell (pfp);
        /* file pos of the current line */
        LINENUM repl_beginning = 0;	/* index of --- line */
        register LINENUM fillcnt = 0;	/* #lines of missing ptrn or repl */
        register LINENUM fillsrc;	/* index of first line to copy */
        register LINENUM filldst;	/* index of first missing line */
        bool ptrn_spaces_eaten = false;	/* ptrn was slightly misformed */
        bool some_context = false;	/* (perhaps internal) context seen */
        register bool repl_could_be_missing = true;
        bool ptrn_missing = false;	/* The pattern was missing.  */
        bool repl_missing = false;	/* Likewise for replacement.  */
        file_offset repl_backtrack_position = 0;
        /* file pos of first repl line */
        LINENUM repl_patch_line;	/* input line number for same */
        LINENUM repl_context;		/* context for same */
        LINENUM ptrn_prefix_context = -1; /* lines in pattern prefix context */
        LINENUM ptrn_suffix_context = -1; /* lines in pattern suffix context */
        LINENUM repl_prefix_context = -1; /* lines in replac. prefix context */
        LINENUM ptrn_copiable = 0;	/* # of copiable lines in ptrn */
        LINENUM repl_copiable = 0;	/* Likewise for replacement.  */

        /* Pacify `gcc -Wall'.  */
        fillsrc = filldst = repl_patch_line = repl_context = 0;

        chars_read = get_line ();
        if (chars_read == (size_t) -1
                || chars_read <= 8
                || strncmp (buf, "********", 8) != 0) {
            next_intuit_at(line_beginning,p_input_line);
            return chars_read == (size_t) -1 ? -1 : 0;
        }
        s = buf;
        while (*s == '*')
            s++;
        if (*s == ' ')
        {
            p_c_function = s;
            while (*s != '\n')
                s++;
            *s = '\0';
            p_c_function = dupstr (p_c_function);
        }
        p_hunk_beg = p_input_line + 1;
        while (p_end < p_max) {
            chars_read = get_line ();
            if (chars_read == (size_t) -1)
                return -1;
            if (!chars_read) {
                if (repl_beginning && repl_could_be_missing) {
                    repl_missing = true;
                    goto hunk_done;
                }
                if (p_max - p_end < 4) {
                    strcpy (buf, "  \n");  /* assume blank lines got chopped */
                    chars_read = 3;
                } else {
                    diegrace ("unexpected end of file in patch");
                }
            }
            p_end++;
            if (p_end == hunkmax)
                diegrace ("unterminated hunk starting at line %s; giving up at line %s: %s",
                        format_linenum (numbuf0, p_hunk_beg),
                        format_linenum (numbuf1, p_input_line), buf);
            assert(p_end < hunkmax);
            p_Char[p_end] = *buf;
            p_len[p_end] = 0;
            p_line[p_end] = 0;
            switch (*buf) {
                case '*':
                    if (strneq(buf, "********", 8)) {
                        if (repl_beginning && repl_could_be_missing) {
                            repl_missing = true;
                            goto hunk_done;
                        }
                        else
                            diegrace ("unexpected end of hunk at line %s",
                                    format_linenum (numbuf0, p_input_line));
                    }
                    if (p_end != 0) {
                        if (repl_beginning && repl_could_be_missing) {
                            repl_missing = true;
                            goto hunk_done;
                        }
                        diegrace ("unexpected `***' at line %s: %s",
                                format_linenum (numbuf0, p_input_line), buf);
                    }
                    context = 0;
                    p_len[p_end] = strlen (buf);
                    if (! (p_line[p_end] = dupstr (buf))) {
                        p_end--;
                        return -1;
                    }
                    for (s = buf;  *s && !ISDIGIT (*s);  s++)
                        continue;
                    if (strneq(s,"0,0",3))
                        remove_prefix (s, 2);
                    s = scan_linenum (s, &p_first);
                    if (*s == ',') {
                        while (*s && !ISDIGIT (*s))
                            s++;
                        scan_linenum (s, &p_ptrn_lines);
                        p_ptrn_lines += 1 - p_first;
                    }
                    else if (p_first)
                        p_ptrn_lines = 1;
                    else {
                        p_ptrn_lines = 0;
                        p_first = 1;
                    }
                    p_max = p_ptrn_lines + 6;	/* we need this much at least */
                    while (p_max + 1 >= hunkmax)
                        if (! grow_hunkmax ())
                            return -1;
                    p_max = hunkmax;
                    break;
                case '-':
                    if (buf[1] != '-')
                        goto change_line;
                    if (ptrn_prefix_context == -1)
                        ptrn_prefix_context = context;
                    ptrn_suffix_context = context;
                    if (repl_beginning
                            || (p_end
                                != p_ptrn_lines + 1 + (p_Char[p_end - 1] == '\n')))
                    {
                        if (p_end == 1)
                        {
                            /* `Old' lines were omitted.  Set up to fill
                               them in from `new' context lines.  */
                            ptrn_missing = true;
                            p_end = p_ptrn_lines + 1;
                            ptrn_prefix_context = ptrn_suffix_context = -1;
                            fillsrc = p_end + 1;
                            filldst = 1;
                            fillcnt = p_ptrn_lines;
                        }
                        else if (! repl_beginning)
                            diegrace ("%s `---' at line %s; check line numbers at line %s",
                                    (p_end <= p_ptrn_lines
                                     ? "Premature"
                                     : "Overdue"),
                                    format_linenum (numbuf0, p_input_line),
                                    format_linenum (numbuf1, p_hunk_beg));
                        else if (! repl_could_be_missing)
                            diegrace ("duplicate `---' at line %s; check line numbers at line %s",
                                    format_linenum (numbuf0, p_input_line),
                                    format_linenum (numbuf1,
                                        p_hunk_beg + repl_beginning));
                        else
                        {
                            repl_missing = true;
                            goto hunk_done;
                        }
                    }
                    repl_beginning = p_end;
                    repl_backtrack_position = ftell (pfp);
                    repl_patch_line = p_input_line;
                    repl_context = context;
                    p_len[p_end] = strlen (buf);
                    if (! (p_line[p_end] = dupstr (buf)))
                    {
                        p_end--;
                        return -1;
                    }
                    p_Char[p_end] = '=';
                    for (s = buf;  *s && ! ISDIGIT (*s);  s++)
                        continue;
                    s = scan_linenum (s, &p_newfirst);
                    if (*s == ',')
                    {
                        do
                        {
                            if (!*++s)
                                malformed ();
                        }
                        while (! ISDIGIT (*s));
                        scan_linenum (s, &p_repl_lines);
                        p_repl_lines += 1 - p_newfirst;
                    }
                    else if (p_newfirst)
                        p_repl_lines = 1;
                    else
                    {
                        p_repl_lines = 0;
                        p_newfirst = 1;
                    }
                    p_max = p_repl_lines + p_end;
                    while (p_max + 1 >= hunkmax)
                        if (! grow_hunkmax ())
                            return -1;
                    if (p_repl_lines != ptrn_copiable
                            && (p_prefix_context != 0
                                || context != 0
                                || p_repl_lines != 1))
                        repl_could_be_missing = false;
                    context = 0;
                    break;
                case '+':  case '!':
                    repl_could_be_missing = false;
change_line:
                    s = buf + 1;
                    chars_read--;
                    if (*s == '\n' && canonicalize) {
                        strcpy (s, " \n");
                        chars_read = 2;
                    }
                    if (*s == ' ' || *s == '\t') {
                        s++;
                        chars_read--;
                    } else if (repl_beginning && repl_could_be_missing) {
                        repl_missing = true;
                        goto hunk_done;
                    }
                    if (! repl_beginning)
                    {
                        if (ptrn_prefix_context == -1)
                            ptrn_prefix_context = context;
                    }
                    else
                    {
                        if (repl_prefix_context == -1)
                            repl_prefix_context = context;
                    }
                    chars_read -=
                        (1 < chars_read
                         && p_end == (repl_beginning ? p_max : p_ptrn_lines)
                         && incomplete_line ());
                    p_len[p_end] = chars_read;
                    p_line[p_end] = dupbuf (s, chars_read);
                    if (chars_read && ! p_line[p_end]) {
                        p_end--;
                        return -1;
                    }
                    context = 0;
                    break;
                case '\t': case '\n':	/* assume spaces got eaten */
                    s = buf;
                    if (*buf == '\t') {
                        s++;
                        chars_read--;
                    }
                    if (repl_beginning && repl_could_be_missing &&
                            (!ptrn_spaces_eaten || diff == NEW_CONTEXT_DIFF) ) {
                        repl_missing = true;
                        goto hunk_done;
                    }
                    chars_read -=
                        (1 < chars_read
                         && p_end == (repl_beginning ? p_max : p_ptrn_lines)
                         && incomplete_line ());
                    p_len[p_end] = chars_read;
                    p_line[p_end] = dupbuf (buf, chars_read);
                    if (chars_read && ! p_line[p_end]) {
                        p_end--;
                        return -1;
                    }
                    if (p_end != p_ptrn_lines + 1) {
                        ptrn_spaces_eaten |= (repl_beginning != 0);
                        some_context = true;
                        context++;
                        if (repl_beginning)
                            repl_copiable++;
                        else
                            ptrn_copiable++;
                        p_Char[p_end] = ' ';
                    }
                    break;
                case ' ':
                    s = buf + 1;
                    chars_read--;
                    if (*s == '\n' && canonicalize) {
                        strcpy (s, "\n");
                        chars_read = 2;
                    }
                    if (*s == ' ' || *s == '\t') {
                        s++;
                        chars_read--;
                    } else if (repl_beginning && repl_could_be_missing) {
                        repl_missing = true;
                        goto hunk_done;
                    }
                    some_context = true;
                    context++;
                    if (repl_beginning)
                        repl_copiable++;
                    else
                        ptrn_copiable++;
                    chars_read -=
                        (1 < chars_read
                         && p_end == (repl_beginning ? p_max : p_ptrn_lines)
                         && incomplete_line ());
                    p_len[p_end] = chars_read;
                    p_line[p_end] = dupbuf (s, chars_read);
                    if (chars_read && ! p_line[p_end]) {
                        p_end--;
                        return -1;
                    }
                    break;
                default:
                    if (repl_beginning && repl_could_be_missing) {
                        repl_missing = true;
                        goto hunk_done;
                    }
                    malformed ();
            }
        }

hunk_done:
        if (p_end >=0 && !repl_beginning)
            diegrace ("no `---' found in patch at line %s",
                    format_linenum (numbuf0, p_hunk_beg));

        if (repl_missing) {

            /* reset state back to just after --- */
            p_input_line = repl_patch_line;
            context = repl_context;
            for (p_end--; p_end > repl_beginning; p_end--)
                free(p_line[p_end]);
            Fseek (pfp, repl_backtrack_position, SEEK_SET);

            /* redundant 'new' context lines were omitted - set */
            /* up to fill them in from the old file context */
            fillsrc = 1;
            filldst = repl_beginning+1;
            fillcnt = p_repl_lines;
            p_end = p_max;
        }
        else if (! ptrn_missing && ptrn_copiable != repl_copiable)
            diegrace ("context mangled in hunk at line %s",
                    format_linenum (numbuf0, p_hunk_beg));
        else if (!some_context && fillcnt == 1) {
            /* the first hunk was a null hunk with no context */
            /* and we were expecting one line -- fix it up. */
            while (filldst < p_end) {
                p_line[filldst] = p_line[filldst+1];
                p_Char[filldst] = p_Char[filldst+1];
                p_len[filldst] = p_len[filldst+1];
                filldst++;
            }
#if 0
            repl_beginning--;		/* this doesn't need to be fixed */
#endif
            p_end--;
            p_first++;			/* do append rather than insert */
            fillcnt = 0;
            p_ptrn_lines = 0;
        }

        p_prefix_context = ((repl_prefix_context == -1
                    || (ptrn_prefix_context != -1
                        && ptrn_prefix_context < repl_prefix_context))
                ? ptrn_prefix_context : repl_prefix_context);
        p_suffix_context = ((ptrn_suffix_context != -1
                    && ptrn_suffix_context < context)
                ? ptrn_suffix_context : context);
        assert (p_prefix_context != -1 && p_suffix_context != -1);

        if (diff == CONTEXT_DIFF
                && (fillcnt
                    || (p_first > 1
                        && p_prefix_context + p_suffix_context < ptrn_copiable))) {
            if (verbosity == VERBOSE)
                say ("%s\n%s\n%s\n",
                        "(Fascinating -- this is really a new-style context diff but without",
                        "the telltale extra asterisks on the *** line that usually indicate",
                        "the new style...)");
            diff_type = diff = NEW_CONTEXT_DIFF;
        }

        /* if there were omitted context lines, fill them in now */
        if (fillcnt) {
            p_bfake = filldst;		/* remember where not to free() */
            p_efake = filldst + fillcnt - 1;
            while (fillcnt-- > 0) {
                while (fillsrc <= p_end && fillsrc != repl_beginning
                        && p_Char[fillsrc] != ' ')
                    fillsrc++;
                if (p_end < fillsrc || fillsrc == repl_beginning)
                {
                    diegrace ("replacement text or line numbers mangled in hunk at line %s",
                            format_linenum (numbuf0, p_hunk_beg));
                }
                p_line[filldst] = p_line[fillsrc];
                p_Char[filldst] = p_Char[fillsrc];
                p_len[filldst] = p_len[fillsrc];
                fillsrc++; filldst++;
            }
            while (fillsrc <= p_end && fillsrc != repl_beginning)
            {
                if (p_Char[fillsrc] == ' ')
                    diegrace ("replacement text or line numbers mangled in hunk at line %s",
                            format_linenum (numbuf0, p_hunk_beg));
                fillsrc++;
            }
            if (debug)
                printf ("fillsrc %s, filldst %s, rb %s, e+1 %s\n",
                        format_linenum (numbuf0, fillsrc),
                        format_linenum (numbuf1, filldst),
                        format_linenum (numbuf2, repl_beginning),
                        format_linenum (numbuf3, p_end + 1));
            assert(fillsrc==p_end+1 || fillsrc==repl_beginning);
            assert(filldst==p_end+1 || filldst==repl_beginning);
        }
    }
    else if (diff == UNI_DIFF) {
        file_offset line_beginning = ftell (pfp);
        /* file pos of the current line */
        register LINENUM fillsrc;	/* index of old lines */
        register LINENUM filldst;	/* index of new lines */
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
                    diegrace ("unexpected end of file in patch");
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
    }
    else {				/* normal diff--fake it up */
        char hunk_type;
        register int i;
        LINENUM min, max;
        file_offset line_beginning = ftell (pfp);

        p_prefix_context = p_suffix_context = 0;
        chars_read = get_line ();
        if (chars_read == (size_t) -1 || !chars_read || !ISDIGIT (*buf)) {
            next_intuit_at(line_beginning,p_input_line);
            return chars_read == (size_t) -1 ? -1 : 0;
        }
        s = scan_linenum (buf, &p_first);
        if (*s == ',') {
            s = scan_linenum (s + 1, &p_ptrn_lines);
            p_ptrn_lines += 1 - p_first;
        }
        else
            p_ptrn_lines = (*s != 'a');
        hunk_type = *s;
        if (hunk_type == 'a')
            p_first++;			/* do append rather than insert */
        s = scan_linenum (s + 1, &min);
        if (*s == ',')
            scan_linenum (s + 1, &max);
        else
            max = min;
        if (hunk_type == 'd')
            min++;
        p_end = p_ptrn_lines + 1 + max - min + 1;
        while (p_end + 1 >= hunkmax)
            if (! grow_hunkmax ())
            {
                p_end = -1;
                return -1;
            }
        p_newfirst = min;
        p_repl_lines = max - min + 1;
        sprintf (buf, "*** %s,%s\n",
                format_linenum (numbuf0, p_first),
                format_linenum (numbuf1, p_first + p_ptrn_lines - 1));
        p_len[0] = strlen (buf);
        if (! (p_line[0] = dupstr (buf))) {
            p_end = -1;
            return -1;
        }
        p_Char[0] = '*';
        for (i=1; i<=p_ptrn_lines; i++) {
            chars_read = get_line ();
            if (chars_read == (size_t) -1)
            {
                p_end = i - 1;
                return -1;
            }
            if (!chars_read)
                diegrace ("unexpected end of file in patch at line %s",
                        format_linenum (numbuf0, p_input_line));
            if (buf[0] != '<' || (buf[1] != ' ' && buf[1] != '\t'))
                diegrace ("`<' expected at line %s of patch",
                        format_linenum (numbuf0, p_input_line));
            chars_read -= 2 + (i == p_ptrn_lines && incomplete_line ());
            p_len[i] = chars_read;
            p_line[i] = dupbuf (buf + 2, chars_read);
            if (chars_read && ! p_line[i]) {
                p_end = i-1;
                return -1;
            }
            p_Char[i] = '-';
        }
        if (hunk_type == 'c') {
            chars_read = get_line ();
            if (chars_read == (size_t) -1)
            {
                p_end = i - 1;
                return -1;
            }
            if (! chars_read)
                diegrace ("unexpected end of file in patch at line %s",
                        format_linenum (numbuf0, p_input_line));
            if (*buf != '-')
                diegrace ("`---' expected at line %s of patch",
                        format_linenum (numbuf0, p_input_line));
        }
        sprintf (buf, "--- %s,%s\n",
                format_linenum (numbuf0, min),
                format_linenum (numbuf1, max));
        p_len[i] = strlen (buf);
        if (! (p_line[i] = dupstr (buf))) {
            p_end = i-1;
            return -1;
        }
        p_Char[i] = '=';
        for (i++; i<=p_end; i++) {
            chars_read = get_line ();
            if (chars_read == (size_t) -1)
            {
                p_end = i - 1;
                return -1;
            }
            if (!chars_read)
                diegrace ("unexpected end of file in patch at line %s",
                        format_linenum (numbuf0, p_input_line));
            if (buf[0] != '>' || (buf[1] != ' ' && buf[1] != '\t'))
                diegrace ("`>' expected at line %s of patch",
                        format_linenum (numbuf0, p_input_line));
            chars_read -= 2 + (i == p_end && incomplete_line ());
            p_len[i] = chars_read;
            p_line[i] = dupbuf (buf + 2, chars_read);
            if (chars_read && ! p_line[i]) {
                p_end = i-1;
                return -1;
            }
            p_Char[i] = '+';
        }
    }
    if (rev)				/* backwards patch? */
        if (!pch_swap())
            say ("Not enough memory to swap next hunk!\n");
    assert (p_end + 1 < hunkmax);
    p_Char[p_end + 1] = '^';  /* add a stopper for apply_hunk */
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
    char **tp_line;		/* the text of the hunk */
    size_t *tp_len;		/* length of each line */
    char *tp_char;		/* +, -, and ! */
    register LINENUM i;
    register LINENUM n;
    bool blankline = false;
    register char *s;

    i = p_first;
    p_first = p_newfirst;
    p_newfirst = i;

    /* make a scratch copy */

    tp_line = p_line;
    tp_len = p_len;
    tp_char = p_Char;
    p_line = 0;	/* force set_hunkmax to allocate again */
    p_len = 0;
    p_Char = 0;
    set_hunkmax();
    if (!p_line || !p_len || !p_Char) {
        if (p_line)
            free (p_line);
        p_line = tp_line;
        if (p_len)
            free (p_len);
        p_len = tp_len;
        if (p_Char)
            free (p_Char);
        p_Char = tp_char;
        return false;		/* not enough memory to swap hunk! */
    }

    /* now turn the new into the old */

    i = p_ptrn_lines + 1;
    if (tp_char[i] == '\n') {		/* account for possible blank line */
        blankline = true;
        i++;
    }
    if (p_efake >= 0) {			/* fix non-freeable ptr range */
        if (p_efake <= i)
            n = p_end - i + 1;
        else
            n = -i;
        p_efake += n;
        p_bfake += n;
    }
    for (n=0; i <= p_end; i++,n++) {
        p_line[n] = tp_line[i];
        p_Char[n] = tp_char[i];
        if (p_Char[n] == '+')
            p_Char[n] = '-';
        p_len[n] = tp_len[i];
    }
    if (blankline) {
        i = p_ptrn_lines + 1;
        p_line[n] = tp_line[i];
        p_Char[n] = tp_char[i];
        p_len[n] = tp_len[i];
        n++;
    }
    assert(p_Char[0] == '=');
    p_Char[0] = '*';
    for (s=p_line[0]; *s; s++)
        if (*s == '-')
            *s = '*';

    /* now turn the old into the new */

    assert(tp_char[0] == '*');
    tp_char[0] = '=';
    for (s=tp_line[0]; *s; s++)
        if (*s == '*')
            *s = '-';
    for (i=0; n <= p_end; i++,n++) {
        p_line[n] = tp_line[i];
        p_Char[n] = tp_char[i];
        if (p_Char[n] == '-')
            p_Char[n] = '+';
        p_len[n] = tp_len[i];
    }
    assert(i == p_ptrn_lines + 1);
    i = p_ptrn_lines;
    p_ptrn_lines = p_repl_lines;
    p_repl_lines = i;
    p_Char[p_end + 1] = '^';
    if (tp_line)
        free (tp_line);
    if (tp_len)
        free (tp_len);
    if (tp_char)
        free (tp_char);
    return true;
}
/* Enlarge the arrays containing the current hunk of patch. */

bool PatchParser::grow_hunkmax (void)
{
    hunkmax *= 2;
    assert (p_line && p_len && p_Char);
    if ((p_line = (char **) realloc (p_line, hunkmax * sizeof (*p_line)))
	&& (p_len = (size_t *) realloc (p_len, hunkmax * sizeof (*p_len)))
	&& (p_Char = (char *) realloc (p_Char, hunkmax * sizeof (*p_Char))))
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
  register FILE *fp = pfp;
  register int c;
  register file_offset line_beginning = ftell (fp);

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
        diegrace ("missing line number at line %s: %s",
                format_linenum (numbuf, p_input_line), buf);

    if (overflow)
        diegrace ("line number %.*s is too large at line %s: %s",
                (int) (s - s0), s0, format_linenum (numbuf, p_input_line), buf);

    *linenum = n;
    return s;
}

/* Make this a function for better debugging.  */
void PatchParser::malformed (void)
{
    char numbuf[LINENUM_LENGTH_BOUND + 1];
    diegrace("malformed patch at line %s: %s", format_linenum (numbuf, p_input_line), buf);
		/* about as informative as "Syntax error" in C */
}

char * PatchParser::fetchname (char *at, int strip_leading, char **ptimestr, time_t *pstamp)
{
    char *name;
    char *timestr = NULL;
    register char *t;
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
    register char const *q;
    register char const *p;

    if (line < 1 || line > input_lines) {
	*psize = 0;
	return "";
    }
    if (using_plan_a) {
	p = i_ptr[line];
	*psize = i_ptr[line + 1] - p;
	return p;
    } else {
	LINENUM offline = line % lines_per_buf;
	LINENUM baseline = line - offline;

	if (tiline[0] == baseline)
	    whichbuf = false;
	else if (tiline[1] == baseline)
	    whichbuf = true;
	else {
	    tiline[whichbuf] = baseline;
	    if ((lseek (tifd, baseline/lines_per_buf * tibufsize, SEEK_SET)
		 == -1)
		|| read (tifd, tibuf[whichbuf], tibufsize) < 0)
	      read_fatal ();
	}
	p = tibuf[whichbuf] + (tireclen*offline);
	if (line == input_lines)
	    *psize = last_line_size;
	else {
	    for (q = p;  *q++ != '\n';  )
		continue;
	    *psize = q - p;
	}
	return p;
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

	    if (!inerrno) {
		if (!elsewhere
		    && (instat.st_mode & (S_IWUSR|S_IWGRP|S_IWOTH)) != 0)
		    /* Somebody can write to it.  */
		  diegrace ("File %s seems to be locked by somebody else under %s",
			 quotearg (filename), cs);
		if (diffbuf)
		  {
		    /* It might be checked out unlocked.  See if it's safe to
		       check out the default version locked.  */

		    if (verbosity == VERBOSE)
		      say ("Comparing file %s to default %s version...\n",
			   quotearg (filename), cs);

		    if (systemic (diffbuf) != 0)
		      {
			say ("warning: Patching file %s, which does not match default %s version\n",
			     quotearg (filename), cs);
			cs = 0;
		      }
		  }
		if (dry_run)
		  cs = 0;
	    }

	    if (cs && version_get (filename, cs, ! inerrno, elsewhere, getbuf,
				   &instat))
	      inerrno = 0;

	    free (getbuf);
	    if (diffbuf)
	      free (diffbuf);
      }

    if (inerrno)
      {
	instat.st_mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;
	instat.st_size = 0;
      }
    else if (! S_ISREG (instat.st_mode))
      diegrace ("File %s is not a regular file -- can't patch",
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

/* Report whether a desired revision was found.  */

void PatchParser::report_revision (bool found_revision)
{
    char const *rev = quotearg (revision);

    if (found_revision)
    {
        if (verbosity == VERBOSE)
            say ("Good.  This file appears to be the %s version.\n", rev);
    }
    else if (force)
    {
        if (verbosity != SILENT)
            say ("Warning: this file doesn't appear to be the %s version -- patching anyway.\n",
                    rev);
    }
    else if (batch)
        diegrace ("This file doesn't appear to be the %s version -- aborting.",
                rev);
    else
    {
        ask ("This file doesn't appear to be the %s version -- patch anyway? [n] ",
                rev);
        if (*buf != 'y')
            diegrace ("aborted");
    }
}

/* Try keeping everything in memory. */

bool PatchParser::plan_a(char const *filename)
{
    register char const *s;
    register char const *lim;
    register char const **ptr;
    register char *buffer;
    register LINENUM iline;
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
        char const *rev = revision;
        int rev0 = rev[0];
        bool found_revision = false;
        size_t revlen = strlen (rev);

        if (revlen <= size)
        {
            char const *limrev = lim - revlen;

            for (s = buffer;  (s = (char *) memchr (s, rev0, limrev - s));  s++)
                if (memcmp (s, rev, revlen) == 0
                        && (s == buffer || ISSPACE ((unsigned char) s[-1]))
                        && (s + 1 == limrev || ISSPACE ((unsigned char) s[revlen])))
                {
                    found_revision = true;
                    break;
                }
        }

        report_revision (found_revision);
    }

    /* Plan A will work.  */
    i_buffer = buffer;
    i_ptr = ptr;
    return true;
}

/* Keep (virtually) nothing in memory. */
void PatchParser::plan_b(char const *filename)
{
    diegrace("Ooops, plan B not implmeneted");
}

/* Get FILENAME from version control system CS.  The file already exists if
   EXISTS.  Only readonly access is needed if READONLY.
   Use the command GETBUF to actually get the named file.
   Store the resulting file status into *FILESTAT.
   Return true if successful.  */
bool PatchParser::version_get (char const *filename, char const *cs, bool exists, bool readonly,
    char const *getbuf, struct stat *filestat)
{
    if (patch_get < 0)
    {
        ask ("Get file %s from %s%s? [y] ",
                quotearg (filename), cs, readonly ? "" : " with lock");
        if (*buf == 'n')
            return 0;
    }

    if (dry_run)
    {
        if (! exists)
            diegrace ("can't do dry run on nonexistent version-controlled file %s; invoke `%s' and try again",
                    quotearg (filename), getbuf);
    }
    else
    {
        if (verbosity == VERBOSE)
            say ("Getting file %s from %s%s...\n", quotearg (filename),
                    cs, readonly ? "" : " with lock");
        if (systemic (getbuf) != 0)
            diegrace ("Can't get file %s from %s", quotearg (filename), cs);
        if (stat (filename, filestat) != 0)
            diegrace ("%s", quotearg (filename));
    }

    return 1;
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
