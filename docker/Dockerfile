FROM ubuntu:24.04

# Install dependencies
RUN apt update && apt install -y \
    build-essential \
    wget \
    git

ENV SDK_DIR=/workspace/sdk
ENV PATH="${SDK_DIR}/gcc/bin:${SDK_DIR}/llvm/bin:${SDK_DIR}/make:${SDK_DIR}/cmake/bin:${SDK_DIR}/python/bin:${SDK_DIR}/stcubeprog/bin:${PATH}"

WORKDIR /workspace

COPY . .

RUN git config --global --add safe.directory '*'
