#define XTERN extern
#include<common.h>
#undef XTERN
#include<handy.h>
#include<hash.h>
#include<quotesys.h>
#include<full-write.h>
#include<dirname.h>

typedef struct
{
  dev_t dev;
  ino_t ino;
} file_id;

static Hash_table *file_id_table;
static const char *SOURCE_SUFFIX[] = {
    "c",
    "h",
    "cc",
    "cpp",
    "C",
    "hpp",
    0
};

char *dupstr(const char *src)
{
    if (NULL == src) {
        return NULL;
    }
    return dupbuf(src, strlen(src) + 1); // assuming ending '\0'
}

char *dupbuf(const char *src, size_t size)
{
    if (NULL == src || 0 == size) {
        return NULL;
    }
    char *dst = (char *) xmalloc(size);
    memcpy(dst, src, size);
    return dst;
}

void remove_prefix (char *p, size_t prefixlen)
{
    const char *s = p + prefixlen;
    while ((*p++ = *s++))
        continue;
}


bool endswith(const char *s, const char *ending)
{
    if (NULL == s || NULL == ending) {
        return false;
    }
    int l1 = strlen(s);
    int l2 = strlen(ending);
    if (l1 < l2) {
        return false;
    }
    return (strncmp(s + (l1 - l2), ending, l2) == 0);
}

bool isempty(const char * str)
{
    if (str == NULL || *str == '\0') {
        return true;
    }
    else {
        return false;
    }
}

bool issource(const char *name)
{
    const char **suffix = SOURCE_SUFFIX;
    while(*suffix) {
        if(endswith(name, *suffix)) {
            return true;
        }
        suffix++;
    }
    return false;
}

/* Do two lines match with canonicalized white space? */
bool similar (const char *a, size_t alen,
	 const char *b, size_t blen)
{
  /* Ignore presence or absence of trailing newlines.  */
  alen  -=  alen && a[alen - 1] == '\n';
  blen  -=  blen && b[blen - 1] == '\n';

  for (;;)
    {
      if (!blen || (*b == ' ' || *b == '\t'))
	{
	  while (blen && (*b == ' ' || *b == '\t'))
	    b++, blen--;
	  if (alen)
	    {
	      if (!(*a == ' ' || *a == '\t'))
		return false;
	      do a++, alen--;
	      while (alen && (*a == ' ' || *a == '\t'));
	    }
	  if (!alen || !blen)
	    return alen == blen;
	}
      else if (!alen || *a++ != *b++)
	return false;
      else
	alen--, blen--;
    }
}

void abortgrace()
{
    /**
    PPParser parser = current_parser;
    if (parser != NULL) {
        parser->cleanup();
    }
    **/
    abort();
}

void diegrace(const char * format, ...)
{
    va_list args;
    
    int err = errno;
    va_start(args, format);
    fprintf(stderr, "Dying...:"); 
    vfprintf(stderr, format, args);
    va_end(args);
    fflush(stderr);
    errno = err;
    perror(" ");
    fflush(stderr);
    exit(1);
}

/* The difference between diegrace and errgrace is:
 *   diegrace will be more versatile and call perror in
 *   the end, so the typical usage scenario is system call
 *   failure where errno is set.
 *   errgrace just emit the error message and exit.
 *
 * */
void errgrace(const char * format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fflush(stderr);
    fflush(stderr);
    exit(1);
}


void Fseek(FILE *fp, file_offset offset, int origin)
{
    if (fseek(fp, offset, origin) != 0) {
        diegrace("Fseek error");
    }
}

void * xmalloc(size_t size)
{
    if (size <= 0) {
        errgrace("Invalid malloc size");
    }
    void *p = malloc(size);
    if (p == NULL) {
        errgrace("Out of memory");
    }
    return p;
}

/* Change the size of an allocated block of memory P to N bytes,
   with error checking.  */
void * xrealloc (void *p, size_t n)
{
    p = realloc (p, n);
    if (!p && n != 0)
        xalloc_die ();
    return p;
}


/* Change the size of an allocated block of memory P to an array of N
   objects each of S bytes, with error checking.  S must be nonzero.  */
inline void * xnrealloc (void *p, size_t n, size_t s)
{
  if (xalloc_oversized (n, s))
    xalloc_die ();
  return xrealloc (p, n * s);
}

