/**
 *  @file          LLVMHelper.cpp
 *
 *  @version       1.0
 *  @created       02/13/2013 09:39:17 PM
 *  @revision      $Id$
 *
 *  @author        Ryan Huang <ryanhuang@cs.ucsd.edu>
 *  @organization  University of California, San Diego
 *  
 *  Copyright (c) 2013, Ryan Huang
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *  http://www.apache.org/licenses/LICENSE-2.0
 *     
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  @section       DESCRIPTION
 *  
 *  LLVMHelper implementation
 *
 */

#include <string>
#include <iostream>
#include <algorithm>

#include "llvm/Analysis/DebugInfo.h"

#include "llvm/Support/IRReader.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/TargetRegistry.h"

#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include "llvm/Target/TargetLowering.h"

#include "commons/handy.h"
#include "commons/LLVMHelper.h"

#define LLVMHELPER_DEBUG

gen_dbg(helper)

#ifdef LLVMHELPER_DEBUG
gen_dbg_impl(helper)
#else
gen_dbg_nop(helper)
#endif

namespace llvm {

static const char * HotTypeName[HOTTYPES] = {
  "INVALIDTYPE",
  "SYSCALL",
  "LOCKCALL",
  "EXPCALL",
  "FREQCALL"
};

static const char * HotTypeStr[HOTTYPES] = {
  "invalid type",
  "system call",
  "lock call",
  "expensive call",
  "frequent call"
};

const char * toHotStr(const HotFuncType type)
{
  if (type < 0 || type >= HOTTYPES)
    return "UNKNOWN";
  return HotTypeStr[type];
}

const char * toHotName(const HotFuncType type)
{
  if (type < 0 || type >= HOTTYPES)
    return "UNKNOWN";
  return HotTypeName[type];
}

HotFuncType fromHotTypeName(const char * name)
{
  int i;
  HotFuncType type;
  for (i = SYSCALL; i <= FREQCALL; i++) {
    type = (HotFuncType) i;
    if (strcmp(name, toHotName(type)) == 0)
      break;
  }
  if (i > FREQCALL)
    return INVALIDTYPE;
  return type;
}

size_t count_strips(Module * M)
{
  if (NamedMDNode *CU_Nodes = M->getNamedMetadata("llvm.dbg.cu")) {
    size_t buf_len = 128;
    char *buf = (char *) malloc(buf_len);
    for (unsigned i = 0, e = CU_Nodes->getNumOperands(); i != e; ++i) {
      DICompileUnit CU1(CU_Nodes->getOperand(i));
      if (i == 0) {
        if (i + 1 != e) {
          i++;
          DICompileUnit CU2(CU_Nodes->getOperand(i));
          buf = common_prefix(buf, buf_len, CU1.getDirectory().data(), CU2.getDirectory().data());
        }
        else {
          size_t cu_len = CU1.getDirectory().size();
          if (cu_len > buf_len) {
            buf = (char *) realloc(buf, cu_len);
            buf_len = cu_len;
          }
          strcpy(buf, CU1.getDirectory().data()); 
        }
      }
      else {
        buf = common_prefix(buf, buf_len, buf, CU1.getDirectory().data());
      }
    }
    unsigned cnt = countnchr(buf, -1, '/');
    if (buf[strlen(buf) - 1] != '/')
      cnt++;
    //TODO dirty hacks
    // workaround problem where a single bc file inside the project's
    // subdirectory is provided. e.g. /home/ryan/Project/mysql/storage/innodb_plugin
    // will be the inferred root path.
    if (endswith(buf, "innodb_plugin"))
      cnt -= 2;
    else if (endswith(buf, "src/")) // hack for postgresql/src/
      cnt -= 1;

    return  cnt;
  }
  return 0;
}

Module *ReadModule(LLVMContext &Context, StringRef Name)
{
  SMDiagnostic Diag;
  Module *M = ParseIRFile(Name, Diag, Context);
  if (!M)
    std::cerr << "IR file parsing failed: " << Diag.getMessage() << std::endl;
  return M;
}

TargetMachine * getTargetMachine()
{
  const std::string TripleStr = llvm::sys::getHostTriple();
  const std::string CPUStr = llvm::sys::getHostCPUName();
  const std::string FeatureStr;
  helper_debug("Triple: %s CPU: %s\n", TripleStr.c_str(), CPUStr.c_str());
  std::string Err;
  const Target* T;

  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmPrinters();
  InitializeAllAsmParsers();

  T = TargetRegistry::lookupTarget(TripleStr, Err);
  if(!Err.empty()) {
    std::cerr << "Cannot find target: " << Err << std::endl;
    exit(1);
  }
  // Create TargetMachine
  TargetMachine* TM = T->createTargetMachine(TripleStr, CPUStr, FeatureStr);
  if(TM == NULL) {
    std::cerr << "Cannot create target machine" << std::endl;
    exit(1);
  }
  return TM;
}

void initPassRegistry(PassRegistry & Registry)
{
  initializeCore(Registry);
  initializeScalarOpts(Registry);
  initializeIPO(Registry);
  initializeAnalysis(Registry);
  initializeIPA(Registry);
  initializeTransformUtils(Registry);
  initializeInstCombine(Registry);
  initializeInstrumentation(Registry);
  initializeTarget(Registry);
}

TargetData * getTargetData(Module *M)
{
  // Add an appropriate TargetData instance for this module.
  TargetData *TD = 0;
  const std::string &ModuleDataLayout = M->getDataLayout();
  if (!ModuleDataLayout.empty())
    TD = new TargetData(ModuleDataLayout);
  return TD;
}

void indent(raw_ostream & OS, int space)
{
  while (space--) {
    OS << " ";
  }
}

bool parseProfile(const char *fname, Profile &profile)
{
  FILE *fp = fopen(fname,"r");
  if (fp == NULL) {
    perror("Read profile file");
    return false;
  }
  char buf[256];
  bool preamble = false;
  HotFuncType type = SYSCALL; 
  unsigned line = 0;
  while (fgetline(fp, buf, 256) != NULL) {
    line++;
    if (strcmp(buf, PROFILE_SEGMENT_BEGIN) == 0) {
      if (preamble) {
        fprintf(stderr, "Warning: empty profile segment %s\n", toHotStr(type));
      }
      // Preamble detected. Expect next line to be 
      // segment type
      if (fgetline(fp, buf, 256) == NULL) {
        syntaxerr("expected profile segment type", line);
        return false;
      }
      line++;
      type = fromHotTypeName(buf);
      if (type == INVALIDTYPE) {
        syntaxerr("unknown profile segment type", line);
        return false;
      }
      if (fgetline(fp, buf, 256) == NULL || strcmp(buf, PROFILE_SEGMENT_END) != 0) {
        syntaxerr("expected profile segment end", line);
        return false;
      }
      line++;
      preamble = true;
      continue;
    }
    else
      if (strcmp(buf, PROFILE_SEGMENT_END) == 0) {
        syntaxerr("expected profile segment begin", line);
        return false;
      }
      else {
        preamble = false;
        if (strlen(buf))
          profile[type].push_back(buf);
      }
  }
  for (Profile::iterator it = profile.begin(), ie = profile.end(); it != ie; ++it) {
    std::vector<std::string> &vec = it->second;
    std::sort(vec.begin(), vec.end());
  }
  fclose(fp);
  return true;
}

} // End of llvm namespace
