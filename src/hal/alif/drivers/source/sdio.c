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
 * @file     sdio.c
 * @author   Deepak Kumar
 * @email    deepak@alifsemi.com
 * @version  V0.0.1
 * @date     28-Nov-2022
 * @brief    SDIO Driver APIs.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "sd_host.h"
#include "sd.h"
#include "sdio.h"

extern sd_handle_t Hsd;

/**
  \fn           SD_DRV_STATUS sdio_read_cia(uint8_t *pcia, uint8_t fn, uint8_t offset)
  \brief        read SDIO Common I/O Area with given offset
  \param[in]    pointer to Common I/O Area
  \param[in]    Function number(0-7)
  \param[in]    Function offset(0x00 - 0xFF)
  \return       SD driver status
  */
SD_DRV_STATUS sdio_read_cia(uint8_t *pcia, uint8_t fn, uint8_t offset){
    sd_handle_t *pHsd =  &Hsd;
    if(hc_io_rw_direct(pHsd, 0, fn, offset, 0, pcia) != SDMMC_HC_STATUS_OK)
        return SD_DRV_STATUS_RD_ERR;
    return SD_DRV_STATUS_OK;
}

/**
  \fn           SD_DRV_STATUS sdio_write_cia(uint8_t cia, uint8_t fn, uint8_t offset)
  \brief        write SDIO Common I/O Area with given offset
  \param[in]    pointer to Common I/O Area
  \param[in]    Function number(0-7)
  \param[in]    Function offset(0x00 - 0xFF)
  \return       SD driver status
  */
SD_DRV_STATUS sdio_write_cia(uint8_t cia, uint8_t fn, uint8_t offset){
    sd_handle_t *pHsd =  &Hsd;
    if(hc_io_rw_direct(pHsd, 1, fn, offset, cia, 0) != SDMMC_HC_STATUS_OK)
        return SD_DRV_STATUS_RD_ERR;
    return SD_DRV_STATUS_OK;
}

/**
  \fn           SD_DRV_STATUS sdio_read_cccr(uint8_t *pcccr)
  \brief        read SDIO Common I/O Area with given offset
  \param[in]    pointer to Common I/O Area
  \param[in]    Function number(0-7)
  \param[in]    Function offset(0x00 - 0xFF)
  \return       SD driver status
  */
SD_DRV_STATUS sdio_read_cccr(uint8_t *pcccr){
    sd_handle_t *pHsd =  &Hsd;
    if(hc_io_rw_direct(pHsd, 0, 0, 0, 0, pcccr) != SDMMC_HC_STATUS_OK)
        return SD_DRV_STATUS_RD_ERR;
    return SD_DRV_STATUS_OK;
}

/**
  \fn           SD_DRV_STATUS sdio_write_cccr(uint8_t cccr)
  \brief        write SDIO Card control register
  \param[in]    cccr to be written
  \return       SD driver status
  */
SD_DRV_STATUS sdio_write_cccr(uint8_t cccr){
    sd_handle_t *pHsd =  &Hsd;
    if(hc_io_rw_direct(pHsd, 1, 0, 0, cccr, 0) != SDMMC_HC_STATUS_OK)
        return SD_DRV_STATUS_RD_ERR;
    return SD_DRV_STATUS_OK;
}

