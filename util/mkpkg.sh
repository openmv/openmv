#!/bin/sh
# This script generates a Linux package.
# Execute in openmv/user ./mkpkg.sh
DIST_DIR=dist
BUILD_DIR=build
OPENMV_DIR=openmv
PACKAGE=$(python -c "import sys,platform; print(''.join(sys.platform+'_'+platform.machine()).lower())")
SPEC_FILE=openmv-ide.spec

rm -fr $DIST_DIR $BUILD_DIR $PACKAGE
pyinstaller $SPEC_FILE
(cd $DIST_DIR && tar -cvzf ../$PACKAGE $OPENMV_DIR)
rm -fr $DIST_DIR $BUILD_DIR
