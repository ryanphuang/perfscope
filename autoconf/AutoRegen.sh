#!/bin/sh
die () {
	echo "$@" 1>&2
	exit 1
}

askforpath() {
  while true ; do
    echo "LLVM source root not found." 
    read -p "Enter full path to LLVM source:" REPLY
    if test -d "$REPLY/autoconf/m4" ; then
      llvm_src_root="$REPLY"
      llvm_m4="$REPLY/autoconf/m4"
      read -p "Enter full path to LLVM objects (empty for same as source):" REPLY
      if test -d "$REPLY" ; then
        llvm_obj_root="$REPLY"
      else
        llvm_obj_root="$llvm_src_root"
      fi
      break
    fi
  done
}

test -d autoconf && test -f autoconf/configure.ac && cd autoconf
test -f configure.ac || die "Can't find 'autoconf' dir; please cd into it first"
autoconf --version | egrep '2\.[56][0-9]' > /dev/null
if test $? -ne 0 ; then
  die "Your autoconf was not detected as being 2.5x or 2.6x"
fi
cwd=`pwd`
if test -d ../../../autoconf/m4 ; then
  cd ../../../autoconf/m4
  llvm_m4=`pwd`
  llvm_src_root=../..
  llvm_obj_root=../..
  cd $cwd
elif test -d ../../llvm/autoconf/m4 ; then
  cd ../../llvm/autoconf/m4
  llvm_m4=`pwd`
  llvm_src_root=..
  llvm_obj_root=..
  cd $cwd
else
  src_tmp=$(grep "^LLVM_SRC_ROOT" configure.ac | awk -F"=" '{print $2}')
  obj_tmp=$(grep "^LLVM_OBJ_ROOT" configure.ac | awk -F"=" '{print $2}')
  if [ ! -z "$src_tmp" ] && [ ! -z "$obj_tmp" ]; then
    src_tmp=$(echo "$src_tmp" | sed -e 's/^"//' -e 's/"$//')
    obj_tmp=$(echo "$obj_tmp" | sed -e 's/^"//' -e 's/"$//')
    echo $src_tmp $obj_tmp
    if [ -d $src_tmp ] && [ -d $obj_tmp ]; then
      echo "Found LLVM source root at: $src_tmp" 
      cd $src_tmp
      llvm_src_root=`pwd`
      llvm_m4="$llvm_src_root/autoconf/m4"
      cd $cwd
      cd $obj_tmp
      llvm_obj_root=`pwd`
      cd $cwd
    else
      askforpath
    fi
  else
    askforpath
  fi
fi
# Patch the LLVM_ROOT in configure.ac, if it needs it
cp configure.ac configure.bak
sed -e "s#^LLVM_SRC_ROOT=.*#LLVM_SRC_ROOT=\"$llvm_src_root\"#" \
    -e "s#^LLVM_OBJ_ROOT=.*#LLVM_OBJ_ROOT=\"$llvm_obj_root\"#" configure.bak > configure.ac
echo "Regenerating aclocal.m4 with aclocal"
rm -f aclocal.m4
aclocal -I $llvm_m4 -I "$llvm_m4/.." || die "aclocal failed"
echo "Regenerating configure with autoconf"
autoconf --warnings=all -o ../configure configure.ac || die "autoconf failed"
cd ..
exit 0
