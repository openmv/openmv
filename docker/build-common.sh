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
# Callers may set BUILD_OPTS for extra `make` flags. Never pass BUILD= through it:
# it leaks into sub-makes via MAKEFLAGS and breaks per-core nesting (e.g. AE3).
set -e -x

make -j$(nproc) TARGET=${TARGET} submodules
make -j$(nproc) -C lib/micropython/mpy-cross
make -j$(nproc) ${BUILD_OPTS:-} TARGET=${TARGET}
