#!/bin/bash
# Shared firmware build sequence sourced by build.sh and build-dev.sh.
#
# Callers must already have:
#   - the working directory at the openmv repo root
#   - the toolchain on PATH (Dockerfile-baked for build.sh; runtime export
#     from $SDK_DIR for build-dev.sh)
#   - submodules initialised (each caller does this with its own protocol
#     scoping; build-dev.sh allows file:// for worktree alternates)
#
# Callers may set BUILD_OPTS for additional flags passed to the firmware
# `make`. build.sh sets BUILD=<workspace path> to keep artifacts under
# /workspace/build; build-dev.sh leaves BUILD_OPTS empty so the top-level
# Makefile's default applies and per-port subdir nesting (e.g. AE3
# multi-core) works.
set -e -x

make -j$(nproc) TARGET=${TARGET} submodules
# The Unix target builds its own mpy-cross.
if [ "${TARGET}" != UNIX ]; then
    make -j$(nproc) -C lib/micropython/mpy-cross
fi
make -j$(nproc) ${BUILD_OPTS:-} TARGET=${TARGET}
