/**
 *  @file          LLVMHelper.h
 *
 *  @version       1.0
 *  @created       02/13/2013 09:38:45 PM
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
 *  LLVM Helper
 *
 */

#ifndef __LLVMHELPER_H_
#define __LLVMHELPER_H_

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "llvm/Module.h"
#include "llvm/PassRegistry.h"

#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {

struct ModuleArg {
  std::string name;
  Module *module;
  int strips;
  ModuleArg(std::string n, Module *m = NULL, int s = 0) : name(n), module(m), strips(s) {}
};

#define HOTTYPES 5

enum HotFuncType {INVALIDTYPE, SYSCALL, LOCKCALL, EXPCALL, FREQCALL};

HotFuncType fromHotTypeName(const char * name);

const char * toHotStr(const HotFuncType type);
const char * toHotName(const HotFuncType type);

typedef std::map<HotFuncType, std::vector<std::string> > Profile;

/// Infer the level of strips in the module.
/// The algorithm is to use the length of longest 
/// common prefix in the CUs inside the module
size_t count_strips(Module * M);

/// Construct a module from a file. The module is 
/// returned.
///
/// On error, messages are written to stderr
/// and null is returned.
Module *ReadModule(LLVMContext &Context, StringRef Name);

/// Get the TargetMachine representing the executing
/// machine's architecture
TargetMachine * getTargetMachine();

/// Initialize the given registry with common passes
void initPassRegistry(PassRegistry & Registry);

/// Get the TargetData in the given Module
/// Return NULL if it cannot be created
TargetData * getTargetData(Module *M);

void indent(raw_ostream & OS, int space);

#define PROFILE_SEGMENT_BEGIN "===="
#define PROFILE_SEGMENT_TYPE(type) #type
#define PROFILE_SEGMENT_END "===="

bool parseProfile(const char *fname, Profile &profile);

} // End of llvm namespace

#endif /* __LLVMHELPER_H_ */
