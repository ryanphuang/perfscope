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

raw_ostream & operator<<(raw_ostream& os, const Scope & scope)
{
    os << "[#" << scope.begin << ",#" << scope.end <<"]";
    return os;
}

std::ostream & operator<<(std::ostream& os, const Scope & scope)
{
    os << "[#" << scope.begin << ",#" << scope.end <<"]";
    return os;
}

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

void Hunk::putBuf(char c, size_t &len)
{
    if (len > 0 && c == ADDC && c == gBuf[len - 1]) // Don't add consecutive ADDC
        return;
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

Mod * Hunk::newMod(unsigned line, char c)
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
    m->scope.begin = line;
    m->scope.end = line;
    return m;
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
 * b). collapse: collapse all consecutive same chars to the corresponding region. 
 *
 *
 */
bool Hunk::reduce() 
{
    if (seqlen == 0 || start_line == 0)
        return false;

    unsigned line = start_line - 1; // start one line before the beginning
    unsigned chunk_line = 0; 
     
    char c;
    size_t i, j, chunk_beg, buf_len;
    i = j = chunk_beg = buf_len = 0;
    unsigned del_cnt, add_cnt;
    del_cnt = add_cnt = 0;
    Mod *m = NULL;
    bool printed = false;
    for (; i < seqlen; i++) {
        c = ctrlseq[i];
        switch(c) {
            case SAMC:
                line++; // SAMC belongs to OLD, so we increment line.
                // Need to check chunk end first:
                // example "~~~--++~+". If not, the first chunk won't be merged
                if (i > 0 && ctrlseq[i - 1] != SAMC) { // chunk end boundary 
                    ///////////////////////////////////////////////
                    //           Second approach
                    ///////////////////////////////////////////////
                    if (DEBUG) {
                        if (!printed) {
                            printed = true;
                            printf("*****Translation result*******\n");
                        }
                        dumpBuf(buf_len);
                    }
                    merge(chunk_line, buf_len);
                    /////////////////////////////////////////////////
                }
                if (i != seqlen - 1 && ctrlseq[i + 1] != SAMC) { // chunk begin boundary
                    if (ctrlseq[i + 1] == ADDC)
                        chunk_line = line; // 
                    else
                        chunk_line = line + 1;
                    chunk_beg = i + 1;
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

                ///////////////////////////////////////////////
                //           First approach
                ///////////////////////////////////////////////
                /**********************************************
                // no '-' remains and no '+' remains, it's the start of add region.
                if (del_cnt == 0) {
                    if (add_cnt == 1) {
                        // start of add region.
                        // if there is replace region before add, fill up its end.
                        if (m != NULL) {
                            assert(m->type == REP); // must be rep
                            m->scope.end = line;
                            mods.push_back(m);
                        }

                        m = new Mod();
                        if (m == NULL)
                            diegrace("out of memory");
                        //ADD modification only stores the beginning boundary of add region.
                        m->scope.begin = line;
                        m->scope.end = line;
                        m->type = ADD;
                        mods.push_back(m);
                        m = NULL; // reset m
                    }
                }
                else { // del_cnt > 0
                    // start of replace region.
                    m = new Mod();
                    if (m == NULL)
                        diegrace("out of memory");
                    //REP's scope end isn't known yet, needs to be filled up later.
                    m->scope.begin = line;
                    m->type = REP;
                    // decrease both add and del cnt
                    add_cnt--;
                    del_cnt--; 
                }
                ********************************************/
                break;
    
            case DELC:
                line++; // DELC belongs to OLD, so we increment line.
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
    }
    return true;
}

bool Hunk::merge(unsigned start_line, size_t pos)
{
    if (start_line == 0 || pos == 0)
        return false;
    size_t i;
    char c = gBuf[0];
    unsigned line = start_line;
    Mod * m = newMod(line, c);
    unsigned prev_line;
    for (i = 1; i < pos; i++) {
        prev_line = line;
        if (gBuf[i] != ADDC) { // don't increment line on ADD
                line++;
        }
        if (c != gBuf[i]) {
            m->scope.end = prev_line; // only update scope end for non-add
            c = gBuf[i];
            mods.push_back(m);
            m = newMod(line, c);
        }
    }
    mods.push_back(m);
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
        warn("Decoder is NULL");
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
    unsigned no = 0;
    const char *s = line + PLEN;
    char c;
    int n;
    while ((c = *s++) != '\0')
    {
        n = c - '0';
        if (n < 0 || n > 9) {
            decoder->syntaxError("Invalid hunk line number");
        }
        lineno = no * 10 + n;
        if (lineno / 10 != no) {
            decoder->syntaxError("Hunk line number too big");
        }
        no = lineno;
    }
    if (lineno <= 0) {
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
    hunk = new Hunk(lineno, line, chars_read);
    if (hunk == NULL) {
        diegrace("out of memory");
    }
    return hunk;
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
        warn("Decoder is null");
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
        warn("File pointer is null");
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



