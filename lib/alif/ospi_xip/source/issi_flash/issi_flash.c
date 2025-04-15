/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     issi_flash.c
 * @version  V1.0.0
 * @brief    Flash driver to set up flash in XIP mode
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include "issi_flash_private.h"
#include "ospi_drv.h"
#include "ospi_xip_user.h"

static ospi_flash_cfg_t ospi_flash_config;

/**
  \fn         static void issi_write_enable(ospi_flash_cfg_t *ospi_cfg)
  \brief      Write enable for ISSI Flash before any write / program / erase
  \param[in]  ospi_cfg : OSPI configuration structure
  \return     none
 */
static void issi_write_enable(ospi_flash_cfg_t *ospi_cfg)
{
    /* Write WEL bit in OctalSPI mode */
    ospi_setup_write(ospi_cfg, ADDR_LENGTH_0_BITS);
    ospi_send_blocking(ospi_cfg, ISSI_WRITE_ENABLE);
}

/**
  \fn         static uint8_t issi_decode_id(ospi_flash_cfg_t *ospi_cfg, uint8_t *buffer)
  \brief      Decode the Device ID sent by the flash in single mode but read in octal mode.
  \param[in]  ospi_cfg : OSPI configuration structure
  \param[in]  buffer : ID read in SDR mode
  \return     ID of NOR flash
 */
static uint8_t issi_decode_id(ospi_flash_cfg_t *ospi_cfg, uint8_t *buffer)
{
    uint8_t iter, id = 0;

    for (iter = 0 ; iter < 8; iter++)
    {
        if (*buffer & 0x2)
        {
            id |= 1;
        }
        if (iter < 7)
        {
            id <<= 1;
        }
        buffer++;
    }

    ospi_cfg->device_id = id;

    return id;
}

/**
  \fn        uint8_t ospi_flash_ReadID(ospi_flash_cfg_t *ospi_cfg)
  \brief     This function reads the Device ID , if the ISSI NOR Flash boots up in SDR mode
  \param[in] ospi_cfg : OSPI configuration structure
  \return    Device ID of NOR Flash
 */
static uint8_t issi_flash_ReadID(ospi_flash_cfg_t *ospi_cfg)
{
    uint8_t buffer[8];

    ospi_setup_read(ospi_cfg, ADDR_LENGTH_0_BITS, 8, 0);
    ospi_recv_blocking(ospi_cfg, ISSI_READ_ID, buffer);

    return issi_decode_id(ospi_cfg, buffer);
}

/**
  \fn      static void issi_flash_set_configuration_register_SDR(ospi_flash_cfg_t *ospi_cfg, uint8_t cmd, uint8_t address, uint8_t value)
  \brief   This function sets the configuration register of the ISSI NOR Flash in SDR mode
  \param[in] ospi_cfg : OSPI configuration structure
  \param[in] cmd : Command
  \param[in] address : Address of register
  \param[in] value : Value to be set
  \return    none
 */
static void issi_flash_set_configuration_register_SDR(ospi_flash_cfg_t *ospi_cfg, uint8_t cmd, uint8_t address, uint8_t value)
{
    issi_write_enable(ospi_cfg);
    ospi_setup_write_sdr(ospi_cfg, ADDR_LENGTH_24_BITS);
    ospi_push(ospi_cfg, cmd);
    ospi_push(ospi_cfg, 0x00);
    ospi_push(ospi_cfg, 0x00);
    ospi_push(ospi_cfg, address);
    ospi_send_blocking(ospi_cfg, value);
}

/**
  \fn      static int issi_flash_probe (ospi_flash_cfg_t * ospi_cfg)
  \brief   Probe for an ISSI Flash device connected on the specified OSPI instance
  \param[in]  ospi_cfg : OSPI Flash Configuration
  \return     Success or Fail
 */
static int issi_flash_probe (ospi_flash_cfg_t *ospi_cfg)
{
    /* Initialize SPI in Single mode 1-1-1 and read Flash ID */
    if (issi_flash_ReadID(ospi_cfg) == DEVICE_ID_ISSI_FLASH_IS25WX256)
    {
        /* Set wrap configuration to 32 bytes */
        issi_flash_set_configuration_register_SDR(ospi_cfg, ISSI_WRITE_VOLATILE_CONFIG_REG, 0x07, WRAP_32_BYTE);

        /* Set the wait cycles needed for read operations */
        issi_flash_set_configuration_register_SDR(ospi_cfg, ISSI_WRITE_VOLATILE_CONFIG_REG, 0x01, ospi_cfg->wait_cycles);

        /* Switch the flash to Octal DDR mode */
        issi_flash_set_configuration_register_SDR(ospi_cfg, ISSI_WRITE_VOLATILE_CONFIG_REG, 0x00, OCTAL_DDR_DQS);

        return 0;
    }

    return -1;
}

/**
  \fn         static int flash_xip_init(ospi_flash_cfg_t *ospi_cfg)
  \brief      This function initializes the flash in XIP mode
  \param[in]  ospi_cfg : OSPI configuration structure
  \return     Success or Fail
 */
static int flash_xip_init(ospi_flash_cfg_t *ospi_cfg)
{
    ospi_xip_enter(ospi_cfg, ISSI_DDR_OCTAL_IO_FAST_READ, ISSI_DDR_OCTAL_IO_FAST_READ);
    return 0;
}


/**
  \fn         bool flash_xip_enabled(void)
  \brief      Return the status of xip initialization
  \param[in]  none
  \return     true or false
 */
bool flash_xip_enabled(void)
{
    ospi_flash_cfg_t *ospi_cfg = &ospi_flash_config;
#if OSPI_XIP_INSTANCE == OSPI0
    ospi_cfg->aes_regs = (aes_regs_t *) AES0_BASE;
#else
    ospi_cfg->aes_regs = (aes_regs_t *) AES1_BASE;
#endif
    return ospi_xip_enabled(ospi_cfg);
}

/**
  \fn         int setup_flash_xip(void)
  \brief      This function initializes the NOR Flash and OSPI and enters the XIP mode
  \param[in]  none
  \return     Success or Fail
 */
int setup_flash_xip(void)
{
    ospi_flash_cfg_t *ospi_cfg = &ospi_flash_config;

#if OSPI_XIP_INSTANCE == OSPI0
    ospi_cfg->regs = (ssi_regs_t *) OSPI0_BASE;
    ospi_cfg->aes_regs = (aes_regs_t *) AES0_BASE;
    ospi_cfg->xip_base = (volatile void *) OSPI0_XIP_BASE;
#else
    ospi_cfg->regs = (ssi_regs_t *) OSPI1_BASE;
    ospi_cfg->aes_regs = (aes_regs_t *) AES1_BASE;
    ospi_cfg->xip_base = (volatile void *) OSPI1_XIP_BASE;
#endif

    ospi_cfg->ser = 1;
    ospi_cfg->addrlen = ADDR_LENGTH_32_BITS;
    ospi_cfg->ospi_clock = OSPI_CLOCK;
    ospi_cfg->ddr_en = 0;
    ospi_cfg->wait_cycles = OSPI_XIP_FLASH_WAIT_CYCLES;

    ospi_init(ospi_cfg);

    if (issi_flash_probe(ospi_cfg))
    {
        return -1;
    }

    ospi_cfg->ddr_en = 1;

    if (flash_xip_init(ospi_cfg))
    {
        return -1;
    }

    return 0;
}
