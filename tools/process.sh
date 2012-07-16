#!/bin/bash
files=$(ls *.result | sort -n)
for f in $files
do
    data=$(sed -n 's/.* (\([0-9]*.[0-9]*\) per sec.)/\1/p' $f)
    i=1
    for d in $data
    do
        echo $d >> $i.data
        i=$(( $i + 1 ))
    done
done
