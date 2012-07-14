/* * File: Scope.h
 * *
 * *    Scope Data Structure
 * *
 * * Author: Ryan Huang <ryanhuang@cs.ucsd.edu>
 */

#ifndef ___PERF_SCOPE__H_ 
#define ___PERF_SCOPE__H_ 

#include <iostream>
#include "llvm/Support/raw_ostream.h"

typedef struct Scope {
    unsigned long begin;
    unsigned long end;
    
    Scope() : begin(0), end(0) {}
    Scope(unsigned long b, unsigned long e) : begin(b), end(e) {}
    Scope(const Scope &another) : begin(another.begin), end(another.end) {}

    bool includes(const Scope &another) { return (another.begin >= begin) && (another.end <= end); }

    bool operator==(const Scope & another) const { return begin == another.begin && end == another.end; }

} Scope;

llvm::raw_ostream & operator<<(llvm::raw_ostream& , const Scope & );
std::ostream & operator<<(std::ostream& , const Scope & );

#endif
