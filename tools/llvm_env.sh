export CC="clang -flto"
export CXX="clang++ -flto"
export CFLAGS="-g3 -O0 -v"
export CXXFLAGS="-g3 -O0 -v"
export RANLIB=/bin/true
export AR="clang-ar"
export NM="nm --plugin /home/ryan/Projects/llvm/build/lib/LLVMgold.so"
#export ARFLAGS="cru --plugin /home/ryan/Projects/llvm/build/lib/LLVMgold.so"
