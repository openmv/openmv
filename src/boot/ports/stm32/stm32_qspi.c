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
 * STM32 QSPI Flash driver.
 */
#include <string.h>
#include STM32_HAL_H
#include "omv_boardconfig.h"
#include "omv_bootconfig.h"
#include "stm32_flash.h"

#if OMV_BOOT_QSPI_FLASH_SIZE

#define QSPI_PAGE_SIZE              (0x100)      // 256Bytes pages.
#define QSPI_BLOCK_SIZE             (0x10000)    // 64KBytes blocks.

#define QSPI_COMMAND_TIMEOUT        (1000)
#define QSPI_SECTOR_ERASE_TIMEOUT   (400)
#define QSPI_BLOCK_ERASE_TIMEOUT    (2000)
#define QSPI_CHIP_ERASE_TIMEOUT     (400000)

#define QSPI_CMD_RESET_ENABLE       (0x66)
#define QSPI_CMD_RESET_MEMORY       (0x99)

#define QSPI_CMD_READ_QUAD          (0xEC)
#define QSPI_CMD_PROG_QUAD          (0x34)
#define QSPI_CMD_QUAD_DUMMY         (6)

#define QSPI_CMD_BLOCK_ERASE        (0xDC)
#define QSPI_CMD_CHIP_ERASE         (0xC7)

#define QSPI_CMD_ADDR4_ENABLE       (0xB7)
#define QSPI_CMD_ADDR4_DISABLE      (0xE9)

#define QSPI_CMD_WRITE_ENABLE       (0x06)
#define QSPI_CMD_WRITE_DISABLE      (0x04)

#define QSPI_CMD_READ_STATUS_REG    (0x05)
#define QSPI_CMD_WRITE_STATUS_REG   (0x01)

#define QSPI_SR_WIP_MASK            (1 << 0)
#define QSPI_SR_WEL_MASK            (1 << 1)

static QSPI_HandleTypeDef qspi = {0};

static int qspi_flash_reset();
static int qspi_flash_addr_4byte(bool);
static int qspi_flash_poll_status_flag(uint32_t, uint32_t, uint32_t);

static int qspi_flash_init() {
    if (qspi.Instance != NULL) {
        return 0;
    }

    qspi.Instance = QUADSPI;
    qspi.Init.ClockPrescaler = 1;     // clock = 200MHz / (1+1) = 100MHz
    qspi.Init.FifoThreshold = 3;
    qspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
    qspi.Init.FlashSize = __builtin_ctz(OMV_BOOT_QSPI_FLASH_SIZE) - 1;
    qspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;
    qspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
    qspi.Init.FlashID = QSPI_FLASH_ID_1;
    qspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;

    // Reset QSPI.
    HAL_QSPI_DeInit(&qspi);
    __HAL_RCC_QSPI_FORCE_RESET();
    __HAL_RCC_QSPI_RELEASE_RESET();

    // Enable QSPI clock.
    __HAL_RCC_QSPI_CLK_ENABLE();

    // Initialize the QSPI
    if (HAL_QSPI_Init(&qspi) != HAL_OK) {
        return -1;
    }

    // Reset the QSPI
    if (qspi_flash_reset() != 0) {
        return -1;
    }

    // Enable 4-byte address mode.
    if (qspi_flash_addr_4byte(true) != 0) {
        return -1;
    }

    return 0;//qspi_flash_memory_test();
}

