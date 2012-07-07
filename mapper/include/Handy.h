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

size_t fgetline(FILE *, char *, size_t & , unsigned &);

const char *strnchr(const char *, size_t, int);
unsigned countnchr(const char *, size_t, int);

#define warn(format, ...) do { \
    fprintf(stderr, format,  ##__VA_ARGS__); \
    fprintf(stderr, "\n"); \
} while (0)

void diegrace(const char *, ...) __attribute__ ((noreturn, format (printf, 1, 2)));
void *xmalloc(size_t) __attribute__ ((__malloc__));
void *xrealloc (void *, size_t);

#endif
