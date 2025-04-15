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
 * STM32 XSPI Flash driver.
 */
#include <string.h>
#include STM32_HAL_H
#include "omv_boardconfig.h"
#include "omv_bootconfig.h"

#if OMV_BOOT_XSPI_FLASH_SIZE

#define XSPI_PAGE_SIZE              (0x100)      // 256Bytes pages.
#define XSPI_BLOCK_SIZE             (0x10000)    // 64KBytes blocks.

#define XSPI_COMMAND_TIMEOUT        (1000)
#define XSPI_SECTOR_ERASE_TIMEOUT   (400)
#define XSPI_BLOCK_ERASE_TIMEOUT    (2000)
#define XSPI_CHIP_ERASE_TIMEOUT     (460000)

#define XSPI_CMD_RDID               (0x9F)
#define XSPI_CMD_RDID_DUMMY         (0)

#define XSPI_CMD_RESET_ENABLE       (0x66)
#define XSPI_CMD_RESET_MEMORY       (0x99)

#define XSPI_CMD_WRITE              (0x12)
#define XSPI_CMD_WRITE_DUMMY        (0)

#define XSPI_CMD_WRITE_DTR          (0x12EDU)
#define XSPI_CMD_WRITE_DTR_DUMMY    (0)

#define XSPI_CMD_WRITE_STR          (0x12EDU)
#define XSPI_CMD_WRITE_STR_DUMMY    (0)

#define XSPI_CMD_READ               (0x0C)
#define XSPI_CMD_READ_DUMMY         (8)

#define XSPI_CMD_READ_DTR           (0xEE11U)
#define XSPI_CMD_READ_DTR_DUMMY     (20)

#define XSPI_CMD_READ_STR           (0xEC13U)
#define XSPI_CMD_READ_STR_DUMMY     (20)

#define XSPI_CMD_ERASE_BLOCK        (0xDC)
#define XSPI_CMD_ERASE_CHIP         (0xC7)

#define XSPI_CMD_WRITE_ENABLE       (0x06)
#define XSPI_CMD_WRITE_DISABLE      (0x04)

#define XSPI_CMD_READ_SR            (0x05)
#define XSPI_CMD_READ_SR_DUMMY      (0)
#define XSPI_CMD_WRITE_CR2          (0x72)

#define XSPI_SR_WIP_MASK            (1 << 0)
#define XSPI_SR_WEL_MASK            (1 << 1)

static XSPI_HandleTypeDef xspi = {0};

static int xspi_flash_reset();
static int xspi_flash_read_id(uint8_t *);

