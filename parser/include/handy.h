#ifndef __HANDY_H__
#define __HANDY_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define streq(a,b) (!strcmp((a), (b)))
#define strneq(a,b,n) (!strncmp((a), (b), (n)))


#define XTERN extern
#include<common.h>
#undef XTERN

#ifndef TMPDIR
#define TMPDIR "/tmp"
#endif

bool isempty(const char *);

void diegrace(const char *, ...) __attribute__ ((noreturn, format (printf, 1, 2)));
void abortgrace(void);
void Fseek (FILE *, file_offset, int);

char *dupstr(const char *);
char *dupbuf(const char *, size_t);
void remove_prefix (char *, size_t);

void *xmalloc(size_t) __attribute__ ((__malloc__));
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


void ask(char const *, ...) __attribute__ ((format (printf, 1, 2)));
#define say printf
#define vsay vprintf
#define quotearg(s) s

void read_fatal (void);
void write_fatal (void);
void xalloc_die(void);
char const *version_controller (char const *, bool, struct stat const *, char **, char **);
int systemic (char const *);

void too_many_lines(const char *) __attribute__((noreturn));


void removedirs (char *);
void makedirs (char *);

void move_file (char const *, int volatile *, struct stat const *, char *, mode_t, bool);
void copy_to_fd (const char *, int);
void copy_file (char const *, char const *, struct stat *, int, mode_t, bool);
int create_file (char const *, int, mode_t, bool);
char const * make_temp(char);


void init_backup_hash_table (void);


#ifdef __cplusplus
}
#endif

#endif
