// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

#include "llvm/LLVMContext.h"
#include "llvm/Instructions.h"
#include "llvm/Function.h"
#include "llvm/Pass.h"
#include "llvm/Value.h"
#include "llvm/Support/CommandLine.h"

#include "llvmslicer/Callgraph.h"
#include "llvmslicer/Modifies.h"
#include "llvmslicer/PointsTo.h"
#include "llvmslicer/StaticSlicer.h"
#include "mapper/Matcher.h"

using namespace llvm;

namespace llvm { namespace slicing { namespace detail {

  void fillParamsToArgs(CallInst const* const C,
      Function const* const F,
      ParamsToArgs& toArgs)
  {
    Function::const_arg_iterator p = F->arg_begin();
    std::size_t a = 0;
    for ( ; a < C->getNumArgOperands(); ++a, ++p)
    {
      Value const* const P = &*p;
      Value const* const A = C->getArgOperand(a);
      if (!isConstantValue(A))
        toArgs[P] = A;
    }
  }

  void mapArgsToParams(CallInst const* const C,
      Function const* const F,
      ParamsToArgs& toArgs)
  {
    Function::const_arg_iterator p = F->arg_begin();
    std::size_t a = 0;
    for ( ; a < C->getNumArgOperands(); ++a, ++p)
    {
      Value const* const P = &*p;
      Value const* const A = C->getArgOperand(a);
      if (!isConstantValue(A))
        toArgs[A] = P;
    }
  }

}}}

namespace llvm {namespace slicing {

  static bool setAdd(WorkList &list, const Function * elem)
  {
    bool included = false;
    for (WorkList::iterator WI = list.begin(), WE = list.end(); WI != WE; ++WI) {
      if (*WI == elem) {
        included = true;
        break;
      }
    }
    if (!included) {
      list.push_back(elem);
    }
    return !included;
  }

  void StaticSlicer::buildDicts(const ptr::PointsToSets &PS)
  {
    typedef Module::iterator FunctionsIter;
    for (FunctionsIter f = m_module->begin(); f != m_module->end(); ++f)
      if (!f->isDeclaration() && !memoryManStuff(&*f))
        for (inst_iterator i = inst_begin(*f);
            i != inst_end(*f); i++)
          if (CallInst const* c =
              dyn_cast<CallInst const>(&*i)) {
            if (isInlineAssembly(c)) {
              continue;
            }
            typedef std::vector<const Function *> FunCon;
            FunCon G;
            // A CallInst may have multiple possible call target due to
            // function pointer
            getCalledFunctions(c, PS, std::back_inserter(G));

            for (FunCon::const_iterator g = G.begin();
                g != G.end(); ++g) {
              Function const* const h = *g;
              if (!memoryManStuff(h) && !h->isDeclaration()) {
                m_funcsToCalls.insert(std::make_pair(h, c));
                m_callsToFuncs.insert(std::make_pair(c, h));
              }
            }
          }
  }

  StaticSlicer::StaticSlicer(bool forward) : ModulePass(ID), m_module(NULL), 
    m_forward(forward), m_slicers(), m_initFuns(), m_funcsToCalls(), 
    m_callsToFuncs(), m_ps(NULL), m_cg(NULL), m_mod(NULL), m_criteriaInit(false) 
  {
    m_funci = m_sliceFuncs.begin();
    m_instInit = false;
  }
  
  bool StaticSlicer::runOnModule(Module &M) {
    m_module = &M;
    errs() << "Computing PointsToSet...\n";
    m_ps = new ptr::PointsToSets();
    {
      ptr::ProgramStructure P(M);
      computePointsToSets(P, *m_ps);
    }

    errs() << "Computing CallGraph...\n";
    m_cg = new callgraph::Callgraph(M, *m_ps);

    errs() << "Computing Modifies...\n";
    m_mod = new mods::Modifies();
    {
      mods::ProgramStructure P1(M);
      computeModifies(P1, *m_cg, *m_ps, *m_mod);
    }

    errs() << "Building CallDicts...\n";
    buildDicts(*m_ps);

    errs() << "Building FSS...\n";
    for (Module::iterator MI = M.begin(), ME = M.end(); MI != ME; ++MI) {
      Function * f = MI;
      if (f->isIntrinsic())
        continue;
      // errs() << "\t|" << f << "@" << f->getName() << "|\n";
      newFSS(f);
    }
    errs() << "Done.\n";
    return false;
  }
  
  StaticSlicer::~StaticSlicer() {
    if (m_ps)
      delete m_ps;
    if (m_cg)
      delete m_cg;
    if (m_mod)
      delete m_mod;
    for (Slicers::const_iterator I = m_slicers.begin(), E = m_slicers.end();
        I != E; ++I)
      delete I->second;
#ifdef DEBUG_SLICER
    errs() << "Slicer destructed!\n";
#endif
  }

