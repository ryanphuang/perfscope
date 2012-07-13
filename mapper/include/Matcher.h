//===---- Matcher.h - Match Scope To IR Elements----*- C++ -*-===//
//
//===----------------------------------------------------------------------===//
#ifndef ___MATCHER__H_
#define ___MATCHER__H_

#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"

#include "llvm/Analysis/DebugInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Metadata.h"
#include "llvm/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/Support/CFG.h"
#include "llvm/Constants.h"
#include "llvm/Instruction.h"
#include "llvm/InstrTypes.h"

#include <vector>
#include <deque>

#include "PatchDecoder.h"

using namespace llvm;

typedef DomTreeNodeBase<BasicBlock> BBNode;

template <class T1, class T2> struct Pair
{
  typedef T1 first_type;
  typedef T2 second_type;

  T1 first;
  T2 second;

  Pair() : first(T1()), second(T2()) {}
  Pair(const T1& x, const T2& y) : first(x), second(y) {}
  template <class U, class V> Pair (const Pair<U,V> &p) : first(p.first), second(p.second) { }
};

typedef struct MetadataElement {
    unsigned key;
    DIDescriptor value;
} MetadataElement;

typedef struct MetadataNode {
    MetadataNode *parent;
    MetadataNode *left;
    MetadataNode *right;
    MetadataElement Element;
} MetadataNode;

class MetadataTree {
    public:
        MetadataNode *root;
        void insert(MetadataNode *);
        MetadataNode *search(unsigned key);
};

bool cmpDIS(const DISubprogram &, const DISubprogram &);
bool skipFunction(Function *);


class ScopeInfoFinder {
    protected:
        //DebugInfoFinder Finder;
        std::vector<DISubprogram> MySPs;
    public:
        typedef std::vector<DISubprogram>::const_iterator sp_iterator;

    public:

        void processSubprograms(Module &M);
        void processSubprograms();
        void processInst(Function *);
        void processBasicBlock(Function *);
        void processLoops(LoopInfo &);
        void processDomTree(DominatorTree &);

        static unsigned getInstLine(Instruction *);
        static bool getBlockScope(Scope & , BasicBlock *);
        static bool getLoopScope(Scope & , Loop *);


        sp_iterator subprogram_begin() const { return MySPs.begin(); }
        sp_iterator subprogram_end() const { return MySPs.end(); }
};

class Matcher {
    public:
        int patchstrips;
        int debugstrips;

    protected:
        ScopeInfoFinder Finder;
        bool initialized;
        std::string filename;

    public:
        Matcher(Module &M, int p_strips = 0, int d_strips = 0) 
        {
            patchstrips = p_strips; 
            debugstrips = d_strips; 
            initialized = false;
            init(M); 
        }
        Matcher() {initialized = false; patchstrips = 0; debugstrips = 0; }
        
        void init(Module &M) { Finder.processSubprograms(M); } 
        void setstrips(int p_strips, int d_strips) 
        {
            patchstrips = p_strips; 
            debugstrips = d_strips; 
        }

        ScopeInfoFinder::sp_iterator initMatch(StringRef filename);


        Function * matchFunction(ScopeInfoFinder::sp_iterator, Scope &);

        static Loop * matchLoop(LoopInfo &li, Scope &);

        ScopeInfoFinder & getFinder() { return Finder; }


        void preTraversal(Function *);
        void succTraversal(Function *);

};

#endif
