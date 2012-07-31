#include "Handy.h"
#include "PatchDecoder.h"

static bool DEBUG = true;

static const char * PHEADER = "****";
static const int PLEN = 4;

static const char * FHEADER = "====";
static const int FLEN = 4;

static const char * HHEADER = "####";
static const int HLEN = 4;

static const char ADDC = '+';
static const char DELC = '-';
static const char SAMC = '~';

static const char REPC = '^';

static unsigned gBufSize = 4 * 1024;
static char * gBuf = (char *) xmalloc(gBufSize);

static const char *MODSTR[3] = { "ADD", "DELETE", "REPLACE" };

raw_ostream & operator<<(raw_ostream& os, const MODTYPE & type)
{
    os << MODSTR[type];
    return os;
}

std::ostream & operator<<(std::ostream& os, const MODTYPE & type)
{
    os << MODSTR[type];
    return os;
}

raw_ostream & operator<<(raw_ostream& os, const Mod & mod)
{
    os << mod.type << "-" << mod.scope;
    return os;
}
std::ostream & operator<<(std::ostream& os, const Mod & mod)
{
    os << mod.type << "-" << mod.scope;
    return os;
}

unsigned parseInt(const char **s, PatchDecoder *decoder)
{
    char c;
    unsigned lineno = 0;
    unsigned no = 0;
    int n;
    const char *ss = *s;
    while ((c = *ss) != '\0')
    {
        if (c == ',')
            break;
        n = c - '0';
        if (n < 0 || n > 9) {
            decoder->syntaxError("Invalid hunk line number");
        }
        lineno = no * 10 + n;
        if (lineno / 10 != no) {
            decoder->syntaxError("Hunk line number too big");
        }
        no = lineno;
        ss++;
    }
    *s = ss;
    return lineno;
}

void Hunk::putBuf(char c, size_t &len)
{
    //if (len > 0 && c == ADDC && c == gBuf[len - 1]) // Don't add consecutive ADDC
    //    return;
    if (len >= gBufSize) {
        gBufSize *= 2; // double the size
        gBuf = (char *) xrealloc(gBuf, gBufSize);
    }
    gBuf[len++] = c;
}

void Hunk::dumpBuf(size_t len)
{
    size_t i;
    for (i = 0; i < len; i++)
        printf("%c", gBuf[i]);
    printf("\n");
}

Mod * Hunk::newMod(unsigned start, unsigned end, unsigned repstart, unsigned repend, char c)
{
    MODTYPE type;
    switch (c) {
        case ADDC:
            type = ADD;
            break;
        case DELC:
            type = DEL;
            break;
        case REPC:
            type = REP;
            break;
        default:
            diegrace("Invalid mod type char %c", c);
    }
    Mod * m = new Mod();
    if (m == NULL)
        diegrace("out of memory");
    m->type = type;
    m->scope.begin = start;
    m->scope.end = end;
    m->rep_scope.begin = repstart;
    m->rep_scope.end = repend;
    return m;
}

bool Hunk::translate(char * buf, size_t len)
{
    return true;
}

bool Hunk::check(char * buf, size_t len)
{
    size_t i, j;
    for (i = 0; i < len; i++) 
        if (buf[i] == REPC)
            for (j = 0; j < i; j++)
                if (buf[j] == ADDC) // e.g. +++++^^
                    return false;
    return true;
}

/**
 * Reduce the original control sequence to a list of Mods.
 * The Reduction rule is as follows:
 *
 * 1). The sequence is divided into chunks with '~' as the boundary.
 * 2). For each '~',
 *     a). If the next char is not '~', it marks the start of a new
 *         chunk, reset '+','-' counters, chunk_len etc. 
 *     b). Else if the previous char is not '~', it marks the end of 
 *         a new chunk. Do any wrap up work.
 *     c). In either case, increment line pointer.
 * 3). For each '+', 
 *     a). If counter of '-' is positive in the same chunk before its position,
 *         cancel '+' with the furthest '-' and reduce the type to 'MOD' 
 *         at the line of '-'. Decrement the counters of both '-' and '+'.
 *     b). Else if counter of '+' is zero, this marks the start of an add region.
 *         (Note that here we assume if it's modification, '-' must proceed '+', 
 *          e.g. '--++' or '-+-+' are legal, but '+-+-' is illegal.) 
 *         We only record the start of add region instead of its whole scope, 
 *         because we are only interested in the impact to the OLD file, where
 *         the add region doesn't exist.
 *     c).  Else it's an continuation of the add region. Increment counter of '+'.
 *
 * 4). For each '-',
 *
 * To implement this, there are two ways. One is strictly follows the above logic.
 * But it might be error prone.
 * Second is to do two passes, which is less efficient but more safe: 
 * a). translate: for each chunk, reduce all pairs of '-' and '+' to '^' 
 * b). coalesce: coalesce all consecutive same chars to the corresponding region. 
 *
 *
 */