/* Get a response from the user, somehow or other. */
void ask (const char *format, ...)
{
    static int ttyfd = -2;
    ssize_t r;
    va_list args;

    va_start (args, format);
    vfprintf (stdout, format, args);
    va_end (args);
    fflush (stdout);

    if (ttyfd == -2)
    {
        /* If standard output is not a tty, don't bother opening /dev/tty,
           since it's unlikely that stdout will be seen by the tty user.
           The isatty test also works around a bug in GNU Emacs 19.34 under Linux
           which makes a call-process `patch' hang when it reads from /dev/tty.
           POSIX.1-2001 XCU line 26599 requires that we read /dev/tty,
           though.  */
        ttyfd = (posixly_correct || isatty (STDOUT_FILENO)
                ? open (TTY_DEVICE, O_RDONLY)
                : -1);
    }

    if (ttyfd < 0)
    {
        /* No terminal at all -- default it.  */
        printf ("\n");
        gbuf[0] = '\n';
        gbuf[1] = '\0';
    }
    else
    {
        size_t s = 0;
        while ((r = read (ttyfd, gbuf + s, gbufsize - 1 - s)) == gbufsize - 1 - s
                && gbuf[gbufsize - 2] != '\n')
        {
            s = gbufsize - 1;
            gbufsize *= 2;
            gbuf = xrealloc (gbuf, gbufsize);
        }
        if (r == 0)
            printf ("EOF\n");
        else if (r < 0)
        {
            perror ("tty read");
            fflush (stderr);
            close (ttyfd);
            ttyfd = -1;
            r = 0;
        }
        gbuf[s + r] = '\0';
    }
}

void read_fatal()
{
    diegrace("read error");
}

void write_fatal()
{
    diegrace("write error");
}

void xalloc_die()
{
    errgrace("out of memory");
}

inline void too_many_lines(const char *filename)
{
    errgrace("File %s has too many lines", quotearg (filename));
}

static const char DEV_NULL[] = NULL_DEVICE;

static const char RCSSUFFIX[] = ",v";
static const char CHECKOUT[] = "co %s";
static const char CHECKOUT_LOCKED[] = "co -l %s";
static const char RCSDIFF1[] = "rcsdiff %s";

static const char SCCSPREFIX[] = "s.";
static const char *GET = "get ";
static const char *GET_LOCKED = "get -e ";
static const char SCCSDIFF1[] = "get -p ";
static const char SCCSDIFF2[] = "|diff - %s";

static const char CLEARTOOL_CO[] = "cleartool co -unr -nc ";

static const char PERFORCE_CO[] = "p4 edit ";

/* Return "RCS" if FILENAME is controlled by RCS,
   "SCCS" if it is controlled by SCCS,
   "ClearCase" if it is controlled by Clearcase,
   "Perforce" if it is controlled by Perforce,
   and 0 otherwise.
   READONLY is true if we desire only readonly access to FILENAME.
   FILESTAT describes FILENAME's status or is 0 if FILENAME does not exist.
   If successful and if GETBUF is nonzero, set *GETBUF to a command
   that gets the file; similarly for DIFFBUF and a command to diff the file
   (but set *DIFFBUF to 0 if the diff operation is meaningless).
   *GETBUF and *DIFFBUF must be freed by the caller.  */
const char *
version_controller (const char *filename, bool readonly,
		    struct stat const *filestat, char **getbuf, char **diffbuf)
{
    struct stat cstat;
    char *dir = dir_name (filename);
    char *filebase = base_name (filename);
    const char *dotslash = *filename == '-' ? "./" : "";
    size_t dirlen = strlen (dir) + 1;
    size_t maxfixlen = sizeof "SCCS/" - 1 + sizeof SCCSPREFIX - 1;
    size_t maxtrysize = dirlen + strlen (filebase) + maxfixlen + 1;
    size_t quotelen = quote_system_arg (0, dir) + quote_system_arg (0, filebase);
    size_t maxgetsize = sizeof CLEARTOOL_CO + quotelen + maxfixlen;
    size_t maxdiffsize =
        (sizeof SCCSDIFF1 + sizeof SCCSDIFF2 + sizeof DEV_NULL - 1
         + 2 * quotelen + maxfixlen);
    char *trybuf = (char *) xmalloc (maxtrysize);
    const char *r = 0;

    sprintf (trybuf, "%s/", dir);

#define try1(f,a1)    (sprintf (trybuf + dirlen, f, a1),    stat (trybuf, &cstat) == 0)
#define try2(f,a1,a2) (sprintf (trybuf + dirlen, f, a1,a2), stat (trybuf, &cstat) == 0)

