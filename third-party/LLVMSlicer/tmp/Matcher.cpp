//===---- Matcher.h - Match Scope To IR Elements----*- C++ -*-===//
//
// Author: Ryan Huang <ryanhuang@cs.ucsd.edu>
//
//===----------------------------------------------------------------------===//
//

#include "Matcher.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <errno.h>


#define __set_errno(e) (errno = (e))
#define MAX_PATH 512
#define ISSLASH(C) ((C) == '/')

static bool DEBUG_MATCHER = false;
static char PBUF[MAX_PATH];


void * xmalloc(size_t size)
{
    if (size <= 0) {
        fprintf(stderr, "Invalid malloc size %lu", size);
        exit(1);
    }
    void *p = malloc(size);
    if (p == NULL) {
        fprintf(stderr, "Out of memory");
        exit(1);
    }
    return p;
}

const char * stripname(const char *name, int strips)
{
    if (strips == 0)
        return name;
    const char * str = name;
    const char * p = name;
    int left = strips;
    while(*str) {
        if (ISSLASH(*str)) {
            while(ISSLASH(str[1])) // care, e,g, a//b/c//
                str++;
            if (strips < 0 || --left >= 0) {
                p = str + 1;
            }
            if (left == 0)
                break;
        }
        str++;
    }
    return p;
}

/* Return the canonical absolute name of file NAME.  A canonical name
   does not contain any `.', `..' components nor any repeated path
   separators ('/').  
   Differences from the libc's realpath: 
   1). do not expand symlink
   2). do not add pwd to the name
   3). do not check for existence of each component

   The following implementation is modified from glibc 2.9(canonicalize.c)
*/

char * canonpath (const char *name, char *resolved)
{
    char *rpath, *dest;
    const char *start, *end, *rpath_limit;
    long int path_max;

    if (name == NULL)
    {
      /* As per Single Unix Specification V2 we must return an error if
         either parameter is a null pointer.  We extend this to allow
         the RESOLVED parameter to be NULL in case the we are expected to
         allocate the room for the return value.  */
      __set_errno (EINVAL);
      return NULL;
    }

    if (name[0] == '\0')
    {
      /* As per Single Unix Specification V2 we must return an error if
         the name argument points to an empty string.  */
      __set_errno (ENOENT);
      return NULL;
    }

    path_max = MAX_PATH;

    if (resolved == NULL)
    {
      rpath = (char *) xmalloc (path_max);
    }
    else
        rpath = resolved;
    rpath_limit = rpath + path_max;

    if (name[0] == '/') {
      rpath[0] = '/';
      dest = rpath + 1;
    }
    else {
      // here the `correct' implementation should be the 
      // following commented one
      // but we especially are interested when name is not
      // rooted at `/'
      // This is dangerous if name is illegal such as '../gcc/xxx'
      // For now we just check the simple error. 
      // TODO
      /*
      if (!getcwd(rpath, path_max)) {
        rpath[0] = '\0';
        goto error;
      }
      dest = strchr(rpath, '\0');
      */
      if (name[0] == '.' && name[1] == '.')
        goto error;
      dest = rpath;
    }

    for (start = end = name; *start; start = end)
    {
      /* Skip sequence of multiple path-separators.  */
      while (*start == '/')
        ++start;
      /* Find end of path component.  */
      for (end = start; *end && *end != '/'; ++end)
        /* Nothing.  */;
      if (end - start == 0)
        break;
      else if (end - start == 1 && start[0] == '.')
        /* nothing */;
      else if (end - start == 2 && start[0] == '.' && start[1] == '.')
      {
        /* Back up to previous component, ignore if at root already.  */
        while ((dest > rpath + 1) && (--dest)[-1] != '/');
        if (dest[-1] != '/')
          --dest; //move to the beginning
      }
      else
      {
        size_t new_size;
        if (dest > rpath && dest[-1] != '/')
          *dest++ = '/';
        if (dest + (end - start) >= rpath_limit)
        {
          off_t dest_offset = dest - rpath;
          char *new_rpath;
          if (resolved)
          {
            __set_errno (ENAMETOOLONG);
            if (dest > rpath + 1)
              dest--;
            *dest = '\0';
            goto error;
          }
          new_size = rpath_limit - rpath;
          if (end - start + 1 > path_max)
            new_size += end - start + 1;
          else
            new_size += path_max;
          new_rpath = (char *) realloc (rpath, new_size);
          if (new_rpath == NULL)
            goto error;
          rpath = new_rpath;
          rpath_limit = rpath + new_size;
          dest = rpath + dest_offset;
        }

        mempcpy (dest, start, end - start);
        dest += (end - start);
        *dest = '\0';
      }
    }
    if (dest > rpath + 1 && dest[-1] == '/')
      --dest;
    *dest = '\0';

    assert (resolved == NULL || resolved == rpath);
    return rpath;

error:
    assert (resolved == NULL || resolved == rpath);
    if (resolved == NULL)
      free (rpath);
    fprintf(stderr, "fatal: %s\n", name);
    return NULL;
}

