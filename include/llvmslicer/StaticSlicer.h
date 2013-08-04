#ifndef SLICING_STATICSLICER_H
#define SLICING_STATICSLICER_H

#include <map>
#include <set>
#include <vector>
#include <iterator>
#include <algorithm>

#include "llvm/ADT/STLExtras.h" /* tie */
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/CallGraph.h"

#include "mapper/Matcher.h"
#include "llvmslicer/FunctionStaticSlicer.h"
#include "llvmslicer/Callgraph.h"
#include "llvmslicer/PointsTo.h"
#include "llvmslicer/LLVM.h"
#include "llvmslicer/LLVMSupport.h"

// #define DEBUG_SLICER
// #define DEBUG_EMIT

namespace llvm { namespace slicing {
  typedef llvm::SmallVector<const llvm::Function *, 20> WorkList;
  typedef WorkList::iterator FuncIter;

  class StaticSlicer : public ModulePass {
    public:
      static char ID;
      static const char * PassName; 

      typedef std::map<llvm::Function const*, FunctionStaticSlicer *> Slicers;
      typedef std::multimap<llvm::Function const*,llvm::CallInst const*>
        FuncsToCalls;
      typedef std::multimap<llvm::CallInst const*,llvm::Function const*>
        CallsToFuncs;

    public:
      StaticSlicer(bool forward, bool sound = false);

      ~StaticSlicer();

      virtual bool runOnModule(Module &M);
      virtual const char *getPassName() const { return PassName;}
      virtual void getAnalysisUsage(AnalysisUsage &AU) const {
        AU.setPreservesAll();
        AU.addRequired<PostDominatorTree>();
        AU.addRequired<PostDominanceFrontier>();
        AU.addRequired<CallGraph>();
      }

      template<typename OutIterator>
      void addCriteria(Function *F, OutIterator I, OutIterator E)
      {
        m_criteriaInit = true;
        if (F->isIntrinsic()) // don't do anything for intrinsic
          return;
#ifdef DEBUG_SLICER
        errs() << "add criteria from " << F << "@" << F->getName() << "\n";
#endif
        FunctionStaticSlicer *FSS = getFSS(F);
        for (OutIterator i = I; i != E; ++i) {
#ifdef DEBUG_SLICER
          (*i)->dump(); 
#endif
          addInitRC(FSS, *i);
        }
        m_initFuns.push_back(F);
      }

      void computeSlice();

      const Instruction * next();
    private:
      void addInitRC(FunctionStaticSlicer *FSS, const Instruction *inst);

    private:
      typedef llvm::SmallVector<const llvm::Function *, 20> InitFuns;
    
      void parseInitialCriterion();

      FunctionStaticSlicer * getFSS(const Function * F);
      FunctionStaticSlicer * newFSS(Function * F);
      void buildDicts(const ptr::PointsToSets &PS);

      template<typename OutIterator>
        void emitToCalls(llvm::Function const* const f, OutIterator out);

      template<typename OutIterator>
        void emitToExits(llvm::Function const* const f, OutIterator out);

      template<typename OutIterator>
        void emitToForwardExits(llvm::Function const* const f, OutIterator out);

      template<typename OutIterator>
        void emitToForwardCalls(llvm::Function const* const f, OutIterator out);

      Module *m_module;
      bool m_forward;
      Slicers m_slicers;
      InitFuns m_initFuns;
      WorkList m_sliceFuncs;
      FuncsToCalls m_funcsToCalls;
      CallsToFuncs m_callsToFuncs;
      FuncIter m_funci;
      const_inst_iterator m_insti;
      ptr::PointsToSets * m_ps;
      callgraph::Callgraph * m_cg;
      mods::Modifies * m_mod;
      bool m_criteriaInit;
      bool m_instInit;
      bool m_funcInit;
      bool m_sound;
  };


}}

namespace llvm { namespace slicing { namespace detail {

  typedef std::map<llvm::Value const*,llvm::Value const*>
    ParamsToArgs;

  void mapArgsToParams(CallInst const* const C,
      Function const* const F,
      ParamsToArgs& toArgs);

  void fillParamsToArgs(llvm::CallInst const* const C,
      llvm::Function const* const F,
      ParamsToArgs& toArgs);

  template<typename RelevantsIterator, typename OutIterator>
    void getRelevantVarsAtCall(llvm::CallInst const* const C,
        llvm::Function const* const F,
        RelevantsIterator b, RelevantsIterator const e,
        //PointsToSets const& PS,
        OutIterator out) {
      assert(!isInlineAssembly(C) && "Inline assembly is not supported!");
      ParamsToArgs toArgs;
      fillParamsToArgs(C,F,toArgs);
      for ( ; b != e; ++b)
      {
        ParamsToArgs::const_iterator it = toArgs.find(*b);
        if (it != toArgs.end())
          *out++ = it->second;
        else if (!isLocalToFunction(*b,F))
          *out++ = *b;
      }
    }

