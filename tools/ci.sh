#!/bin/bash

########################################################################################
# Install ARM GCC.
TOOLCHAIN="gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2"
TOOLCHAIN_PATH=${HOME}/cache/gcc
TOOLCHAIN_URL="https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/10-2020q4/${TOOLCHAIN}"

ci_install_arm_gcc_apt() {
    sudo apt-get install gcc-arm-none-eabi libnewlib-arm-none-eabi
    arm-none-eabi-gcc --version
}

ci_install_arm_gcc() {
    mkdir -p ${TOOLCHAIN_PATH}
    wget --no-check-certificate -O - ${TOOLCHAIN_URL} | tar --strip-components=1 -jx -C ${TOOLCHAIN_PATH}
    export PATH=${TOOLCHAIN_PATH}/bin:${PATH}
    arm-none-eabi-gcc --version
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
    export PATH=${TOOLCHAIN_PATH}/bin:${PATH}
    make -j$(nproc) -C src/micropython/mpy-cross
    make -j$(nproc) TARGET=${1} -C src
    mkdir firmware
    mv src/build/bin firmware/${1}
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
