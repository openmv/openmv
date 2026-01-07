/*
 * Copyright (C) 2023-2024 OpenMV, LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Any redistribution, use, or modification in source or binary form
 *    is done solely for personal benefit and not for any commercial
 *    purpose or for monetary gain. For commercial licensing options,
 *    please contact openmv@openmv.io
 *
 * THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
 * OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * I2C to UART bridge register definitions.
 */

#include "serialPortAdapter.h"

#include "py/mphal.h"
#include "sc16is741a.h"

static omv_csi_t *omv_csi = NULL;

void FSLP_set_csi(omv_csi_t *csi) {
    omv_csi = csi;
}

static uint8_t read_reg(uint8_t addr){
    uint8_t buf;
    omv_i2c_write(omv_csi->i2c, omv_csi->slv_addr, &addr, 1, OMV_I2C_XFER_NO_STOP);
    omv_i2c_read(omv_csi->i2c, omv_csi->slv_addr, &buf, 1, OMV_I2C_XFER_NO_FLAGS);
    return buf;
}

static void write_reg(uint8_t addr, uint8_t data) {
    uint8_t buf[] = {addr, data};
    omv_i2c_write(omv_csi->i2c, omv_csi->slv_addr, buf, 2, OMV_I2C_XFER_NO_FLAGS);
}

uint8_t FSLP_open_port() {
    write_reg(SC16IS741A_SUB_ADDR(SC16IS741A_REG_UARTRST, 0), SC16IS741A_UARTRST);

    write_reg(SC16IS741A_SUB_ADDR(SC16IS741A_REG_FCR, 0), SC16IS741A_FCR_ENABLE_FIFO);

    // Set a baud rate of 921600 (14,745,600 / 16 / 1)
    write_reg(SC16IS741A_SUB_ADDR(SC16IS741A_REG_LCR, 0), SC16IS741A_LCR_ENABLE_LATCH);
    write_reg(SC16IS741A_SUB_ADDR(SC16IS741A_REG_DLH, 0), 0x00);
    write_reg(SC16IS741A_SUB_ADDR(SC16IS741A_REG_DLL, 0), 0x01);

    // Set 8, N, 1.
    write_reg(SC16IS741A_SUB_ADDR(SC16IS741A_REG_LCR, 0), SC16IS741A_LCR_WORD_SZ_8);

    return 0;
}

void FSLP_close_port() {
}

// Return type is int16_t, so that the full uint8_t value can be represented
// without overlapping with negative error codes.
int16_t FSLP_read_byte_with_timeout(double timeout)
{
    int32_t timeout_ms = (int32_t) (timeout*1000);
    uint32_t start_ms = mp_hal_ticks_ms();

    while (!read_reg(SC16IS741A_SUB_ADDR(SC16IS741A_REG_RXLVL, 0))) {
        if ((mp_hal_ticks_ms() - start_ms) > timeout_ms) {
            return -1;
        }
    }

    return read_reg(SC16IS741A_SUB_ADDR(SC16IS741A_REG_RHR, 0));
}

void FSLP_flush_write_buffer() {
}

int32_t FSLP_write_buffer(uint8_t *frame_buf, int32_t len) {
    uint32_t start_ms = mp_hal_ticks_ms();

    for (int32_t i = 0; i < len; ) {
        if ((mp_hal_ticks_ms() - start_ms) > 1000) {
            return i;
        }

        int32_t bytes_left = len - i;
        int32_t space_avail = read_reg(SC16IS741A_SUB_ADDR(SC16IS741A_REG_TXLVL, 0));
        int32_t byte_to_write = (bytes_left < space_avail) ? bytes_left : space_avail;

        for (int32_t j = 0; j < byte_to_write; j++) {
            write_reg(SC16IS741A_SUB_ADDR(SC16IS741A_REG_THR, 0), frame_buf[i+j]);
        }

        i += byte_to_write;
    }

    return len;
}
