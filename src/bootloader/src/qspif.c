/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * QSPI Flash driver.
 */
#include STM32_HAL_H
#include <string.h>
#include "qspif.h"
#include "omv_boardconfig.h"
#include "omv_bootconfig.h"

#if defined(OMV_BOOT_QSPIF_LAYOUT)

#define HAL_QSPI_TIMEOUT          (5000)
#define SECTOR_ERASE_TIMEOUT      (400)
#define BLOCK_ERASE_TIMEOUT       (2000)
#define CHIP_ERASE_TIMEOUT        (400000)

#define CMD_RESET_ENABLE          (0x66)
#define CMD_RESET_MEMORY          (0x99)

#define CMD_READ_QUADIO           (0xEC)
#define CMD_PROG_QUADIO           (0x34)

#define CMD_64K_BLOCK_ERASE       (0xDC)
#define CMD_CHIP_ERASE            (0xC7)

#define CMD_4BYTE_ADDR_ENABLE     (0xB7)
#define CMD_4BYTE_ADDR_DISABLE    (0xE9)

#define CMD_WRITE_ENABLE          (0x06)
#define CMD_WRITE_DISABLE         (0x04)

#define CMD_READ_STATUS_REG       (0x05)
#define CMD_WRITE_STATUS_REG      (0x01)

static QSPI_HandleTypeDef QSPIHandle = {0};
static int qspif_write_enable();
static int qspif_4byte_addr_mode_enable();
static int qspif_poll_status_flag(uint32_t mask, uint32_t match, uint32_t timeout);

int qspif_init() {
    QSPIHandle.Instance = QUADSPI;
    QSPIHandle.Init.ClockPrescaler = 1;     // clock = 200MHz / (1+1) = 100MHz
    QSPIHandle.Init.FifoThreshold = 3;
    QSPIHandle.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
    QSPIHandle.Init.FlashSize = OMV_BOOT_QSPIF_SIZE_BITS - 1;
    QSPIHandle.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;
    QSPIHandle.Init.ClockMode = QSPI_CLOCK_MODE_0;
    QSPIHandle.Init.FlashID = QSPI_FLASH_ID_1;
    QSPIHandle.Init.DualFlash = QSPI_DUALFLASH_DISABLE;

    // Initialize the QSPI
    HAL_QSPI_DeInit(&QSPIHandle);
    if (HAL_QSPI_Init(&QSPIHandle) != HAL_OK) {
        // Initialization Error
        return -1;
    }

    // Reset the QSPI
    if (qspif_reset() != 0) {
        return -1;
    }

    // Enable 4-byte address mode.
    if (qspif_4byte_addr_mode_enable() != 0) {
        return -1;
    }

    return 0;
}

int qspif_deinit() {
    if (QSPIHandle.Instance != NULL &&
        HAL_QSPI_DeInit(&QSPIHandle) != HAL_OK) {
        return -1;
    }
    QSPIHandle.Instance = NULL;
    return 0;
}

int qspif_reset() {
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

    if (QSPIHandle.Instance == NULL) {
        return -1;
    }

    command.Instruction = CMD_RESET_ENABLE;
    if (HAL_QSPI_Command(&QSPIHandle, &command, HAL_QSPI_TIMEOUT) != HAL_OK) {
        return -1;
    }

    command.Instruction = CMD_RESET_MEMORY;
    if (HAL_QSPI_Command(&QSPIHandle, &command, HAL_QSPI_TIMEOUT) != HAL_OK) {
        return -1;
    }

    if (qspif_poll_status_flag(OMV_BOOT_QSPIF_SR_WIP_MASK, 0, HAL_QSPI_TIMEOUT) != 0) {
        return -1;
    }

    return 0;
}


