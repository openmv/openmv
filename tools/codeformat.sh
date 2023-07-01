#!/bin/sh
CFG_PATH=`dirname $0`/uncrustify.cfg
BASE_DIR=`dirname $0`

if [ $# -lt 1 ]; then
    echo "usage `basename $0` file.(h/c)"
    exit 1
fi

for file in "$@"; do
    ${BASE_DIR}/uncrustify -c ${CFG_PATH} --no-backup $file
done
