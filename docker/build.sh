#!/bin/bash
set -e -x

BUILD_DIR=/workspace/build/${TARGET}

# Update submodules.
git submodule update --init --depth=1
make -j$(nproc) TARGET=${TARGET} submodules

# Build the firmware.
make -j$(nproc) BUILD=${BUILD_DIR} clean
make -j$(nproc) -C lib/micropython/mpy-cross
make -j$(nproc) BUILD=${BUILD_DIR} TARGET=${TARGET} LLVM_PATH=/workspace/llvm/bin

# Fix permissions.
chown -R ${HOST_UID:-1000}:${HOST_GID:-1000} /workspace/build
