#!/bin/bash
# This file is part of the OpenMV project.
#
# Copyright (C) 2025 OpenMV, LLC.
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# Assembles a platform-specific OpenMV SDK.
#
# Versioning: bump SDK_VERSION when the SDK contents change.
#   MAJOR — layout or ABI breaking changes
#   MINOR — new tools or component upgrades
#   PATCH — checksum or repackaging fixes

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

########################################################################################
# Configuration
SDK_VERSION="1.1.0"
SDK_PLATFORM="$(uname -s | tr '[:upper:]' '[:lower:]')-$(uname -m)"
SDK_NAME="openmv-sdk-${SDK_VERSION}-${SDK_PLATFORM}"
BUILD_DIR="sdk"
SDK_STAGE="${BUILD_DIR}/${SDK_NAME}"
NPROC=$(nproc 2>/dev/null || sysctl -n hw.ncpu)

########################################################################################
# Helper functions

download() {
    local url="$1" dest="$2"
    echo "  Downloading $(basename "${url}")..."
    curl -fL --progress-bar -o "${dest}" "${url}"
}

verify_sha256() {
    local file="$1" expected="$2"
    echo "  Verifying $(basename "${file}")..."
    if command -v sha256sum &>/dev/null; then
        echo "${expected}  ${file}" | sha256sum -c -
    else
        echo "${expected}  ${file}" | shasum -a 256 -c -
    fi
}

sha256_file() {
    if command -v sha256sum &>/dev/null; then
        sha256sum "$1"
    else
        shasum -a 256 "$1"
    fi
}

########################################################################################
# Base URLs (platform-independent prefix)
GCC_BASE="https://developer.arm.com/-/media/Files/downloads/gnu/14.3.rel1/binrel/arm-gnu-toolchain-14.3.rel1"
LLVM_BASE="https://github.com/ARM-software/LLVM-embedded-toolchain-for-Arm/releases/download/release-18.1.3/LLVM-ET-Arm-18.1.3"
PYTHON_BASE="https://github.com/astral-sh/python-build-standalone/releases/download/20231002/cpython-3.12.0+20231002"
STEDGEAI_BASE="https://download.openmv.io/stedgeai/STEdgeAI-3.0.0"
CUBEPROG_BASE="https://download.openmv.io/stcubeprog/STCubeProg-2.21.0"
CMAKE_BASE="https://github.com/Kitware/CMake/releases/download/v3.30.2/cmake-3.30.2"
GNU_MAKE_URL="https://ftpmirror.gnu.org/make/make-4.4.1.tar.gz"

########################################################################################
# Platform-specific downloads: triples of dest, url, sha256
case "${SDK_PLATFORM}" in
    linux-x86_64)
    DOWNLOADS=(
        "gcc.tar.xz"
        "${GCC_BASE}-x86_64-arm-none-eabi.tar.xz"
        "8f6903f8ceb084d9227b9ef991490413014d991874a1e34074443c2a72b14dbd"

        "llvm.tar.xz"
        "${LLVM_BASE}-Linux-x86_64.tar.xz"
        "7afae248ac33f7daee95005d1b0320774d8a5495e7acfb9bdc9475d3ad400ac9"

        "cmake.tar.gz"
        "${CMAKE_BASE}-linux-x86_64.tar.gz"
        "cdd7fb352605cee3ae53b0e18b5929b642900e33d6b0173e19f6d4f2067ebf16"

        "stedgeai.tar.gz"
        "${STEDGEAI_BASE}-linux-x86_64.tar.gz"
        "55e31c2a541048616d7a0899de3906acf68655c21fe497eae3c983d067393099"

        "stcubeprog.tar.gz"
        "${CUBEPROG_BASE}-linux-x86_64.tar.gz"
        "74b5c5f88ae3ed34ee7eaf8d5987016d71b4bfa456e1933c4b8b909ad281cd95"

        "python.tar.gz"
        "${PYTHON_BASE}-x86_64-unknown-linux-gnu-install_only.tar.gz"
        "e51a5293f214053ddb4645b2c9f84542e2ef86870b8655704367bd4b29d39fe9"

        "make.tar.gz"
        "${GNU_MAKE_URL}"
        "dd16fb1d67bfab79a72f5e8390735c49e3e8e70b4945a15ab1f81ddb78658fb3"
    )
    ;;
    darwin-arm64)
    DOWNLOADS=(
        "gcc.tar.xz"
        "${GCC_BASE}-darwin-arm64-arm-none-eabi.tar.xz"
        "30f4d08b219190a37cded6aa796f4549504902c53cfc3c7e044a8490b6eba1f7"

        "llvm.dmg"
        "${LLVM_BASE}-Darwin-universal.dmg"
        "2864324ddff4d328e4818cfcd7e8c3d3970e987edf24071489f4182b80187a48"

        "cmake.tar.gz"
        "${CMAKE_BASE}-macos-universal.tar.gz"
        "c6fdda745f9ce69bca048e91955c7d043ba905d6388a62e0ff52b681ac17183c"

        "stedgeai.tar.gz"
        "${STEDGEAI_BASE}-darwin-arm64.tar.gz"
        "a4bc2605a78bc866b94517e90fac1e930431f4061e0c1265cd78f987963e383f"

        "stcubeprog.tar.gz"
        "${CUBEPROG_BASE}-darwin-arm64.tar.gz"
        "8c268db7f4a6ff9195c166d90154435eb745cc9dd03ebb9c3f9707e861214da7"

        "python.tar.gz"
        "${PYTHON_BASE}-aarch64-apple-darwin-install_only.tar.gz"
        "4734a2be2becb813830112c780c9879ac3aff111a0b0cd590e65ec7465774d02"

        "make.tar.gz"
        "${GNU_MAKE_URL}"
        "dd16fb1d67bfab79a72f5e8390735c49e3e8e70b4945a15ab1f81ddb78658fb3"
    )
    ;;
    *)
        echo "Unsupported platform: ${SDK_PLATFORM}"
        exit 1
        ;;
