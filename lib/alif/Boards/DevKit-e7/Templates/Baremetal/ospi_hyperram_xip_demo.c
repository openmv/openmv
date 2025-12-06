/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/******************************************************************************
 * @file     ospi_hyperram_xip_demo.c
 * @author   Silesh C V
 * @email    silesh@alifsemi.com
 * @version  V1.0.0
 * @date     20-Jul-2023
 * @brief    Demo program for the OSPI hyperram XIP library API.
 ******************************************************************************/

/*
 * Below are the constraints to make use of HyperRAM memory on Ensemble:
 * 1. Hardcoded DFS for writes is not supported. Writes matching the same data frame size as designated for
 *    hardcoded DFS reads must be used to avoid unexpected results.
 * 2. 8-bit writes and 8-bit DFS are not supported at all for Octal DDR operation, including HyperBus.
 * 3. AXI bus sparse writes are not supported.
 * 4. In general, RTSS Cortex-M55 HP, RTSS Cortex-M55 HE cannot use the HyperRAM as a general-purpose
 *    memory due to the above limitations. The Cortex-M55s can write to HyperRAM as long as accesses
 *    are controlled to stay within the above constraints — this would  require writing with the
 *    memory mapped as Device Mode for either Cortex-M55.
 */

#include "ospi_hyperram_xip.h"
#include "pinconf.h"
#include "Driver_GPIO.h"
#include "RTE_Components.h"
#include CMSIS_device_header
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#define OSPI_RESET_PORT     LP
#define OSPI_RESET_PIN      6
#define OSPI0_XIP_BASE      0xA0000000

extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(OSPI_RESET_PORT);
ARM_DRIVER_GPIO *GPIODrv = &ARM_Driver_GPIO_(OSPI_RESET_PORT);

/* OSPI0 region index is 5 in mpu table defined in the same testapp */
#define MPU_OSPI0_REGION_INDEX  5U

#define DDR_DRIVE_EDGE      0
#define RXDS_DELAY          11
#define OSPI_BUS_SPEED      100000000           /* 100MHz */
#define ISSI_WAIT_CYCLES    6
#define OSPI_DFS            16

#define HRAM_SIZE_BYTES     (32 * 1024 * 1024)  /* 32MB */

#define BUFFER_SIZE      (16 * 1024)
static uint16_t buff[BUFFER_SIZE/sizeof(uint16_t)]; /* Buffer size of 16KB */

static const ospi_hyperram_xip_config issi_config = {
    .instance       = OSPI_INSTANCE_0,
    .bus_speed      = OSPI_BUS_SPEED,
    .hyperram_init  = NULL, /* No special initialization needed by the hyperram device */
    .ddr_drive_edge = DDR_DRIVE_EDGE,
    .rxds_delay     = RXDS_DELAY,
    .wait_cycles    = ISSI_WAIT_CYCLES,
    .slave_select   = 0,
    .dfs            = OSPI_DFS
};

