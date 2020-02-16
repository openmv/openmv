/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * SDRAM Driver.
 *
 */
#include STM32_HAL_H
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "systick.h"
#include "sdram.h"
#include "omv_boardconfig.h"

#define SDRAM_TIMEOUT                               ((uint32_t)0xFFFF)
#define SDRAM_MODEREG_BURST_LENGTH_1                ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2                ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4                ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8                ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL         ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED        ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2                 ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3                 ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD       ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE        ((uint16_t)0x0200)

// Timing configuration for 200MHz/2=100MHz (10ns)
#define SDRAM_CLOCK_PERIOD                          2
#define SDRAM_CAS_LATENCY                           2
#define SDRAM_FREQUENCY                             (100000) // 100 MHz
#define SDRAM_TIMING_TMRD                           (2)
#define SDRAM_TIMING_TXSR                           (7)
#define SDRAM_TIMING_TRAS                           (5)
#define SDRAM_TIMING_TRC                            (6)
#define SDRAM_TIMING_TWR                            (3)
#define SDRAM_TIMING_TRP                            (2)
#define SDRAM_TIMING_TRCD                           (2)

// 32-bit SDRAM
#define SDRAM_ROW_BITS_NUM                          12
#define SDRAM_MEM_BUS_WIDTH                         32
#define SDRAM_REFRESH_CYCLES                        4096

#define SDRAM_COLUMN_BITS_NUM                       9
#define SDRAM_INTERN_BANKS_NUM                      4
#define SDRAM_RPIPE_DELAY                           0
#define SDRAM_RBURST                                (1)
#define SDRAM_WRITE_PROTECTION                      (0)

#define SDRAM_AUTOREFRESH_NUM                       (8)
#define SDRAM_BURST_LENGTH                          1

#define SDRAM_REFRESH_RATE                          (64) // ms

#define FMC_SDRAM_BANK FMC_SDRAM_BANK1
#define FMC_SDRAM_CMD_TARGET_BANK   (FMC_SDRAM_CMD_TARGET_BANK1)
#define SDRAM_START_ADDRESS         (0xC0000000)

// Provides the MPU_REGION_SIZE_X value when passed the size of region in bytes
// "m" must be a power of 2 between 32 and 4G (2**5 and 2**32) and this formula
// computes the log2 of "m", minus 1
#define MPU_REGION_SIZE(m) (((m) - 1) / (((m) - 1) % 255 + 1) / 255 % 255 * 8 + 7 - 86 / (((m) - 1) % 255 + 12) - 1)

#define SDRAM_MPU_REGION_SIZE (MPU_REGION_SIZE(OMV_SDRAM_SIZE))

#ifdef OMV_SDRAM_SIZE

static void sdram_init_seq(SDRAM_HandleTypeDef
        *hsdram, FMC_SDRAM_CommandTypeDef *command);
extern void __fatal_error(const char *msg);

