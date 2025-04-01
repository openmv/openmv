DOCKER_IMAGE_NAME = firmware-builder
DOCKER_TAG = latest
CONTAINER_NAME = firmware-container
DOCKERFILE_PATH = Dockerfile

# Build the Docker image
build-image:
	docker build \
        -t $(DOCKER_IMAGE_NAME):$(DOCKER_TAG) \
        -f $(DOCKERFILE_PATH) ../

# Run the container with a volume mount to build the firmware
build-firmware: build-image
	docker run --rm \
        -e TARGET=$(TARGET) \
        -v $(PWD)/build:/workspace/build \
        --name $(CONTAINER_NAME) \
        $(DOCKER_IMAGE_NAME):$(DOCKER_TAG) docker/build.sh

# Run an interactive shell in the container
shell:
	docker run --rm -it \
        -e shell="true" \
        -v $(PWD)/build:/workspace/build \
        $(DOCKER_IMAGE_NAME):$(DOCKER_TAG) bash

.DEFAULT_GOAL := build-firmware
