/* * File: PatchDecoder.h
 * *
 * *     Decode the patch parser's intermediate result and translate into scope and modifications
 * *
 * * Author: Ryan Huang <ryanhuang@cs.ucsd.edu>
 */

#ifndef ___PATCH_DECODER__H_
#define ___PATCH_DECODER__H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "llvm/ADT/SmallVector.h"

using namespace llvm;

#define BUFSIZE 8 * 1024

typedef struct Scope {
    unsigned long begin;
    unsigned long end;
    
    Scope() : begin(0), end(0) {}
    Scope(unsigned long b, unsigned long e) : begin(b), end(e) {}
    Scope(const Scope &another) : begin(another.begin), end(another.end) {}

} Scope;

enum MODTYPE {ADD, DEL, REP};

typedef struct Mod {
    Scope scope;
    MODTYPE type;
} Mod; 

class PatchDecoder;

//FIXME: Currently, the interactions between class Hunk, Patch and PatchDecoder are
// unsafe. Lower layer class like Hunk assumes its upper layer to exist. This
// is due to the efficiency here. We don't want to soak up the memory by parsing
// all hunks at once and put them in the container. 
// Instead, we parse one at a time. Because the it's unlikely they will be revisited
// or random accessed later in the future. Otherwise, the abandoned implementation
// of using iterators should be restored.
//
class Hunk {
    public:
        unsigned start_line;
        const char *ctrlseq;
        size_t seqlen;
        typedef SmallVector<Mod *, 16>::const_iterator iterator;

    protected:
        SmallVector<Mod *, 16> mods;

    public:
        Hunk(unsigned start, const char *seq, size_t len) : start_line(start), ctrlseq(seq), 
            seqlen(len) {}
        bool reduce();

        iterator begin() { return mods.begin(); }
        iterator end() { return mods.end(); }

private:
        void putBuf(char, size_t &);
        void dumpBuf(size_t);
        Mod * newMod(unsigned, char);
        bool merge(unsigned, size_t);

};

class Chapter {
    public:
        std::string directory;
        std::string filename;
        PatchDecoder *parser;

    protected:
        Hunk *hunk;

    public:
        Chapter(PatchDecoder *, const char *, const char *);
        Hunk * next_hunk();


    protected:
        const char * next_line(size_t &);
        void unget_line();
};

class Patch {
    public:
        PatchDecoder *parser;
        std::string patchname;
        typedef SmallVector<Hunk*, 8>::const_iterator iterator;

    protected:
//          SmallVector<Hunk *, 8> hunks;
        Chapter * chap;

    public:
        Patch(PatchDecoder *, const char *);
        Patch(PatchDecoder *p , const std::string & name) : parser(p), patchname(name),
                chap(NULL) {}

//            iterator begin() { return hunks.begin(); }
//            iterator end() { return hunks.end(); }

};

class PatchDecoder{
    public:
        std::string inputname;
//          typedef SmallVector<Patch *, 8>::const_iterator iterator;


    protected:
//            SmallVector<Patch *, 8> patches;
        Patch * patch;
        unsigned lineno;
        
    private:
        size_t bufsize;
        size_t readchars;
        char *buf;
        FILE *fp;
        bool rewind;
        long file_offset;


    protected:
        void init();

    public:
        PatchDecoder(const char *input) : inputname(input) { init(); }
        PatchDecoder(const std::string & input) : inputname(input) { init(); }
        ~PatchDecoder()
        {
            if (buf)
                free(buf);
            if (fp)
                fclose(fp);
        }

        const Patch * next_patch();
        bool unget_line();
        const char * next_line(size_t &);
    
        // don't to shift the n,m in attribute because of *this*
        void syntaxError(const char *, ...) __attribute__ ((format (printf, 2, 3)));
    
//          iterator begin() { return hunks.begin(); }
//          iterator end() { return hunks.end(); }
};

#endif