bool sdram_init(void) {
    SDRAM_HandleTypeDef hsdram;
    FMC_SDRAM_TimingTypeDef SDRAM_Timing;
    FMC_SDRAM_CommandTypeDef command;

    __HAL_RCC_FMC_CLK_ENABLE();

    GPIO_InitTypeDef  GPIO_Init_Structure;
    GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
    GPIO_Init_Structure.Pull      = GPIO_PULLUP;
    GPIO_Init_Structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_Init_Structure.Alternate = GPIO_AF12_FMC;
  
    GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 |\
                              GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14|\
                              GPIO_PIN_15;
    HAL_GPIO_Init(GPIOF, &GPIO_Init_Structure);

    GPIO_Init_Structure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 |\
                              GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOH, &GPIO_Init_Structure);

    GPIO_Init_Structure.Pin = GPIO_PIN_4 | GPIO_PIN_5;
    HAL_GPIO_Init(GPIOC, &GPIO_Init_Structure);

    GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |\
                              GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOD, &GPIO_Init_Structure);

    GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |\
                              GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &GPIO_Init_Structure);

    GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 |\
                              GPIO_PIN_15;
    HAL_GPIO_Init(GPIOG, &GPIO_Init_Structure);

    GPIO_Init_Structure.Pin = GPIO_PIN_7;
    HAL_GPIO_Init(GPIOA, &GPIO_Init_Structure);

    GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 |\
                              GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_10;
    HAL_GPIO_Init(GPIOI, &GPIO_Init_Structure);

    /* SDRAM device configuration */
    hsdram.Instance = FMC_SDRAM_DEVICE;
    /* Timing configuration for 90 Mhz of SD clock frequency (180Mhz/2) */
    /* TMRD: 2 Clock cycles */
    SDRAM_Timing.LoadToActiveDelay    = SDRAM_TIMING_TMRD;
    /* TXSR: min=70ns (6x11.90ns) */
    SDRAM_Timing.ExitSelfRefreshDelay = SDRAM_TIMING_TXSR;
    /* TRAS */
    SDRAM_Timing.SelfRefreshTime      = SDRAM_TIMING_TRAS;
    /* TRC */
    SDRAM_Timing.RowCycleDelay        = SDRAM_TIMING_TRC;
    /* TWR */
    SDRAM_Timing.WriteRecoveryTime    = SDRAM_TIMING_TWR;
    /* TRP */
    SDRAM_Timing.RPDelay              = SDRAM_TIMING_TRP;
    /* TRCD */
    SDRAM_Timing.RCDDelay             = SDRAM_TIMING_TRCD;

    #define _FMC_INIT(x, n) x ## _ ## n
    #define FMC_INIT(x, n) _FMC_INIT(x,  n)

    hsdram.Init.SDBank             = FMC_SDRAM_BANK;
    hsdram.Init.ColumnBitsNumber   = FMC_INIT(FMC_SDRAM_COLUMN_BITS_NUM, SDRAM_COLUMN_BITS_NUM);
    hsdram.Init.RowBitsNumber      = FMC_INIT(FMC_SDRAM_ROW_BITS_NUM, SDRAM_ROW_BITS_NUM);
    hsdram.Init.MemoryDataWidth    = FMC_INIT(FMC_SDRAM_MEM_BUS_WIDTH, SDRAM_MEM_BUS_WIDTH);
    hsdram.Init.InternalBankNumber = FMC_INIT(FMC_SDRAM_INTERN_BANKS_NUM, SDRAM_INTERN_BANKS_NUM);
    hsdram.Init.CASLatency         = FMC_INIT(FMC_SDRAM_CAS_LATENCY, SDRAM_CAS_LATENCY);
    hsdram.Init.SDClockPeriod      = FMC_INIT(FMC_SDRAM_CLOCK_PERIOD, SDRAM_CLOCK_PERIOD);
    hsdram.Init.ReadPipeDelay      = FMC_INIT(FMC_SDRAM_RPIPE_DELAY, SDRAM_RPIPE_DELAY);
    hsdram.Init.ReadBurst          = (SDRAM_RBURST) ? FMC_SDRAM_RBURST_ENABLE : FMC_SDRAM_RBURST_DISABLE;
    hsdram.Init.WriteProtection    = (SDRAM_WRITE_PROTECTION) ? FMC_SDRAM_WRITE_PROTECTION_ENABLE : FMC_SDRAM_WRITE_PROTECTION_DISABLE;

    /* Initialize the SDRAM controller */
    if(HAL_SDRAM_Init(&hsdram, &SDRAM_Timing) != HAL_OK) {
        return false;
    }

    sdram_init_seq(&hsdram, &command);
    return true;
}

void *sdram_start(void) {
    return (void*)SDRAM_START_ADDRESS;
}

void *sdram_end(void) {
    return (void*)(SDRAM_START_ADDRESS + OMV_SDRAM_SIZE);
}

