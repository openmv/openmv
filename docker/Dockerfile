FROM ubuntu:24.04

# Install dependencies
RUN apt update && apt install -y \
    build-essential \
    wget \
    git \
    python3 \
    libglib2.0-0

ENV SDK_DIR=/workspace/sdk
ENV PATH="${SDK_DIR}/gcc/bin:${SDK_DIR}/llvm/bin:${SDK_DIR}/make:${SDK_DIR}/cmake/bin:${SDK_DIR}/python/bin:${SDK_DIR}/stcubeprog/bin:${PATH}"

WORKDIR /workspace

RUN git config --global --add safe.directory '*'