  template<typename RelevantsIterator, typename OutIterator>
    void getRelevantVarsAtExit(llvm::CallInst const* const C,
        llvm::ReturnInst const* const R,
        RelevantsIterator b, RelevantsIterator const e,
        OutIterator out) {
      assert(!isInlineAssembly(C) && "Inline assembly is not supported!");
      if (callToVoidFunction(C)) {
        std::copy(b, e, out);
        return;
      }
      for ( ; b != e; ++b)
        if (*b == C) {
          Value *ret = R->getReturnValue();
          if (!ret) {
            return;
          }
#ifdef DEBUG_EMIT
          errs() << "\trel == callinst: add new";
          R->dump();
#endif
          *out++ = ret;
        } else {
#ifdef DEBUG_EMIT
#endif
          errs() << "\trel != callinst: add orig";
          (*b)->dump();
          *out++ = *b;
        }
    }

}}}

namespace llvm { namespace slicing {
  template<typename OutIterator>
    void StaticSlicer::emitToForwardCalls(llvm::Function const* const f,
        OutIterator out) {
#ifdef DEBUG_EMIT
      errs() << __func__ << " for " << f->getName() << "\n";
#endif
      if (isVoidFunction(f)) {
#ifdef DEBUG_EMIT
        errs() << " skip void function\n";
#endif
        return;
      }
      if (m_funcsToCalls.empty()) {
        errs() << "Empty <func, call> map\n";
        return;
      }
      typedef std::vector<const llvm::ReturnInst *> ExitsVec;
      ExitsVec E;
      getFunctionExits(f, std::back_inserter(E));
      FunctionStaticSlicer * fss = getFSS(f);
      bool returnAffected = false;
      for (ExitsVec::const_iterator ei = E.begin(); ei != E.end(); ++ei) {
        if (!fss->isSliced(*ei)) {
          returnAffected = true;
          break;
        }
      }
      // if no return statement is affected, skip UP()
      if (!returnAffected) 
        return;
      FuncsToCalls::const_iterator c, e;
      llvm::tie(c,e) = m_funcsToCalls.equal_range(f);
      for ( ; c != e; ++c) {
        const llvm::CallInst *CI = c->second;
        // if a call is void call
        if (callToVoidFunction(CI)) {
#ifdef DEBUG_EMIT
          errs() << " skip void call";
          CI->print(errs());
          errs() << "\n";
          continue;
#endif
        }
        const llvm::Function *g = CI->getParent()->getParent();
#ifdef DEBUG_EMIT
        errs() << " caller " << g->getName() << "\n";
#endif
        FunctionStaticSlicer *FSS = getFSS(g);
        if (FSS->addCriterion(CI, FSS->DEF_begin(CI), FSS->DEF_end(CI))) {
          *out++ = g;
        }
      }
    }

  template<typename OutIterator>
    void StaticSlicer::emitToCalls(llvm::Function const* const f,
        OutIterator out) {
#ifdef DEBUG_EMIT
      errs() << __func__ << " for " << f->getName() << "\n";
#endif
      if (m_funcsToCalls.empty()) {
        errs() << "Empty <func, call> map\n";
        return;
      }
      FunctionStaticSlicer * fss = getFSS(f);
      const ValSet::const_iterator relBgn =
        fss->relevant_begin(getFunctionEntry(f));
      const ValSet::const_iterator relEnd =
        fss->relevant_end(getFunctionEntry(f));
      FuncsToCalls::const_iterator c, e;
      llvm::tie(c,e) = m_funcsToCalls.equal_range(f);
      for ( ; c != e; ++c) {
        const llvm::CallInst *CI = c->second;
        const llvm::Function *g = CI->getParent()->getParent();
#ifdef DEBUG_EMIT
        errs() << " caller " << g->getName() << "\n";
#endif
        FunctionStaticSlicer *FSS = getFSS(g);
        std::set<const llvm::Value *> R;
        detail::getRelevantVarsAtCall(c->second, f, relBgn, relEnd,
            std::inserter(R, R.end()));
        if (FSS->addCriterion(CI, R.begin(), R.end(),
              !FSS->shouldSkipAssert(CI))) {
          FSS->addCriterion(CI, FSS->REF_begin(CI), FSS->REF_end(CI));
          *out++ = g;
        }
      }
    }

