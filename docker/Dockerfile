FROM ubuntu:20.04 AS dapper
ARG DAPPER_HOST_ARCH
ENV DAPPER_ENV TARGET
ENV ARCH $DAPPER_HOST_ARCH
ENV DAPPER_OUTPUT ./docker/build
ENV DAPPER_DOCKER_SOCKET true
ENV DAPPER_TARGET dapper
RUN apt update && apt install -y build-essential wget git python3 python-is-python3
ENV GCC_URL="https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/10-2020q4/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2" 
RUN mkdir ./gcc
RUN wget --no-check-certificate -O - ${GCC_URL} | tar --strip-components=1 -jx -C ./gcc
RUN mkdir ./cmake && \
    CMAKE_URL="https://cmake.org/files/v3.20/cmake-3.20.0-linux-x86_64.tar.gz" && \
    wget --no-check-certificate -O - ${CMAKE_URL} | tar --strip-components=1 -xz -C ./cmake


RUN mkdir -p /source/gcc && mkdir -p /source/cmake && cp -r ./gcc /source/gcc && cp -r ./cmake /source/cmake
WORKDIR /source
ENTRYPOINT ["./docker/build.sh"]