void MPU_Load_Regions(void)
{
/* Define the memory attribute index with the below properties */
#define MEMATTRIDX_NORMAL_WT_RA_TRANSIENT    0
#define MEMATTRIDX_DEVICE_nGnRE              1
#define MEMATTRIDX_NORMAL_WB_RA_WA           2
#define MEMATTRIDX_NORMAL_WT_RA              3
#define MEMATTRIDX_NORMAL_NON_CACHEABLE      4

    static const ARM_MPU_Region_t mpu_table[] =
    {
        {   /* SRAM0 - 4MB : RO-0, NP-1, XN-0 */
            .RBAR = ARM_MPU_RBAR(0x02000000, ARM_MPU_SH_NON, 0, 1, 0),
            .RLAR = ARM_MPU_RLAR(0x023FFFFF, MEMATTRIDX_NORMAL_WT_RA_TRANSIENT)
        },
        {   /* SRAM1 - 2.5MB : RO-0, NP-1, XN-0 */
            .RBAR = ARM_MPU_RBAR(0x08000000, ARM_MPU_SH_NON, 0, 1, 0),
            .RLAR = ARM_MPU_RLAR(0x0827FFFF, MEMATTRIDX_NORMAL_WB_RA_WA)
        },
        {   /* Host Peripherals - 16MB : RO-0, NP-1, XN-1 */
            .RBAR = ARM_MPU_RBAR(0x1A000000, ARM_MPU_SH_NON, 0, 1, 1),
            .RLAR = ARM_MPU_RLAR(0x1AFFFFFF, MEMATTRIDX_DEVICE_nGnRE)
        },
        {   /* MRAM - 5.5MB : RO-1, NP-1, XN-0  */
            .RBAR = ARM_MPU_RBAR(0x80000000, ARM_MPU_SH_NON, 1, 1, 0),
            .RLAR = ARM_MPU_RLAR(0x8057FFFF, MEMATTRIDX_NORMAL_WT_RA)
        },
        {   /* OSPI Regs - 16MB : RO-0, NP-1, XN-1  */
            .RBAR = ARM_MPU_RBAR(0x83000000, ARM_MPU_SH_NON, 0, 1, 1),
            .RLAR = ARM_MPU_RLAR(0x83FFFFFF, MEMATTRIDX_DEVICE_nGnRE)
        },
        {   /* OSPI0 XIP(eg:hyperram) - 512MB : RO-1, NP-1, XN-0  */
            .RBAR = ARM_MPU_RBAR(0xA0000000, ARM_MPU_SH_NON, 1, 1, 0),
            .RLAR = ARM_MPU_RLAR(0xBFFFFFFF, MEMATTRIDX_NORMAL_WB_RA_WA)
        },
        {   /* OSPI1 XIP(eg:flash) - 512MB : RO-1, NP-1, XN-0  */
            .RBAR = ARM_MPU_RBAR(0xC0000000, ARM_MPU_SH_NON, 1, 1, 0),
            .RLAR = ARM_MPU_RLAR(0xDFFFFFFF, MEMATTRIDX_NORMAL_WT_RA)
        },
    };

    /* Mem Attribute for 0th index */
    ARM_MPU_SetMemAttr(MEMATTRIDX_NORMAL_WT_RA_TRANSIENT, ARM_MPU_ATTR(
                                         /* NT=0, WB=0, RA=1, WA=0 */
                                         ARM_MPU_ATTR_MEMORY_(0,0,1,0),
                                         ARM_MPU_ATTR_MEMORY_(0,0,1,0)));

    /* Mem Attribute for 1st index */
    ARM_MPU_SetMemAttr(MEMATTRIDX_DEVICE_nGnRE, ARM_MPU_ATTR(
                                         /* Device Memory */
                                         ARM_MPU_ATTR_DEVICE,
                                         ARM_MPU_ATTR_DEVICE_nGnRE));

    /* Mem Attribute for 2nd index */
    ARM_MPU_SetMemAttr(MEMATTRIDX_NORMAL_WB_RA_WA, ARM_MPU_ATTR(
                                         /* NT=1, WB=1, RA=1, WA=1 */
                                         ARM_MPU_ATTR_MEMORY_(1,1,1,1),
                                         ARM_MPU_ATTR_MEMORY_(1,1,1,1)));

    /* Mem Attribute for 3th index */
    ARM_MPU_SetMemAttr(MEMATTRIDX_NORMAL_WT_RA, ARM_MPU_ATTR(
                                         /* NT=1, WB=0, RA=1, WA=0 */
                                         ARM_MPU_ATTR_MEMORY_(1,0,1,0),
                                         ARM_MPU_ATTR_MEMORY_(1,0,1,0)));

    /* Mem Attribute for 4th index */
    ARM_MPU_SetMemAttr(MEMATTRIDX_NORMAL_NON_CACHEABLE, ARM_MPU_ATTR(
                                         ARM_MPU_ATTR_NON_CACHEABLE,
                                         ARM_MPU_ATTR_NON_CACHEABLE));

    /* Load the regions from the table */
    ARM_MPU_Load(0, mpu_table, sizeof(mpu_table)/sizeof(ARM_MPU_Region_t));
}

static void mpu_set_ospi0_xip_device_attr(void)
{
    __DSB();

    ARM_MPU_SetRegion(MPU_OSPI0_REGION_INDEX,
        ARM_MPU_RBAR(0xA0000000, ARM_MPU_SH_NON, 0, 1, 0),  /* Non-shareable, RO-0, NP-1, XN-0 */
        ARM_MPU_RLAR(0xBFFFFFFF, MEMATTRIDX_DEVICE_nGnRE)
    );

    __DSB();
    __ISB();
}

