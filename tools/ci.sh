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
        make -j$(nproc) TARGET=${1} PROFILE_ENABLE=${3}
        # Copy artifacts if enabled
        if [ "$4" == "true" ]; then
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
# Install STEdgeAI tools
STEDGEAI_BASE_URL="https://upload.openmv.io/stedgeai"
STEDGEAI_CACHE="${HOME}/cache/stedgeai"

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        STEDGEAI_URL="${STEDGEAI_BASE_URL}/STEdgeAI-3.0.0-linux-x86_64.tar.gz"
        STEDGEAI_SHA256="55e31c2a541048616d7a0899de3906acf68655c21fe497eae3c983d067393099"
        ;;
    Darwin-arm64)
        STEDGEAI_URL="${STEDGEAI_BASE_URL}/STEdgeAI-3.0.0-darwin-arm64.tar.gz"
        STEDGEAI_SHA256="a4bc2605a78bc866b94517e90fac1e930431f4061e0c1265cd78f987963e383f"
        ;;
    *)
        echo "Unsupported platform: $(uname -s)-$(uname -m)"
        ;;
esac

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

########################################################################################
# Run QEMU unit tests
ci_run_qemu_tests() {
    TARGET="${1}"

    export PATH=${GNU_MAKE_PATH}:${GCC_TOOLCHAIN_PATH}/bin:${PATH}

    # Patch micropython's mpremote for QEMU serial communication
    echo "Patching micropython mpremote for QEMU..."
    patch -N -p1 -d lib/micropython < tools/mpremote-qemu-serial.patch || echo "Patch already applied or failed"

    # Start QEMU in background and capture output
    echo "Starting QEMU for ${TARGET}..."
    make TARGET=${TARGET} run > qemu_output.txt 2>&1 &

    # Wait for QEMU to start and find the serial port from output
    echo "Waiting for QEMU to start..."
    for i in {1..30}; do
        if grep -q "char device redirected to" qemu_output.txt; then
            break
        fi
        sleep 1
    done

    # Extract the pts device from QEMU output
    PTS_DEVICE=$(grep "redirected to" qemu_output.txt | sed 's/.*redirected to \(\/dev\/pts\/[0-9]*\).*/\1/')

    if [ -z "$PTS_DEVICE" ]; then
        echo "Error: Could not find QEMU serial port"
        cat qemu_output.txt
        return 1
    else
        echo "Found QEMU serial port: $PTS_DEVICE"
    fi

    # Run unit tests using patched mpremote from micropython
    echo "Running unit tests..."
    python3 lib/micropython/tools/mpremote/mpremote.py connect $PTS_DEVICE \
        mount scripts/unittest/ run scripts/unittest/run.py 2>&1 | tee test_output.txt
    TEST_EXIT_CODE=${PIPESTATUS[0]}

    if [ $TEST_EXIT_CODE -ne 0 ]; then
        echo "❌ mpremote command failed with exit code $TEST_EXIT_CODE"
        return 1
    fi

    # Check if tests failed
    if grep -q "Some tests FAILED" test_output.txt; then
        return 1
    fi
    return 0
}
