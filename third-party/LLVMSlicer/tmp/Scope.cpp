#include "Scope.h"

llvm::raw_ostream & operator<<(llvm::raw_ostream& os, const Scope & scope)
{
    os << "[#" << scope.begin << ",#" << scope.end <<"]";
    return os;
}

std::ostream & operator<<(std::ostream& os, const Scope & scope)
{
    os << "[#" << scope.begin << ",#" << scope.end <<"]";
    return os;
}
