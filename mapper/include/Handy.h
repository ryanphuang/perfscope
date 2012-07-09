/* * File: Handy.h
 * *
 * * Handy gadget functions   
 * *
 * * Author: Ryan Huang <ryanhuang@cs.ucsd.edu>
 */

#ifndef ___HANDY__H_
#define ___HANDY__H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <errno.h>

#define __set_errno(e) (errno = (e))

# ifndef DIRECTORY_SEPARATOR
#  define DIRECTORY_SEPARATOR '/'
# endif

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

# ifndef ISSLASH
#  define ISSLASH(C) ((C) == DIRECTORY_SEPARATOR)
# endif

size_t fgetline(FILE *, char *, size_t & , unsigned &);

const char *strnchr(const char *, size_t, char);
const char *lastof(const char *, char);
unsigned countnchr(const char *, size_t, char);

const char *stripname(const char *, int);
char * canonpath (const char *, char *);
bool pathnmeq(const char *, const char *, int , int);
bool pathneq(const char *, const char *, int);

#define streq(a,b) (!strcmp((a), (b)))

#define warn(format, ...) do { \
    fprintf(stderr, format,  ##__VA_ARGS__); \
    fprintf(stderr, "\n"); \
} while (0)

void diegrace(const char *, ...) __attribute__ ((noreturn, format (printf, 1, 2)));
void *xmalloc(size_t) __attribute__ ((__malloc__));
void *xrealloc (void *, size_t);

#endif