int xspi_flash_init() {
    if (xspi.Instance != NULL) {
        return 0;
    }

    uint32_t xspi_kclk = 0;

    if (OMV_BOOT_XSPI_INSTANCE == 1) {
        xspi.Instance = XSPI1;
        xspi_kclk = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_XSPI1);
    } else if (OMV_BOOT_XSPI_INSTANCE == 2) {
        xspi.Instance = XSPI2;
        xspi_kclk = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_XSPI2);
    } else if (OMV_BOOT_XSPI_INSTANCE == 3) {
        xspi.Instance = XSPI3;
        xspi_kclk = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_XSPI3);
    }

    xspi.Init.FifoThresholdByte = 4U;
    xspi.Init.MemoryMode = HAL_XSPI_SINGLE_MEM;
    xspi.Init.MemoryType = HAL_XSPI_MEMTYPE_MACRONIX;
    xspi.Init.MemorySize = __builtin_ctz(OMV_BOOT_XSPI_FLASH_SIZE) - 1;
    xspi.Init.MemorySelect = HAL_XSPI_CSSEL_NCS1;
    xspi.Init.ChipSelectHighTimeCycle = 2U;
    xspi.Init.FreeRunningClock = HAL_XSPI_FREERUNCLK_DISABLE;
    xspi.Init.WrapSize = HAL_XSPI_WRAP_NOT_SUPPORTED;
    xspi.Init.ClockMode = HAL_XSPI_CLOCK_MODE_0;
    xspi.Init.ClockPrescaler = (xspi_kclk / OMV_BOOT_XSPI_FREQUENCY) - 1;
    xspi.Init.SampleShifting = HAL_XSPI_SAMPLE_SHIFT_NONE;
    xspi.Init.DelayHoldQuarterCycle = HAL_XSPI_DHQC_DISABLE;
    xspi.Init.ChipSelectBoundary = HAL_XSPI_BONDARYOF_NONE;

    // Reset and enable XSPI clock.
    if (OMV_BOOT_XSPI_INSTANCE == 1) {
        __HAL_RCC_XSPI1_FORCE_RESET();
        __HAL_RCC_XSPI1_RELEASE_RESET();
        __HAL_RCC_XSPI1_CLK_ENABLE();
        __HAL_RCC_XSPI1_CLK_SLEEP_ENABLE();
    } else if (OMV_BOOT_XSPI_INSTANCE == 2) {
        __HAL_RCC_XSPI2_FORCE_RESET();
        __HAL_RCC_XSPI2_RELEASE_RESET();
        __HAL_RCC_XSPI2_CLK_ENABLE();
        __HAL_RCC_XSPI2_CLK_SLEEP_ENABLE();
    } else if (OMV_BOOT_XSPI_INSTANCE == 3) {
        __HAL_RCC_XSPI3_FORCE_RESET();
        __HAL_RCC_XSPI3_RELEASE_RESET();
        __HAL_RCC_XSPI3_CLK_ENABLE();
        __HAL_RCC_XSPI3_CLK_SLEEP_ENABLE();
    }

    // Initialize the XSPI
    if (HAL_XSPI_Init(&xspi) != HAL_OK) {
        return -1;
    }

    // Hardware reset the flash
    #if defined(OMV_BOOT_XSPI_FLASH_RST_PIN)
    port_pin_write(OMV_BOOT_XSPI_FLASH_RST_PIN, 0);
    port_delay_ms(1);
    port_pin_write(OMV_BOOT_XSPI_FLASH_RST_PIN, 1);
    port_delay_ms(10);
    #endif

    // Reset the XSPI
    if (xspi_flash_reset() != 0) {
        return -1;
    }

    // Read flash ID.
    uint8_t buf[3] = {0};
    xspi_flash_read_id(buf);

    if (buf[0] != 0xc2 || buf[1] != 0x80 || buf[2] != 0x39) {
        while (1) {
            ;
        }
    }

    return 0;
}