static void mpu_set_ospi0_xip_noncacheable_attr(void)
{
    __DSB();

    ARM_MPU_SetRegion(MPU_OSPI0_REGION_INDEX,
        ARM_MPU_RBAR(0xA0000000, ARM_MPU_SH_NON, 1, 1, 0),  /* Non-shareable, RO-1, NP-1, XN-0 */
        ARM_MPU_RLAR(0xBFFFFFFF, MEMATTRIDX_NORMAL_NON_CACHEABLE)
    );

    __DSB();
    __ISB();
}

static void mpu_set_ospi0_xip_cacheable_attr(void)
{
    __DSB();

    ARM_MPU_SetRegion(MPU_OSPI0_REGION_INDEX,
        ARM_MPU_RBAR(0xA0000000, ARM_MPU_SH_NON, 1, 1, 0),  /* Non-shareable, RO-1, NP-1, XN-0 */
        ARM_MPU_RLAR(0xBFFFFFFF, MEMATTRIDX_NORMAL_WB_RA_WA)
        );

    __DSB();
    __ISB();
}

static int32_t pinmux_setup()
{
    int32_t ret;

    ret = pinconf_set(PORT_2, PIN_0, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE );
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_2, PIN_1, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE );
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_2, PIN_2, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE);
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_2, PIN_3, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE );
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_2, PIN_4, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE );
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_2, PIN_5, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE);
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_2, PIN_6, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE );
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_2, PIN_7, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE );
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_3, PIN_0, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE );
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_3, PIN_1, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE );
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_3, PIN_2, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE );
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_1, PIN_6, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE );
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_15, PIN_6, PINMUX_ALTERNATE_FUNCTION_0, 0);
    if (ret)
    {
        return -1;
    }

    ret = GPIODrv->Initialize(OSPI_RESET_PIN, NULL);
    if (ret != ARM_DRIVER_OK)
    {
        return -1;
    }

    ret = GPIODrv->PowerControl(OSPI_RESET_PIN, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK)
    {
        return -1;
    }

    ret = GPIODrv->SetDirection(OSPI_RESET_PIN, GPIO_PIN_DIRECTION_OUTPUT);
    if (ret != ARM_DRIVER_OK)
    {
        return -1;
    }

    ret = GPIODrv->SetValue(OSPI_RESET_PIN, GPIO_PIN_OUTPUT_STATE_LOW);
    if (ret != ARM_DRIVER_OK)
    {
        return -1;
    }

    ret = GPIODrv->SetValue(OSPI_RESET_PIN, GPIO_PIN_OUTPUT_STATE_HIGH);
    if (ret != ARM_DRIVER_OK)
    {
        return -1;
    }

    return 0;
}

void read_8bit(const void *ptr, const void *buff, uint32_t size)
{
    uint32_t errors = 0;
    volatile const uint8_t *ptr8 = ptr;
    const uint8_t *buff8 = buff;

    for (int i = 0; i < size; i++)
    {
        if (ptr8[i] != buff8[i])
        {
            printf("Data error at addr %x, got %x, expected %x\n", i, ptr8[i], buff8[i]);
            errors++;
        }
    }

    printf("Total Errors for 8-bit read memory %d\n", errors);
}

void read_16bit(const void *ptr, const void *buff, uint32_t size)
{
    uint32_t errors = 0;
    volatile const uint16_t *ptr16 = ptr;
    const uint16_t *buff16 = buff;

    for (int i = 0; i < (size/sizeof(uint16_t)); i++)
    {
        if (ptr16[i] != buff16[i])
        {
            printf("Data error at addr %x, got %x, expected %x\n", i, ptr16[i], buff16[i]);
            errors++;
        }
    }

    printf("Total Errors for 16-bit read memory %d\n", errors);
}

void read_32bit(const void *ptr, const void *buff, uint32_t size)
{
    uint32_t errors = 0;
    volatile const uint32_t *ptr32 = ptr;
    const uint32_t *buff32 = buff;

    for (int i = 0; i < (size/sizeof(uint32_t)); i++)
    {
        if (ptr32[i] != buff32[i])
        {
            printf("Data error at addr %x, got %x, expected %x\n", i, ptr32[i], buff32[i]);
            errors++;
        }
    }

    printf("Total Errors for 32-bit read memory %d\n", errors);
}

