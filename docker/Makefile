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

SDK_VERSION = 1.0.0
SDK_DIR ?= $(HOME)/openmv-sdk-$(SDK_VERSION)

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
        -v $(PWD)/build:/workspace/build \
        -v $(SDK_DIR):/workspace/sdk \
        --name $(CONTAINER_NAME) \
        $(DOCKER_IMAGE_NAME):$(DOCKER_TAG) docker/build.sh

# Run an interactive shell in the container
shell:
	docker run --rm -it \
        -e shell="true" \
        -v $(PWD)/build:/workspace/build \
        $(DOCKER_IMAGE_NAME):$(DOCKER_TAG) bash

.DEFAULT_GOAL := build-firmware
