#include "Handy.h"

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

unsigned countnchr(const char *str, size_t n, char ch)
{
    const char * s = str;
    unsigned cnt = 0;
    while (n-- && *s != '\0') {
        if (ch == *s++)
            cnt++;
    }
    return cnt;
}