bool endswith(const char *path1, const char *path2)
{
  if (canonpath(path1, PBUF)) {
    int len1 = strlen(PBUF);
    int len2 = strlen(path2);
    if (len1 < len2)
      return false;
    int i = len1 - len2;
    int j = 0;
    if (PBUF[0] == '/' && i > 0 && path2[j] != '/' && PBUF[i - 1] != '/') {
      // A: /tmp/atest.c
      // B: test.c
      return false;
    }
    while (i < len1 && j < len2 && PBUF[i] == path2[j]) {
      i++;
      j++;
    }
    if (i != len1) {
      return false;
    }
    return true;
  }
  else
    return false;
}

bool pathneq(const char *path1, const char *path2, int n)
{
    if (canonpath(path1, PBUF)) {
        return strcmp(stripname(PBUF, n), path2) == 0;
    }
    else
        return false;
}

bool cmpDICU(const DICompileUnit & CU1, const DICompileUnit & CU2) 
{ 
  int cmp = CU1.getDirectory().compare(CU2.getDirectory());
  if (cmp == 0)
    cmp = CU1.getFilename().compare(CU2.getFilename());
  return cmp >= 0 ? false : true;
}

bool cmpDISP(const DISubprogram & SP1, const DISubprogram & SP2) 
{ 
  int cmp = SP1.getDirectory().compare(SP2.getDirectory());
  if (cmp == 0) {
    cmp = SP1.getFilename().compare(SP2.getFilename());
    if (cmp == 0) {
      cmp = SP1.getLineNumber() - SP2.getLineNumber();
    }
  }
  return cmp >= 0 ? false : true;
}

bool cmpDISPCopy(const DISPCopy & SP1, const DISPCopy & SP2) 
{ 
  int cmp = SP1.directory.compare(SP2.directory);
  if (cmp == 0) {
    cmp = SP1.filename.compare(SP2.filename);
    if (cmp == 0) {
      cmp = SP1.linenumber - SP2.linenumber;
    }
  }
  return cmp >= 0 ? false : true;
}

bool skipFunction(Function *F)
{
  // Skip intrinsic functions and function declaration because DT only 
  // works with function definition.
  if (F == NULL || F->getName().startswith("llvm.dbg") || 
      F->isDeclaration()) 
    return true;
  return false;
}

unsigned ScopeInfoFinder::getInstLine(const Instruction *I)
{
  DebugLoc Loc = I->getDebugLoc();
  if (Loc.isUnknown()) {
    if (DEBUG_MATCHER) {
      errs() << "Unknown LOC" << "\n";
    }
    return 0;
  }
  return Loc.getLine();
}

unsigned ScopeInfoFinder::getLastLine(Function *F)
{
  if (F == NULL || F->begin() == F->end()) //empty block
    return 0;
  const BasicBlock & BB = F->back();
  const Instruction & I = BB.back();
  DebugLoc Loc = I.getDebugLoc();
  if (Loc.isUnknown()) 
    return 0;
  return Loc.getLine();
}

bool ScopeInfoFinder::getBlockScope(Scope & scope, BasicBlock *B)
{
  if (B->begin() == B->end()) // empty block
    return false;

  /** Use first and last instruction to get the scope information **/
  Instruction *first = & B->front();
  Instruction *last = & B->back();
  if (first == NULL || last == NULL) {
    errs() << "NULL scope instructions " << "\n";
    return false;
  }
  unsigned b = getInstLine(first);
  unsigned e = getInstLine(last);
  if (b == 0 || e == 0) {
    return false;
  }
  scope.begin = b;
  scope.end = e;
  return true;
}