    /* Check that RCS file is not working file.
       Some hosts don't report file name length errors.  */

    if ((try2 ("RCS/%s%s", filebase, RCSSUFFIX)
                || try1 ("RCS/%s", filebase)
                || try2 ("%s%s", filebase, RCSSUFFIX))
            && ! (filestat
                && filestat->st_dev == cstat.st_dev
                && filestat->st_ino == cstat.st_ino))
    {
        if (getbuf)
        {
            char *p = *getbuf = (char *)  xmalloc (maxgetsize);
            sprintf (p, readonly ? CHECKOUT : CHECKOUT_LOCKED, dotslash);
            p += strlen (p);
            p += quote_system_arg (p, filename);
            *p = '\0';
        }

        if (diffbuf)
        {
            char *p = *diffbuf = (char *) xmalloc (maxdiffsize);
            sprintf (p, RCSDIFF1, dotslash);
            p += strlen (p);
            p += quote_system_arg (p, filename);
            *p++ = '>';
            strcpy (p, DEV_NULL);
        }

        r = "RCS";
    }
    else if (try2 ("SCCS/%s%s", SCCSPREFIX, filebase)
            || try2 ("%s%s", SCCSPREFIX, filebase))
    {
        if (getbuf)
        {
            char *p = *getbuf = (char *) xmalloc (maxgetsize);
            sprintf (p, readonly ? GET : GET_LOCKED);
            p += strlen (p);
            p += quote_system_arg (p, trybuf);
            *p = '\0';
        }

        if (diffbuf)
        {
            char *p = *diffbuf = (char *) xmalloc (maxdiffsize);
            strcpy (p, SCCSDIFF1);
            p += sizeof SCCSDIFF1 - 1;
            p += quote_system_arg (p, trybuf);
            sprintf (p, SCCSDIFF2, dotslash);
            p += strlen (p);
            p += quote_system_arg (p, filename);
            *p++ = '>';
            strcpy (p, DEV_NULL);
        }

        r = "SCCS";
    }
    else if (!readonly && filestat
            && try1 ("%s@@", filebase) && S_ISDIR (cstat.st_mode))
    {
        if (getbuf)
        {
            char *p = *getbuf = (char *) xmalloc (maxgetsize);
            strcpy (p, CLEARTOOL_CO);
            p += sizeof CLEARTOOL_CO - 1;
            p += quote_system_arg (p, filename);
            *p = '\0';
        }

        if (diffbuf)
            *diffbuf = 0;

        r = "ClearCase";
    }
    else if (!readonly && filestat &&
            (getenv("P4PORT") || getenv("P4USER") || getenv("P4CONFIG")))
    {
        if (getbuf)
        {
            char *p = *getbuf = (char *) xmalloc (maxgetsize);
            strcpy (p, PERFORCE_CO);
            p += sizeof PERFORCE_CO - 1;
            p += quote_system_arg (p, filename);
            *p = '\0';
        }

        if (diffbuf)
            *diffbuf = 0;

        r = "Perforce";
    }

    free (trybuf);
    free (filebase);
    free (dir);
    return r;
}

int systemic(const char * cmd)
{
    fflush(stdout);
    return system(cmd);
}

const char * make_temp (char letter)
{
    char *r;
    const char *tmpdir = getenv ("TMPDIR");	/* Unix tradition */
    if (!tmpdir) tmpdir = getenv ("TMP");		/* DOS tradition */
    if (!tmpdir) tmpdir = getenv ("TEMP");	/* another DOS tradition */
    if (!tmpdir) tmpdir = TMPDIR;
    r = (char *) xmalloc (strlen (tmpdir) + 10);
    sprintf (r, "%s/p%cXXXXXX", tmpdir, letter);

    /* It is OK to use mktemp here, since the rest of the code always
       opens temp files with O_EXCL.  It might be better to use mkstemp
       to avoid some DoS problems, but simply substituting mkstemp for
       mktemp here will not fix the DoS problems; a more extensive
       change would be needed.  */
    mktemp (r);

    if (!*r)
        diegrace("mktemp");
    return r;
}


/* Insert a file with status ST into the hash table.  */
static void insert_file (struct stat const *st)
{
    file_id *p;
    static file_id *next_slot;

    if (!next_slot)
        next_slot = (file_id *) xmalloc (sizeof *next_slot);
    next_slot->dev = st->st_dev;
    next_slot->ino = st->st_ino;
    p = (file_id *) hash_insert (file_id_table, next_slot);
    if (!p)
        xalloc_die ();
    if (p == next_slot)
        next_slot = NULL;
}

