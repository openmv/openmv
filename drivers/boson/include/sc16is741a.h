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

#define SC16IS741A_SUB_ADDR(reg, channel) (((reg) << 3) | ((channel) << 1))

#define SC16IS741A_REG_RHR 0x00
#define SC16IS741A_REG_THR 0x00
#define SC16IS741A_REG_IER 0x01
#define SC16IS741A_REG_FCR 0x02
#define SC16IS741A_REG_IIR 0x02
#define SC16IS741A_REG_LCR 0x03
#define SC16IS741A_REG_MCR 0x04
#define SC16IS741A_REG_LSR 0x05
#define SC16IS741A_REG_MSR 0x06
#define SC16IS741A_REG_SPR 0x07
#define SC16IS741A_REG_TCR 0x06
#define SC16IS741A_REG_TLR 0x07
#define SC16IS741A_REG_TXLVL 0x08
#define SC16IS741A_REG_RXLVL 0x09
#define SC16IS741A_REG_UARTRST 0x0e
#define SC16IS741A_REG_EFCR 0x0f
#define SC16IS741A_REG_DLL 0x00
#define SC16IS741A_REG_DLH 0x01
#define SC16IS741A_REG_EFR 0x02
#define SC16IS741A_REG_XON1 0x04
#define SC16IS741A_REG_XON2 0x05
#define SC16IS741A_REG_XOFF1 0x06
#define SC16IS741A_REG_XOFF2 0x07

#define SC16IS741A_FCR_ENABLE_FIFO 0x01
#define SC16IS741A_FCR_RX_FIFO_RESET 0x02
#define SC16IS741A_FCR_TX_FIFO_RESET 0x04

#define SC16IS741A_LCR_WORD_SZ_5 0x00
#define SC16IS741A_LCR_WORD_SZ_6 0x01
#define SC16IS741A_LCR_WORD_SZ_7 0x02
#define SC16IS741A_LCR_WORD_SZ_8 0x03
#define SC16IS741A_LCR_STOP_BIT 0x04
#define SC16IS741A_LCR_PARITY_EN 0x08
#define SC16IS741A_LCR_PARITY_EVEN 0x10
#define SC16IS741A_LCR_PARITY_ODD 0x00
#define SC16IS741A_LCR_PARITY_FORCE 0x20
#define SC16IS741A_LCR_PARTIY_FORCE_1 0x00
#define SC16IS741A_LCR_PARTIY_FORCE_0 0x10
#define SC16IS741A_LCR_BREAK_EN 0x40
#define SC16IS741A_LCR_ENABLE_LATCH 0x80

#define SC16IS741A_UARTRST 0x08