static int qspi_flash_reset() {
    QSPI_CommandTypeDef command = {
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .AddressMode = QSPI_ADDRESS_NONE,
        .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
        .DataMode = QSPI_DATA_NONE,
        .DummyCycles = 0,
        .DdrMode = QSPI_DDR_MODE_DISABLE,
        .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,
        .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
    };

    command.Instruction = QSPI_CMD_RESET_ENABLE;
    if (HAL_QSPI_Command(&qspi, &command, QSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    command.Instruction = QSPI_CMD_RESET_MEMORY;
    if (HAL_QSPI_Command(&qspi, &command, QSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    if (qspi_flash_poll_status_flag(QSPI_SR_WIP_MASK, 0, QSPI_COMMAND_TIMEOUT) != 0) {
        return -1;
    }

    return 0;
}

static int qspi_flash_write_enable() {
    QSPI_CommandTypeDef command = {
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .Instruction = QSPI_CMD_WRITE_ENABLE,
        .AddressMode = QSPI_ADDRESS_NONE,
        .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
        .DataMode = QSPI_DATA_NONE,
        .DummyCycles = 0,
        .DdrMode = QSPI_DDR_MODE_DISABLE,
        .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,
        .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
    };

    if (HAL_QSPI_Command(&qspi, &command, QSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    if (qspi_flash_poll_status_flag(QSPI_SR_WEL_MASK, QSPI_SR_WEL_MASK, QSPI_COMMAND_TIMEOUT) != 0) {
        return -1;
    }

    return 0;
}

static int qspi_flash_addr_4byte(bool enable) {
    QSPI_CommandTypeDef command = {
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .AddressMode = QSPI_ADDRESS_NONE,
        .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
        .DataMode = QSPI_DATA_NONE,
        .DummyCycles = 0,
        .DdrMode = QSPI_DDR_MODE_DISABLE,
        .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,
        .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
        .Instruction = enable ? QSPI_CMD_ADDR4_ENABLE : QSPI_CMD_ADDR4_DISABLE,
    };

    if (qspi_flash_write_enable() != 0) {
        return -1;
    }

    if (HAL_QSPI_Command(&qspi, &command, QSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    if (qspi_flash_poll_status_flag(QSPI_SR_WIP_MASK, 0, QSPI_COMMAND_TIMEOUT) != 0) {
        return -1;
    }

    return 0;
}

static int qspi_flash_poll_status_flag(uint32_t mask, uint32_t match, uint32_t timeout) {
    QSPI_AutoPollingTypeDef config = {
        .Mask = mask,
        .Match = match,
        .MatchMode = QSPI_MATCH_MODE_AND,
        .Interval = 0x10,
        .AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE,
        .StatusBytesSize = 1
    };

    QSPI_CommandTypeDef command = {
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .Instruction = QSPI_CMD_READ_STATUS_REG,
        .AddressMode = QSPI_ADDRESS_NONE,
        .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
        .DataMode = QSPI_DATA_1_LINE,
        .DummyCycles = 0,
        .DdrMode = QSPI_DDR_MODE_DISABLE,
        .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,
        .SIOOMode = QSPI_SIOO_INST_EVERY_CMD
    };

    if (HAL_QSPI_AutoPolling(&qspi, &command, &config, timeout) != HAL_OK) {
        return -1;
    }

    return 0;
}

static int qspi_flash_erase_chip() {
    QSPI_CommandTypeDef command = {
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .Instruction = QSPI_CMD_CHIP_ERASE,
        .AddressMode = QSPI_ADDRESS_NONE,
        .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
        .DataMode = QSPI_DATA_NONE,
        .DummyCycles = 0,
        .DdrMode = QSPI_DDR_MODE_DISABLE,
        .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,
        .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
    };

    if (qspi.Instance == NULL && qspi_flash_init() != 0) {
        return -1;
    }

    if (qspi_flash_write_enable(&qspi) != 0) {
        return -1;
    }

    if (HAL_QSPI_Command(&qspi, &command, QSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    if (qspi_flash_poll_status_flag(QSPI_SR_WIP_MASK, 0, QSPI_CHIP_ERASE_TIMEOUT) != 0) {
        return -1;
    }

    return 0;
}

static int qspi_flash_erase_block(uint32_t addr) {
    QSPI_CommandTypeDef command = {
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .Instruction = QSPI_CMD_BLOCK_ERASE,
        .AddressMode = QSPI_ADDRESS_1_LINE,
        .AddressSize = QSPI_ADDRESS_32_BITS,
        .Address = addr,
        .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
        .DataMode = QSPI_DATA_NONE,
        .DummyCycles = 0,
        .DdrMode = QSPI_DDR_MODE_DISABLE,
        .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,
        .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
    };

    if (qspi.Instance == NULL && qspi_flash_init() != 0) {
        return -1;
    }

    if (qspi_flash_write_enable(&qspi) != 0) {
        return -1;
    }

    if (HAL_QSPI_Command(&qspi, &command, QSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    if (qspi_flash_poll_status_flag(QSPI_SR_WIP_MASK, 0, QSPI_BLOCK_ERASE_TIMEOUT) != 0) {
        return -1;
    }

    return 0;
}

int __attribute__((optimize("O0"))) qspi_flash_memory_test() {
    uint8_t buf[QSPI_PAGE_SIZE];
    uint8_t pat[QSPI_PAGE_SIZE];

    uint32_t n_pages = OMV_BOOT_QSPI_FLASH_SIZE / QSPI_PAGE_SIZE;
    uint32_t n_blocks = OMV_BOOT_QSPI_FLASH_SIZE / QSPI_BLOCK_SIZE;

    if (qspi.Instance == NULL || qspi_flash_init() != 0) {
        return -1;
    }

    for (int i = 0; i < QSPI_PAGE_SIZE; i++) {
        pat[i] = ((i % 2) == 0) ? 0xaa : 0x55;
    }

    for (uint32_t i = 0, addr = 0; i < n_blocks; i++, addr += QSPI_BLOCK_SIZE) {
        if (qspi_flash_erase_block(addr) != 0) {
            return -2;
        }
    }

    for (uint32_t i = 0, addr = 0; i < n_pages; i++, addr += QSPI_PAGE_SIZE) {
        if (spi_flash_write(addr, pat, QSPI_PAGE_SIZE) != 0) {
            return -3;
        }
    }

    for (uint32_t i = 0, addr = 0; i < n_pages; i++, addr += QSPI_PAGE_SIZE) {
        memset(buf, 0, QSPI_PAGE_SIZE);
        if (spi_flash_read(addr, buf, QSPI_PAGE_SIZE) != 0) {
            return -4;
        }
        if (memcmp(buf, pat, QSPI_PAGE_SIZE) != 0) {
            return -5;
        }
    }

    if (qspi_flash_erase_chip() != 0) {
        return -6;
    }

    return 0;
}

static int qspi_flash_read_page(uint32_t addr, uint8_t *buf, uint32_t size) {
    QSPI_CommandTypeDef command = {
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .Instruction = QSPI_CMD_READ_QUAD,
        .AddressMode = QSPI_ADDRESS_4_LINES,
        .AddressSize = QSPI_ADDRESS_32_BITS,
        .Address = addr,
        .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
        .DataMode = QSPI_DATA_4_LINES,
        .DummyCycles = QSPI_CMD_QUAD_DUMMY,
        .NbData = size,
        .DdrMode = QSPI_DDR_MODE_DISABLE,
        .DdrHoldHalfCycle = QSPI_DDR_HHC_HALF_CLK_DELAY,
        .SIOOMode = QSPI_SIOO_INST_EVERY_CMD
    };

    if (qspi.Instance == NULL && qspi_flash_init() != 0) {
        return -1;
    }

    if (HAL_QSPI_Command(&qspi, &command, QSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    if (HAL_QSPI_Receive(&qspi, buf, QSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    return 0;
}

static int qspi_flash_write_page(uint32_t addr, const uint8_t *buf, uint32_t size) {
    QSPI_CommandTypeDef command = {
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .Instruction = QSPI_CMD_PROG_QUAD,
        .Address = addr,
        .AddressMode = QSPI_ADDRESS_1_LINE,
        .AddressSize = QSPI_ADDRESS_32_BITS,
        .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
        .NbData = size,
        .DataMode = QSPI_DATA_4_LINES,
        .DummyCycles = 0,
        .DdrMode = QSPI_DDR_MODE_DISABLE,
        .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,
        .SIOOMode = QSPI_SIOO_INST_EVERY_CMD
    };

    // Address must be page-aligned and the size <= page size.
    if ((addr % QSPI_PAGE_SIZE) != 0 ||
        size <= 0 || size > QSPI_PAGE_SIZE) {
        return -1;
    }

    if (qspi.Instance == NULL && qspi_flash_init() != 0) {
        return -1;
    }

    // Enable write operations
    if (qspi_flash_write_enable(&qspi) != 0) {
        return -1;
    }

    // Configure the command
    if (HAL_QSPI_Command(&qspi, &command, QSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    // Transmit the data
    if (HAL_QSPI_Transmit(&qspi, (uint8_t *) buf, QSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    // Poll the status register.
    if (qspi_flash_poll_status_flag(QSPI_SR_WIP_MASK, 0, QSPI_COMMAND_TIMEOUT) != 0) {
        return -1;
    }

    return 0;
}

int spi_flash_read(uint32_t addr, uint8_t *buf, uint32_t size) {
    for (size_t i = 0; i < size / QSPI_PAGE_SIZE; i++) {
        if (qspi_flash_read_page(addr, buf, QSPI_PAGE_SIZE) != 0) {
            return -1;
        }
        buf += QSPI_PAGE_SIZE;
        addr += QSPI_PAGE_SIZE;
    }

    if ((size % QSPI_PAGE_SIZE)) {
        if (qspi_flash_read_page(addr, buf, (size % QSPI_PAGE_SIZE)) != 0) {
            return -1;
        }
    }
    return 0;
}

int spi_flash_write(uint32_t addr, const uint8_t *buf, uint32_t size) {
    // If at a block boundary, erase the block first.
    if ((addr % QSPI_BLOCK_SIZE) == 0) {
        if (qspi_flash_erase_block(addr) != 0) {
            return -1;
        }
    }

    for (size_t i = 0; i < size / QSPI_PAGE_SIZE; i++) {
        if (qspi_flash_write_page(addr, buf, QSPI_PAGE_SIZE) != 0) {
            return -1;
        }
        buf += QSPI_PAGE_SIZE;
        addr += QSPI_PAGE_SIZE;
    }

    if ((size % QSPI_PAGE_SIZE)) {
        if (qspi_flash_write_page(addr, buf, (size % QSPI_PAGE_SIZE)) != 0) {
            return -1;
        }
    }
    return 0;
}


int spi_flash_deinit() {
    if (qspi.Instance != NULL) {
        HAL_QSPI_DeInit(&qspi);
        qspi.Instance = NULL;
    }

    // Reset QSPI.
    __HAL_RCC_QSPI_FORCE_RESET();
    __HAL_RCC_QSPI_RELEASE_RESET();

    // Disable QSPI clock.
    __HAL_RCC_QSPI_CLK_DISABLE();
    memset(&qspi, 0, sizeof(QSPI_HandleTypeDef));
    return 0;
}
#endif // OMV_BOOT_QSPI_FLASH_SIZE
