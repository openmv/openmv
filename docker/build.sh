#!/bin/bash
set -e -x

cd $(dirname $0)/..


export PATH=/source/gcc/bin:/source/cmake/bin:$PATH
git submodule update --init --depth=1
git -C src/lib/micropython/ submodule update --init --depth=1

# Build the firmware.
make -j$(nproc) -C src/lib/micropython/mpy-cross
make -j$(nproc) TARGET=$TARGET LLVM_PATH=/source/llvm/bin -C src
mkdir -p ./docker/build/$TARGET
cp -r src/build/bin/* ./docker/build/$TARGET
