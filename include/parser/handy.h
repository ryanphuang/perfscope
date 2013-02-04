#ifndef __HANDY_H__
#define __HANDY_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define streq(a,b) (!strcmp((a), (b)))
#define strneq(a,b,n) (!strncmp((a), (b), (n)))


#define XTERN extern
#include "parser/common.h"
#undef XTERN

#ifndef TMPDIR
#define TMPDIR "/tmp"
#endif

#define UNIMPL_STR_FMT "Unimplemented block %s:%d, %s\n", \
    __FILE__, __LINE__, __PRETTY_FUNCTION__

#define WARN(...) fprintf(stderr, __VA_ARGS__)

#define DEBUG(...) debug ? fprintf(stdout, __VA_ARGS__) : NULL

#define WARN_UNIMPL fprintf(stderr, UNIMPL_STR_FMT); 

#define FREE(buf) do { \
    if ((buf)) \
        free(buf); \
} while (0)

#define DIE_UNIMPL  do { \
    fprintf(stderr, UNIMPL_STR_FMT); \
    exit(1); \
} while(0)

#define init_array(arr, n) do { \
    int i; \
    for (i = 0; i < (n); i++) { \
        (arr)[i] = 0; \
    } \
} while(0)

#define free_bufs(buf, n) do { \
    int i; \
    for (i = 0; i < (n); i++) { \
            if (buf[i]) \
                free(buf[i]); \
    } \
} while(0) 

typedef struct filenode {
    char *file;
    struct filenode *next;
} filenode;

bool isempty(const char *);
char *dupstr(const char *);
char *dupbuf(const char *, size_t);
bool endswith(const char*, const char*);
void remove_prefix (char *, size_t);
bool issource(const char *);
bool ignore(const char *);

void diegrace(const char *, ...) __attribute__ ((noreturn, format (printf, 1, 2)));

void errgrace(const char *, ...) __attribute__ ((noreturn, format (printf, 1, 2)));

#define errreturn(val, format, ...) do { \
    fprintf(stderr, format,  ##__VA_ARGS__); \
    fprintf(stderr, "\n"); \
    return val; \
} while (0)
    

void abortgrace(void);
void Fseek (FILE *, file_offset, int);


void *xmalloc(size_t) __attribute__ ((__malloc__));
void *xrealloc (void *p, size_t s);
/* Return 1 if an array of N objects, each of size S, cannot exist due
   to size arithmetic overflow.  S must be positive and N must be
   nonnegative.  This is a macro, not an inline function, so that it
   works correctly even when SIZE_MAX < N.

   By gnulib convention, SIZE_MAX represents overflow in size
   calculations, so the conservative dividend to use here is
   SIZE_MAX - 1, since SIZE_MAX might represent an overflowed value.
   However, malloc (SIZE_MAX) fails on all known hosts where
   sizeof (ptrdiff_t) <= sizeof (size_t), so do not bother to test for
   exactly-SIZE_MAX allocations on such hosts; this avoids a test and
   branch when S is known to be 1.  */
# define xalloc_oversized(n, s) \
    ((size_t) (sizeof (ptrdiff_t) <= sizeof (size_t) ? -1 : -2) / (s) < (n))
void * xnrealloc (void *, size_t, size_t);


void ask(const char *, ...) __attribute__ ((format (printf, 1, 2)));
#define say printf
#define vsay vprintf
#define quotearg(s) s

void read_fatal (void);
void write_fatal (void);
void xalloc_die(void);
const char *version_controller (const char *, bool, struct stat const *, char **, char **);
int systemic (const char *);

void too_many_lines(const char *) __attribute__((noreturn));


void removedirs (char *);
void makedirs (char *);

void move_file (const char *, int volatile *, struct stat const *, char *, mode_t, bool);
void copy_to_fd (const char *, int);
void copy_file (const char *, const char *, struct stat *, int, mode_t, bool);
int create_file (const char *, int, mode_t, bool);
const char * make_temp(char);
filenode *listdir(const char *);


bool similar (const char*, size_t , const char*, size_t);

void init_backup_hash_table (void);
bool file_already_seen (struct stat const *);


#ifdef __cplusplus
}

/* C++ does not allow conversions from void * to other pointer types
   without a cast.  Use templates to work around the problem when
   possible.  */

template <typename T> inline T *
xrealloc (T *p, size_t s)
{
  return (T *) xrealloc ((void *) p, s);
}

#endif

#endif