bool Hunk::reduce() 
{
    if (seqlen == 0 || start_line == 0)
        return false;

    unsigned line = start_line - 1; // start one line before the beginning
    unsigned repline = rep_start_line - 1; // start one line before the beginning
    unsigned del_cnt = 0, add_cnt = 0;

    char c;
    size_t i = 0, j = 0, buf_len = 0;

    Mod *m = NULL;
    bool printed = false;
    for (; i < seqlen; i++) {
        c = ctrlseq[i];
        switch(c) {
            case SAMC:
                // Need to check chunk end first:
                // example "~~~--++~+". If not, the first chunk won't be merged
                line++; // only increment line number for same, let the merge handling rest
                repline++;
                if (i != seqlen - 1 && ctrlseq[i + 1] != SAMC) { // chunk begin boundary
                    buf_len = 0;
                    del_cnt = 0;
                    add_cnt = 0;
                    m = NULL;
                }
                break;
            case ADDC:
                add_cnt++;
                ///////////////////////////////////////////////
                //           Second approach
                ///////////////////////////////////////////////
                if (del_cnt == 0) {
                    putBuf(c, buf_len);
                }
                else {
                    // Look Back and Translate
                    // We can be more efficient by look ahead but that would
                    // involve delete which we are trying to avoid.
                    for(j = 0; j < buf_len; j++) {
                        if (gBuf[j] == DELC) {
                            gBuf[j] = REPC;
                            break;
                        }
                    }
                    assert(j != buf_len);
                    add_cnt--;
                    del_cnt--;
                }
                /////////////////////////////////////////////////

                break;
    
            case DELC:
                del_cnt++;
                ///////////////////////////////////////////////
                //           Second approach
                ///////////////////////////////////////////////
                putBuf(c, buf_len);
                break;
            
            default:
                fprintf(stderr, "Invalid control character at position %d", i);
                return false;
        }

        // when encounter last char which is not SAMC or next char is SAMC, do merge
        if (ctrlseq[i] != SAMC && (i == seqlen - 1 || ctrlseq[i + 1] == SAMC)) {
            assert(buf_len > 0);
            if (i == seqlen - 1)
                fprintf(stderr, "last chunk");
            if (DEBUG) {
                if (!printed) {
                    printed = true;
                    printf("*****Translation result*******\n");
                }
                dumpBuf(buf_len);
            }
            merge(line, repline, buf_len);
        }
    }
    if (!mods.empty()) {
        enclosing_scope.begin = mods.front()->scope.begin;
        enclosing_scope.end = mods.back()->scope.end;
    }
    return true;
}

