#!/bin/bash 
################
#
# Script to back up file using current timestamp as suffix.
# 
# Author: Ryan
# 
# Date: 2012/02/05

usage()
{
cat << EOF
    Usage: $0 [OPTIONS] [FILES] - backup the current directory(default) or a specified file(s)

    OPTIONS:

        -h help

EOF
}
while getopts "f:h" OP
do
    case $OP in
        h)
            usage
            exit 1
            ;;
        ?)
            usage
            exit 
            ;;
    esac
done
shift $(($OPTIND - 1))
if [ $# -eq 0 ]; then
    tar czvf `date +%Y.%m.%d.%H.%M.%S`.tar.gz *
    exit 1
fi
for FILE in "$@"
do
    cp $FILE $FILE.`date +%Y.%m.%d.%H.%M`
done
