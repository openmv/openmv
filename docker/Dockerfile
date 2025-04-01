FROM ubuntu:24.04

# Install dependencies
RUN apt update && apt install -y \
    build-essential \
    wget \
    git \
    python3 \
    python-is-python3

# Download and extract GCC
RUN mkdir -p /workspace/gcc
ENV GCC_URL="https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz"
RUN wget --no-check-certificate -O - ${GCC_URL} | tar --strip-components=1 -Jx -C /workspace/gcc

# Download and extract LLVM
RUN mkdir -p /workspace/llvm
ENV LLVM_URL="https://github.com/ARM-software/LLVM-embedded-toolchain-for-Arm/releases/download/release-18.1.3/LLVM-ET-Arm-18.1.3-Linux-x86_64.tar.xz"
RUN wget --no-check-certificate -O - ${LLVM_URL} | tar --strip-components=1 -Jx -C /workspace/llvm

# Download and extract CMake
RUN mkdir -p /workspace/cmake
ENV CMAKE_URL="https://github.com/Kitware/CMake/releases/download/v3.30.2/cmake-3.30.2.tar.gz"
RUN wget --no-check-certificate -O - ${CMAKE_URL} | tar --strip-components=1 -xz -C /workspace/cmake

# Download, build, and install Make
RUN mkdir -p /workspace/make
ENV MAKE_URL="https://ftp.gnu.org/gnu/make/make-4.4.1.tar.gz"
RUN wget --no-check-certificate -O - ${MAKE_URL} | tar --strip-components=1 -xz -C /workspace/make
RUN cd /workspace/make && ./configure && make -j$(nproc)

# Set up directories
WORKDIR /workspace

COPY . .

RUN git config --global --add safe.directory '*'
ENV PATH="/workspace/gcc/bin:/workspace/llvm/bin:/workspace/cmake/bin:/workspace/make:$PATH"
