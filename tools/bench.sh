#!/bin/bash

SOCKFILE=/home/ryan/mysqld.sock
MYSQLBASE=/home/ryan/experiment/5.1-build
DRYRUN=

# log $msg
function log()
{
    echo -e "$@" | tee -a bench.log
}

function err()
{
    echo -e "$@" | tee -a bench.err
}

function fatal()
{
    err "$@"
    exit 1
}

function shutmysql()
{
    if [ -e $SOCKFILE ]; then
        log "\tShutting down mysqld..."
        $MYSQLBASE/bin/mysqladmin -S $SOCKFILE -u root shutdown
        if [ $? -ne 0 -o -e $SOCKFILE ]; then
            fatal "\t\tFailed to shutdown mysqld." 
        fi
        log "\t\tmysqld shutdown."
    fi
}

function benchmark()
{
    log "Running benchmark..."
    shutmysql
    log "\tStarting mysqld..."
    $MYSQLBASE/bin/mysqld_safe --defaults-file=$MYSQLBASE/my.cnf &
    if [ $? -ne 0 ]; then
        fatal "\t\tFailed to start mysqld." 
    fi
    try=1
    while [ $try -le 5 ]
    do
        if [ -e $SOCKFILE ]; then
            sleep 1 # be conservative
            break
        fi
        log "\t\tmysqld hasn't fully started, wait $try time(s)..."
        try=$(( $try + 1 ))
        sleep 1
    done
    if [ ! -e $SOCKFILE ]; then
        fatal "\t\tFailed to start mysqld." 
    fi
    log "\t\tmysqld started."
    log "\tRunning sysbench..."
    sysbench --num-threads=16 --max-requests=10000 --test=oltp --oltp-table-size=1000000 --mysql-socket=$SOCKFILE --mysql-user=root run >$1.result 2>&1 
    if [ $? -ne 0 ]; then
        shutmysql
        fatal "\t\tsysbench failed." 
    fi
    log "\t\tsysbench finished."
    log "Benchmark finished."
    shutmysql
}

> bench.log
> bench.err

for ((i = 3632; i < 3732; i = i + 5))
do
    log "[[Switching to revision $i..."
    bzr revert -r $i
    if [ $? -ne 0 ]; then
        fatal "Failed to switch revision." 
    fi
    log "\tNow at revision $i"
    log "[[Building revision $i..."
    if [ -z "$DRYRUN" ]; then
        time BUILD/compile-pentium64 --prefix=$MYSQLBASE 2>&1 | tee compile.log
    fi
    if [ $? -ne 0 ]; then
        fatal "Failed to build revision." 
    fi
    log "\tRevision $i builti"
    log "[[Installing revision $i..."
    if [ -z "$DRYRUN" ]; then
        make install 
    fi
    if [ $? -ne 0 ]; then
        fatal "Failed to install revision." 
    fi
    log "\tRevision $i installed"
    benchmark $i
    log "[[Sleeping for 20 seconds..."
    sleep 20
done
