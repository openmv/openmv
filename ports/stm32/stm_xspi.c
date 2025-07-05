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
 * STM32 XSPI PSRAM driver.
 */
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include STM32_HAL_H
#include "omv_boardconfig.h"
#include "stm_xspi.h"

#if defined(OMV_XSPI_PSRAM_ID)

#define XSPI_CMD_READ               (0x00)
#define XSPI_CMD_READ_LATENCY       (7)
#define XSPI_CMD_READ_LINEAR_BURST  (0x20)

#define XSPI_CMD_WRITE              (0x80)
#define XSPI_CMD_WRITE_LATENCY      (7)
#define XSPI_CMD_WRITE_LINEAR_BURST (0xA0)

#define XSPI_CMD_READ_REG           (0x40)
#define XSPI_CMD_WRITE_REG          (0xC0)

#define XSPI_COMMAND_TIMEOUT        (1000)

static XSPI_HandleTypeDef xspi;

static int xspi_psram_read_reg(XSPI_HandleTypeDef *xspi, uint32_t addr, uint8_t *data) {
    XSPI_RegularCmdTypeDef command = {
        .OperationType = HAL_XSPI_OPTYPE_COMMON_CFG,
        .Instruction = XSPI_CMD_READ_REG,
        .InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES,
        .InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS,
        .InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE,
        .Address = addr,
        .AddressMode = HAL_XSPI_ADDRESS_8_LINES,
        .AddressWidth = HAL_XSPI_ADDRESS_32_BITS,
        .AddressDTRMode = HAL_XSPI_ADDRESS_DTR_ENABLE,
        .AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE,
        .DataMode = HAL_XSPI_DATA_8_LINES,
        .DataDTRMode = HAL_XSPI_DATA_DTR_ENABLE,
        .DataLength = 2,
        .DummyCycles = XSPI_CMD_READ_LATENCY - 1,
        .DQSMode = HAL_XSPI_DQS_ENABLE,
    };

    uint16_t regval = 0;
    if (HAL_XSPI_Command(xspi, &command, XSPI_COMMAND_TIMEOUT) != HAL_OK ||
        HAL_XSPI_Receive(xspi, (uint8_t *) &regval, XSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    *data = (uint8_t) regval;
    return 0;
}

static int xspi_psram_write_reg(XSPI_HandleTypeDef *xspi, uint32_t addr, uint8_t data) {
    XSPI_RegularCmdTypeDef command = {
        .OperationType = HAL_XSPI_OPTYPE_COMMON_CFG,
        .Instruction = XSPI_CMD_WRITE_REG,
        .InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES,
        .InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS,
        .InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE,
        .Address = addr,
        .AddressMode = HAL_XSPI_ADDRESS_8_LINES,
        .AddressWidth = HAL_XSPI_ADDRESS_32_BITS,
        .AddressDTRMode = HAL_XSPI_ADDRESS_DTR_ENABLE,
        .DataMode = HAL_XSPI_DATA_8_LINES,
        .DataDTRMode = HAL_XSPI_DATA_DTR_ENABLE,
        .DataLength = 2,
        .DummyCycles = 0,
        .AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE,
        .DQSMode = HAL_XSPI_DQS_DISABLE,
    };

    uint16_t regval = (data << 8) | data;
    if (HAL_XSPI_Command(xspi, &command, XSPI_COMMAND_TIMEOUT) != HAL_OK ||
        HAL_XSPI_Transmit(xspi, (uint8_t *) &regval, XSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return 0;
}

static int xspi_psram_write_verify(XSPI_HandleTypeDef *xspi, uint32_t addr, uint8_t data) {
    uint8_t reg = 0;
    if (xspi_psram_write_reg(xspi, addr, data) != 0 ||
        xspi_psram_read_reg(xspi, addr, &reg) != 0) {
        return -1;
    }
    return reg == data ? 0 : -1;
}

// Note: Linear burst commands force read/write commands to do 2KByte Wrap(X8)/1K Word(X16)
static int xspi_psram_memory_map(XSPI_HandleTypeDef *xspi, uint32_t burst_enable) {
    XSPI_RegularCmdTypeDef command = {
        .InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES,
        .InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS,
        .InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE,
        .AddressMode = HAL_XSPI_ADDRESS_8_LINES,
        .AddressWidth = HAL_XSPI_ADDRESS_32_BITS,
        .AddressDTRMode = HAL_XSPI_ADDRESS_DTR_ENABLE,
        .DataMode = HAL_XSPI_DATA_16_LINES,
        .DataDTRMode = HAL_XSPI_DATA_DTR_ENABLE,
        .DataLength = 0,
        .AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE,
        .DQSMode = HAL_XSPI_DQS_ENABLE,
    };

    // Initialize the write command
    command.OperationType = HAL_XSPI_OPTYPE_WRITE_CFG;
    command.Instruction = (burst_enable ? XSPI_CMD_WRITE_LINEAR_BURST : XSPI_CMD_WRITE);
    command.DummyCycles = (XSPI_CMD_WRITE_LATENCY - 1U);

    if (HAL_XSPI_Command(xspi, &command, XSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    // Initialize the read command
    command.OperationType = HAL_XSPI_OPTYPE_READ_CFG;
    command.Instruction = (burst_enable ? XSPI_CMD_READ_LINEAR_BURST : XSPI_CMD_READ);
    command.DummyCycles = (XSPI_CMD_READ_LATENCY - 1U);
    if (HAL_XSPI_Command(xspi, &command, XSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    XSPI_MemoryMappedTypeDef mmap_config = {
        .TimeoutPeriodClock = 0x34,
        .TimeOutActivation = HAL_XSPI_TIMEOUT_COUNTER_ENABLE,
    };

    if (HAL_XSPI_MemoryMapped(xspi, &mmap_config) != HAL_OK) {
        return -1;
    }

    return 0;
}

int stm_xspi_psram_init(void) {
    uint32_t xspi_clk = 0;

    // Reset and enable XSPI clock.
    if (OMV_XSPI_PSRAM_ID == 1) {
        xspi_clk = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_XSPI1);
    } else if (OMV_XSPI_PSRAM_ID == 2) {
        xspi_clk = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_XSPI2);
    } else if (OMV_XSPI_PSRAM_ID == 3) {
        xspi_clk = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_XSPI3);
    } else {
        return -1;
    }

    memset(&xspi, 0, sizeof(XSPI_HandleTypeDef));
    xspi.Instance = XSPI1;
    xspi.Init.FifoThresholdByte = 8;
    xspi.Init.MemoryType = HAL_XSPI_MEMTYPE_APMEM_16BITS;
    xspi.Init.MemoryMode = HAL_XSPI_SINGLE_MEM;
    xspi.Init.MemorySize = __builtin_ctz(OMV_XSPI_PSRAM_SIZE) - 1;
    xspi.Init.MemorySelect = HAL_XSPI_CSSEL_NCS1;
    xspi.Init.ChipSelectHighTimeCycle = 5;
    xspi.Init.ClockMode = HAL_XSPI_CLOCK_MODE_0;
    xspi.Init.ClockPrescaler = (xspi_clk / OMV_XSPI_PSRAM_FREQUENCY) - 1;
    xspi.Init.FreeRunningClock = HAL_XSPI_FREERUNCLK_DISABLE;
    xspi.Init.SampleShifting = HAL_XSPI_SAMPLE_SHIFT_NONE;
    xspi.Init.DelayHoldQuarterCycle = HAL_XSPI_DHQC_DISABLE;
    xspi.Init.ChipSelectBoundary = HAL_XSPI_BONDARYOF_16KB;
    xspi.Init.WrapSize = HAL_XSPI_WRAP_NOT_SUPPORTED;

    if (HAL_XSPI_Init(&xspi) != HAL_OK) {
        return -1;
    }

    // Read Latency=7 up to 200MHz
    if (xspi_psram_write_verify(&xspi, 0, 0x30) != 0) {
        return -1;
    }
    // Write Latency=7 up to 200MHz
    if (xspi_psram_write_verify(&xspi, 4, 0x20) != 0) {
        return -1;
    }
    // x16 | RBX | 2K Byte burst
    if (xspi_psram_write_verify(&xspi, 8, 0x4B) != 0) {
        return -1;
    }
    // Switch to memory-mapped mode.
    if (xspi_psram_memory_map(&xspi, 1) != 0) {
        return -1;
    }
    return 0;
}

extern void __fatal_error(const char *msg);
#if __GNUC__ >= 11
// Prevent array bounds warnings when accessing SDRAM_START_ADDRESS as a memory pointer.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif

bool __attribute__((optimize("Os"))) stm_xspi_psram_test(bool exhaustive) {
    uint8_t const pattern = 0xaa;
    uint8_t const antipattern = 0x55;
    volatile uint8_t *const mem_base = (uint8_t *) 0x90000000;

    char error_buffer[1024];
    uint32_t mem_size = OMV_XSPI_PSRAM_SIZE;

    #if (__DCACHE_PRESENT == 1)
    bool i_cache_disabled = false;
    bool d_cache_disabled = false;

    // Disable caches for testing.
    if (SCB->CCR & (uint32_t) SCB_CCR_IC_Msk) {
        SCB_DisableICache();
        i_cache_disabled = true;
    }

    if (SCB->CCR & (uint32_t) SCB_CCR_DC_Msk) {
        SCB_DisableDCache();
        d_cache_disabled = true;
    }
    #endif

    // Test data bus
    for (uint32_t i = 0; i < 16; i++) {
        *((volatile uint32_t *) mem_base) = (1 << i);
        __DSB();
        if (*((volatile uint32_t *) mem_base) != (1 << i)) {
            snprintf(error_buffer, sizeof(error_buffer),
                     "Data bus test failed at 0x%p expected 0x%x found 0x%lx",
                     &mem_base[0], (1 << i), ((volatile uint32_t *) mem_base)[0]);
            __fatal_error(error_buffer);
            return false;
        }
    }

    // Test address bus
    for (uint32_t i = 1; i < mem_size; i <<= 1) {
        mem_base[i] = pattern;
        __DSB();
        if (mem_base[i] != pattern) {
            snprintf(error_buffer, sizeof(error_buffer),
                     "Address bus test failed at 0x%p expected 0x%x found 0x%x",
                     &mem_base[i], pattern, mem_base[i]);
            __fatal_error(error_buffer);
            return false;
        }
    }

    // Check for aliasing (overlapping addresses)
    mem_base[0] = antipattern;
    __DSB();
    for (uint32_t i = 1; i < mem_size; i <<= 1) {
        if (mem_base[i] != pattern) {
            snprintf(error_buffer, sizeof(error_buffer),
                     "Address bus overlap at 0x%p expected 0x%x found 0x%x",
                     &mem_base[i], pattern, mem_base[i]);
            __fatal_error(error_buffer);
            return false;
        }
    }

    mem_size = 512 * 1024;
    // Test all RAM cells
    if (exhaustive) {
        // Write all memory first then compare, so even if the cache
        // is enabled, it's not just writing and reading from cache.
        // Note: This test should also detect refresh rate issues.
        for (uint32_t i = 0; i < mem_size; i++) {
            mem_base[i] = ((i % 2) ? pattern : antipattern);
        }

        for (uint32_t i = 0; i < mem_size; i++) {
            if (mem_base[i] != ((i % 2) ? pattern : antipattern)) {
                snprintf(error_buffer, sizeof(error_buffer),
                         "Address bus slow test failed at 0x%p expected 0x%x found 0x%x",
                         &mem_base[i], ((i % 2) ? pattern : antipattern), mem_base[i]);
                __fatal_error(error_buffer);
                return false;
            }
        }
    }

    #if (__DCACHE_PRESENT == 1)
    // Re-enable caches if they were enabled before the test started.
    if (i_cache_disabled) {
        SCB_EnableICache();
    }

    if (d_cache_disabled) {
        SCB_EnableDCache();
    }
    #endif

    return true;
}

#if __GNUC__ >= 11
#pragma GCC diagnostic pop
#endif

#endif
