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
 * @file     ospi_hyperram_xip.c
 * @author   Silesh C V
 * @email    silesh@alifsemi.com
 * @version  V1.0.0
 * @date     19-Jul-2023
 * @brief    Implementation of the OSPI hyperram XIP init library.
 ******************************************************************************/

#include <stdint.h>
#include <stddef.h>

#include "ospi_hyperram_xip.h"
#include "ospi.h"
#include "sys_ctrl_aes.h"
#include "global_map.h"
#include "clk.h"

/**
  \fn          int ospi_hyperram_xip_init(const ospi_hyperram_xip_config *config)
  \brief       Initialize OSPI Hyerbus xip configuration. After a successful return
               from this function, the OSPI XIP region (for the OSPI instance specified
               in the ospi_hyerram_xip_config input parameter) will be active and can be
               used to directly read/write the memory area provided by the hyperram device.
  \param[in]   config    Pointer to hyperram configuration information
  \return      -1 on configuration error, 0 on success
*/
int ospi_hyperram_xip_init(const ospi_hyperram_xip_config *config)
{
    OSPI_Type *ospi;
    AES_Type *aes;
    uint32_t clk = GetSystemAXIClock();

    if (config == NULL || config->instance < OSPI_INSTANCE_0 || config->instance > OSPI_INSTANCE_1)
    {
        return -1;
    }

    /* Setup the OSPI/AES register map pointers based on the OSPI instance */
    if (config->instance == OSPI_INSTANCE_0)
    {
        ospi = (OSPI_Type *) OSPI0_BASE;
        aes  = (AES_Type *) AES0_BASE;
    }
    else
    {
        ospi = (OSPI_Type *) OSPI1_BASE;
        aes  = (AES_Type *) AES1_BASE;
    }

    ospi_set_ddr_drive_edge(ospi, config->ddr_drive_edge);

    ospi_set_bus_speed(ospi, config->bus_speed, clk);

    aes_set_rxds_delay(aes, config->rxds_delay);

    /* If the user has provided a function pointer to initialize the hyperram, call it */
    if (config->hyperram_init)
    {
        config->hyperram_init();
    }

    /* Initialize OSPI hyperbus xip configuration */
    ospi_hyperbus_xip_init(ospi, config->wait_cycles);

    ospi_control_xip_ss(ospi, config->slave_select, SPI_SS_STATE_ENABLE);

    aes_enable_xip(aes);

    return 0;
}
