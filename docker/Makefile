# SPDX-License-Identifier: MIT
#
# Copyright (C) 2013-2024 OpenMV, LLC.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
DOCKER_IMAGE_NAME = firmware-builder
DOCKER_TAG = latest
CONTAINER_NAME = firmware-container
DOCKERFILE_PATH = Dockerfile

SDK_VERSION := $(shell cat ../SDK_VERSION)
SDK_DIR ?= $(HOME)/openmv-sdk-$(SDK_VERSION)

# Build the Docker image
build-image:
	docker build \
        -t $(DOCKER_IMAGE_NAME):$(DOCKER_TAG) \
        -f $(DOCKERFILE_PATH) .

# Run the container with a volume mount to build the firmware
build-firmware: build-image
	docker run --rm \
        -e TARGET=$(TARGET) \
        -e HOST_UID=$(shell id -u) \
        -e HOST_GID=$(shell id -g) \
        -v $(realpath ../):/workspace \
        -v $(SDK_DIR):/workspace/sdk \
        --name $(CONTAINER_NAME) \
        $(DOCKER_IMAGE_NAME):$(DOCKER_TAG) docker/build.sh

# Run an interactive shell in the container
shell:
	docker run --rm -it \
        -e shell="true" \
        -v $(realpath ../):/workspace \
        $(DOCKER_IMAGE_NAME):$(DOCKER_TAG) bash

# ----------------------------------------------------------------------------
# Local development targets.
#
# These mirror build-firmware / shell but mount the repo (and SDK) at the
# same absolute path inside the container as on the host. This keeps debug
# info paths in build artifacts matching the host filesystem (so gdb,
# addr2line and IDE source navigation work without remapping). Worktrees
# are supported by also mounting the main repo so the worktree's .git
# pointer file resolves.
#
# Unlike the default targets, build-firmware-dev does not run `make clean`
# before building, so subsequent invocations are incremental.
# ----------------------------------------------------------------------------

# Repo root and git layout, resolved on the host.
DEV_REPO_ROOT := $(realpath ../)
DEV_GIT_DIR := $(realpath $(shell git -C .. rev-parse --git-dir 2>/dev/null))
DEV_GIT_COMMON_DIR := $(realpath $(shell git -C .. rev-parse --git-common-dir 2>/dev/null))

# Resolve SDK_DIR via realpath when the directory exists; fall back to the
# abspath form so error messages report a concrete path. The _check-sdk-dev
# target fails fast before any docker invocation if the resolved path does
# not exist, which is what actually keeps the docker run safe (the mount
# line itself would be malformed for a missing path: `-v :` with no LHS).
DEV_SDK_DIR := $(or $(realpath $(SDK_DIR)),$(abspath $(SDK_DIR)))

# A worktree has git-dir != git-common-dir. Mount the main repo too so the
# worktree's .git pointer file (and the parent .git/worktrees/... entry it
# references) resolves inside the container.
#
# Resolve the main repo path via `git worktree list --porcelain` rather than
# string-stripping the common-dir, which only works when the main repo's git
# directory is named exactly `.git` (breaks for `--separate-git-dir`,
# core.worktree, bare-style layouts).
DEV_IS_WORKTREE := $(shell [ -n "$(DEV_GIT_DIR)" ] && [ "$(DEV_GIT_DIR)" != "$(DEV_GIT_COMMON_DIR)" ] && echo yes || echo no)
DEV_MAIN_REPO := $(if $(filter yes,$(DEV_IS_WORKTREE)),$(shell git -C .. worktree list --porcelain 2>/dev/null | awk '/^worktree / {print $$2; exit}'))

# Any new target that consumes $(DEV_MOUNTS) MUST also depend on
# _check-sdk-dev, otherwise a missing SDK_DIR yields a malformed `-v :`
# mount and docker auto-creates an empty root-owned directory on the host.
DEV_MOUNTS = -v $(DEV_REPO_ROOT):$(DEV_REPO_ROOT) \
             -v $(DEV_SDK_DIR):$(DEV_SDK_DIR)
