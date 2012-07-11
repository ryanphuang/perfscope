#!/bin/bash
function die()
{
    echo "$@"
    exit 1
}

# checkarg $arg1 $arg2
function checkarg()
{
    if [ $1 -eq $1 -a $2 -eq $2 2>/dev/null ]; then
        if [ $1 -le 0 -o $2 -le 0 ]; then
            die "Revision number must be positive integer"
        fi
        if [ $1 -gt $2 ]; then
            die "rev1 should be less or equal than rev2"
        fi
    else
        die "Revision number must be integer"
    fi
}

if [ $# -ne 2 ]; then
    die "Usage: $0 rev1 rev2"
fi

checkarg $1 $2
for ((i = $1; i <= $2; i++))
do
    #bzr diff -c$i > diffs/$i
    if [ $? -ne 0 ]; then
        die "Fail to get diff $i"
    fi
    echo "diff $i generated"
done
