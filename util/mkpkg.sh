#!/bin/sh
# This script generates a Linux package.
# Execute in openmv/user ./mkpkg.sh
DIST_DIR=dist
BUILD_DIR=build
OPENMV_DIR=openmv-ide
PACKAGE=openmv.tar.gz
SPEC_FILE=openmv-ide.spec

rm -fr $DIST_DIR $PACKAGE
pyinstaller --clean --hidden-import=usb --hidden-import=numpy openmv-ide.py
cp -r openmv-ide.glade $DIST_DIR/$OPENMV_DIR
cp -r examples $DIST_DIR/$OPENMV_DIR
cp -r ../udev $DIST_DIR/$OPENMV_DIR
cp openmv-cascade.py $DIST_DIR/$OPENMV_DIR
(cd $DIST_DIR && tar -cvzf ../$PACKAGE $OPENMV_DIR)
rm -fr $DIST_DIR
rm -fr $BUILD_DIR
rm $SPEC_FILE
