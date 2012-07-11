#!/bin/bash

BASEDIR=$(pwd)
REPLAYDIR=$BASEDIR/.replay
MAKEFILENAME=CMakeLists.txt
MAKEFILECACHE=CMakeCache.txt
DIFFDIR=diffs
OBJDIR=objs
LOGFILE=replay.log
ERRFILE=replay.err
PARSER=./parser
BUILDDIR=bld
MAXREV=$(bzr revno)
ENVFILE=llvm_env.sh
DRYRUN=
REMAKE=0

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
        if [ $1 -gt $MAXREV -o $2 -gt $MAXREV ]; then
            die "Maximum revision number is $MAXREV"
        fi
    else
        die "Revision number must be integer"
    fi
}

# log $msg
function log()
{
    echo -e "$@" | tee -a $REPLAYDIR/replay.log
}

# err $msg
function err()
{
    echo -e "$@" | tee -a $REPLAYDIR/replay.err
}

# fatal $msg
function fatal()
{
    err "$@"
    exit 1
}

# build $revno
function build()
{
    . $ENVFILE
    log "[[Building revision $1..."
    cd $BUILDDIR
    if [ $REMAKE -eq 1 -o ! -f Makefile ]; then
        log "\tRe-configuring makefile..."
        if [ -z "$DRYRUN" ]; then
            rm -f $MAKEFILECACHE  2>/dev/null
            cmake $BASEDIR 2>&1 > build.log
            if [ $? -ne 0 ]; then
                fatal "\tReconfiguring failed."
            fi
        fi
    fi
    if [ -z "$DRYRUN" ]; then
        time make -j8 > make.log 2>&1
    fi
    if [ $? -ne 0 ]; then 
        fatal "Failed to build revision $i"
    fi
    log "\tRevision $i is built."
    log "[[Going back to $BASEDIR..."
    cd $BASEDIR
    if [ $? -ne 0 ]; then 
        fatal "Failed to go back to base dir $BASEDIR"
    fi
    log "\t Back to $BASEDIR"
}

if [ ! -x $PARSER ]; then
    die "$PARSER does not exist or is not executable"
fi

if [ ! -f $ENVFILE ]; then
    die "$ENVFILE does not exist"
fi

if [ $# -ne 2 ]; then
    die "Usage: $0 rev1 rev2"
fi

checkarg $1 $2

if [ ! -d $BUILDDIR ]; then
    mkdir $BUILDDIR
fi

if [ ! -d $REPLAYDIR/$DIFFDIR ]; then
    mkdir -p $REPLAYDIR/$DIFFDIR 
fi

if [ ! -d $REPLAYDIR/$OBJDIR ]; then
    mkdir -p $REPLAYDIR/$OBJDIR
fi

echo "" > .replay/replay.log
echo "" > .replay/replay.err
NOOBJ=1
for (( i = $1; i <= $2; i++))
do
    echo "========Transaction begin for revision $i ============" 

    ####Gen Diff####
    log "[[Generating diffs from revision $i..." 
    ##TODO replace with getdiff script
    if [ -z "$DRYRUN" ]; then
        bzr diff -c$i > $REPLAYDIR/$DIFFDIR/$i.diff
    fi
    if [ $? -eq 3 ]; then #bzr diff use 3 as the error status code
        fatal "Failed to generate diff"
    fi
    log "\tDiffs generated"
    
    ####Parse Diff####
    log "[[Parsing diffs..." 
    grep "$MAKEFILENAME" $REPLAYDIR/$DIFFDIR/$i.diff > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        log "\tDiff touched Makefile. Will re-configure."
        REMAKE=1
    else
        log "\tDiff didn't touch Makefile. Will re-use."
        REMAKE=0
    fi
    $PARSER $REPLAYDIR/$DIFFDIR/$i.diff > $REPLAYDIR/$DIFFDIR/$i.pout 2>&1
    if [ $? -eq 3 ]; then #bzr diff use 3 as the error status code
        fatal "Failed to generate diff"
    fi
    log "\tDiffs parsed"

    #if [ ! -s $REPLAYDIR/$DIFFDIR/$i.diff.id ]; then
    #    log "@@Revision $i didn't touch any source files. Skipping..."
    #    echo "********Transaction end  for revision $i ************" 
    #    continue
    #fi

    ####Switch Revision####
    log "[[Switching to revision $i..."
    ##TODO replace with revert script
    if [ -z "$DRYRUN" ]; then
        bzr revert -r $i 2> /dev/null
    fi
    if [ $? -ne 0 ]; then
        fatal "Failed to switch revision." 
    fi
    log "\tNow at revision $i"

    ####Backing up previous revision####
    if [ $NOOBJ -eq 1 ]; then
        ####First Time Build####
        log "@@$i is the start revision and doesn't have old object files."
    else
        ##TODO programmatic
        j=$((i - 1))
        echo $j
        log "[[Backing up revision $j object files..."
        if [ -z "$DRYRUN" ]; then
            cp $BUILDDIR/sql/mysqld.bc $REPLAYDIR/$OBJDIR/mysqld.bc.$j 2>/dev/null
        fi
        if [ $? -ne 0 ]; then
            fatal "Failed to backing up old object files..."
        fi
        log "\tOld object files backed..."
    fi

    ####Build Revision####
    build $i
    NOOBJ=0
    echo "********Transaction end  for revision $i ************" 
    ####Impact Analysis####
done


