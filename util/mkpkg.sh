#!/bin/bash
# This script generates a Linux package.
# Execute in openmv/user ./mkpkg.sh
DIST_DIR=dist
BUILD_DIR=build
OPENMV_DIR=openmv_$1
if [ "$(uname)" == "Darwin" ]; then
    OPENMV_DIR=openmv_$1.app
fi
PACKAGE=$(python -c "import sys,platform; print('openmv_'+sys.platform+'_'+platform.machine()+'_$1.zip'.lower())")
SPEC_FILE=openmv-ide.spec

# Check args
if [ "$#" = "0" ]; then
    echo "usage mkpkg <version>"
    exit 1
fi

rm -fr $DIST_DIR $BUILD_DIR $PACKAGE
pyinstaller $SPEC_FILE $OPENMV_DIR
(cd $DIST_DIR && zip ../$PACKAGE -r $OPENMV_DIR)
rm -fr $DIST_DIR $BUILD_DIR
