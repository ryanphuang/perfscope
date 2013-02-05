#include <cxxabi.h>

#include "commons/handy.h"

static char PBUF1[MAX_PATH];
static char PBUF2[MAX_PATH];
static char *MBUF = NULL;
static size_t MBUF_LEN = 0;

static const char *SOURCE_SUFFIX[] = {
    ".c",
    ".cc",
    ".cpp",
    ".C",
    ".hpp",
    0
};

static const int SUFFIX_LEN[] = {
    1,
    2,
    3,
    1,
    3
};

char *dupstr(const char *src)
{
    if (NULL == src) {
        return NULL;
    }
    size_t size = strlen(src) + 1;
    char *dst = (char *) xmalloc(size);
    memcpy(dst, src, size);
    return dst;
}

void diegrace(const char * format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);
    exit(1);
}

void * xmalloc(size_t size)
{
    if (size <= 0) {
        diegrace("Invalid malloc size %d", size);
    }
    void *p = malloc(size);
    if (p == NULL) {
        diegrace("Out of memory");
    }
    return p;
}

/* Change the size of an allocated block of memory P to N bytes,
   with error checking.  */
void * xrealloc (void *p, size_t n)
{
    p = realloc (p, n);
    if (!p && n != 0)
        diegrace("out of memory");
    return p;
}

size_t fgetline(FILE * fp, char *buf, size_t & bufsize, unsigned &lineno)
{
    if (fp == NULL) 
        return 0;
    int c;
    size_t i = 0;
    for (;;) {
        c = getc(fp);
        if (i == bufsize - 1) {
            bufsize *= 2;
            buf = (char *) xrealloc(buf, bufsize);
        }
        if (c == '\n') {
            buf[i] = '\0'; // don't increment i for '\0'
            lineno++;
            break;
        }
        // this shouldn't happen because EOF should be after '\n'
        else 
            if (c == EOF) { 
                if (i == 0)
                {
                    buf[0] = '\0';
                    return 0;
                }
                if (ferror(fp)) {
                    perror("reading patch failed");
                    exit(1);
                }
                diegrace("unexpected end of file");
            }
        buf[i++] = c;
    }
    return i;
}

char *src2obj(const char *name, char *outbuf, int *buflen)
{
    if (outbuf != NULL && buflen == NULL)
        return NULL;
    const char **suffix = SOURCE_SUFFIX;
    const int*slen = SUFFIX_LEN;
    while(*suffix) {
        if(endswith(name, *suffix)) {
            char *dst;
            int baselen = strlen(name) - *slen;
            if (outbuf == NULL) {
                dst = (char *) xmalloc(baselen + 2);
                if (buflen != NULL)
                    *buflen = baselen + 2;
            }
            else {
                if (baselen + 2 > *buflen)
                    dst = (char *) realloc(outbuf, baselen + 2);
                else
                    dst = outbuf;
            }
            strncpy(dst, name, baselen);
            dst[baselen] = 'o'; // obj suffix
            dst[baselen + 1] = '\0';
            return dst;
        }
        suffix++;
        slen++;
    }
    return NULL;
}

const char *strnchr(const char *str, size_t n, char ch)
{
    const char * s = str;
    while (n-- && *s != '\0') {
        if (ch == *s)
            return s;
        else
            ++s;
    }
    return NULL;
}

const char * lastof(const char * str, char ch)
{
    const char *s = str;
    const char *p = NULL;
    while (*s != '\0') {
        if (*s == ch)
            p = s;
        s++;
    }
    return p;
}

/* Return the canonical absolute name of file NAME.  A canonical name
   does not contain any `.', `..' components nor any repeated path
   separators ('/').  
   Differences from the libc's realpath: 
   1). do not expand symlink
   2). do not add pwd to the name
   3). do not check for existence of each component

   The following implementation is modified from glibc 2.9(canonicalize.c)
*/

