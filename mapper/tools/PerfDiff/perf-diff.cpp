//===-- llvm-diff.cpp - Module comparator command-line driver ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the command-line driver for the difference engine.
//
//===----------------------------------------------------------------------===//

#include "DiffLog.h"
#include "DifferenceEngine.h"

#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Type.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/IRReader.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SourceMgr.h"

#include <string>
#include <utility>
#include <stdio.h>


using namespace llvm;

/// Reads a module from a file.  On error, messages are written to stderr
/// and null is returned.
static Module *ReadModule(LLVMContext &Context, StringRef Name) {
    SMDiagnostic Diag;
    Module *M = ParseIRFile(Name, Diag, Context);
    if (!M)
        Diag.Print("llvmdiff", errs());
    return M;
}

int main(int argc, char **argv) {
    if (argc <= 2) {
        fprintf(stderr, "Usage: %s module1 module2\n", argv[0]);
        exit(1);
    }

    LLVMContext Context;

    // Load both modules.  Die if that fails.
    Module *LModule = ReadModule(Context, argv[1]);
    Module *RModule = ReadModule(Context, argv[2]);
    if (!LModule || !RModule) return 1;

    DiffConsumer Consumer(LModule, RModule);
    DifferenceEngine Engine(Context, Consumer);

    Engine.diff(LModule, RModule);

    delete LModule;
    delete RModule;

    return Consumer.hadDifferences();
}
