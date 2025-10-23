DOCKER_IMAGE_NAME = firmware-builder
DOCKER_TAG = latest
CONTAINER_NAME = firmware-container
DOCKERFILE_PATH = Dockerfile

# Detect repository structure
REPO_ROOT := $(shell git rev-parse --show-toplevel)
GIT_DIR := $(shell realpath $$(git rev-parse --git-dir))
GIT_COMMON_DIR := $(shell realpath $$(git rev-parse --git-common-dir))

# Detect worktree: git-dir != git-common-dir
IS_WORKTREE := $(shell [ "$(GIT_DIR)" != "$(GIT_COMMON_DIR)" ] && echo "yes" || echo "no")

# Working directory inside container
WORKDIR = $(REPO_ROOT)

# Volume mounts: always mount repo at actual path
ifeq ($(IS_WORKTREE),yes)
    # For worktrees, also mount the main repo (for .git references)
    MAIN_REPO_PATH := $(shell dirname $(GIT_COMMON_DIR))
    VOLUME_MOUNTS = -v $(MAIN_REPO_PATH)/.git:$(MAIN_REPO_PATH)/.git \
                    -v $(REPO_ROOT):$(WORKDIR)
else
    VOLUME_MOUNTS = -v $(REPO_ROOT):$(WORKDIR)
endif

# Build the Docker image
build-image:
	docker build \
        -t $(DOCKER_IMAGE_NAME):$(DOCKER_TAG) \
        -f $(DOCKERFILE_PATH) ../

# Run the container with a volume mount to build the firmware
build-firmware: build-image
	docker run --rm \
        -e TARGET=$(TARGET) \
        -e HOST_UID=$(shell id -u) \
        -e HOST_GID=$(shell id -g) \
        -w $(WORKDIR) \
        $(VOLUME_MOUNTS) \
        --name $(CONTAINER_NAME) \
        $(DOCKER_IMAGE_NAME):$(DOCKER_TAG) docker/build.sh

# Run an interactive shell in the container
shell:
	docker run --rm -it \
        -e shell="true" \
        -w $(WORKDIR) \
        $(VOLUME_MOUNTS) \
        $(DOCKER_IMAGE_NAME):$(DOCKER_TAG) bash

# Debug target to show detected paths
debug-paths:
	@echo "IS_WORKTREE: $(IS_WORKTREE)"
	@echo "REPO_ROOT: $(REPO_ROOT)"
	@echo "GIT_DIR: $(GIT_DIR)"
	@echo "GIT_COMMON_DIR: $(GIT_COMMON_DIR)"
	@echo "MAIN_REPO_PATH: $(MAIN_REPO_PATH)"
	@echo "WORKDIR: $(WORKDIR)"
	@echo "VOLUME_MOUNTS: $(VOLUME_MOUNTS)"

.DEFAULT_GOAL := build-firmware