/* Move a file FROM (where *FROM_NEEDS_REMOVAL is nonzero if FROM
   needs removal when cleaning up at the end of execution, and where
   *FROMST is FROM's status if known),
   to TO, renaming it if possible and copying it if necessary.
   If we must create TO, use MODE to create it.
   If FROM is null, remove TO.
   FROM_NEEDS_REMOVAL must be nonnull if FROM is nonnull.
   and FROMST must be nonnull if both FROM and BACKUP are nonnull.
   Back up TO if BACKUP is true.  */

void move_file (const char *from, int volatile *from_needs_removal,
	   struct stat const *fromst,
	   char *to, mode_t mode, bool backup)
{
    struct stat to_st;
    int to_errno = -1;

    if (from)
    {
        if (rename (from, to) != 0)
        {
            bool to_dir_known_to_exist = false;

            if (errno == ENOENT
                    && (to_errno == -1 || to_errno == ENOENT))
            {
                makedirs (to);
                to_dir_known_to_exist = true;
                if (rename (from, to) == 0)
                    goto rename_succeeded;
            }

            if (errno == EXDEV)
            {
                struct stat tost;
                if (! backup)
                {
                    if (unlink (to) == 0)
                        to_dir_known_to_exist = true;
                    else if (errno != ENOENT)
                        diegrace ("Can't remove file %s", quotearg (to));
                }
                copy_file (from, to, &tost, 0, mode, to_dir_known_to_exist);
                insert_file (&tost);
                return;
            }

            diegrace ("Can't rename file");
        }

rename_succeeded:
        if (fromst)
            insert_file (fromst);
        /* Do not clear *FROM_NEEDS_REMOVAL if it's possible that the
           rename returned zero because FROM and TO are hard links to
           the same file.  */
        if (0 < to_errno
                || (to_errno == 0 && to_st.st_nlink <= 1))
            *from_needs_removal = 0;
    }
    else if (! backup)
    {
        if (debug & 4)
            say ("Removing file %s\n", quotearg (to));
        if (unlink (to) != 0 && errno != ENOENT)
            diegrace ("Can't remove file %s", quotearg (to));
    }
}


/* Create FILE with OPEN_FLAGS, and with MODE adjusted so that
   we can read and write the file and that the file is not executable.
   Return the file descriptor.  */
int create_file (const char *file, int open_flags, mode_t mode,
	     bool to_dir_known_to_exist)
{
    int try_makedirs_errno = to_dir_known_to_exist ? 0 : ENOENT;
    int fd;
    mode |= S_IRUSR | S_IWUSR;
    mode &= ~ (S_IXUSR | S_IXGRP | S_IXOTH);
    do
    {
        if (! (O_CREAT && O_TRUNC))
            close (creat (file, mode));
        fd = open (file, O_CREAT | O_TRUNC | open_flags, mode);
        if (fd < 0)
        {
            char *f;
            if (errno != try_makedirs_errno)
                diegrace ("Can't create file %s", quotearg (file));
            f = strdup (file);
            makedirs (f);
            free (f);
            try_makedirs_errno = 0;
        }
    } while (fd < 0);
    return fd;
}

void copy_to_fd (const char *from, int tofd)
{

    int fromfd;
    ssize_t i;

    if ((fromfd = open (from, O_RDONLY | O_BINARY)) < 0)
        diegrace("Can't reopen file %s", quotearg (from));
    while ((i = read (fromfd, gbuf, gbufsize)) != 0)
    {
        if (i == (ssize_t) -1)
            read_fatal ();
        if (full_write (tofd, gbuf, i) != i)
            write_fatal ();
    }
    if (close (fromfd) != 0)
        read_fatal ();
}

/* Copy a file. */
void copy_file (const char *from, const char *to, struct stat *tost,
	   int to_flags, mode_t mode, bool to_dir_known_to_exist)
{
    int tofd;
    tofd = create_file (to, O_WRONLY | O_BINARY | to_flags, mode,
            to_dir_known_to_exist);
    copy_to_fd (from, tofd);
    if ((tost && fstat (tofd, tost) != 0)
            || close (tofd) != 0)
        write_fatal ();
}

