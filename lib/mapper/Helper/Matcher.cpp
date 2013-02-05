#include "commons/handy.h"
#include "mapper/Matcher.h"

static bool LOCAL_DEBUG = true;

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
    if (LOCAL_DEBUG) {
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

bool ScopeInfoFinder::getLoopScope(Scope & scope, Loop * L) 
{
  if (L == NULL)
    return false;
  BasicBlock * header= L->getHeader();
  if (getBlockScope(scope, header)) {
    /*
Wrong: use the last instruction of the last BB to approximate ending scope

The last BB in Loop may not be last based on location.

BasicBlock *back  = L->getBlocks().back();
Scope es;
if (getBlockScope(es, back)) {
scope.end = es.end; 
return true;
}
*/

  /* The safe way: iterate all BBs and get the last inst. 
   * with largest line number.
   */
  unsigned end = 0;
  for (Loop::block_iterator LI = L->block_begin(), LE = L->block_end();
      LI != LE; LI++) {
    Scope es;
    if (getBlockScope(es, *LI)) {
      //errs() << es << "\n";
      if (es.end > end) {
        end = es.end;
      }
    }
  }
scope.end = end;
return true;

}
return false;
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
void Matcher::processLoops(LoopInfo & li)
{
  Scope scope;
  for (LoopInfo::iterator LII = li.begin(),  LIE = li.end(); LII != LIE; LII++) {
    // dump loops including all subloops
    // (*LII)->dump();
    if (ScopeInfoFinder::getLoopScope(scope, *LII)) { //Top level loops
      errs() << scope << "\n";
    }
    for (Loop::iterator LIBI = (*LII)->begin(), LIBE = (*LII)->end(); LIBI != LIBE; LIBI++) {
      if (ScopeInfoFinder::getLoopScope(scope, *LIBI)) { //Sub loops
        errs() << scope << "\n";
      }
    }
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
  if (LOCAL_DEBUG) {
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
      //MySPs.push_back(DIS);
    }
  }
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

  /*
     for (DebugInfoFinder::iterator I = sp_begin(), E = sp_end(); I != E; I++) {
     DISubprogram DIS(*I);
     errs() << "@" << DIS.getDirectory() << "/" << DIS.getFilename() << 
     ":" << DIS.getLineNumber() << "# " << DIS.getLinkageName() << 
     "(" << DIS.getName() << ") \n";
     }
     */

  ////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////


  /** DIY SP finder **/
  MySPs.clear();
  if (NamedMDNode *CU_Nodes = M.getNamedMetadata("llvm.dbg.cu"))
    for (unsigned i = 0, e = CU_Nodes->getNumOperands(); i != e; ++i) {
      DICompileUnit DICU(CU_Nodes->getOperand(i));
      if (LOCAL_DEBUG)
        errs() << "CU: " << DICU.getDirectory() << "/" << DICU.getFilename() << "\n";
      processSubprograms(DICU);
    }

  if (NamedMDNode *NMD = M.getNamedMetadata("llvm.dbg.sp"))
    for (unsigned i = 0, e = NMD->getNumOperands(); i != e; ++i) {
      DISubprogram DIS(NMD->getOperand(i));
      errs() << "From SP!! \n";
      if (LOCAL_DEBUG)
        errs() << "DIS: " << DIS.getName() << ", " << DIS.getDisplayName() << "\n";
    }

  /** Sort based on file name, directory and line number **/
  std::sort(MySPs.begin(), MySPs.end(), cmpDISPCopy);
  if (LOCAL_DEBUG)
    dumpSPs();
}

void Matcher::processDomTree(DominatorTree & DT)
{
  /** BFS Traversal  **/
  BBNode *Node = DT.getRootNode();
  std::deque< Pair<BBNode *, unsigned> > ques;
  ques.push_back(Pair<BBNode *, unsigned>(Node, 1));
  while (!ques.empty()) {
    Pair<BBNode *, unsigned> pair = ques.front();
    Node = pair.first;
    ques.pop_front();
    if (Node) {
      BasicBlock * BB = Node->getBlock();
      errs() << "[" << pair.second << "] " << BB->getName();
      Scope scope;
      if (ScopeInfoFinder::getBlockScope(scope, BB)) {
        errs() << ": " << scope << "\n";
      }
      for (BBNode::iterator BI = Node->begin(), BE = Node->end(); BI != BE; BI++) {
        BBNode * N = *BI;
        ques.push_back(Pair<BBNode *, unsigned>(N, pair.second + 1));
      }
    }
  }
}


Matcher::cu_iterator Matcher::matchCompileUnit(StringRef fullname)
{
  if (!initName(fullname))
    return MyCUs.end();

  initialized = true;

  /* TODO use binary search here */

  cu_iterator I = cu_begin(), E = cu_end();

  while(I != E) {
    std::string debugname = I->getDirectory().str() + "/" + I->getFilename().str();
    if (pathneq(debugname.c_str(), patchname, debugstrips))
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
  patchname = stripname(filename.c_str(), patchstrips);
  if (strlen(patchname) == 0) {
    errs() << "Warning: patchname is empty after strip\n";
    return false;
  }
  return true;
}

Matcher::sp_iterator Matcher::initMatch(cu_iterator & ci)
{
  if (!initialized) {
    errs() << "Matcher is not initialized\n";
    return MySPs.end();
  }
  MySPs.clear();
  processSubprograms(*ci);
  std::sort(MySPs.begin(), MySPs.end(), cmpDISPCopy);
  if (LOCAL_DEBUG) 
    dumpSPs();
  // shouldn't just return MySPs.begin(). because a CU may contain SPs from other CUs.
  return initMatch(filename); 
}

Matcher::sp_iterator Matcher::initMatch(StringRef fname)
{
  if (!processed) {
    errs() << "Warning: Matcher hasn't processed module\n";
    return sp_end();
  }
  if (fname.empty()) {
    patchname="";
    initialized = true;
    return sp_begin();
  }
  if (!initialized) {
    // If argument is empty, we assume it's a self-testing:
    // i.e., the beginning of the compilation unit will be
    // used.
    if (!initName(fname))
      return sp_end();
    initialized = true;
  }
  sp_iterator I = sp_begin(), E = sp_end();

  while(I != E) {
    std::string debugname = I->directory + "/" + I->filename;
    if (pathneq(debugname.c_str(), patchname, debugstrips)) {
      break;
    }
    I++;
  }
  if (I == E)
    errs() << "Warning: no matching file(" << patchname << ") was found in the CU\n";
  return I;
}

Instruction * Matcher::matchInstruction(inst_iterator &fi, Function * f, Scope & scope)
{
  Instruction * inst = NULL;
  unsigned line = scope.begin;
  for (inst_iterator fe = inst_end(f); fi != fe; ++fi) {
    inst = &*fi;
    unsigned l = ScopeInfoFinder::getInstLine(inst);
    if (l == 0)
      continue;
    if (l == line)
      return inst;
    if (l > line) {
      while (line < l && line <= scope.end)
        line++;
      if (line > scope.end || line > l)
        return NULL; // already passed
      return inst;
    }
  }
  return NULL;
}

Loop * Matcher::matchLoop(LoopInfo &li, const Scope & scope)
{
  Scope ls;
  Loop * found = NULL;
  //TODO make loop matching also progressive
  for (LoopInfo::iterator LII = li.begin(),  LIE = li.end(); LII != LIE; LII++) {
    // dump loops including all subloops
    // (*LII)->dump();
    if (ScopeInfoFinder::getLoopScope(ls, *LII)) { //Top level loops
      if (LOCAL_DEBUG)
        errs() << ls<< "\n";
    }
    if (ls.intersects(scope)) {
      //if (ls.includes(scope)) {
      found = *LII; // at least should be this Top level loop
      for (Loop::iterator LIBI = (*LII)->begin(), LIBE = (*LII)->end(); LIBI != LIBE; LIBI++) {
        if (ScopeInfoFinder::getLoopScope(ls, *LIBI)) { //Sub loops
          if (LOCAL_DEBUG)
            errs() << ls << "\n";
        }
        if (ls.intersects(scope))
          //if (ls.includes(scope))
          found = *LIBI; // return the innermost loop
      }
      break; // Return the first matching in top level
    }
    }
    return found;
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
  Function * Matcher::matchFunction(sp_iterator & I, Scope &scope)
  {
    if (!initialized) {
      errs() << "Matcher is not initialized\n";
      return NULL;
    }
    // hit the boundary
    if (scope.begin == 0 || scope.end == 0 || scope.end < scope.begin) {
      return NULL;
    }

    /** Off-the-shelf SP finder **/
    sp_iterator E = sp_end();
    while (I != E) {
      if (strlen(patchname) != 0) {
        std::string debugname = I->directory + "/" + I->filename;
        if (!pathneq(debugname.c_str(), patchname,  debugstrips)) {
          errs() << "Warning: Reaching the end of " << patchname << " in current CU\n";
          return NULL;
        }
      }
      if (I->lastline == 0) {
        if (I + 1 == E)
          return I->function; // It's tricky to return I here. Maybe NULL is better
        // Line number is guaranteed to be positive, no need to check overflow here.
        I->lastline = (I + 1)->linenumber - 1;             
        assert(I->lastline >= I->linenumber); // Unless the two are modifying the same line.
      }
      // For boundary case, we only break if that function is one line function.
      if (I->lastline > scope.begin || (I->lastline == scope.begin && I->lastline == I->linenumber))
        break;
      I++;
    }
    if (I == E)
      return NULL;
    // Lies in the gap
    if (I->linenumber > scope.end || (I->linenumber == scope.end && I->lastline > I->linenumber))
      return NULL;
    if (I->lastline < scope.end)
      scope.begin = I->lastline + 1;  // Multiple functions
    else
      scope.begin = 0;
    return I->function;
  }

  /**
   * @Deprecated
   *
   */
  Function * Matcher::__matchFunction(sp_iterator I, Scope &scope)
  {
    if (!initialized) {
      errs() << "Matcher is not initialized\n";
      return NULL;
    }
    // hit the boundary
    if (scope.begin == 0 || scope.end == 0 || scope.end < scope.begin) {
      initialized = false;
      return NULL;
    }
    /** Off-the-shelf SP finder **/
    unsigned long e;
    Function *f1 = NULL, *f2 = NULL;
    sp_iterator E;
    patchname = stripname(filename.c_str(), patchstrips);
    for (E = sp_end(); I != E; I++) {
      //std::string debugname = I->getDirectory().str() + "/" + I->getFilename().str();
      std::string debugname = I->directory + "/" + I->filename;
      if (!pathneq(debugname.c_str(), patchname,  debugstrips))
        continue; // Should break here, because initMatch already adjust the iterator to the matching file.
      //e = I->getLineNumber();
      e = I->linenumber;
      f1 = f2;
      //f2 = I->getFunction();
      f2 = I->function;
      if (scope.begin < e) {
        if (f1 == NULL) { 
          // boundary case, the modification begins before the first function
          // we need to adjust the beginning to the first function and let the
          // iteration continue
          scope.begin = e; 
        }
        else {
          break;
        }
      }
    }

    /*****
     *
     * typical case #1:
     *
     * | foo1
     * | foo2   <- b, f1
     * | [scope.begin
     * | scope.end]
     * | foo3   <- e, f2
     * | foo4
     * # scope is modifying foo2;
     *
     * typical case #2:
     *
     * | foo1
     * | foo2   <- b, f1
     * | [scope.begin
     * | foo3   <- e, f2
     * | scope.end]
     * | foo4
     * # scope is modifying foo2 & foo3;
     *
     * boundary case 1:
     *
     * | 0      <- b, f1
     * | [scope.begin
     * | scope.end]
     * | foo1   <- e, f2 # advance scope.begin
     * | foo2
     *
     * boundary case 2:
     * | 0      <- b, f1
     * | [scope.begin
     * | foo1   <- e, f2 # advance scope.begin
     * | foo2
     * | scope.end]
     * | foo3
     * | foo4
     *
     * boundary case 3:
     * | 0      <- b, f1
     * | [scope.begin
     * | foo1   <- e, f2 # advance scope.begin, adjust f1 to f2
     * | scope.end]
     *
     * boundary case 4:
     * | 0
     * | foo1   <- b, f1   
     * | foo2   <- e, f2 # adjust f1 to f2
     * | [scope.begin
     * | scope.end]
     *
     *****/

    if (scope.end < scope.begin) { // we over-advanced scope.begin, boundary case #1
      initialized = false;
      return NULL; 
    }

    if (I == E) { 
      // we've come to the end instead of jumping out from break, 
      // need to adjust f1 to f2, boundary case #3, #4
      // also, make sure we finish the matching
      scope.end = 0; 
      return f2;
    }

    if (scope.end > e) { // span multiple functions
      scope.begin = e; // boundary case #2
    }
    else { // span at most one function 
      scope.end = 0; // finish the matching
    }
    return f1;
  }

  void Matcher::succTraversal(Function *F)
  {
    for (Function::iterator FI = F->begin(), FE = F->end(); FI != FE; FI++) {
      /** Traverse the successors **/
      errs() << FI->getName() << "\n";
      for (succ_iterator SI = succ_begin(FI), SE = succ_end(FI); SI != SE; SI++) {
        errs() << "\t" << (*SI)->getName() << "\n";
      }
    }
  }


  void Matcher::preTraversal(Function *F)
  {
    std::deque< Pair<Function::iterator, unsigned> > ques;
    BasicBlock *BB = F->begin();
    SmallPtrSet<const BasicBlock *, 8> Visited;
    Visited.insert(BB);
    ques.push_back(Pair<Function::iterator, unsigned>(BB, 1));
    /** BFS traversal **/
    while(!ques.empty()) {
      Pair<Function::iterator, unsigned> pair = ques.front();
      BB = pair.first;
      ques.pop_front();
      if (BB) {
        errs() << "[" << pair.second << "] " << BB->getName();
        Scope scope;
        if (ScopeInfoFinder::getBlockScope(scope, BB)) {
          errs() << ": " << scope << "\n";
        }
        for (succ_iterator SI = succ_begin(pair.first), SE = succ_end(pair.first); SI != SE; SI++) {
          BB = *SI;
          if (!Visited.count(BB)) {
            ques.push_back(Pair<Function::iterator, unsigned>(BB, pair.second + 1));
            Visited.insert(BB);
          }
        }
      }
    }
  }
