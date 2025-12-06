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
 * @file     ospi_hyperram_xip.h
 * @author   Silesh C V
 * @email    silesh@alifsemi.com
 * @version  V1.0.0
 * @date     19-Jul-2023
 * @brief    Public header file for OSPI hyperram XIP init library.
 ******************************************************************************/

#ifndef OSPI_HYPERRAM_XIP_H
#define OSPI_HYPERRAM_XIP_H

#include <stdint.h>

#ifdef  __cplusplus
extern "C"
{
#endif

/**
 * enum OSPI_INSTANCE.
 * OSPI instances.
 */
typedef enum _OSPI_INSTANCE {
    OSPI_INSTANCE_0,
    OSPI_INSTANCE_1,
} OSPI_INSTANCE;

typedef struct _ospi_hyperram_xip_config {
    /**< The OSPI instance to be setup in hyperram XIP mode */
    OSPI_INSTANCE instance;

    /**< OSPI bus speed */
    uint32_t bus_speed;

    /**< Optional device specific initialization needed by the hyperram device  */
    void (*hyperram_init)(void);

    /**< Drive edge configuration for the OSPI */
    uint8_t ddr_drive_edge;

    /**< Delay applied to the OSPI RXDS signal */
    uint8_t rxds_delay;

    /**< Wait cycles needed by the hyperram device */
    uint8_t wait_cycles;

    /**< Slave select (Chip select) line used for the hyperram device */
    uint8_t slave_select;

    /**< Data Frame Size used for the hyperram device */
    uint8_t dfs;
} ospi_hyperram_xip_config;

/**
  \fn          int ospi_hyperram_xip_init(const ospi_hyperram_xip_config *config)
  \brief       Initialize OSPI Hyerbus xip configuration. After a successful return
               from this function, the OSPI XIP region (for the OSPI instance specified
               in the ospi_hyerram_xip_config input parameter) will be active and can be
               used to directly read/write the memory area provided by the hyperram device.
  \param[in]   config    Pointer to hyperram configuration information
  \return      -1 on configuration error, 0 on success
*/
int ospi_hyperram_xip_init(const ospi_hyperram_xip_config *config);
#ifdef  __cplusplus
}
#endif
#endif /* OSPI_HYPERRAM_XIP_H */