  template<typename OutIterator>
    void StaticSlicer::emitToForwardExits(llvm::Function const* const f,
        OutIterator out) {
#ifdef DEBUG_EMIT
      errs() << __func__ << " for " << f->getName() << "\n";
#endif
      if (m_callsToFuncs.empty()) {
        errs() << "Empty <call, func> map\n";
        return;
      }
      typedef std::vector<const llvm::CallInst *> CallsVec;
      CallsVec C;
      getFunctionCalls(f, std::back_inserter(C));
      FunctionStaticSlicer * fss = getFSS(f);
      for (CallsVec::const_iterator c = C.begin(); c != C.end(); ++c) {
        const llvm::CallInst * CI = *c;
        if (isa<IntrinsicInst>(CI)) {
          continue;
        }
        // if call inst REF does not intersect with any RC, skip the DOWN()
        if (fss->isSliced(CI)) {
#ifdef DEBUG_EMIT
          errs() << "skip CallInst that's not in slice:";
          CI->dump();
#endif
          continue;
        }
        CallsToFuncs::const_iterator g, e;
        llvm::tie(g, e) = m_callsToFuncs.equal_range(*c);
        for ( ; g != e; ++g) {
          const Function *callee = g->second;
          FunctionStaticSlicer * FSS = getFSS(callee);
#ifdef DEBUG_EMIT
          errs() << " callee " << callee->getName() << "\n";
#endif
          Function::const_arg_iterator cai = callee->arg_begin();
          unsigned args = 0;
          for (; args < CI->getNumArgOperands() && cai != callee->arg_end(); args++, cai++) {
            Value const * param = &*cai;
            Value const * argument = CI->getArgOperand(args);
            if (isConstantValue(argument)) { // skip constant argument
#ifdef DEBUG_EMIT
              errs() << " argument " << argument->getName();
              argument->print(errs());
              errs() << " is constant\n";
#endif
              continue;
            }
            ValSet::const_iterator  rci, rce;
            // add the argument that is influenced and propagate 
            std::set<const llvm::Value *> R;
            for (rci = fss->relevant_begin(CI), rce = fss->relevant_end(CI); rci != rce; rci++) {
              if (argument == *rci) {
#ifdef DEBUG_EMIT
                errs() << " argument ";
                argument->print(errs());
                errs() << " is in relevant set\n";
#endif
                R.insert(param);
              }
            }
            if (FSS->addCriterion(getFunctionEntry(callee), R.begin(), R.end())) {
              *out++ = callee;
            }
          }
        }
      }
    }

  template<typename OutIterator>
    void StaticSlicer::emitToExits(llvm::Function const* const f,
        OutIterator out) {
      // Pseudo-code:
      //
      // foreach (callinst CI in f) {
      //   foreach (callee in CI) {
      //     foreach (returninst RI in callee) {
      //       R = {}
      //       foreach (rel in relevance(callinst->successor) {
      //         if (CI == rel) {
      //           R.append(RI)
      //         }
      //         else {
      //           R.append(rel) // should we really add 'rel'?
      //         }
      //       }
      //       callee.addCriterion(RI, R) 
      //     }
      //   }
      // }


#ifdef DEBUG_EMIT
      errs() << __func__ << " for " << f->getName() << "\n";
#endif
      if (m_callsToFuncs.empty()) {
        errs() << "Empty <call, func> map\n";
        return;
      }
      typedef std::vector<const llvm::CallInst *> CallsVec;
      CallsVec C;
      // Get all the call instructions in 'f'
      getFunctionCalls(f, std::back_inserter(C));
      FunctionStaticSlicer * fss = getFSS(f);
      for (CallsVec::const_iterator c = C.begin(); c != C.end(); ++c) {
        const ValSet::const_iterator relBgn =
          fss->relevant_begin(getSuccInBlock(*c));
        const ValSet::const_iterator relEnd =
          fss->relevant_end(getSuccInBlock(*c));
        CallsToFuncs::const_iterator g, e;
        // each call instruction might have multiple possible targets
        llvm::tie(g, e) = m_callsToFuncs.equal_range(*c);
        for ( ; g != e; ++g) {
          // for each possible callee
          typedef std::vector<const llvm::ReturnInst *> ExitsVec;
          const Function *callee = g->second;
#ifdef DEBUG_EMIT
          errs() << " callee " << callee->getName() << "\n";
#endif
          ExitsVec E;
          getFunctionExits(callee, std::back_inserter(E));
          // find all the exits (return statements)
          for (ExitsVec::const_iterator e = E.begin(); e != E.end(); ++e) {
#ifdef DEBUG_EMIT
            errs() << " return ";
            (*e)->dump();
#endif
            std::set<const llvm::Value *> R;
            detail::getRelevantVarsAtExit(*c, *e, relBgn, relEnd,
                std::inserter(R, R.end()));
            FunctionStaticSlicer * FSS = getFSS(callee);
            if (FSS->addCriterion(*e, R.begin(), R.end())) {
              *out++ = callee;
            }
          }
        }
      }
    }

}}

#endif