  FunctionStaticSlicer * StaticSlicer::newFSS(Function * F) {
    FunctionStaticSlicer *FSS = new FunctionStaticSlicer(*F, this, *m_ps, *m_mod, m_forward);
    m_slicers.insert(Slicers::value_type(F, FSS));
    return FSS;
  }

  FunctionStaticSlicer * StaticSlicer::getFSS(const Function * F) {
    Slicers::iterator si;
    si = m_slicers.find(F);
    if (si == m_slicers.end()) {
      errs() << "No slicer for " << F << "@" << F->getName() << "\n";
      return NULL;
    }
    // assert(si != m_slicers.end());
    return si->second;
  }
  
  void StaticSlicer::addInitRC(FunctionStaticSlicer * FSS, const Instruction *inst)
  {
      if (m_forward) {
        InsInfo * insInfo = FSS->getInsInfo(inst);
        if (insInfo == NULL)
          return;
        ValSet::const_iterator ci, ce;
        for (ci = insInfo->DEF_begin(), ce = insInfo->DEF_end(); ci != ce; ci++) {
          const Value * val = *ci;
          FSS->addInitialCriterion(inst, val);
        }
      } else {
        const Value *LHS = NULL;
        if (const LoadInst *LI = dyn_cast<LoadInst>(inst)) {
          LHS = LI->getPointerOperand();
        } else if (const StoreInst * SI = dyn_cast<StoreInst>(inst)) {
          LHS = SI->getPointerOperand();
        }
        if (LHS) {
          FSS->addInitialCriterion(inst, LHS);
        }
      }
  }

  void StaticSlicer::computeSlice() {
    if (!m_criteriaInit) {
      errs() << "No criteria is added to the slicer, call addCriteria\n";
      return;
    }
    WorkList Q(m_initFuns);
    WorkList P;

    // Backward:  DOWN*(UP*({C}))
    // Forward:   UP*(DOWN*({C}))

    // Phase 1
    //   Backward:  UP*({C})
    //   Forward:   DOWN*({C})

    while (!Q.empty()) {
      for (WorkList::iterator WI = Q.begin(), WE = Q.end(); WI != WE; ++WI) {
        const Function * F = *WI;
        if (F->isIntrinsic()) // skip intrinsic
          continue;
        FunctionStaticSlicer *fss = getFSS(F);
        fss->calculateStaticSlice();
        if (setAdd(P, F)) {
          m_sliceFuncs.push_back(F);
        }
      }
      WorkList tmp;
      for (WorkList::iterator WI = Q.begin(), WE = Q.end(); WI != WE; ++WI) {
        const Function * F = *WI;
        if (!m_forward)
          emitToCalls(F, std::inserter(tmp, tmp.end()));
        else
          emitToForwardExits(F, std::inserter(tmp, tmp.end()));
      }
      std::swap(tmp,Q);
    }
    
    // Phase 2
    //     Backward: DOWN*(XXX)
    //     Forward:  UP*(XXX)
    while (!P.empty()) {
      for (WorkList::iterator WI = P.begin(), WE = P.end(); WI != WE; ++WI) {
        const Function * F = *WI;
        if (F->isIntrinsic()) // skip intrinsic
          continue;
        FunctionStaticSlicer *fss = getFSS(F);
        fss->calculateStaticSlice();
        setAdd(m_sliceFuncs, F);
      }
      WorkList tmp;
      for (WorkList::iterator WI = P.begin(), WE = P.end(); WI != WE; ++WI) {
        const Function * F = *WI;
        if (!m_forward)
          emitToExits(F, std::inserter(tmp, tmp.end()));
        else
          emitToForwardCalls(F, std::inserter(tmp, tmp.end()));
      }
      std::swap(tmp,P);
    }
    errs() << "sliced functions:\n";
    for (FuncIter fi = m_sliceFuncs.begin(), fe = m_sliceFuncs.end(); fi != fe; ++fi) {
// #ifdef DEBUG_SLICER
// #endif
      getFSS(*fi)->dump(true);
      errs() << (*fi)->getName() << "\n";
    }
  }

  const Instruction * StaticSlicer::next()
  {
    for (; m_funci != m_sliceFuncs.end(); m_funci++) {
      const Function * F = *m_funci;
      if (!m_instInit) {
        m_insti = inst_begin(F);
        m_instInit = true;
      }
      const_inst_iterator ie = inst_end(F);
      for (; m_insti != ie; ++m_insti) {
        const Instruction *inst = &(*m_insti);
        if (!getFSS(F)->isSliced(inst)) {
          m_insti++;
          return inst;
        }
      }
      m_instInit = false;
    }
    return NULL;
  }

  char StaticSlicer::ID = 0;
  const char * StaticSlicer::PassName = "Inter-procedural slicer pass";
}}