int qspif_read(uint8_t *buf, uint32_t addr, uint32_t size) {
    QSPI_CommandTypeDef command = {
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .Instruction = CMD_READ_QUADIO,
        .AddressMode = QSPI_ADDRESS_4_LINES,
        .AddressSize = QSPI_ADDRESS_32_BITS,
        .Address = addr,
        .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
        .DataMode = QSPI_DATA_4_LINES,
        .DummyCycles = OMV_BOOT_QSPIF_READ_QUADIO_DCYC,
        .NbData = size,
        .DdrMode = QSPI_DDR_MODE_DISABLE,
        .DdrHoldHalfCycle = QSPI_DDR_HHC_HALF_CLK_DELAY,
        .SIOOMode = QSPI_SIOO_INST_EVERY_CMD
    };

    if (QSPIHandle.Instance == NULL) {
        return -1;
    }

    if (HAL_QSPI_Command(&QSPIHandle, &command, HAL_QSPI_TIMEOUT) != HAL_OK) {
        return -1;
    }

    if (HAL_QSPI_Receive(&QSPIHandle, buf, HAL_QSPI_TIMEOUT) != HAL_OK) {
        return -1;
    }

    return 0;
}

int qspif_write(uint8_t *buf, uint32_t addr, uint32_t size) {
    QSPI_CommandTypeDef command = {
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .Instruction = CMD_PROG_QUADIO,
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

    if (QSPIHandle.Instance == NULL) {
        return -1;
    }

    // Address must be page-aligned and size between 0->OMV_BOOT_QSPIF_PAGE_SIZE bytes.
    if ((addr % OMV_BOOT_QSPIF_PAGE_SIZE) != 0 || size <= 0 || size > OMV_BOOT_QSPIF_PAGE_SIZE) {
        return -1;
    }

    // Enable write operations
    if (qspif_write_enable(&QSPIHandle) != 0) {
        return -1;
    }

    // Configure the command
    if (HAL_QSPI_Command(&QSPIHandle, &command, HAL_QSPI_TIMEOUT) != HAL_OK) {
        return -1;
    }

    // Transmit the data
    if (HAL_QSPI_Transmit(&QSPIHandle, buf, HAL_QSPI_TIMEOUT) != HAL_OK) {
        return -1;
    }

    // Poll the status register.
    if (qspif_poll_status_flag(OMV_BOOT_QSPIF_SR_WIP_MASK, 0, HAL_QSPI_TIMEOUT) != 0) {
        return -1;
    }

    return 0;
}

int qspif_erase_block(uint32_t addr) {
    QSPI_CommandTypeDef command = {
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .Instruction = CMD_64K_BLOCK_ERASE,
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

    if (QSPIHandle.Instance == NULL) {
        return -1;
    }

    if (qspif_write_enable(&QSPIHandle) != 0) {
        return -1;
    }

    if (HAL_QSPI_Command(&QSPIHandle, &command, HAL_QSPI_TIMEOUT) != HAL_OK) {
        return -1;
    }

    if (qspif_poll_status_flag(OMV_BOOT_QSPIF_SR_WIP_MASK, 0, BLOCK_ERASE_TIMEOUT) != 0) {
        return -1;
    }

    return 0;
}

int qspif_erase_chip() {
    QSPI_CommandTypeDef command = {
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .Instruction = CMD_CHIP_ERASE,
        .AddressMode = QSPI_ADDRESS_NONE,
        .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
        .DataMode = QSPI_DATA_NONE,
        .DummyCycles = 0,
        .DdrMode = QSPI_DDR_MODE_DISABLE,
        .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,
        .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
    };

    if (QSPIHandle.Instance == NULL) {
        return -1;
    }

    if (qspif_write_enable(&QSPIHandle) != 0) {
        return -1;
    }

    if (HAL_QSPI_Command(&QSPIHandle, &command, HAL_QSPI_TIMEOUT) != HAL_OK) {
        return -1;
    }

    if (qspif_poll_status_flag(OMV_BOOT_QSPIF_SR_WIP_MASK, 0, CHIP_ERASE_TIMEOUT) != 0) {
        return -1;
    }

    return 0;
}

static int qspif_write_enable() {
    QSPI_CommandTypeDef command = {
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .Instruction = CMD_WRITE_ENABLE,
        .AddressMode = QSPI_ADDRESS_NONE,
        .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
        .DataMode = QSPI_DATA_NONE,
        .DummyCycles = 0,
        .DdrMode = QSPI_DDR_MODE_DISABLE,
        .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,
        .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
    };

    if (HAL_QSPI_Command(&QSPIHandle, &command, HAL_QSPI_TIMEOUT) != HAL_OK) {
        return -1;
    }

    if (qspif_poll_status_flag(OMV_BOOT_QSPIF_SR_WEL_MASK, OMV_BOOT_QSPIF_SR_WEL_MASK, HAL_QSPI_TIMEOUT) != 0) {
        return -1;
    }

    return 0;
}

static int qspif_4byte_addr_mode_enable() {
    QSPI_CommandTypeDef command = {
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .Instruction = CMD_4BYTE_ADDR_ENABLE,
        .AddressMode = QSPI_ADDRESS_NONE,
        .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
        .DataMode = QSPI_DATA_NONE,
        .DummyCycles = 0,
        .DdrMode = QSPI_DDR_MODE_DISABLE,
        .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,
        .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
    };

    if (qspif_write_enable() != 0) {
        return -1;
    }

    if (HAL_QSPI_Command(&QSPIHandle, &command, HAL_QSPI_TIMEOUT) != HAL_OK) {
        return -1;
    }

    if (qspif_poll_status_flag(OMV_BOOT_QSPIF_SR_WIP_MASK, 0, HAL_QSPI_TIMEOUT) != 0) {
        return -1;
    }

    return 0;
}

static int qspif_poll_status_flag(uint32_t mask, uint32_t match, uint32_t timeout) {
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
        .Instruction = CMD_READ_STATUS_REG,
        .AddressMode = QSPI_ADDRESS_NONE,
        .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
        .DataMode = QSPI_DATA_1_LINE,
        .DummyCycles = 0,
        .DdrMode = QSPI_DDR_MODE_DISABLE,
        .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,
        .SIOOMode = QSPI_SIOO_INST_EVERY_CMD
    };

    if (HAL_QSPI_AutoPolling(&QSPIHandle, &command, &config, timeout) != HAL_OK) {
        return -1;
    }

    return 0;
}

int __attribute__((optimize("O0"))) qspif_memory_test() {
    uint8_t buf[OMV_BOOT_QSPIF_PAGE_SIZE];
    uint8_t pat[OMV_BOOT_QSPIF_PAGE_SIZE];

    if (qspif_init() != 0) {
        return -1;
    }

    for (int i = 0; i < OMV_BOOT_QSPIF_PAGE_SIZE; i++) {
        pat[i] = ((i % 2) == 0) ? 0xaa : 0x55;
    }

    for (uint32_t i = 0, addr = 0; i < OMV_BOOT_QSPIF_NUM_BLOCKS; i++, addr += OMV_BOOT_QSPIF_BLOCK_SIZE) {
        if (qspif_erase_block(addr) != 0) {
            return -2;
        }
    }

    for (uint32_t i = 0, addr = 0; i < OMV_BOOT_QSPIF_NUM_PAGES; i++, addr += OMV_BOOT_QSPIF_PAGE_SIZE) {
        if (qspif_write(pat, addr, OMV_BOOT_QSPIF_PAGE_SIZE) != 0) {
            return -3;
        }
    }

    for (uint32_t i = 0, addr = 0; i < OMV_BOOT_QSPIF_NUM_PAGES; i++, addr += OMV_BOOT_QSPIF_PAGE_SIZE) {
        memset(buf, 0, OMV_BOOT_QSPIF_PAGE_SIZE);
        if (qspif_read(buf, addr, OMV_BOOT_QSPIF_PAGE_SIZE) != 0) {
            return -4;
        }
        if (memcmp(buf, pat, OMV_BOOT_QSPIF_PAGE_SIZE) != 0) {
            return -5;
        }
    }

    if (qspif_erase_chip() != 0) {
        return -6;
    }

    return 0;
}
#endif //defined(OMV_BOOT_QSPIF_LAYOUT)
