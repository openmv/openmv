#!/bin/sh
CFG_PATH=`dirname $0`/uncrustify.cfg
BASE_DIR=`dirname $0`

if [ $# -lt 1 ]; then
    echo "usage `basename $0` file.(h/c)"
    exit 1
fi

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        UNCRUSTIFY=${BASE_DIR}/uncrustify.x86_64
        ;;
    Darwin-arm64)
        UNCRUSTIFY=${BASE_DIR}/uncrustify.arm64
        ;;
    *)
        echo "Unsupported platform: $(uname -s)-$(uname -m)"
        exit 1
        ;;
esac

for file in "$@"; do
    ${UNCRUSTIFY} -c ${CFG_PATH} --no-backup $file
done