/* Remove empty ancestor directories of FILENAME.
   Ignore errors, since the path may contain ".."s, and when there
   is an EEXIST failure the system may return some other error number.  */
void removedirs (char *filename)
{
    size_t i;

    for (i = strlen (filename);  i != 0;  i--)
        if (ISSLASH (filename[i])
                && ! (ISSLASH (filename[i - 1])
                    || (filename[i - 1] == '.'
                        && (i == 1
                            || ISSLASH (filename[i - 2])
                            || (filename[i - 2] == '.'
                                && (i == 2
                                    || ISSLASH (filename[i - 3])))))))
        {
            filename[i] = '\0';
            if (rmdir (filename) == 0 && verbosity == VERBOSE)
                say ("Removed empty directory %s\n", quotearg (filename));
            filename[i] = '/';
        }
}

/* Make sure we'll have the directories to create a file.
   Ignore the last element of `filename'.  */

/* Replace '/' with '\0' in FILENAME if it marks a place that
   needs testing for the existence of directory.  Return the address
   of the last location replaced, or 0 if none were replaced.  */
static char * replace_slashes (char *filename)
{
    char *f;
    char *last_location_replaced = 0;
    const char *component_start;

    for (f = filename + FILE_SYSTEM_PREFIX_LEN (filename);  ISSLASH (*f);  f++)
        continue;

    component_start = f;

    for (; *f; f++)
        if (ISSLASH (*f))
        {
            char *slash = f;

            /* Treat multiple slashes as if they were one slash.  */
            while (ISSLASH (f[1]))
                f++;

            /* Ignore slashes at the end of the path.  */
            if (! f[1])
                break;

            /* "." and ".." need not be tested.  */
            if (! (slash - component_start <= 2
                        && component_start[0] == '.' && slash[-1] == '.'))
            {
                *slash = '\0';
                last_location_replaced = slash;
            }

            component_start = f + 1;
        }

    return last_location_replaced;
}


void makedirs (char *filename)
{
    char *f;
    char *flim = replace_slashes (filename);

    if (flim)
    {
        /* Create any missing directories, replacing NULs by '/'s.
           Ignore errors.  We may have to keep going even after an EEXIST,
           since the path may contain ".."s; and when there is an EEXIST
           failure the system may return some other error number.
           Any problems will eventually be reported when we create the file.  */
        for (f = filename;  f <= flim;  f++)
            if (!*f)
            {
                mkdir (filename,
                        S_IRUSR|S_IWUSR|S_IXUSR
                        |S_IRGRP|S_IWGRP|S_IXGRP
                        |S_IROTH|S_IWOTH|S_IXOTH);
                *f = '/';
            }
    }
}

/* Return an index for ENTRY into a hash table of size TABLE_SIZE.  */
static size_t file_id_hasher (void const *entry, size_t table_size)
{
    file_id const *e = (file_id *) entry;
    size_t i = e->ino + e->dev;
    return i % table_size;
}

/* Do ENTRY1 and ENTRY2 refer to the same files?  */
static bool file_id_comparator (void const *entry1, void const *entry2)
{
  file_id const *e1 = (file_id const *) entry1;
  file_id const *e2 = (file_id const *) entry2;
  return (e1->ino == e2->ino && e1->dev == e2->dev);
}

/* Initialize the hash table.  */
void init_backup_hash_table (void)
{
    file_id_table = hash_initialize (0, NULL, file_id_hasher,
            file_id_comparator, free);
    if (!file_id_table)
        xalloc_die ();
}

/* Has the file identified by ST already been inserted into the hash
   table?  */
bool file_already_seen (struct stat const *st)
{
    file_id f;
    f.dev = st->st_dev;
    f.ino = st->st_ino;
    return hash_lookup (file_id_table, &f) != 0;
}

/* list all files in the directory specified by 'dir' and save the result to files */
filenode * listdir(const char* dir)
{
    DIR * d = opendir(dir);
    if (NULL == d) {
        diegrace("Cannot open dir %s\n", dir);
    }
    struct dirent * dp;
    filenode *header = NULL;
    filenode *p = header;
    while((dp = readdir(d)) != NULL) {
        if(streq(dp->d_name, ".") || streq(dp->d_name, "..")) {
            continue;
        }
        filenode *node = (filenode *) malloc(sizeof(filenode));
        node->file = dupstr(dp->d_name);
        node->next = NULL;
        if (header == NULL) {
            header = node;
        }
        else {
            p->next = node;
        }
        p = node;
    }
    return header;
}