char * canonpath (const char *name, char *resolved)
{
    char *rpath, *dest;
    const char *start, *end, *rpath_limit;
    long int path_max;

    if (name == NULL)
    {
        /* As per Single Unix Specification V2 we must return an error if
           either parameter is a null pointer.  We extend this to allow
           the RESOLVED parameter to be NULL in case the we are expected to
           allocate the room for the return value.  */
        __set_errno (EINVAL);
        return NULL;
    }

    if (name[0] == '\0')
    {
        /* As per Single Unix Specification V2 we must return an error if
           the name argument points to an empty string.  */
        __set_errno (ENOENT);
        return NULL;
    }

    path_max = MAX_PATH;

    if (resolved == NULL)
    {
        rpath = (char *) malloc (path_max);
        if (rpath == NULL)
            return NULL;
    }
    else
        rpath = resolved;
    rpath_limit = rpath + path_max;

    if (name[0] == '/') {
        rpath[0] = '/';
        dest = rpath + 1;
    }
    else
        dest = rpath;

    for (start = end = name; *start; start = end)
    {
        /* Skip sequence of multiple path-separators.  */
        while (*start == '/')
            ++start;

        /* Find end of path component.  */
        for (end = start; *end && *end != '/'; ++end)
            /* Nothing.  */;

        if (end - start == 0)
            break;
        else if (end - start == 1 && start[0] == '.')
            /* nothing */;
        else if (end - start == 2 && start[0] == '.' && start[1] == '.')
        {
            /* Back up to previous component, ignore if at root already.  */
            while ((dest > rpath + 1) && (--dest)[-1] != '/');
            if (dest[-1] != '/')
                --dest; //move to the beginning
        }
        else
        {
            size_t new_size;

            if (dest > rpath && dest[-1] != '/')
                *dest++ = '/';

            if (dest + (end - start) >= rpath_limit)
            {
                off_t dest_offset = dest - rpath;
                char *new_rpath;

                if (resolved)
                {
                    __set_errno (ENAMETOOLONG);
                    if (dest > rpath + 1)
                        dest--;
                    *dest = '\0';
                    goto error;
                }
                new_size = rpath_limit - rpath;
                if (end - start + 1 > path_max)
                    new_size += end - start + 1;
                else
                    new_size += path_max;
                new_rpath = (char *) realloc (rpath, new_size);
                if (new_rpath == NULL)
                    goto error;
                rpath = new_rpath;
                rpath_limit = rpath + new_size;
                dest = rpath + dest_offset;
            }

            mempcpy (dest, start, end - start);
            dest += (end - start);
            *dest = '\0';
        }
    }
    if (dest > rpath + 1 && dest[-1] == '/')
        --dest;
    *dest = '\0';

    assert (resolved == NULL || resolved == rpath);
    return rpath;

error:
    assert (resolved == NULL || resolved == rpath);
    if (resolved == NULL)
        free (rpath);
    return NULL;
}

const char * stripname(const char *name, int strips)
{
    if (strips == 0)
        return name;
    const char * str = name;
    const char * p = name;
    int left = strips;
    while(*str) {
        if (ISSLASH(*str)) {
            while(ISSLASH(str[1])) // care, e,g, a//b/c//
                str++;
            if (strips < 0 || --left >= 0) {
                p = str + 1;
            }
            if (left == 0)
                break;
        }
        str++;
    }
    return p;
}


bool pathnmeq(const char *path1, const char *path2, int n, int m)
{
    if (canonpath(path1, PBUF1) && canonpath(path2, PBUF2))
        return strcmp(stripname(PBUF1, n), stripname(PBUF2,m)) == 0;
    else
        return false;
}

bool pathneq(const char *path1, const char *path2, int n)
{
    if (canonpath(path1, PBUF1)) {
        return strcmp(stripname(PBUF1, n), path2) == 0;
    }
    else
        return false;
}

unsigned countnchr(const char *str, size_t n, char ch)
{
    const char * s = str;
    unsigned cnt = 0;
    size_t sn = n;
    while (*s != '\0') {
        if (sn > 0 && n-- <= 0)
            break;
        if (ch == *s++)
            cnt++;
    }
    return cnt;
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

char * common_prefix(char * buf, size_t & len, const char *str1, const char *str2)                             
{                                                                                    
    const char *p1 = str1, *p2 = str2;
    size_t pos = 0;
    for (; p1 && p2 && *p1 && *p2 && *p1 == *p2; p1++, p2++)
    {
        if (pos >= len) {
            len *= 2;
            buf = (char *) realloc(buf, len);
        }
        buf[pos++] = *p1;
    }
    if (pos >= len) {
        len *= 2;
        buf = (char *) realloc(buf, len);
    }
    buf[pos] = '\0';
    return buf;
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

/** A simple wrapper for demangling C++ ABI name **/
const char * cpp_demangle(const char *name)
{
    if (MBUF == NULL) {
        MBUF = (char *) xmalloc(MANGLE_LEN);
        MBUF_LEN = MANGLE_LEN;
    }
    int status;
    char * ret = abi::__cxa_demangle(name, MBUF, &MBUF_LEN, &status);
    if (ret == NULL) // normal C names will be demangled to NULL
        return name;
    return ret;
}