void Matcher::processInst(Function *F)
{
  for (Function::iterator FI = F->begin(), FE = F->end(); FI != FE; FI++) {
    /** Get each instruction's scope information **/
    for (BasicBlock::iterator BI = FI->begin(), BE = FI->end(); BI != BE; BI++) {
      DebugLoc Loc = BI->getDebugLoc();
      if (Loc.isUnknown())
        continue;
      LLVMContext & Ctx = BI->getContext();

      DIDescriptor Scope(Loc.getScope(Ctx));
      if (Scope.isLexicalBlock()) {
        DILexicalBlock DILB(Scope);
        errs() << "Block :" << DILB.getLineNumber() << ", " << DILB.getColumnNumber() << "\n";
      }
    }
  }
}

void Matcher::processBasicBlock(Function *F)
{
  for (Function::iterator FI = F->begin(), FE = F->end(); FI != FE; FI++) {
    /** Use first and last instruction to get the scope information **/
    Instruction *first = & FI->front();
    Instruction *last = & FI->back();
    if (first == NULL || last == NULL) {
      errs() << "NULL scope instructions " << "\n";
      continue;
    }
    DebugLoc Loc = first->getDebugLoc();
    if (Loc.isUnknown()) {
      errs() << "Unknown LOC information" << "\n";
      continue;
    }
    errs() << "Block :" << Loc.getLine();
    Loc = last->getDebugLoc();
    if (Loc.isUnknown()) {
      errs() << "Unknown LOC information" << "\n";
      continue;
    }
    errs() << ", " << Loc.getLine() << "\n";
  }
}

void Matcher::processCompileUnits(Module &M)
{
  MyCUs.clear();
  if (NamedMDNode *CU_Nodes = M.getNamedMetadata("llvm.dbg.cu"))
    for (unsigned i = 0, e = CU_Nodes->getNumOperands(); i != e; ++i) {
      DICompileUnit DICU(CU_Nodes->getOperand(i));
      if (DICU.getVersion() > LLVMDebugVersion10)
        MyCUs.push_back(DICU);
    }

  /** Sort based on file name, directory and line number **/
  std::sort(MyCUs.begin(), MyCUs.end(), cmpDICU);
  if (DEBUG_MATCHER) {
    cu_iterator I, E;
    for (I = MyCUs.begin(), E = MyCUs.end(); I != E; I++) {
      errs() << "CU: " << I->getDirectory() << "/" << I->getFilename() << "\n";
    }
  }
}

void Matcher::processSubprograms(DICompileUnit &DICU)
{
  if (DICU.getVersion() > LLVMDebugVersion10) {
    DIArray SPs = DICU.getSubprograms();
    for (unsigned i = 0, e = SPs.getNumElements(); i != e; i++) {
      DISubprogram DISP(SPs.getElement(i));
      DISPCopy Copy(DISP);
      if (Copy.name.empty() || Copy.filename.empty() || Copy.linenumber == 0)
        continue;
      Copy.lastline = ScopeInfoFinder::getLastLine(Copy.function);
      MySPs.push_back(Copy);
    }
  }
}

MDNode *Matcher::getFunctionMD(const Function *F) {
  // inefficient, only for infrequent query
  // simply look all subprograms
  if (!processed) // must process the module to build up the debug information
    return NULL;
  for (DebugInfoFinder::iterator i = Finder.subprogram_begin(), e = Finder.subprogram_end();
      i != e; ++i) {
    DISubprogram S(*i);
    if (S.getFunction() == F)
      return *i;
  }
  return NULL;
}

void Matcher::dumpSPs()
{
  sp_iterator I, E;
  for (I = MySPs.begin(), E = MySPs.end(); I != E; I++) {
    errs() << "@" << I->directory << "/" << I->filename;
    errs() << ":" << I->name;
    errs() << "([" << I->linenumber << "," << I->lastline << "]) \n";
  }
}

