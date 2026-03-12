#!/bin/bash

########################################################################################
# Install OpenMV SDK.
SDK_VERSION="1.0.0"
SDK_DIR="${HOME}/openmv-sdk-${SDK_VERSION}"
SDK_BASE_URL="https://download.openmv.io/sdk"
SDK_OS="$(uname -s | tr '[:upper:]' '[:lower:]')"
SDK_ARCH="$(uname -m)"
SDK_TARBALL="openmv-sdk-${SDK_VERSION}-${SDK_OS}-${SDK_ARCH}.tar.gz"
SDK_URL="${SDK_BASE_URL}/${SDK_TARBALL}"

export SDK_DIR
export PATH="${SDK_DIR}/make:${SDK_DIR}/python/bin:${PATH}"

ci_install_sdk() {
    if [ -f "${SDK_DIR}/sdk.version" ] && \
       [ "$(cat ${SDK_DIR}/sdk.version)" = "${SDK_VERSION}" ]; then
        echo "OpenMV SDK ${SDK_VERSION} already installed."
        return 0
    fi

    echo "Installing OpenMV SDK ${SDK_VERSION} to ${SDK_DIR}..."
    mkdir -p "${SDK_DIR}"

    tmpfile=$(mktemp)
    trap 'rm -f "$tmpfile"' EXIT

    wget --no-check-certificate --progress=bar:force -O "$tmpfile" "$SDK_URL" || {
        echo "Download failed: $SDK_URL"; return 1
    }

    expected=$(wget --no-check-certificate -qO- "${SDK_URL}.sha256" | awk '{print $1}') || {
        echo "Could not fetch checksum from ${SDK_URL}.sha256"; return 1
    }
    echo "${expected}  ${tmpfile}" | sha256sum -c - || {
        echo "Checksum verification failed!"; return 1
    }

    tar --strip-components=1 -xzf "$tmpfile" -C "${SDK_DIR}" || {
        echo "Extraction failed!"; return 1
    }

    echo "OpenMV SDK ${SDK_VERSION} installed successfully."
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
# Run QEMU unit tests
ci_run_qemu_tests() {
    TARGET="${1}"

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

    # Patch micropython's mpremote for QEMU serial communication
    echo "Patching micropython mpremote for QEMU..."
    patch -N -p1 -d lib/micropython < tools/mpremote-qemu-serial.patch || echo "Patch already applied or failed"

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

########################################################################################
# Install FVP
FVP_VERSION="11.31.28"
FVP_INSTALL_DIR="${HOME}/fvp-${FVP_VERSION}"

ci_install_fvp() {
    if [ -d "${FVP_INSTALL_DIR}" ]; then
        echo "FVP ${FVP_VERSION} already installed."
        return 0
    fi

    echo "Installing FVP ${FVP_VERSION}..."
    FVP_MAJOR_MINOR="${FVP_VERSION%.*}"
    FVP_PATCH="${FVP_VERSION##*.}"
    FVP_ARCHIVE="avh-linux-x86_${FVP_MAJOR_MINOR}_${FVP_PATCH}_Linux64.tar.gz"
    FVP_URL="https://artifacts.tools.arm.com/avh/${FVP_VERSION}/${FVP_ARCHIVE}"

    tmpfile=$(mktemp)
    wget --no-check-certificate --progress=bar:force -O "$tmpfile" "$FVP_URL" || {
        echo "FVP download failed: $FVP_URL"; return 1
    }

    mkdir -p "${FVP_INSTALL_DIR}"
    tar --strip-components=1 -xzf "$tmpfile" -C "${FVP_INSTALL_DIR}" || {
        echo "FVP extraction failed!"; return 1
    }
    rm -f "$tmpfile"

    echo "FVP ${FVP_VERSION} installed to ${FVP_INSTALL_DIR}"
}

########################################################################################
# Run FVP unit tests
ci_run_fvp_tests() {
    TARGET="${1}"
    FVP_TELNET_PORT=5555
    export PATH="${FVP_INSTALL_DIR}/bin:${PATH}"

    # Start FVP in background with raw TCP UART
    echo "Starting FVP for ${TARGET}..."
    LD_LIBRARY_PATH="${SDK_DIR}/python/lib" make TARGET=${TARGET} EMULATOR=FVP run > fvp_output.txt 2>&1 &
    FVP_PID=$!

    # Wait for FVP telnet port to be ready
    echo "Waiting for FVP to start..."
    for i in {1..60}; do
        if nc -z localhost ${FVP_TELNET_PORT} 2>/dev/null; then
            break
        fi
        sleep 1
    done

    if ! nc -z localhost ${FVP_TELNET_PORT} 2>/dev/null; then
        echo "Error: FVP telnet port ${FVP_TELNET_PORT} not ready"
        cat fvp_output.txt
        kill ${FVP_PID} 2>/dev/null
        return 1
    fi
    echo "FVP ready on port ${FVP_TELNET_PORT}"
    sleep 2

    # Patch micropython's mpremote for FVP socket communication
    echo "Patching micropython mpremote for FVP..."
    patch -N -p1 -d lib/micropython < tools/mpremote-qemu-serial.patch || echo "Patch already applied or failed"

    # Run unit tests using mpremote over socket
    echo "Running unit tests..."
    timeout 300 python3 lib/micropython/tools/mpremote/mpremote.py \
        connect "socket://localhost:${FVP_TELNET_PORT}" \
        mount scripts/unittest/ run scripts/unittest/run.py 2>&1 | tee test_output.txt
    TEST_EXIT_CODE=${PIPESTATUS[0]}

    # Cleanup
    kill ${FVP_PID} 2>/dev/null

    if [ $TEST_EXIT_CODE -ne 0 ]; then
        echo "mpremote command failed with exit code $TEST_EXIT_CODE"
        return 1
    fi

    if grep -q "Some tests FAILED" test_output.txt; then
        return 1
    fi
    return 0
}
