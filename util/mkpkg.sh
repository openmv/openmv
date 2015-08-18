#!/bin/sh
# This script generates a Linux package.
# Execute in openmv/user ./mkpkg.sh
DIST_DIR=dist
BUILD_DIR=build
OPENMV_DIR=openmv
PACKAGE=$(python -c "import sys,platform; print('openmv_'+sys.platform+'_'+platform.machine()+'.tar.gz'.lower())")
SPEC_FILE=openmv-ide.spec

rm -fr $DIST_DIR $BUILD_DIR $PACKAGE
pyinstaller $SPEC_FILE
(cd $DIST_DIR && tar -cvzf ../$PACKAGE $OPENMV_DIR)
rm -fr $DIST_DIR $BUILD_DIR
