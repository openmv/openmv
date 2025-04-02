#!/bin/bash
set -e -x

git submodule update --init --depth=1
make -C src -j$(nproc) TARGET=$TARGET submodules

# Build the firmware.
make -j$(nproc) -C src clean
make -j$(nproc) -C src/lib/micropython/mpy-cross
make -j$(nproc) -C src TARGET=$TARGET LLVM_PATH=/workspace/llvm/bin

rm -fr /workspace/build/$TARGET
mkdir -p /workspace/build/$TARGET
cp -r src/build/bin/* /workspace/build/$TARGET
chown -R ${HOST_UID:-1000}:${HOST_GID:-1000} /workspace/build