static int xspi_flash_poll_status_flag(uint32_t mask, uint32_t match, uint32_t timeout) {
    XSPI_AutoPollingTypeDef config = {
        .MatchMask = mask,
        .MatchValue = match,
        .MatchMode = HAL_XSPI_MATCH_MODE_AND,
        .IntervalTime = 0x10,
        .AutomaticStop = HAL_XSPI_AUTOMATIC_STOP_ENABLE,
    };

    XSPI_RegularCmdTypeDef command = {
        .Instruction = XSPI_CMD_READ_SR,
        .InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE,
        .InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS,
        .InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE,
        .Address = 0,
        .AddressMode = HAL_XSPI_ADDRESS_NONE,
        .AddressWidth = HAL_XSPI_ADDRESS_32_BITS,
        .AddressDTRMode = HAL_XSPI_ADDRESS_DTR_DISABLE,
        .DataMode = HAL_XSPI_DATA_1_LINE,
        .DataLength = 1,
        .DataDTRMode = HAL_XSPI_DATA_DTR_DISABLE,
        .DummyCycles = XSPI_CMD_READ_SR_DUMMY,
        .IOSelect = HAL_XSPI_SELECT_IO_7_0,
        .AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE,
        .DQSMode = HAL_XSPI_DQS_DISABLE,
        .OperationType = HAL_XSPI_OPTYPE_COMMON_CFG,
    };

    if (HAL_XSPI_Command(&xspi, &command, XSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    if (HAL_XSPI_AutoPolling(&xspi, &config, timeout) != HAL_OK) {
        return -1;
    }
    return 0;
}

static int xspi_flash_write_enable() {
    XSPI_RegularCmdTypeDef command = {
        .Instruction = XSPI_CMD_WRITE_ENABLE,
        .InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE,
        .InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS,
        .InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE,
        .Address = 0U,
        .AddressMode = HAL_XSPI_ADDRESS_NONE,
        .AddressWidth = HAL_XSPI_ADDRESS_32_BITS,
        .AddressDTRMode = HAL_XSPI_ADDRESS_DTR_DISABLE,
        .DataMode = HAL_XSPI_DATA_NONE,
        .DataLength = 0,
        .DataDTRMode = HAL_XSPI_DATA_DTR_DISABLE,
        .DummyCycles = 0,
        .IOSelect = HAL_XSPI_SELECT_IO_7_0,
        .AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE,
        .DQSMode = HAL_XSPI_DQS_DISABLE,
        .OperationType = HAL_XSPI_OPTYPE_COMMON_CFG,
    };

    if (HAL_XSPI_Command(&xspi, &command, XSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    if (xspi_flash_poll_status_flag(XSPI_SR_WEL_MASK, XSPI_SR_WEL_MASK, XSPI_COMMAND_TIMEOUT) != 0) {
        return -1;
    }
    return 0;
}

static int spi_flash_write_cr2(uint8_t addr, uint8_t data) {
    XSPI_RegularCmdTypeDef command = {
        .OperationType = HAL_XSPI_OPTYPE_COMMON_CFG,
        .Instruction = XSPI_CMD_WRITE_CR2,
        .InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE,
        .InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE,
        .InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS,
        .Address = addr,
        .AddressMode = HAL_XSPI_ADDRESS_1_LINE,
        .AddressDTRMode = HAL_XSPI_ADDRESS_DTR_DISABLE,
        .AddressWidth = HAL_XSPI_ADDRESS_32_BITS,
        .AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE,
        .DataLength = 1,
        .DataMode = HAL_XSPI_DATA_1_LINE,
        .DataDTRMode = HAL_XSPI_DATA_DTR_DISABLE,
        .IOSelect = HAL_XSPI_SELECT_IO_7_0,
        .DummyCycles = 0,
        .DQSMode = HAL_XSPI_DQS_DISABLE,
    };

    // Enable write operations
    if (xspi_flash_write_enable(&xspi) != 0) {
        return -1;
    }

    if (HAL_XSPI_Command(&xspi, &command, XSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    if (HAL_XSPI_Transmit(&xspi, &data, XSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    return 0;
}

static int xspi_flash_reset() {
    XSPI_RegularCmdTypeDef command = {
        .InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE,
        .InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS,
        .InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE,
        .Address = 0U,
        .AddressMode = HAL_XSPI_ADDRESS_NONE,
        .AddressWidth = HAL_XSPI_ADDRESS_32_BITS,
        .AddressDTRMode = HAL_XSPI_ADDRESS_DTR_DISABLE,
        .DataMode = HAL_XSPI_DATA_NONE,
        .DataLength = 0,
        .DataDTRMode = HAL_XSPI_DATA_DTR_DISABLE,
        .DummyCycles = 0,
        .IOSelect = HAL_XSPI_SELECT_IO_7_0,
        .AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE,
        .DQSMode = HAL_XSPI_DQS_DISABLE,
        .OperationType = HAL_XSPI_OPTYPE_COMMON_CFG,
    };

    command.Instruction = XSPI_CMD_RESET_ENABLE;
    if (HAL_XSPI_Command(&xspi, &command, XSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    command.Instruction = XSPI_CMD_RESET_MEMORY;
    if (HAL_XSPI_Command(&xspi, &command, XSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    if (xspi_flash_poll_status_flag(XSPI_SR_WIP_MASK, 0, XSPI_COMMAND_TIMEOUT) != 0) {
        return -1;
    }
    return 0;
}

static int xspi_flash_read_id(uint8_t *buf) {
    XSPI_RegularCmdTypeDef command = {
        .Instruction = XSPI_CMD_RDID,
        .InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE,
        .InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS,
        .InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE,
        .Address = 0U,
        .AddressMode = HAL_XSPI_ADDRESS_NONE,
        .AddressWidth = HAL_XSPI_ADDRESS_32_BITS,
        .AddressDTRMode = HAL_XSPI_ADDRESS_DTR_DISABLE,
        .DataMode = HAL_XSPI_DATA_1_LINE,
        .DataLength = 3U,
        .DataDTRMode = HAL_XSPI_DATA_DTR_DISABLE,
        .DummyCycles = XSPI_CMD_RDID_DUMMY,
        .IOSelect = HAL_XSPI_SELECT_IO_7_0,
        .AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE,
        .DQSMode = HAL_XSPI_DQS_DISABLE,
        .OperationType = HAL_XSPI_OPTYPE_COMMON_CFG,
    };

    if (HAL_XSPI_Command(&xspi, &command, XSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    if (HAL_XSPI_Receive(&xspi, buf, XSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return 0;
}

static int xspi_flash_erase_chip() {
    XSPI_RegularCmdTypeDef command = {
        .Instruction = XSPI_CMD_ERASE_CHIP,
        .InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE,
        .InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS,
        .InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE,
        .Address = 0U,
        .AddressMode = HAL_XSPI_ADDRESS_NONE,
        .AddressWidth = HAL_XSPI_ADDRESS_32_BITS,
        .AddressDTRMode = HAL_XSPI_ADDRESS_DTR_DISABLE,
        .DataMode = HAL_XSPI_DATA_NONE,
        .DataLength = 0,
        .DataDTRMode = HAL_XSPI_DATA_DTR_DISABLE,
        .DummyCycles = 0,
        .IOSelect = HAL_XSPI_SELECT_IO_7_0,
        .AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE,
        .DQSMode = HAL_XSPI_DQS_DISABLE,
        .OperationType = HAL_XSPI_OPTYPE_COMMON_CFG,
    };

    if (xspi_flash_init() != 0) {
        return -1;
    }

    if (xspi_flash_write_enable(&xspi) != 0) {
        return -1;
    }

    if (HAL_XSPI_Command(&xspi, &command, XSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    if (xspi_flash_poll_status_flag(XSPI_SR_WIP_MASK, 0, XSPI_CHIP_ERASE_TIMEOUT) != 0) {
        return -1;
    }
    return 0;
}

static int xspi_flash_erase_block(uint32_t addr) {
    XSPI_RegularCmdTypeDef command = {
        .Instruction = XSPI_CMD_ERASE_BLOCK,
        .InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE,
        .InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS,
        .InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE,
        .Address = addr,
        .AddressMode = HAL_XSPI_ADDRESS_1_LINE,
        .AddressWidth = HAL_XSPI_ADDRESS_32_BITS,
        .AddressDTRMode = HAL_XSPI_ADDRESS_DTR_DISABLE,
        .DataMode = HAL_XSPI_DATA_NONE,
        .DataLength = 0,
        .DataDTRMode = HAL_XSPI_DATA_DTR_DISABLE,
        .DummyCycles = 0,
        .IOSelect = HAL_XSPI_SELECT_IO_7_0,
        .AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE,
        .DQSMode = HAL_XSPI_DQS_DISABLE,
        .OperationType = HAL_XSPI_OPTYPE_COMMON_CFG,
    };

    if (xspi_flash_init() != 0) {
        return -1;
    }

    if (xspi_flash_write_enable(&xspi) != 0) {
        return -1;
    }

    if (HAL_XSPI_Command(&xspi, &command, XSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    if (xspi_flash_poll_status_flag(XSPI_SR_WIP_MASK, 0, XSPI_BLOCK_ERASE_TIMEOUT) != 0) {
        return -1;
    }
    return 0;
}

static int xspi_flash_read_page(uint32_t addr, uint8_t *buf, uint32_t size) {
    XSPI_RegularCmdTypeDef command = {
        .Instruction = XSPI_CMD_READ,
        .DummyCycles = XSPI_CMD_READ_DUMMY,
        .InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE,
        .InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS,
        .InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE,
        .Address = addr,
        .AddressMode = HAL_XSPI_ADDRESS_1_LINE,
        .AddressWidth = HAL_XSPI_ADDRESS_32_BITS,
        .AddressDTRMode = HAL_XSPI_ADDRESS_DTR_DISABLE,
        .DataMode = HAL_XSPI_DATA_1_LINE,
        .DataLength = size,
        .DataDTRMode = HAL_XSPI_DATA_DTR_DISABLE,
        .IOSelect = HAL_XSPI_SELECT_IO_7_0,
        .AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE,
        .DQSMode = HAL_XSPI_DQS_DISABLE,
        .OperationType = HAL_XSPI_OPTYPE_COMMON_CFG,
    };

    if (xspi_flash_init() != 0) {
        return -1;
    }

    if (HAL_XSPI_Command(&xspi, &command, XSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    if (HAL_XSPI_Receive(&xspi, buf, XSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return 0;
}

static int xspi_flash_write_page(uint32_t addr, const uint8_t *buf, uint32_t size) {
    XSPI_RegularCmdTypeDef command = {
        .Instruction = XSPI_CMD_WRITE,
        .InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE,
        .InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS,
        .InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE,
        .Address = addr,
        .AddressMode = HAL_XSPI_ADDRESS_1_LINE,
        .AddressWidth = HAL_XSPI_ADDRESS_32_BITS,
        .AddressDTRMode = HAL_XSPI_ADDRESS_DTR_DISABLE,
        .DataMode = HAL_XSPI_DATA_1_LINE,
        .DataLength = size,
        .DataDTRMode = HAL_XSPI_DATA_DTR_DISABLE,
        .DummyCycles = 0,
        .IOSelect = HAL_XSPI_SELECT_IO_7_0,
        .AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE,
        .DQSMode = HAL_XSPI_DQS_DISABLE,
        .OperationType = HAL_XSPI_OPTYPE_COMMON_CFG,
    };

    // Address must be page-aligned and the size <= page size.
    if ((addr % XSPI_PAGE_SIZE) != 0 ||
        size <= 0 || size > XSPI_PAGE_SIZE) {
        return -1;
    }

    if (xspi_flash_init() != 0) {
        return -1;
    }

    // Enable write operations
    if (xspi_flash_write_enable(&xspi) != 0) {
        return -1;
    }

    // Configure the command
    if (HAL_XSPI_Command(&xspi, &command, XSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    // Transmit the data
    if (HAL_XSPI_Transmit(&xspi, (uint8_t *) buf, XSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    // Poll the status register.
    if (xspi_flash_poll_status_flag(XSPI_SR_WIP_MASK, 0, XSPI_COMMAND_TIMEOUT) != 0) {
        return -1;
    }
    return 0;
}

int spi_flash_memory_map(bool dtr) {
    XSPI_RegularCmdTypeDef command = {
        .InstructionMode = (dtr) ? HAL_XSPI_INSTRUCTION_8_LINES : HAL_XSPI_INSTRUCTION_1_LINE,
        .InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS,
        .InstructionDTRMode = (dtr) ? HAL_XSPI_INSTRUCTION_DTR_ENABLE : HAL_XSPI_INSTRUCTION_DTR_DISABLE,
        .AddressMode = (dtr) ? HAL_XSPI_ADDRESS_8_LINES : HAL_XSPI_ADDRESS_1_LINE,
        .AddressWidth = HAL_XSPI_ADDRESS_32_BITS,
        .AddressDTRMode = (dtr) ? HAL_XSPI_ADDRESS_DTR_ENABLE : HAL_XSPI_ADDRESS_DTR_DISABLE,
        .DataMode = (dtr) ? HAL_XSPI_DATA_8_LINES : HAL_XSPI_DATA_1_LINE,
        .DataDTRMode = (dtr) ? HAL_XSPI_DATA_DTR_ENABLE : HAL_XSPI_DATA_DTR_DISABLE,
    };

    if (xspi_flash_init() != 0) {
        return -1;
    }

    if (dtr && spi_flash_write_cr2(0x0, 0x02) != 0) {
        return -1;
    }

    // Initialize the write command
    command.Instruction = (dtr) ? XSPI_CMD_WRITE_DTR : XSPI_CMD_WRITE;
    command.DummyCycles = (dtr) ? XSPI_CMD_WRITE_DTR_DUMMY : XSPI_CMD_WRITE_DUMMY;
    command.OperationType = HAL_XSPI_OPTYPE_WRITE_CFG;
    command.DQSMode = HAL_XSPI_DQS_DISABLE;
    if (HAL_XSPI_Command(&xspi, &command, XSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    // Initialize the read command
    command.Instruction = (dtr) ? XSPI_CMD_READ_DTR : XSPI_CMD_READ;
    command.DummyCycles = (dtr) ? XSPI_CMD_READ_DTR_DUMMY : XSPI_CMD_READ_DUMMY;
    command.OperationType = HAL_XSPI_OPTYPE_READ_CFG;
    command.DQSMode = (dtr) ? HAL_XSPI_DQS_ENABLE : HAL_XSPI_DQS_DISABLE;
    if (HAL_XSPI_Command(&xspi, &command, XSPI_COMMAND_TIMEOUT) != HAL_OK) {
        return -1;
    }

    XSPI_MemoryMappedTypeDef s_mem_mapped_cfg = {
        .TimeOutActivation = HAL_XSPI_TIMEOUT_COUNTER_DISABLE
    };
    if (HAL_XSPI_MemoryMapped(&xspi, &s_mem_mapped_cfg) != HAL_OK) {
        return -1;
    }
    return 0;
}

int __attribute__((optimize("O0"))) xspi_flash_memory_test() {
    uint8_t buf[XSPI_PAGE_SIZE];
    uint8_t pat[XSPI_PAGE_SIZE];

    uint32_t n_pages = OMV_BOOT_XSPI_FLASH_SIZE / XSPI_PAGE_SIZE;
    uint32_t n_blocks = OMV_BOOT_XSPI_FLASH_SIZE / XSPI_BLOCK_SIZE;

    if (xspi_flash_init() != 0) {
        return -1;
    }

    for (int i = 0; i < XSPI_PAGE_SIZE; i++) {
        pat[i] = ((i % 2) == 0) ? 0xaa : 0x55;
    }

    for (uint32_t i = 0, addr = 0; i < n_blocks; i++, addr += XSPI_BLOCK_SIZE) {
        if (xspi_flash_erase_block(addr) != 0) {
            return -2;
        }
    }

    for (uint32_t i = 0, addr = 0; i < n_pages; i++, addr += XSPI_PAGE_SIZE) {
        if (xspi_flash_write_page(addr, pat, XSPI_PAGE_SIZE) != 0) {
            return -3;
        }
    }

    for (uint32_t i = 0, addr = 0; i < n_pages; i++, addr += XSPI_PAGE_SIZE) {
        memset(buf, 0, XSPI_PAGE_SIZE);
        if (xspi_flash_read_page(addr, buf, XSPI_PAGE_SIZE) != 0) {
            return -4;
        }
        if (memcmp(buf, pat, XSPI_PAGE_SIZE) != 0) {
            return -5;
        }
    }

    if (xspi_flash_erase_chip() != 0) {
        return -6;
    }
    return 0;
}

int spi_flash_deinit() {
    if (xspi.Instance != NULL) {
        HAL_XSPI_DeInit(&xspi);
        xspi.Instance = NULL;
    }
    // Reset XSPI.
    if (OMV_BOOT_XSPI_INSTANCE == 1) {
        __HAL_RCC_XSPI1_FORCE_RESET();
        __HAL_RCC_XSPI1_RELEASE_RESET();
        __HAL_RCC_XSPI1_CLK_DISABLE();
        __HAL_RCC_XSPI1_CLK_SLEEP_DISABLE();
    } else if (OMV_BOOT_XSPI_INSTANCE == 2) {
        __HAL_RCC_XSPI2_FORCE_RESET();
        __HAL_RCC_XSPI2_RELEASE_RESET();
        __HAL_RCC_XSPI2_CLK_DISABLE();
        __HAL_RCC_XSPI2_CLK_SLEEP_DISABLE();
    } else if (OMV_BOOT_XSPI_INSTANCE == 3) {
        __HAL_RCC_XSPI3_FORCE_RESET();
        __HAL_RCC_XSPI3_RELEASE_RESET();
        __HAL_RCC_XSPI3_CLK_DISABLE();
        __HAL_RCC_XSPI3_CLK_SLEEP_DISABLE();
    }
    memset(&xspi, 0, sizeof(XSPI_HandleTypeDef));
    return 0;
}

int spi_flash_read(uint32_t addr, uint8_t *buf, uint32_t size) {
    for (size_t i = 0; i < size / XSPI_PAGE_SIZE; i++) {
        if (xspi_flash_read_page(addr, buf, XSPI_PAGE_SIZE) != 0) {
            return -1;
        }
        buf += XSPI_PAGE_SIZE;
        addr += XSPI_PAGE_SIZE;
    }

    if ((size % XSPI_PAGE_SIZE)) {
        if (xspi_flash_read_page(addr, buf, (size % XSPI_PAGE_SIZE)) != 0) {
            return -1;
        }
    }
    return 0;
}

int spi_flash_write(uint32_t addr, const uint8_t *buf, uint32_t size) {
    // If at a block boundary, erase the block first.
    if ((addr % XSPI_BLOCK_SIZE) == 0) {
        if (xspi_flash_erase_block(addr) != 0) {
            return -1;
        }
    }

    for (size_t i = 0; i < size / XSPI_PAGE_SIZE; i++) {
        if (xspi_flash_write_page(addr, buf, XSPI_PAGE_SIZE) != 0) {
            return -1;
        }
        buf += XSPI_PAGE_SIZE;
        addr += XSPI_PAGE_SIZE;
    }

    if ((size % XSPI_PAGE_SIZE)) {
        if (xspi_flash_write_page(addr, buf, (size % XSPI_PAGE_SIZE)) != 0) {
            return -1;
        }
    }
    return 0;
}
#endif // OMV_BOOT_XSPI_FLASH_SIZE
