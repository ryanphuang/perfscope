// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

#ifndef SLICING_FUNCTIONSTATICSLICER_H
#define SLICING_FUNCTIONSTATICSLICER_H

#include <map>

#include "llvm/Value.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Support/InstIterator.h"

#include "llvmslicer/PointsTo.h"
#include "llvmslicer/Modifies.h"
#include "llvmslicer/PostDominanceFrontier.h"
#include "mapper/Matcher.h"

namespace llvm { namespace slicing {

  typedef llvm::SmallSetVector<const llvm::Value *, 10> ValSet;

  void printVal(const llvm::Value *val);

  class InsInfo {
    public:
      InsInfo(const llvm::Instruction *i, const llvm::ptr::PointsToSets &PS,
          const llvm::mods::Modifies &MOD);

      const Instruction *getIns() const { return ins; }

      bool addRC(const llvm::Value *var) 
      {
#ifdef DEBUG_RC
        errs() << "\tadding RC to '";
        ins->print(errs());
        errs() << "':";
        if (var->hasName())
          errs() << "  " << var->getName() << "\n";
        else
          var->dump();
#endif
        return RC.insert(var); 
      }
      bool addDEF(const llvm::Value *var) { return DEF.insert(var); }
      bool addREF(const llvm::Value *var) { return REF.insert(var); }
      void deslice() { sliced = false; }

      ValSet::const_iterator RC_begin() const { return RC.begin(); }
      ValSet::const_iterator RC_end() const { return RC.end(); }
      ValSet::const_iterator DEF_begin() const { return DEF.begin(); }
      ValSet::const_iterator DEF_end() const { return DEF.end(); }
      ValSet::const_iterator REF_begin() const { return REF.begin(); }
      ValSet::const_iterator REF_end() const { return REF.end(); }

      bool isSliced() const { return sliced; }

    private:
      const llvm::Instruction *ins;
      ValSet RC, DEF, REF;
      bool sliced;
  };

  class FunctionStaticSlicer {
    public:
      typedef std::map<const llvm::Instruction *, InsInfo *> InsInfoMap;

      FunctionStaticSlicer(llvm::Function &F, llvm::ModulePass *MP,
          const llvm::ptr::PointsToSets &PT, const llvm::mods::Modifies &mods, bool forward = false) :
        fun(F), MP(MP), forward(forward) {
          for (llvm::inst_iterator I = llvm::inst_begin(F), E = llvm::inst_end(F);
              I != E; ++I)
            insInfoMap.insert(InsInfoMap::value_type(&*I, new InsInfo(&*I, PT, mods)));
        }
      ~FunctionStaticSlicer();

      ValSet::const_iterator relevant_begin(const llvm::Instruction *I) const {
        return getInsInfo(I)->RC_begin();
      }
      ValSet::const_iterator relevant_end(const llvm::Instruction *I) const {
        return getInsInfo(I)->RC_end();
      }

      ValSet::const_iterator REF_begin(const llvm::Instruction *I) const {
        return getInsInfo(I)->REF_begin();
      }
      ValSet::const_iterator REF_end(const llvm::Instruction *I) const {
        return getInsInfo(I)->REF_end();
      }
      ValSet::const_iterator DEF_begin(const llvm::Instruction *I) const {
        return getInsInfo(I)->DEF_begin();
      }
      ValSet::const_iterator DEF_end(const llvm::Instruction *I) const {
        return getInsInfo(I)->DEF_end();
      }

      bool isSliced(const llvm::Instruction *I) const {
        return getInsInfo(I)->isSliced();
      }

      template<typename FwdValueIterator>
        bool addCriterion(const llvm::Instruction *ins, FwdValueIterator b,
            FwdValueIterator const e, bool desliceIfChanged = false) {
          if (isa<IntrinsicInst>(ins)) {
#ifdef DEBUG_CRITERION
            errs() << "skip intrinsic inst\n";
#endif
            return false;
          }
          InsInfo *ii = getInsInfo(ins);
          if (ii == NULL) {
#ifdef DEBUG_CRITERION
            errs() << "No associated insinfo for ";
            ins->dump();
#endif
            return false;
          }
          bool change = false;
          for (; b != e; ++b)
            if (ii->addRC(*b))
              change = true;
          if (change && desliceIfChanged)
            ii->deslice();
          return change;
        }

      void addInitialCriterion(const llvm::Instruction *ins,
          const llvm::Value *cond = 0, bool deslice = true) {
        InsInfo *ii = getInsInfo(ins);
        if (cond) {
#ifdef DEBUG_RC
          errs() << "Add initial RC: ";
          printVal(cond);
          errs() << "\n";
#endif
          ii->addRC(cond);
        }
        ii->deslice();
      }
      void calculateStaticSlice();
      void dump(bool outputline = false);
      bool slice();

      InsInfo *getInsInfo(const llvm::Instruction *i) const {
        InsInfoMap::const_iterator I = insInfoMap.find(i);
        if (I == insInfoMap.end()) {
          return NULL;
        }
        // assert(I != insInfoMap.end());
        return I->second;
      }

      static void removeUndefs(ModulePass *MP, Function &F);

      void addSkipAssert(const llvm::CallInst *CI) {
        skipAssert.insert(CI);
      }

      bool shouldSkipAssert(const llvm::CallInst *CI) {
        return skipAssert.count(CI);
      }

    private:
      llvm::Function &fun;
      llvm::ModulePass *MP;
      InsInfoMap insInfoMap;
      llvm::SmallSetVector<const llvm::CallInst *, 10> skipAssert;
      bool forward;

      static bool sameValues(const Value *val1, const Value *val2);
      void crawlBasicBlock(const llvm::BasicBlock *bb);
      bool computeRCi(InsInfo *insInfoi, InsInfo *insInfoj);
      bool computeRCi(InsInfo *insInfoi);
      void computeRC();

      void computeSCi(const llvm::Instruction *i, const llvm::Instruction *j);
      void computeSC();

      bool computeBC();
      bool updateRCSC(llvm::PostDominanceFrontier::DomSetType::const_iterator start,
          llvm::PostDominanceFrontier::DomSetType::const_iterator end);


      static void removeUndefBranches(ModulePass *MP, Function &F);
      static void removeUndefCalls(ModulePass *MP, Function &F);
  };

  bool findInitialCriterion(llvm::Function &F, FunctionStaticSlicer &ss,
      bool startingFunction = false);

}}

#endif