void Matcher::processSubprograms(Module &M)
{
  //////////////Off-the-shelf SP finder Begin//////////////////////
  ////////////////////////////////////////////////////////////////

  //place the following call before invoking this method
  //processModule(M);

  ////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////


  /** DIY SP finder **/
  MySPs.clear();
  if (NamedMDNode *CU_Nodes = M.getNamedMetadata("llvm.dbg.cu"))
    for (unsigned i = 0, e = CU_Nodes->getNumOperands(); i != e; ++i) {
      DICompileUnit DICU(CU_Nodes->getOperand(i));
      if (DEBUG_MATCHER)
        errs() << "CU: " << DICU.getDirectory() << "/" << DICU.getFilename() << "\n";
      processSubprograms(DICU);
    }

  if (NamedMDNode *NMD = M.getNamedMetadata("llvm.dbg.sp"))
    for (unsigned i = 0, e = NMD->getNumOperands(); i != e; ++i) {
      DISubprogram DIS(NMD->getOperand(i));
      errs() << "From SP!! \n";
      if (DEBUG_MATCHER)
        errs() << "DIS: " << DIS.getName() << ", " << DIS.getDisplayName() << "\n";
    }

  /** Sort based on file name, directory and line number **/
  std::sort(MySPs.begin(), MySPs.end(), cmpDISPCopy);
  if (DEBUG_MATCHER)
    dumpSPs();
}

Matcher::cu_iterator Matcher::matchCompileUnit(StringRef fullname)
{
  if (!initName(fullname))
    return MyCUs.end();

  initialized = true;

  /* TODO use binary search here */

  cu_iterator I = cu_begin(), E = cu_end();

  while(I != E) {
    std::string debugname;
    // Filename may already contains the path information
    if (I->getFilename().size() > 0 && I->getFilename()[0] == '/')
      debugname = I->getFilename();
    else
      debugname = I->getDirectory().str() + "/" + I->getFilename().str();
    if (endswith(debugname.c_str(), patchname))
      break;
    I++;
  }
  if (I == E)
    errs() << "Warning: no matching file(" << patchname << ") was found in the CUs\n";
  return I;
}

bool Matcher::initName(StringRef fname)
{
  char *canon = canonpath(fname.data(), NULL);  
  if (canon == NULL) {
    errs() << "Warning: patchname is NULL\n";
    return false;
  }
  filename.assign(canon);
  free(canon);
  patchname = stripname(filename.c_str(), patchstrips);
  if (strlen(patchname) == 0) {
    errs() << "Warning: patchname is empty after strip\n";
    return false;
  }
  return true;
}

Matcher::sp_iterator Matcher::setSourceFile(StringRef target)
{
  // If target is empty, we assume it's a self-testing:
  // i.e., the beginning of the compilation unit will be
  // used.
  if (target.empty()) {
    processSubprograms(module); 
    patchname="";
    initialized = true;
    return sp_begin();
  }
  std::string oldfile = filename;
  if (!initName(target))
    return sp_end();
  if (oldfile == filename) {
    if (DEBUG_MATCHER) 
      errs() << "Target source didn't change since last time, reuse old processing.\n";
  }
  else {
    cu_iterator ci = matchCompileUnit(target);
    if (ci == cu_end())
      return sp_end();
    MySPs.clear();
    processSubprograms(*ci);
    std::sort(MySPs.begin(), MySPs.end(), cmpDISPCopy);
    if (DEBUG_MATCHER) 
      dumpSPs();
  }
  initialized = true;
  // shouldn't just return MySPs.begin(). because a CU may contain SPs from other CUs.
  return slideToFile(filename); 
}

/* *
 * Adjust sp_iterator to the starting position of
 * the target source file region.
 *
 * Assumption: MySPs contains the sorted subprograms
 * */
Matcher::sp_iterator Matcher::slideToFile(StringRef fname)
{
  if (!processed) {
    errs() << "Warning: Matcher hasn't processed module\n";
    return sp_end();
  }
  sp_iterator I = sp_begin(), E = sp_end();

  while(I != E) {
    std::string debugname;
    if (I->filename.size() > 0 && I->filename[0] == '/')
      debugname = I->filename;
    else
      debugname = I->directory + "/" + I->filename;
    if (endswith(debugname.c_str(), patchname)) {
      break;
    }
    I++;
  }
  if (I == E)
    errs() << "Warning: no matching file(" << patchname << ") was found in the CU\n";
  return I;
}