bool Hunk::merge(unsigned & line, unsigned & rep_line, size_t len)
{
    assert(len != 0);
    size_t i;
    char c;
    unsigned start = 0, end = line;
    unsigned rep_start = 0, rep_end = rep_line;
    Mod * m;
    bool newregion = true; // beginning is always a new region
    for (i = 0; i < len; i++) {
        c = gBuf[i];
        if (c != ADDC) {
            end++;
        }
        if (c != DELC) {
            rep_end++;
        }
        if (newregion) {
            start = end;
            rep_start = rep_end;
            newregion = false;
        }
        if (i == len - 1 || c != gBuf[i + 1]) { // mod end boundary
            m = newMod(start, end, rep_start, rep_end, c);
            mods.push_back(m);
            newregion = true; // update start in next iteration
        }
    }
    line = end; // update the line
    rep_line = rep_end; // update the replacement line
    return true;
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
Chapter::Chapter(PatchDecoder * p = NULL, const char *full = NULL, const char *file = NULL)
{
    if (full != NULL) {
        fullname.assign(full);
    }
    if (file != NULL) {
        filename.assign(file);
    }
    decoder = p;
    hunk = NULL;
}

Hunk * Chapter::next_hunk()
{
    if (decoder == NULL) {
        warns("Decoder is NULL");
        return NULL;
    }
    size_t chars_read;
    const char * line = decoder->next_line(chars_read);
    // The feeder is deceased or has nothing to feed us.
    if (line == NULL) { 
        return NULL;
    }

    // We overcross the next section.
    if (strncmp(line, PHEADER, PLEN) == 0 ||
            strncmp(line, FHEADER, FLEN) == 0) {
            unget_line(); //unget the line.
            return NULL;
    }

    if (strncmp(line, HHEADER, PLEN) != 0) {
        decoder->syntaxError("Expecting hunk header");
    }
    unsigned lineno = 0;
    unsigned replineno = 0;
    const char *s = line + PLEN;
    lineno = parseInt(&s, decoder);
    if (lineno <= 0) {
        decoder->syntaxError("Invalid hunk line number");
    }
    if (*s != ',') {
        decoder->syntaxError("Expecting replace hunk line number");
    }
    s++;
    replineno = parseInt(&s, decoder);
    if (replineno <= 0) {
        decoder->syntaxError("Invalid hunk line number");
    }

    line = decoder->next_line(chars_read);
    if (line == NULL) {
        decoder->syntaxError("Invalid hunk control sequence");
    }
    if (hunk != NULL) {
        delete hunk;
        hunk = NULL;
    } 
    hunk = new Hunk(lineno, replineno, line, chars_read);
    if (hunk == NULL) {
        diegrace("out of memory");
    }
    return hunk;
}

bool Chapter::skip_rest_of_hunks()
{
    if (decoder == NULL) {
        warns("Decoder is NULL");
        return false;
    }
    size_t chars_read;
    const char * line;
    while ((line = decoder->next_line(chars_read)) != NULL) {
        if (strncmp(line, PHEADER, PLEN) == 0 ||
                strncmp(line, FHEADER, FLEN) == 0) {
                unget_line(); //unget the line.
                break;
        }
    }
    return true;
}

void Chapter::unget_line()
{
    if (decoder && !decoder->unget_line()) { 
        diegrace("unget line failed");
    }
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
Patch::Patch(PatchDecoder * p = NULL, const char *name = NULL)
{
    if (name!= NULL) {
        patchname.assign(name);
    }
    decoder = p;
    chap = NULL;
}

Chapter * Patch::next_chapter()
{
    if (decoder == NULL) {
        warns("Decoder is null");
        return NULL;
    }
    size_t chars_read;
    const char * line = decoder->next_line(chars_read);
    // The feeder is deceased or has nothing to feed us.
    if (line == NULL) { 
        return NULL;
    }
    
    // We overcross the next patch.
    if (strncmp(line, PHEADER, PLEN) == 0) {
        decoder->unget_line(); //unget the line.
        return NULL;
    }

    if (strncmp(line, FHEADER, FLEN) != 0) {
        decoder->syntaxError("Expecting chapter header");
    }
    line = decoder->next_line(chars_read);
    if (line == NULL)
        return NULL;
    if (chap != NULL) {
        delete chap;
        chap = NULL;
    }
    chap = new Chapter(decoder, line, stripname(line, -1));
    return chap;
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void PatchDecoder::init()
{
    buf = (char *) xmalloc(BUFSIZE);
    bufsize = BUFSIZE;
    fp = fopen(inputname.c_str(), "r");
    if (fp == NULL) {
        perror("read diff error");
        exit(1);
    }
    patch = NULL;
    rewind = false;
    file_offset = 0;
    readchars = 0;
}


void PatchDecoder::syntaxError(const char *format, ...)
{
    fprintf(stderr, "Syntax error(\"");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\") at line %d: %s\n", lineno, buf);
    fflush(stderr);
    exit(1);
}

Patch * PatchDecoder::next_patch()
{
    if (fp == NULL) {
        warns("File pointer is null");
        return NULL;
    }
    size_t chars_read;
    next_line(chars_read);
    if (chars_read <= 0)
        return NULL;
    //every patch must start with patch header
    if (strncmp(buf, PHEADER, PLEN) != 0) {
        syntaxError("Expecting patch header");
        return NULL;
    }
    next_line(chars_read);
    if (chars_read <= 0) {
        syntaxError("Expecting patch name");
        return NULL;
    }
    if (patch != NULL) {
        delete patch;
        patch = NULL;
    }
    patch = new Patch(this, buf);
    if (patch == NULL) {
        diegrace("out of memory");
    }
    return patch;
}

//TODO： we should use ftell and fseek to do rewind(commented), 
//but here we try to be more efficient(but potentially dangerous) by
//simply keep a rewind flag and the content in the buffer
//and make next_line be aware of this hack.
//
bool PatchDecoder::unget_line()
{
    rewind = true;
    // seek to last offset so next call to next_line will start over
//      if (fseek(fp, file_offset, SEEK_SET) != 0) {
//                    return false;
//      }
    return true;
}

const char * PatchDecoder::next_line(size_t & chars_read)
{
    if (rewind) {
        rewind = false;
        chars_read = readchars;
        return buf;
    }
    // file_offset = ftell(fp); // remember the position as
    chars_read = fgetline(fp, buf, bufsize, lineno);
    readchars = chars_read;
    if (chars_read <= 0)
        return NULL;
    return buf;
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////



