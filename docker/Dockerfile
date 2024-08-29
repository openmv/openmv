FROM ubuntu:20.04 AS dapper
ARG DAPPER_HOST_ARCH
ENV DAPPER_ENV TARGET
ENV ARCH $DAPPER_HOST_ARCH
ENV DAPPER_OUTPUT ./docker/build
ENV DAPPER_DOCKER_SOCKET true
ENV DAPPER_TARGET dapper
RUN apt update && apt install -y build-essential wget git python3 python-is-python3

RUN mkdir -p /source/gcc
ENV GCC_URL="https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz" 
RUN wget --no-check-certificate -O - ${GCC_URL} | tar --strip-components=1 -Jx -C /source/gcc

RUN mkdir -p /source/llvm
ENV LLVM_URL="https://github.com/ARM-software/LLVM-embedded-toolchain-for-Arm/releases/download/release-18.1.3/LLVM-ET-Arm-18.1.3-Linux-x86_64.tar.xz"
RUN wget --no-check-certificate -O - ${LLVM_URL} | tar --strip-components=1 -Jx -C /source/llvm

RUN mkdir -p /source/cmake
ENV CMAKE_URL="https://github.com/Kitware/CMake/releases/download/v3.30.2/cmake-3.30.2.tar.gz"
RUN wget --no-check-certificate -O - ${CMAKE_URL} | tar --strip-components=1 -xz -C /source/cmake

WORKDIR /source
ENTRYPOINT ["./docker/build.sh"]
