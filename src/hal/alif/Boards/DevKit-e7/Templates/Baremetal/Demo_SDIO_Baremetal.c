/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
/**************************************************************************//**
 * @file     Demo_SDIO_Baremetal.c
 * @author   Deepak Kumar
 * @email    deepak@alifsemi.com
 * @version  V0.0.1
 * @date     28-Nov-2022
 * @brief    Baremeetal sdio driver test Application.
 * @bug      None.
 * @Note     None
 ******************************************************************************/
/* System Includes */
#include "RTE_Device.h"
#include "stdio.h"
#include "se_services_port.h"

/* include for SD Driver */
#include "sdio.h"
#include "sd.h"

/* include for Pin Mux config */
#include "pinconf.h"

#include "RTE_Components.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#include "Driver_Common.h"
#endif  /* RTE_Compiler_IO_STDOUT */

#define SDC_A 1
#define SDC_B 2
#define SDC_C 3
#define SDC_PINS SDC_A

#define BAREMETAL_SD_TEST_RAW_SECTOR 0x2000     //start reading and writing raw data from partition sector
volatile unsigned char sdbuffer[512*4] __attribute__((section("sd_dma_buf"))) __attribute__((aligned(32)));

const diskio_t  *p_SD_Driver = &SD_Driver;
volatile uint32_t dma_done_irq = 0;

/**
  \fn           sd_cb(uint32_t status)
  \brief        SD interrupt callback
  \param[in]    uint32_t status
  \return       none
*/
void sd_cb(uint32_t status) {
    /* dummy callback definition to resolve linking error */
    /* data transfer in SDIO mode needs Stack (wifi/bluetooth) */
}

/**
  \fn           BareMetalSDTest(uint32_t startSec, uint32_t EndSector)
  \brief        Baremetal SD driver Test Function
  \param[in]    starSecr - Test Read/Write start sector number
  \param[in]    EndSector - Test Read/Write End sector number
  \return       none
*/
void BareMetalSDIOTest(){

    sd_param_t sd_param;

    /* SD Clock and Board Pin mux Configurations */
#if (SDC_PINS == SDC_A)
    pinconf_set(PORT_7, PIN_0, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE); //cmd
    pinconf_set(PORT_7, PIN_1, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE); //clk
    pinconf_set(PORT_5, PIN_0, PINMUX_ALTERNATE_FUNCTION_7, PADCTRL_READ_ENABLE); //d0

#if RTE_SDC_BUS_WIDTH == SDMMC_4_BIT_MODE
    pinconf_set(PORT_5, PIN_1, PINMUX_ALTERNATE_FUNCTION_7, PADCTRL_READ_ENABLE); //d1
    pinconf_set(PORT_5, PIN_2, PINMUX_ALTERNATE_FUNCTION_7, PADCTRL_READ_ENABLE); //d2
    pinconf_set(PORT_5, PIN_3, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE); //d3
#endif

#if RTE_SDC_BUS_WIDTH == SDMMC_8_BIT_MODE
    pinconf_set(PORT_5, PIN_4, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE); //d4
    pinconf_set(PORT_5, PIN_5, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE); //d5
    pinconf_set(PORT_5, PIN_6, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE); //d6
    pinconf_set(PORT_5, PIN_7, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE); //d7
#endif

#elif (SDC_PINS == SDC_B)
    pinconf_set(PORT_14, PIN_0, PINMUX_ALTERNATE_FUNCTION_4, PADCTRL_READ_ENABLE); //cmd
    pinconf_set(PORT_14, PIN_1, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE); //clk
    pinconf_set(PORT_14, PIN_2, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE); //rst
    pinconf_set(PORT_13, PIN_0, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE); //d0
#if RTE_SDC_BUS_WIDTH == SDMMC_4_BIT_MODE
    pinconf_set(PORT_13, PIN_1, PINMUX_ALTERNATE_FUNCTION_4, PADCTRL_READ_ENABLE); //d1
    pinconf_set(PORT_13, PIN_2, PINMUX_ALTERNATE_FUNCTION_4, PADCTRL_READ_ENABLE); //d2
    pinconf_set(PORT_13, PIN_3, PINMUX_ALTERNATE_FUNCTION_4, PADCTRL_READ_ENABLE); //d3
#endif

#elif (SDC_PINS == SDC_C)
    pinconf_set(PORT_9, PIN_0, PINMUX_ALTERNATE_FUNCTION_4, PADCTRL_READ_ENABLE | PADCTRL_OUTPUT_DRIVE_STRENGTH_8MA); //cmd
    pinconf_set(PORT_9, PIN_1, PINMUX_ALTERNATE_FUNCTION_4, PADCTRL_READ_ENABLE | PADCTRL_OUTPUT_DRIVE_STRENGTH_8MA); //clk
    pinconf_set(PORT_8, PIN_0, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE | PADCTRL_OUTPUT_DRIVE_STRENGTH_8MA); //d0
#if RTE_SDC_BUS_WIDTH == SDMMC_4_BIT_MODE
    pinconf_set(PORT_8, PIN_1, PINMUX_ALTERNATE_FUNCTION_4, PADCTRL_READ_ENABLE | PADCTRL_OUTPUT_DRIVE_STRENGTH_8MA); //d1
    pinconf_set(PORT_8, PIN_2, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE | PADCTRL_OUTPUT_DRIVE_STRENGTH_8MA); //d2
    pinconf_set(PORT_8, PIN_3, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE | PADCTRL_OUTPUT_DRIVE_STRENGTH_8MA); //d3
#endif

#else
    #error "Invalid SDMMC Pins defined...\n"

#endif

    sd_param.dev_id         = SDMMC_DEV_ID;
    sd_param.clock_id       = RTE_SDC_CLOCK_SELECT;
    sd_param.bus_width      = RTE_SDC_BUS_WIDTH;
    sd_param.dma_mode       = RTE_SDC_DMA_SELECT;
    sd_param.app_callback   = sd_cb;

    if( p_SD_Driver->disk_initialize(&sd_param) ) {
        printf("SD initialization failed...\n");
        return;
    }

    for(int i=0x0; i<0x1000; i++){
        sdio_read_cia((uint8_t *)&sdbuffer[i], 0, i); //cccr
        printf("0x%x: 0x%x\n",i,sdbuffer[i]);
    }

error:
    return;

}

int main()
{
    uint32_t  service_error_code;
    uint32_t  error_code = SERVICES_REQ_SUCCESS;
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

    /* Initialize the SE services */
    se_services_port_init();

    /* Enable SDMMC Clocks */
    error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_CLK_100M, true, &service_error_code);
    if(error_code){
        printf("SE: SDMMC 100MHz clock enable = %d\n", error_code);
        return 0;
    }

    error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_USB, true, &service_error_code);
    if(error_code){
        printf("SE: SDMMC 20MHz clock enable = %d\n", error_code);
        return 0;
    }

    /* Enter the Baremetal demo Application.  */
    BareMetalSDIOTest();

    error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_CLK_100M, false, &service_error_code);
    if(error_code){
        printf("SE: SDMMC 100MHz clock disable = %d\n", error_code);
        return 0;
    }

    error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_USB, false, &service_error_code);
    if(error_code){
        printf("SE: SDMMC 20MHz clock disable = %d\n", error_code);
        return 0;
    }

    return 0;
}