Instruction * Matcher::matchInstruction(inst_iterator &ii, Function * f, Scope & scope)
{
  if (scope.begin > scope.end)
    return NULL;
  Instruction * inst = NULL;
  unsigned line = scope.begin;
  unsigned l = 0;
  inst_iterator ie = inst_end(f);
  for (; ii != ie; ++ii) {
    inst = &*ii;
    l = ScopeInfoFinder::getInstLine(inst);
    if (l == 0)
      continue;
    if (l == line) {
      break;
    }
    if (l > line) { // (*)
      // Didn't find any instruction at line but find
      // one within the range.
      // Mod: [.........]
      //         Inst
      if (l <= scope.end)
        break;
      return NULL; // already passed
    }
  }
  if (ii == ie)
    return NULL;
  ++ii; // adjust fi to the next instruction
  //TODO there could be multiple instructions at one line
  
  // Always assume the next instruction shares the same line.
  // If it doesn't, in the next call to matchInstruction,
  // begin will be adjusted to the next line by place (*)
  scope.begin = l; 
  return inst;
}


/** A progressive method to match the function(s) in a given scope.
 *  When there are more than one function in the scope, the first function
 *  will be returned and scope's beginning is *modified* to the end of this 
 *  returned function so that the caller could perform a loop of call until
 *  matchFunction return NULL;
 *
 *
 *  Note: finder.processModule(M) should be called before the first call of matchFunction.
 *
 * **/
Function * Matcher::matchFunction(sp_iterator & I, Scope &scope, bool & multiple)
{
  if (!initialized) {
    errs() << "Matcher is not initialized\n";
    return NULL;
  }
  // hit the boundary
  if (scope.end < scope.begin) {
    return NULL;
  }
  /** Off-the-shelf SP finder **/
  sp_iterator E = sp_end();
  while (I != E) {
    if (strlen(patchname) != 0) {
      std::string debugname = I->directory + "/" + I->filename;
      if (I->filename.size() > 0 && I->filename[0] == '/')
        debugname = I->filename;
      else
        debugname = I->directory + "/" + I->filename;
      if (!endswith(debugname.c_str(), patchname)) {
        errs() << "Warning: Reaching the end of " << patchname << " in current CU\n";
        return NULL;
      }
    }
    if (I->lastline == 0) {
      if (I + 1 == E)
        return I->function; // It's tricky to return I here. Maybe NULL is better
      // Line number is guaranteed to be positive, no need to check overflow here.
      if (I->linenumber == (I + 1)->linenumber) {
        errs() << "Warning two functions overlap: ";
        if (I->function && (I + 1)->function) {
          errs() << I->function->getName() << ", ";
          errs() << (I + 1)->function->getName();
        }
        errs() << "\n";
        I->lastline = (I + 1)->linenumber;
      }
      else
        I->lastline = (I + 1)->linenumber - 1;             
      if (I->lastline < I->linenumber) {
        errs() << "Bad things happened in " << patchname << ":" << scope << ", " <<
           I->lastline << "," << I->linenumber << "\n";
        assert(false); // Unless the two are modifying the same line.
      }
    }
    // For boundary case, we only break if that function is one line function.
    if (I->lastline > scope.begin || (I->lastline == scope.begin && I->lastline == I->linenumber))
      break;
    I++;
  }
  if (I == E)
    return NULL;

  //
  //                 |  f1   |        Cases of scope: 
  //  f1.lastline -> |_______|      (1)  (2)  (3)  (4)  (5)
  //                 +       +       ^    ^              ^
  //                 +  GAP  +       |    |              |
  //                 +       +       v    |              |
  // f2.linenumber-> ---------            |              |
  //                 |       |            v    ^    ^    |
  //                 |  f2   |                 |    |    |
  //  f2.lastline -> |_______|                 v    |    |
  //                 +       +                      |    |
  //                    ...                         v    v
  //

  // Case (1)
  if (I->linenumber > scope.end || (I->linenumber == scope.end && I->lastline > I->linenumber))
    return NULL;
  if (I->lastline < scope.end) { // Case (4), (5)
    scope.begin = I->lastline + 1;  // adjust beginning to next
    multiple = true;
  }
  multiple = false;
  return I->function; 
}
