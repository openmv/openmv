# SPDX-License-Identifier: MIT
#
# Copyright (C) 2025 OpenMV, LLC.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# Unix board configuration

# Port selection
PORT=unix

# Unix port uses host CPU (no specific MCU)
MCU=host
CPU=host

# Features available on Unix port
# Note: Hardware-specific modules are disabled
# Standard Unix port features
MICROPY_PY_ULAB = 1 # Enable ulab for ndarray support
MICROPY_PY_BTREE = 1  # Enable btree (standard Unix feature)
MICROPY_PY_SSL = 1
MICROPY_SSL_MBEDTLS = 1

# Hardware modules disabled on Unix
MICROPY_PY_CSI = 0
MICROPY_PY_CSI_NG = 0
MICROPY_PY_FIR = 0
MICROPY_PY_WINC1500 = 0
MICROPY_PY_IMU = 0
MICROPY_PY_TOF = 0
MICROPY_PY_AUDIO = 0
MICROPY_PY_DISPLAY = 0
MICROPY_PY_TV = 0
MICROPY_PY_ML = 0
MICROPY_PY_ML_TFLM = 0
MICROPY_PY_ML_STAI = 0
CUBEAI = 0

# Unix port specific settings
OMV_ENABLE_BL = 0

# CRC module (portable software implementation)
MICROPY_PY_CRC = 1
