#define DEBUG_TYPE "listfiles"
#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"

#include "llvm/Analysis/DebugInfo.h"
#include "llvm/Metadata.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/Constants.h"

#include <stdlib.h>
#include <cxxabi.h>

using namespace llvm;

static char *MBUF = NULL;
static size_t MBUF_LEN = 0;
#define MANGLE_LEN 256


char * common_prefix(char * buf, size_t & len, const char *str1, const char *str2)                             
{                                                                                    
    const char *p1 = str1, *p2 = str2;
    size_t pos = 0;
    for (; p1 && p2 && *p1 && *p2 && *p1 == *p2; p1++, p2++)
    {
        if (pos >= len) {
            len *= 2;
            buf = (char *) realloc(buf, len);
        }
        buf[pos++] = *p1;
    }
    if (pos >= len) {
        len *= 2;
        buf = (char *) realloc(buf, len);
    }
    buf[pos] = '\0';
    return buf;
}

const char * cpp_demangle(const char *name)
{
    if (MBUF == NULL) {
        MBUF = (char *) malloc(MANGLE_LEN);
        MBUF_LEN = MANGLE_LEN;
    }
    int status;
    char * ret = abi::__cxa_demangle(name, MBUF, &MBUF_LEN, &status);
    if (ret == NULL) // normal C names will be demangled to NULL
        return name;
    return ret;
}

namespace {
  struct ListFilePass: public ModulePass {
    static char ID; // Pass identification, replacement for typeid
    ListFilePass() : ModulePass(ID) {}

    virtual bool runOnModule(Module &M) {
        DebugInfoFinder Finder;
        Finder.processModule(M);
        if (NamedMDNode *CU_Nodes = M.getNamedMetadata("llvm.dbg.cu")) {
            size_t buf_len = 128;
            char *buf = (char *) malloc(buf_len);
            strcpy(buf, "");
            for (unsigned i = 0, e = CU_Nodes->getNumOperands(); i != e; ++i) {
                DICompileUnit CU1(CU_Nodes->getOperand(i));
                errs() << CU1.getDirectory() << "#" << CU1.getFilename() << "\n";
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
            errs() << buf << "\n";
        }
        return false;
    }
  };
}

char ListFilePass::ID = 0;
static RegisterPass<ListFilePass> X("listfiles", "List Files In A Module Pass");