void read_with_device_attr(const void *ptr, uint32_t size)
{
    printf("\nReading HyperRAM memory in device memory attr\n");
    mpu_set_ospi0_xip_device_attr();

    read_8bit(ptr, buff, size);

    read_16bit(ptr, buff, size);

    read_32bit(ptr, buff, size);
}

void read_with_normal_non_cacheable_attr(const void *ptr, uint32_t size)
{
    printf("\nReading HyperRAM memory in non-cacheable memory attr\n");
    mpu_set_ospi0_xip_noncacheable_attr();

    read_8bit(ptr, buff, size);

    read_16bit(ptr, buff, size);

    read_32bit(ptr, buff, size);
}

void read_with_normal_cacheable_attr(const void *ptr, uint32_t size)
{
    uint32_t total_errors;

    printf("\nReading HyperRAM memory in cacheable memory attr\n");
    mpu_set_ospi0_xip_cacheable_attr();

    total_errors = memcmp(ptr, buff, size);
    printf("Total Errors %d\n", total_errors);
}

void hyperram_test(void)
{
    volatile uint16_t *ptr = (uint16_t *) OSPI0_XIP_BASE;
    uint32_t errors = 0;
    uint16_t val;

    /* writing 16KB of random data to HyperRAM using the device memory
     * attribute, followed by multiple readbacks using device, non-cacheable
     * normal, and cacheable normal memory types with various access sizes
     * (8-bit, 16-bit, 32-bit). The entire sequence is repeated with a different
     * data pattern to ensure there are no cache-related inconsistencies.
     */
    for (int k = 1; k < 3; k++)
    {
        srand(k);
        mpu_set_ospi0_xip_device_attr();

        SCB_InvalidateDCache_by_Addr(ptr, BUFFER_SIZE);

        /* writing 16KB data to hyperram memory in device attr */
        for (int i = 0; i < (BUFFER_SIZE/sizeof(uint16_t)); i++)
        {
            val = (rand() % 0xFFFF);
            buff[i] = val;
            ptr[i] = val;
        }

        /* read back 16KB data from hyperram memory in device attr */
        read_with_device_attr((const void *)ptr, BUFFER_SIZE);

        /* read back 16KB data from hyperram memory in normal non-cacheable attr */
        read_with_normal_non_cacheable_attr((const void *)ptr, BUFFER_SIZE);

        /* read back 16KB data from hyperram memory in normal cacheable attr */
        read_with_normal_cacheable_attr((const void *)ptr, BUFFER_SIZE);
    }

    /* writing to whole HyperRAM memory and reading back in cacheable attr */
    printf("\nTesting full memory with cache enabled reads\n");
    srand(3);

    mpu_set_ospi0_xip_device_attr();
    SCB_InvalidateDCache_by_Addr((void *)OSPI0_XIP_BASE, HRAM_SIZE_BYTES);

    for (int i = 0; i < (HRAM_SIZE_BYTES/sizeof(uint16_t)); i++)
    {
        ptr[i] = (rand() % 0xFFFF);
    }

    mpu_set_ospi0_xip_cacheable_attr();

    srand(3);
    for (int i = 0; i < (HRAM_SIZE_BYTES/sizeof(uint16_t)); i++)
    {
        val = rand() % 0xFFFF;
        if (ptr[i] != val)
        {
            printf("Data error at addr %x, got %x, expected %x\n", i, ptr[i], val);
            errors++;
        }
    }

    printf("Total errors after read complete: %d\n", errors);
}

int main(void)
{
#if defined(RTE_Compiler_IO_STDOUT_User)
    int32_t ret;
    ret = stdout_init();
    if(ret != ARM_DRIVER_OK)
    {
        while(1)
        {
        }
    }
    #endif

    printf("HyperRAM demo app started\n");

    if (pinmux_setup() < 0)
    {
        printf("Pinmux/GPIO setup failed\n");
        goto error_exit;
    }

    if (ospi_hyperram_xip_init(&issi_config) < 0)
    {
        printf("Hyperram XIP init failed\n");
        goto error_exit;
    }

    hyperram_test();

    printf("\nHyperRAM demo app completed\n");

error_exit:

    while(1);

    return 0;
}
