#!/bin/bash
set -e -x

cd $(dirname $0)/..


export PATH=/source/gcc/gcc/bin:/source/cmake/cmake/bin:$PATH
git submodule update --init --depth=1
git -C src/micropython/ submodule update --init --depth=1

# Build the firmware.
make -j$(nproc) -C src/micropython/mpy-cross
make -j$(nproc) TARGET=$TARGET -C src
mkdir -p ./docker/build/$TARGET
cp -r src/build/bin/* ./docker/build/$TARGET
