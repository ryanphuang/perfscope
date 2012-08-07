#!/bin/bash
if [ $# -ne 1 ]; then
    echo "Usage: $0 revfile"
    exit 1
fi
revisions=$(tac $1 | tr "\\n" " " )
echo "filtering on $revisions"
c=0
for r in $revisions
do
    if [ ! -e .replay/objs/postgres.bc.$r ];then
        echo "$r doesn't exit"
        continue
    fi
    if [ $c -eq 0 ]; then
        j=$r
        (( c += 1))
        continue
    fi
    (( c += 1))
    echo "diff between $r and $j"
    #llvm-diff .replay/objs/postgres.bc.$r .replay/objs/postgres.bc.$j > /dev/null 2>&1
    ~/Projects/perfscope_src/mapper/Debug+Asserts/bin/perfscope .replay/objs/postgres.bc.$r .replay/objs/postgres.bc.$j 
    if [ $? -eq 0 ]; then
        echo "same"
    else
        echo "different"
    fi
    j=$r
done