static void sdram_init_seq(SDRAM_HandleTypeDef
        *hsdram, FMC_SDRAM_CommandTypeDef *command)
{
    /* Program the SDRAM external device */
    __IO uint32_t tmpmrd =0;

    /* Step 3:  Configure a clock configuration enable command */
    command->CommandMode           = FMC_SDRAM_CMD_CLK_ENABLE;
    command->CommandTarget         = FMC_SDRAM_CMD_TARGET_BANK;
    command->AutoRefreshNumber     = 1;
    command->ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, command, 0xFFFF);

    /* Step 4: Insert 100 us delay */
    HAL_Delay(1);

    /* Step 5: Configure a PALL (precharge all) command */
    command->CommandMode           = FMC_SDRAM_CMD_PALL;
    command->CommandTarget         = FMC_SDRAM_CMD_TARGET_BANK;
    command->AutoRefreshNumber     = 1;
    command->ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, command, 0xFFFF);

    /* Step 6 : Configure a Auto-Refresh command */
    command->CommandMode           = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    command->CommandTarget         = FMC_SDRAM_CMD_TARGET_BANK;
    command->AutoRefreshNumber     = SDRAM_AUTOREFRESH_NUM;
    command->ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, command, 0xFFFF);

    /* Step 7: Program the external memory mode register */
    tmpmrd = (uint32_t)FMC_INIT(SDRAM_MODEREG_BURST_LENGTH, SDRAM_BURST_LENGTH) |
        SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
        FMC_INIT(SDRAM_MODEREG_CAS_LATENCY, SDRAM_CAS_LATENCY) |
        SDRAM_MODEREG_OPERATING_MODE_STANDARD |
        SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;
    command->CommandMode           = FMC_SDRAM_CMD_LOAD_MODE;
    command->CommandTarget         = FMC_SDRAM_CMD_TARGET_BANK;
    command->AutoRefreshNumber     = 1;
    command->ModeRegisterDefinition = tmpmrd;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, command, 0xFFFF);

    /* Step 8: Set the refresh rate counter
       RefreshRate = 64 ms / 8192 cyc = 7.8125 us/cyc

       RefreshCycles = 7.8125 us * 90 MHz = 703
       According to the formula on p.1665 of the reference manual,
       we also need to subtract 20 from the value, so the target
       refresh rate is 703 - 20 = 683.
     */
    #define REFRESH_COUNT (SDRAM_REFRESH_RATE * SDRAM_FREQUENCY / SDRAM_REFRESH_CYCLES - 20)
    HAL_SDRAM_ProgramRefreshRate(hsdram, REFRESH_COUNT);
}

bool __attribute__((optimize("O0"))) sdram_test(bool fast) {
    uint8_t const pattern = 0xaa;
    uint8_t const antipattern = 0x55;
    uint8_t *const mem_base = (uint8_t*)sdram_start();

    /* test data bus */
    for (uint8_t i = 1; i; i <<= 1) {
        *mem_base = i;
        if (*mem_base != i) {
            printf("data bus lines test failed! data (%d)\n", i);
            return false;
        }
    }

    /* test address bus */
    /* Check individual address lines */
    for (uint32_t i = 1; i < OMV_SDRAM_SIZE; i <<= 1) {
        mem_base[i] = pattern;
        if (mem_base[i] != pattern) {
            printf("address bus lines test failed! address (%p)\n", &mem_base[i]);
            return false;
        }
    }

    /* Check for aliasing (overlaping addresses) */
    mem_base[0] = antipattern;
    for (uint32_t i = 1; i < OMV_SDRAM_SIZE; i <<= 1) {
        if (mem_base[i] != pattern) {
            printf("address bus overlap %p\n", &mem_base[i]);
            return false;
        }
    }

    /* test all ram cells */
    if (!fast) {
        for (uint32_t i = 0; i < OMV_SDRAM_SIZE; ++i) {
            mem_base[i] = pattern;
            if (mem_base[i] != pattern) {
                printf("address bus test failed! address (%p)\n", &mem_base[i]);
                return false;
            }
        }
    } else {
        memset(mem_base, pattern, OMV_SDRAM_SIZE);
    }

    return true;
}

#endif // OMV_SDRAM_SIZE
