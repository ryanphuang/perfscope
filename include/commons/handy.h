/**
 *  @file          handy.h
 *
 *  @version       1.0
 *  @created       06/25/2012 01:04:01 AM
 *  @revision      $Id$
 *
 *  @author        Ryan Huang <ryanhuang@cs.ucsd.edu>
 *  @organization  University of California, San Diego
 *  
 *  Copyright (c) 2013, Ryan Huang
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *  http://www.apache.org/licenses/LICENSE-2.0
 *     
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  @section       DESCRIPTION
 *  
 *  Handy gadget functions   
 *
 */

#ifndef ___HANDY__H_
#define ___HANDY__H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <errno.h>
#include <vector>
#include <string>

#define __set_errno(e) (errno = (e))

# ifndef DIRECTORY_SEPARATOR
#  define DIRECTORY_SEPARATOR '/'
# endif

#ifndef MAX_PATH
#define MAX_PATH 512
#endif

#ifndef MANGLE_LEN
#define MANGLE_LEN 512
#endif

# ifndef ISSLASH
#  define ISSLASH(C) ((C) == DIRECTORY_SEPARATOR)
# endif

size_t fgetline(FILE *, char *, size_t & , unsigned &);
char * fgetline(FILE *, char *, size_t);

inline void syntaxerr(const char * msg, unsigned line)
{
  fprintf(stderr, "Syntax error at line %u: %s\n", line, msg);
}

inline void indent(int space)
{
  while(space--) printf(" ");
}

char *dupstr(const char *);

bool isempty(const char *);
bool endswith(const char*, const char*);
bool pendswith(const char*, const char*);
const char *strnchr(const char *, size_t, char);
const char *lastof(const char *, char);
unsigned countnchr(const char *, size_t, char);
char * common_prefix(char *, size_t &, const char *, const char *);                             

const char *stripname(const char *, int);
char * canonpath (const char *, char *);
bool pathnmeq(const char *, const char *, int , int);
bool pathneq(const char *, const char *, int);

char *src2obj(const char *, char *, int *);

const char * cpp_demangle(const char *);

void readlines2vector(char *, std::vector<std::string> &);

#define streq(a,b) (!strcmp((a), (b)))


#define warn(format, ...) do { \
    fprintf(stderr, "Warning: "); \
    fprintf(stderr, format,  __VA_ARGS__); \
    fprintf(stderr, "\n"); \
} while (0)

#define warns(msg) warn("%s", msg)

void diegrace(const char *, ...) __attribute__ ((noreturn, format (printf, 1, 2)));
void *xmalloc(size_t) __attribute__ ((__malloc__));
void *xrealloc (void *, size_t);

#define gen_dbg(prefix)                                                             \
static void prefix##_debug(const char * fmt, ...)                                   \
  __attribute__ ((format (printf, 1, 2)));

#define gen_dbg_impl(prefix)                                                        \
static void prefix##_debug(const char * fmt, ...)                                   \
{                                                                                   \
  va_list args;                                                                     \
  va_start(args, fmt);                                                              \
  vprintf(fmt, args);                                                               \
  va_end(args);                                                                     \
}

#define gen_dbg_nop(prefix)                                                         \
static void prefix##_debug(const char * fmt, ...)                                   \
{                                                                                   \
}

#endif
