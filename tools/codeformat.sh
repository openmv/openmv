#!/bin/sh
CFG_PATH=`dirname $0`/uncrustify.cfg

if [ $# -ne 1 ]; then
    echo "usage `basename $0` file.(h/c)"
    exit 1
fi

uncrustify -c ${CFG_PATH} --no-backup -lC $1
