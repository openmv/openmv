#!/bin/bash
set -e -x

# Update submodules.
git submodule update --init --depth=1

# Run the per-target clean separately (build-dev.sh skips this for
# incremental builds), then delegate to the shared build sequence.
# Don't pass BUILD= here: it leaks into sub-makes via MAKEFLAGS and breaks per-core nesting.
make -j$(nproc) TARGET=${TARGET} clean
source "$(dirname "$0")/build-common.sh"

# Fix permissions.
chown -R ${HOST_UID:-1000}:${HOST_GID:-1000} /workspace/build
