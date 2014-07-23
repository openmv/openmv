#include <std.h>
#include <stdbool.h>
#include <stm32f4xx_hal.h>
#include "mdefs.h"
#include "pincfg.h"
#include "systick.h"
//#include "sdram.h"

#define SDRAM_TIMEOUT       ((uint32_t)0xFFFF)
#define REFRESH_COUNT       ((uint32_t)0x0569)   /* SDRAM refresh counter (90Mhz SD clock) */
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

static SDRAM_HandleTypeDef hsdram;
static FMC_SDRAM_TimingTypeDef SDRAM_Timing;
static FMC_SDRAM_CommandTypeDef command;
static void sdram_init_sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command);
extern void __fatal_error(const char *msg);

bool sdram_init()
{
    /* SDRAM device configuration */
    hsdram.Instance = FMC_SDRAM_DEVICE;
    /* Timing configuration for 90 Mhz of SD clock frequency (180Mhz/2) */
    /* TMRD: 2 Clock cycles */
    SDRAM_Timing.LoadToActiveDelay    = 2;
    /* TXSR: min=70ns (6x11.90ns) */
    SDRAM_Timing.ExitSelfRefreshDelay = 7;
    /* TRAS: min=42ns (4x11.90ns) max=120k (ns) */
    SDRAM_Timing.SelfRefreshTime      = 4;
    /* TRC:  min=63 (6x11.90ns) */
    SDRAM_Timing.RowCycleDelay        = 7;
    /* TWR:  2 Clock cycles */
    SDRAM_Timing.WriteRecoveryTime    = 2;
    /* TRP:  15ns => 2x11.90ns */
    SDRAM_Timing.RPDelay              = 2;
    /* TRCD: 15ns => 2x11.90ns */
    SDRAM_Timing.RCDDelay             = 2;

    hsdram.Init.SDBank             = FMC_SDRAM_BANK1;
    hsdram.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_12;
    hsdram.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_10;
    hsdram.Init.MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_8;
    hsdram.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
    hsdram.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_3;
    hsdram.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    hsdram.Init.SDClockPeriod      = FMC_SDRAM_CLOCK_PERIOD_3;
    hsdram.Init.ReadBurst          = FMC_SDRAM_RBURST_DISABLE;
    hsdram.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_1;

    /* Initialize the SDRAM controller */
    if(HAL_SDRAM_Init(&hsdram, &SDRAM_Timing) != HAL_OK) {
        return false;
    }

    /* Program the SDRAM external device */
    sdram_init_sequence(&hsdram, &command);
    return true;
}

static void sdram_init_sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command)
{
  __IO uint32_t tmpmrd =0;
  /* Step 3:  Configure a clock configuration enable command */
  Command->CommandMode           = FMC_SDRAM_CMD_CLK_ENABLE;
  Command->CommandTarget         = FMC_SDRAM_CMD_TARGET_BANK1;
  Command->AutoRefreshNumber     = 1;
  Command->ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);

  /* Step 4: Insert 100 ms delay */
  HAL_Delay(100);

  /* Step 5: Configure a PALL (precharge all) command */
  Command->CommandMode           = FMC_SDRAM_CMD_PALL;
  Command->CommandTarget         = FMC_SDRAM_CMD_TARGET_BANK1;
  Command->AutoRefreshNumber     = 1;
  Command->ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);

  /* Step 6 : Configure a Auto-Refresh command */
  Command->CommandMode           = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  Command->CommandTarget         = FMC_SDRAM_CMD_TARGET_BANK1;
  Command->AutoRefreshNumber     = 4;
  Command->ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);

  /* Step 7: Program the external memory mode register */
  tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2   |
      SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
      SDRAM_MODEREG_CAS_LATENCY_3           |
      SDRAM_MODEREG_OPERATING_MODE_STANDARD |
      SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

  Command->CommandMode           = FMC_SDRAM_CMD_LOAD_MODE;
  Command->CommandTarget         = FMC_SDRAM_CMD_TARGET_BANK1;
  Command->AutoRefreshNumber     = 1;
  Command->ModeRegisterDefinition = tmpmrd;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);

  /* Step 8: Set the refresh rate counter */
  /* (15.62 us x Freq) - 20 */
  /* Set the device refresh counter */
  HAL_SDRAM_ProgramRefreshRate(hsdram, REFRESH_COUNT);
}


bool DISABLE_OPT sdram_test()
{
    uint8_t pattern = 0xAA;
    uint8_t antipattern = 0x55;
    uint32_t mem_size = (16*1024*1024);
    uint8_t * const mem_base = (uint8_t*)0xC0000000;

    printf("sdram test...\n");
    /* test data bus */
    for (uint8_t i=1; i; i<<=1) {
        *mem_base = i;
        if (*mem_base != i) {
            printf("data bus lines test failed! data (%d)\n", i);
            BREAK();
        }
    }

    /* test address bus */
    /* Check individual address lines */
    for (uint32_t i=1; i<mem_size; i<<=1) {
        mem_base[i] = pattern;
        if (mem_base[i] != pattern) {
            printf("address bus lines test failed! address (%p)\n", &mem_base[i]);
            BREAK();
        }
    }

    /* Check for aliasing (overlaping addresses) */
    for (uint32_t i=1; i<mem_size; i<<=1) {
        mem_base[i] = antipattern;
        for (uint32_t p=i<<1; p<mem_size; p<<=1) {
            if (mem_base[p] != pattern) {
                printf("address bus overlap %p %p\n", &mem_base[i], &mem_base[p]);
                BREAK();
            }
        }
    }

    /* test all ram cells */
    for (uint32_t i=0; i<mem_size; i++) {
        mem_base[i] = pattern;
        if (mem_base[i] != pattern) {
            printf("address bus test failed! address (%p)\n", &mem_base[i]);
            BREAK();
        }
    }

    printf("sdram test passed\n");
    return true;
}