esac

########################################################################################
# Setup staging directory
echo "Building ${SDK_NAME}..."
rm -rf "${SDK_STAGE}"
TMPDIR_SDK="${BUILD_DIR}/tmp"
mkdir -p "${SDK_STAGE}" "${BUILD_DIR}" "${TMPDIR_SDK}"

########################################################################################
# Download and verify all components
echo ""
for ((i=0; i<${#DOWNLOADS[@]}; i+=3)); do
    dest="${DOWNLOADS[$i]}"
    url="${DOWNLOADS[$((i+1))]}"
    sha="${DOWNLOADS[$((i+2))]}"
    if [[ -f "${TMPDIR_SDK}/${dest}" ]]; then
        echo "  Skipping $(basename "${url}") (already downloaded)"
    else
        download "${url}" "${TMPDIR_SDK}/${dest}"
        verify_sha256 "${TMPDIR_SDK}/${dest}" "${sha}"
    fi
done

########################################################################################
# Extract: GCC
echo "Extracting GCC 14.3.rel1..."
mkdir -p "${SDK_STAGE}/gcc"
tar --strip-components=1 -Jxf "${TMPDIR_SDK}/gcc.tar.xz" -C "${SDK_STAGE}/gcc"

########################################################################################
# Extract: LLVM
echo "Extracting LLVM 18.1.3..."
mkdir -p "${SDK_STAGE}/llvm"
if [[ "${SDK_PLATFORM}" == darwin-* ]]; then
    # Mount DMG and copy the toolchain directory directly
    MOUNT="${TMPDIR_SDK}/llvm_mount"
    mkdir -p "${MOUNT}"
    hdiutil attach "${TMPDIR_SDK}/llvm.dmg" -nobrowse -quiet -mountpoint "${MOUNT}"
    trap 'hdiutil detach "${MOUNT}" -quiet 2>/dev/null' EXIT
    LLVM_ROOT=$(find "${MOUNT}" -maxdepth 1 -mindepth 1 -type d | grep -v Applications | head -1)
    cp -r "${LLVM_ROOT}/." "${SDK_STAGE}/llvm/"
    hdiutil detach "${MOUNT}" -quiet
    trap - EXIT
else
    tar --strip-components=1 -Jxf "${TMPDIR_SDK}/llvm.tar.xz" -C "${SDK_STAGE}/llvm"
fi

########################################################################################
# Build: GNU Make 4.4.1 from source
echo "Building GNU Make 4.4.1..."
mkdir -p "${TMPDIR_SDK}/make_src"
tar --strip-components=1 -zxf "${TMPDIR_SDK}/make.tar.gz" -C "${TMPDIR_SDK}/make_src"
(cd "${TMPDIR_SDK}/make_src" && ./configure --quiet --without-guile CFLAGS="-w" && make -j${NPROC} --quiet)
mkdir -p "${SDK_STAGE}/make"
cp "${TMPDIR_SDK}/make_src/make" "${SDK_STAGE}/make/make"

########################################################################################
# Extract: CMake 3.30.2
echo "Extracting CMake 3.30.2..."
mkdir -p "${SDK_STAGE}/cmake"
tar --strip-components=1 -zxf "${TMPDIR_SDK}/cmake.tar.gz" -C "${SDK_STAGE}/cmake"

########################################################################################
# Extract: STEdgeAI 3.0.0
echo "Extracting STEdgeAI 3.0.0..."
mkdir -p "${SDK_STAGE}/stedgeai"
tar --strip-components=1 -zxf "${TMPDIR_SDK}/stedgeai.tar.gz" -C "${SDK_STAGE}/stedgeai"

########################################################################################
# Extract: ST CubeProgrammer 2.21.0
echo "Extracting ST CubeProgrammer 2.21.0..."
mkdir -p "${SDK_STAGE}/stcubeprog"
tar --strip-components=1 -zxf "${TMPDIR_SDK}/stcubeprog.tar.gz" -C "${SDK_STAGE}/stcubeprog"

########################################################################################
# Extract: Python 3.12.0 + install packages
echo "Extracting Python 3.12.0..."
mkdir -p "${SDK_STAGE}/python"
tar --strip-components=1 -zxf "${TMPDIR_SDK}/python.tar.gz" -C "${SDK_STAGE}/python"
PYTHON="${SDK_STAGE}/python/bin/python3"
UV="${SDK_STAGE}/python/bin/uv"
echo "Installing Python packages..."
"${PYTHON}" -m pip install uv --quiet
# Create a temporary pyvenv.cfg so uv generates relocatable entry-point scripts
printf 'home = %s\nrelocatable = true\n' "${SDK_STAGE}/python/bin" > "${SDK_STAGE}/python/pyvenv.cfg"
VIRTUAL_ENV="${SDK_STAGE}/python" "${UV}" pip install \
    flake8==6.0.0 \
    pytest==7.4.0 \
    ethos-u-vela==4.2.0 \
    tabulate==0.9.0 \
    cryptography==46.0.5 \
    pyelftools==0.27 \
    colorama==0.4.6 \
    mpremote==1.27.0 \
    "pyserial @ git+https://github.com/pyserial/pyserial.git@911a0b8c110f3d3513bab67e64d95d1310517454"
rm "${SDK_STAGE}/python/pyvenv.cfg"

########################################################################################
# Write version file and package
echo ""
echo "Packaging ${SDK_NAME}..."
echo "${SDK_VERSION}" > "${SDK_STAGE}/sdk.version"

cd "${BUILD_DIR}"
echo "  Creating archive..."
if command -v pigz &>/dev/null; then
    tar --use-compress-program="pigz -p ${NPROC}" -cf "${SDK_NAME}.tar.gz" "${SDK_NAME}"
else
    tar -czf "${SDK_NAME}.tar.gz" "${SDK_NAME}"
fi
echo "  Computing checksum..."
sha256_file "${SDK_NAME}.tar.gz" > "${SDK_NAME}.tar.gz.sha256"

echo ""
echo "Done:"
echo "  ${BUILD_DIR}/${SDK_NAME}.tar.gz"
echo "  ${BUILD_DIR}/${SDK_NAME}.tar.gz.sha256"

########################################################################################
# Upload to R2
if [[ "${1:-}" == "--upload" ]]; then
    echo ""
    echo "Uploading to R2..."
    [[ -f "${SCRIPT_DIR}/sdk_keys.sh" ]] && source "${SCRIPT_DIR}/sdk_keys.sh"
    : "${AWS_ENDPOINT_URL:?AWS_ENDPOINT_URL is not set}"
    echo "  ${SDK_NAME}.tar.gz"
    aws s3 cp "${SDK_NAME}.tar.gz" "s3://download/sdk/${SDK_NAME}.tar.gz"
    echo "  ${SDK_NAME}.tar.gz.sha256"
    aws s3 cp "${SDK_NAME}.tar.gz.sha256" "s3://download/sdk/${SDK_NAME}.tar.gz.sha256"
    echo "Done."
fi
