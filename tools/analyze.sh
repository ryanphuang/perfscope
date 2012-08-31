#!/bin/bash

function log()
{
    echo -e "$@"
    #echo -e "$@" | tee -a analysis.log
}

if [ $# -ne 2 ]; then
    echo "Usage: $0 rev1 rev2"
    exit 1
fi

#if [ $# -ne 1 ]; then
#    echo "Usage: $0 revfile"
#    exit 1
#fi

if [ ! -d analysis ]; then
    mkdir analysis
fi

#revisions=$(cat $1)
#log "analyze patches for $(echo $revisions | tr "\n" ",") " 
log "analyze patches between $1 and $2"
#j=$1
#for ((i=$j+1; i<=$2; i++))
#for i in $revisions
for ((i=$1; i<=$2; i++))
do
    log "============$i=============="
    if [ ! -s diffs/$i.id ]; then
        log "ignore $i"
    else
         
        if [ -e .replay/objs/innodb_plugin.bc.$i ]; then
            log "analyze $i using innodb_plugin.bc.$i and mysqld.bc.$i"
            ~/Projects/perfscope_src/mapper/Debug+Asserts/bin/perfscope -b .replay/objs/innodb_plugin.bc.$i,.replay/objs/mysqld.bc.$i -s syscall -e mysql.profile diffs/$i.id  2>&1 | tee analysis/$i.result
            #~/Projects/perfscope_src/mapper/Debug+Asserts/bin/mapperdriver -d diffs/$i.id .replay/objs/innodb_plugin.bc.$j .replay/objs/mysqld.bc.$j 2>&1 | tee analysis/$i.result
        else
            log "analyze $i using mysqld.bc.$i"
            ~/Projects/perfscope_src/mapper/Debug+Asserts/bin/perfscope -b .replay/objs/mysqld.bc.$i -s syscall -e mysql.profile diffs/$i.id 2>&1 | tee analysis/$i.result
            #~/Projects/perfscope_src/mapper/Debug+Asserts/bin/mapperdriver -d diffs/$i.id .replay/objs/mysqld.bc.$j  2>&1 | tee analysis/$i.result
        fi
    fi
    #if [ -e .replay/objs/mysqld.bc.$i ]; then
    #    j=$i
    #else
    #    if [ -s diffs/$i.id ]; then
    #        log "Warning: $i has source change but no bc generated"
    #    fi
    #fi
    log "=============================="
done
