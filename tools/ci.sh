#!/bin/bash

########################################################################################
# Install ARM GCC.
GCC_TOOLCHAIN_PATH=${HOME}/cache/gcc
GCC_TOOLCHAIN_URL="https://developer.arm.com/-/media/Files/downloads/gnu/14.3.rel1/binrel/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi.tar.xz"

ci_install_arm_gcc() {
    mkdir -p ${GCC_TOOLCHAIN_PATH}
    wget --no-check-certificate -O - ${GCC_TOOLCHAIN_URL} | tar --strip-components=1 -Jx -C ${GCC_TOOLCHAIN_PATH}
    export PATH=${GCC_TOOLCHAIN_PATH}/bin:${PATH}
    arm-none-eabi-gcc --version
}

########################################################################################
# Install ARM LLVM.
LLVM_TOOLCHAIN_PATH=${HOME}/cache/llvm
LLVM_TOOLCHAIN_URL="https://github.com/ARM-software/LLVM-embedded-toolchain-for-Arm/releases/download/release-18.1.3/LLVM-ET-Arm-18.1.3-Linux-x86_64.tar.xz"

ci_install_arm_llvm() {
    mkdir -p ${LLVM_TOOLCHAIN_PATH}
    wget --no-check-certificate -O - ${LLVM_TOOLCHAIN_URL} | tar --strip-components=1 -Jx -C ${LLVM_TOOLCHAIN_PATH}
    export PATH=${LLVM_TOOLCHAIN_PATH}/bin:${PATH}
    clang --version
}

########################################################################################
# Install GNU Make.
GNU_MAKE_PATH=${HOME}/cache/make
GNU_MAKE_URL="https://ftp.gnu.org/gnu/make/make-4.4.1.tar.gz"

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
  git -C lib/micropython/ submodule update --init --depth=1
}

########################################################################################
# Build Targets.
ci_build_target() {
    export LLVM_PATH=${LLVM_TOOLCHAIN_PATH}/bin
    export PATH=${GNU_MAKE_PATH}:${GCC_TOOLCHAIN_PATH}/bin:${PATH}
    if [ "$1" == "DOCKER" ]; then
        BOARD=ARDUINO_NICLA_VISION
        make -j$(nproc) -C docker TARGET=${BOARD}
    else
        make -j$(nproc) -C lib/micropython/mpy-cross
        make -j$(nproc) TARGET=${1} PROFILE_ENABLE=${2}
        # Don't copy artifacts for profiling builds
        if [ "$2" = 0 ]; then
            mv build/bin ${1}
        fi
    fi
}

########################################################################################
# Prepare Firmware Packages.
ci_package_firmware_release() {
    # Add WiFi firmware blobs
    cp -rf drivers/cyw4343/firmware firmware/CYW4343
    cp -rf drivers/winc1500/firmware firmware/WINC1500
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

########################################################################################
# Install STEdgeAI tools
STEDGEAI_URL="https://upload.openmv.io/stedgeai/STEdgeAI-2.1.0.tar.gz"
STEDGEAI_SHA256="888e71715127ff6384e38fcde96eea28f53f8370b2bb9cf0d2f6f939001b350c"
STEDGEAI_CACHE="${HOME}/cache/stedgeai"

ci_install_stedgeai() {
    STEDGEAI_PATH="${1}"
    
    # If cached in CI, copy from cache to build.
    if [ -d "${STEDGEAI_CACHE}" ]; then
        mkdir -p "${STEDGEAI_PATH}"
        cp -r "${STEDGEAI_CACHE}/." "${STEDGEAI_PATH}"
        touch "${STEDGEAI_PATH}/stedgeai.stamp"
        return 0
    fi

    # Download and install to STEDGEAI_PATH
    echo "Downloading STEdge AI tools..."
    mkdir -p "${STEDGEAI_PATH}"
    
    # Create temporary file
    tmpfile=$(mktemp)
    trap 'rm -f "$tmpfile"' EXIT
    
    # Download and verify checksum
    wget --no-check-certificate -O "$tmpfile" "$STEDGEAI_URL" || {
        echo "Download failed!"
        return 1
    }
    
    echo "${STEDGEAI_SHA256}  ${tmpfile}" | sha256sum -c - || {
        echo "Checksum failed!"
        return 1
    }
    
    # Extract the tools
    echo "Extracting to ${STEDGEAI_PATH}..."
    tar -xzf "$tmpfile" -C "${STEDGEAI_PATH}" --strip-components=1 || {
        echo "Extraction failed!"
        return 1
    }
    
    touch "${STEDGEAI_PATH}/stedgeai.stamp"
   
    echo "STEdgeAI installed successfully to ${STEDGEAI_PATH}"
    return 0
}
