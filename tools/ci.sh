#!/bin/bash

########################################################################################
# Install ARM GCC.
GCC_TOOLCHAIN_PATH=${HOME}/cache/gcc
GCC_TOOLCHAIN_URL="https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz"

LLVM_TOOLCHAIN_PATH=${HOME}/cache/llvm
LLVM_TOOLCHAIN_URL="https://github.com/ARM-software/LLVM-embedded-toolchain-for-Arm/releases/download/release-18.1.3/LLVM-ET-Arm-18.1.3-Linux-x86_64.tar.xz"

GNU_MAKE_PATH=${HOME}/cache/make
GNU_MAKE_URL="https://ftp.gnu.org/gnu/make/make-4.4.1.tar.gz"

ci_install_arm_gcc() {
    mkdir -p ${GCC_TOOLCHAIN_PATH}
    wget --no-check-certificate -O - ${GCC_TOOLCHAIN_URL} | tar --strip-components=1 -Jx -C ${GCC_TOOLCHAIN_PATH}
    export PATH=${GCC_TOOLCHAIN_PATH}/bin:${PATH}
    arm-none-eabi-gcc --version
}

ci_install_arm_llvm() {
    mkdir -p ${LLVM_TOOLCHAIN_PATH}
    wget --no-check-certificate -O - ${LLVM_TOOLCHAIN_URL} | tar --strip-components=1 -Jx -C ${LLVM_TOOLCHAIN_PATH}
    export PATH=${LLVM_TOOLCHAIN_PATH}/bin:${PATH}
    clang --version
}

ci_install_gnu_make() {
    mkdir -p ${GNU_MAKE_PATH}
    wget --no-check-certificate -O - ${GNU_MAKE_URL} | tar --strip-components=1 -zx -C ${GNU_MAKE_PATH}
    cd ${GNU_MAKE_PATH} && ./configure && make -j$(nproc)
    export PATH=${GNU_MAKE_PATH}:${PATH}
    make --version
}

########################################################################################
# Update Submodules.

ci_update_submodules() {
  git submodule update --init --depth=1 --no-single-branch
  git -C src/micropython/ submodule update --init --depth=1
#  (cd src/micropython/ && for remote in `git branch -r | grep -v /HEAD | grep -v master`; do git checkout --track $remote ; done)
}

########################################################################################
# Build Targets.

ci_build_target() {
    export PATH=${GNU_MAKE_PATH}:${GCC_TOOLCHAIN_PATH}/bin:${PATH}
    make -j$(nproc) -C src/micropython/mpy-cross
    make -j$(nproc) TARGET=${1} LLVM_PATH=${LLVM_TOOLCHAIN_PATH}/bin -C src
    mv src/build/bin ${1}
}

########################################################################################
# Prepare Firmware Packages.

ci_package_firmware_release() {
    # Add WiFi firmware blobs
    cp -rf src/drivers/cyw4343/firmware firmware/CYW4343
    cp -rf src/drivers/winc1500/firmware firmware/WINC1500
    (cd firmware && zip -r ../firmware_${1}.zip *)
}

ci_package_firmware_development() {
    (cd firmware && for i in *; do zip -r -j "firmware_${i%/}.zip" "$i"; done)
}

########################################################################################
# Install code formatter deps
CODEFORMAT_PATH=${HOME}/cache/deps/
UNCRUSTIFY_PATH=${CODEFORMAT_PATH}/uncrustify
UNCRUSTIFY_URL="https://github.com/uncrustify/uncrustify/archive/uncrustify-0.75.0.tar.gz"

ci_install_code_format_deps() {
    sudo apt-get install wget cmake build-essential colordiff

    mkdir -p ${UNCRUSTIFY_PATH}
    wget --no-check-certificate -O - ${UNCRUSTIFY_URL} | tar xvz --strip-components=1 -C ${UNCRUSTIFY_PATH}
    (cd ${UNCRUSTIFY_PATH} && mkdir build && cd build && cmake .. && cmake --build .)

    # Copy binaries to cache
    mkdir -p ${CODEFORMAT_PATH}/bin
    cp ${UNCRUSTIFY_PATH}/build/uncrustify ${CODEFORMAT_PATH}/bin/
    cp `which colordiff` ${CODEFORMAT_PATH}/bin/
    chmod +x ${CODEFORMAT_PATH}/bin/uncrustify
}

########################################################################################
# Run code formatter

ci_run_code_format_check() {
    export PATH=${CODEFORMAT_PATH}/bin:${PATH}
    UNCRUSTIFY_CONFIG=tools/uncrustify.cfg

    exit_code=0
    for file in "$@"; do
        file_fmt="${file}.tmp"
        uncrustify -q -c ${UNCRUSTIFY_CONFIG} -f ${file} -o ${file_fmt} || true

        diff -q -u ${file} ${file_fmt} >> /dev/null 2>&1 || {
            colordiff -u ${file} ${file_fmt} || true
            exit_code=1
        }
    done
    exit $exit_code
}
