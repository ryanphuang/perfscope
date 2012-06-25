#ifndef __COMMON_H__
#define __COMMON_H__

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<stdarg.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<ctype.h>
#include<time.h>
#include<assert.h>
#include<limits.h>
#include<stdint.h>

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t) -1)
#endif

#ifndef _O_BINARY
#define _O_BINARY 0
#endif
#ifndef O_BINARY
#define O_BINARY _O_BINARY
#endif

#define CTYPE_DOMAIN(c) ((unsigned) (c) <= 0177)

#ifndef ISSPACE
#define ISSPACE(c) (CTYPE_DOMAIN (c) && isspace (c))
#endif

#ifndef ISDIGIT
#define ISDIGIT(c) ((unsigned) (c) - '0' <= 9)
#endif

# ifndef DIRECTORY_SEPARATOR
#  define DIRECTORY_SEPARATOR '/'
# endif

# ifndef ISSLASH
#  define ISSLASH(C) ((C) == DIRECTORY_SEPARATOR)
# endif

#ifndef NULL_DEVICE
#define NULL_DEVICE "/dev/null"
#endif

#define binary_transput 0

#ifndef TTY_DEVICE
#define TTY_DEVICE "/dev/tty"
#endif

typedef off_t LINENUM;			/* must be signed */
typedef long file_offset;
enum loglevel {DEFAULT_VERBOSITY, SILENT, VERBOSE};
XTERN enum loglevel verbosity;

XTERN bool posixly_correct;
XTERN bool batch;
XTERN time_t initial_time;
XTERN bool no_strip_trailing_cr;
XTERN bool explicit_inname;

XTERN char *revision;

XTERN int debug;  /* debug flag with big indicating levels */

XTERN char *gbuf;			/* general purpose buffer */
XTERN size_t gbufsize;			/* allocated size of buf */

#define KB 1024
#endif