ifeq ($(DEV_IS_WORKTREE),yes)
ifeq ($(strip $(DEV_MAIN_REPO)),)
$(error In a worktree but could not resolve main repo path via 'git worktree list'. Run 'make debug-paths-dev' to diagnose.)
endif
DEV_MOUNTS += -v $(DEV_MAIN_REPO):$(DEV_MAIN_REPO)
endif

_check-sdk-dev:
	@test -d "$(DEV_SDK_DIR)" || { \
        echo "ERROR: SDK_DIR=$(SDK_DIR) does not exist."; \
        echo "Run 'make install-sdk' to fetch the SDK pinned in SDK_VERSION,"; \
        echo "or pass SDK_DIR=<path> on the make command line."; \
        exit 1; \
    }

build-firmware-dev: _check-sdk-dev build-image
	docker run --rm \
        --user $(shell id -u):$(shell id -g) \
        -e HOME=/tmp \
        -e TARGET=$(TARGET) \
        -e SDK_DIR=$(DEV_SDK_DIR) \
        -w $(DEV_REPO_ROOT) \
        $(DEV_MOUNTS) \
        $(DOCKER_IMAGE_NAME):$(DOCKER_TAG) docker/build-dev.sh

shell-dev: _check-sdk-dev build-image
	docker run --rm -it \
        --user $(shell id -u):$(shell id -g) \
        -e HOME=/tmp \
        -e SDK_DIR=$(DEV_SDK_DIR) \
        -w $(DEV_REPO_ROOT) \
        $(DEV_MOUNTS) \
        $(DOCKER_IMAGE_NAME):$(DOCKER_TAG) bash

clean-dev:
	rm -rf $(DEV_REPO_ROOT)/build $(DEV_REPO_ROOT)/lib/micropython/mpy-cross/build

# Download and extract the OpenMV SDK to $(SDK_DIR). Version is pinned in
# the repo's SDK_VERSION file; tools/ci.sh::ci_install_sdk is the canonical
# installer (verifies sha256, idempotent via sdk.version stamp file).
install-sdk:
	@command -v wget >/dev/null 2>&1 || { echo "wget is required to download the SDK."; exit 1; }
	@bash -c 'source ../tools/ci.sh && ci_install_sdk'

# Print resolved dev-mode paths for debugging mount detection.
debug-paths-dev:
	@echo "DEV_REPO_ROOT     = $(DEV_REPO_ROOT)"
	@echo "DEV_GIT_DIR       = $(DEV_GIT_DIR)"
	@echo "DEV_GIT_COMMON_DIR= $(DEV_GIT_COMMON_DIR)"
	@echo "DEV_IS_WORKTREE   = $(DEV_IS_WORKTREE)"
	@echo "DEV_MAIN_REPO     = $(DEV_MAIN_REPO)"
	@echo "DEV_SDK_DIR       = $(DEV_SDK_DIR)"
	@echo "DEV_MOUNTS        = $(DEV_MOUNTS)"

.DEFAULT_GOAL := build-firmware

# Unix targets do not mount the SDK; output goes to build/UNIX.
UNIX_DOCKER_ARGS = --rm \
        --user $(shell id -u):$(shell id -g) \
        -e HOME=/tmp \
        $(foreach v,DEBUG MICROPY_PY_UNITTEST MICROPY_PY_UMALLOC,$(if $(strip $($(v))),-e $(v)=$($(v)))) \
        -w $(DEV_REPO_ROOT) \
        -v $(DEV_REPO_ROOT):$(DEV_REPO_ROOT) \
        $(if $(strip $(DEV_MAIN_REPO)),-v $(DEV_MAIN_REPO):$(DEV_MAIN_REPO))

build-unix: build-image
	docker run $(UNIX_DOCKER_ARGS) -e TARGET=UNIX \
        $(DOCKER_IMAGE_NAME):$(DOCKER_TAG) docker/build-dev.sh

test-unix: build-image
	docker run $(UNIX_DOCKER_ARGS) \
        $(DOCKER_IMAGE_NAME):$(DOCKER_TAG) \
        bash -c 'source tools/ci.sh && ci_run_unix_tests $(DEV_REPO_ROOT)/build/UNIX/bin/micropython'

shell-unix: build-image
	docker run $(UNIX_DOCKER_ARGS) -it \
        $(DOCKER_IMAGE_NAME):$(DOCKER_TAG) bash
