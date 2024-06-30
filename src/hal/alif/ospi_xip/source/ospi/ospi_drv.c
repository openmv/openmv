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
 * @file     ospi_drv.c
 * @version  V1.0.0
 * @brief    OSPI driver to set up flash in XIP mode
 * @bug      None.
 * @Note     None
 ******************************************************************************/
#include "clk.h"
#include "ospi_drv.h"
#include "ospi_xip_user.h"

/**
  \fn        static void ospi_xip_disable(ospi_flash_cfg_t *ospi_cfg)
  \brief     Disables XIP - Switch SSI Host controller from XiP mode to regular read-write mode
  \param[in] ospi_cfg : OSPI configuration structure
  \return    none
*/
static void ospi_xip_disable(ospi_flash_cfg_t *ospi_cfg)
{
    ospi_cfg->aes_regs->aes_control &= ~AES_CONTROL_XIP_EN;
}

/**
  \fn        static void ospi_xip_enable(ospi_flash_cfg_t *ospi_cfg)
  \brief     Enables XIP - Switch SSI Host controller to XiP mode
  \param[in] ospi_cfg : OSPI configuration structure
  \return    none
*/
static void ospi_xip_enable(ospi_flash_cfg_t *ospi_cfg)
{
    ospi_cfg->aes_regs->aes_control |= AES_CONTROL_XIP_EN;
#if OSPI_XIP_ENABLE_AES_DECRYPTION
    ospi_cfg->aes_regs->aes_control |= (AES_CONTROL_LD_KEY | AES_CONTROL_DECRYPT_EN);
#endif
}

/**
  \fn        bool ospi_xip_enabled(ospi_flash_cfg_t *ospi_cfg)
  \brief     Returns the status of XIP initialization
  \param[in] ospi_cfg : OSPI configuration structure
  \return    true or false
*/
bool ospi_xip_enabled(ospi_flash_cfg_t *ospi_cfg)
{
    return (ospi_cfg->aes_regs->aes_control & AES_CONTROL_XIP_EN) == AES_CONTROL_XIP_EN;
}

/**
  \fn        void ospi_setup_read(ospi_flash_cfg_t *ospi_cfg, uint32_t addr_len, uint32_t read_len, uint32_t wait_cycles)
  \brief     Set up for Flash read operation
  \param[in] ospi_cfg : OSPI configuration structure
  \param[in] addr_len : Address length
  \param[in] read_len : No. of bytes to be read
  \param[in] wait_cycles : Cycles required to read the data
  \return    none
*/
void ospi_setup_read(ospi_flash_cfg_t *ospi_cfg, uint32_t addr_len, uint32_t read_len, uint32_t wait_cycles)
{
    uint32_t val;

    ospi_writel(ospi_cfg, ser, 0);
    spi_disable(ospi_cfg);

    val = CTRLR0_IS_MST
        |(OCTAL << CTRLR0_SPI_FRF_OFFSET)
        |(TMOD_RO << CTRLR0_TMOD_OFFSET)
        |(CTRLR0_DFS_8bit << CTRLR0_DFS_OFFSET);

    ospi_writel(ospi_cfg, ctrlr0, val);
    ospi_writel(ospi_cfg, ctrlr1, read_len - 1);

    if (ospi_cfg->ddr_en)
    {
        val = TRANS_TYPE_FRF_DEFINED
            |((ospi_cfg->ddr_en) << CTRLR0_SPI_DDR_EN_OFFSET)
            |(CTRLR0_INST_L_8bit << CTRLR0_INST_L_OFFSET)
            |(addr_len << CTRLR0_ADDR_L_OFFSET)
            |(wait_cycles << CTRLR0_WAIT_CYCLES_OFFSET);
    }
    else
    {
        val = TRANS_TYPE_STANDARD
            |((ospi_cfg->ddr_en) << CTRLR0_SPI_DDR_EN_OFFSET)
            |(CTRLR0_INST_L_8bit << CTRLR0_INST_L_OFFSET)
            |(addr_len << CTRLR0_ADDR_L_OFFSET)
            |(wait_cycles << CTRLR0_WAIT_CYCLES_OFFSET);
    }

    ospi_writel(ospi_cfg, spi_ctrlr0, val);
    ospi_cfg->rx_req = read_len;
    spi_enable(ospi_cfg);
}

/**
  \fn        void ospi_setup_write_sdr(ospi_flash_cfg_t *ospi_cfg, uint32_t addr_len)
  \brief     Set up for Flash write operation
  \param[in] ospi_cfg : OSPI configuration structure
  \param[in] addr_len : Address length
  \return    none
*/
void ospi_setup_write_sdr(ospi_flash_cfg_t *ospi_cfg, uint32_t addr_len)
{
    uint32_t val;

    spi_disable(ospi_cfg);
    ospi_writel(ospi_cfg, ser, 0);

    val = CTRLR0_IS_MST
        |(SINGLE << CTRLR0_SPI_FRF_OFFSET)
        |(TMOD_TO << CTRLR0_TMOD_OFFSET)
        |(CTRLR0_DFS_8bit << CTRLR0_DFS_OFFSET);

    ospi_writel(ospi_cfg, ctrlr0, val);
    ospi_writel(ospi_cfg, ctrlr1, 0);

    val = TRANS_TYPE_FRF_DEFINED
        |(CTRLR0_INST_L_8bit << CTRLR0_INST_L_OFFSET)
        |(addr_len << CTRLR0_ADDR_L_OFFSET)
        |(0 << CTRLR0_WAIT_CYCLES_OFFSET);

    ospi_writel(ospi_cfg, spi_ctrlr0, val);
    spi_enable(ospi_cfg);
}

/**
  \fn        void ospi_setup_write(ospi_flash_cfg_t *ospi_cfg, uint32_t addr_len)
  \brief     Set up for Flash write operation
  \param[in] ospi_cfg : OSPI configuration structure
  \param[in] addr_len : Address length
  \return    none
*/
void ospi_setup_write(ospi_flash_cfg_t *ospi_cfg, uint32_t addr_len)
{
    uint32_t val;

    ospi_writel(ospi_cfg, ser, 0);
    spi_disable(ospi_cfg);

    val = CTRLR0_IS_MST
        |(OCTAL << CTRLR0_SPI_FRF_OFFSET)
        |(TMOD_TO << CTRLR0_TMOD_OFFSET)
        |(CTRLR0_DFS_8bit << CTRLR0_DFS_OFFSET);

    ospi_writel(ospi_cfg, ctrlr0, val);
    ospi_writel(ospi_cfg, ctrlr1, 0);

    if (ospi_cfg->ddr_en)
    {
        val = TRANS_TYPE_FRF_DEFINED
            |((ospi_cfg->ddr_en) << CTRLR0_SPI_DDR_EN_OFFSET)
            |(CTRLR0_INST_L_8bit << CTRLR0_INST_L_OFFSET)
            |(addr_len << CTRLR0_ADDR_L_OFFSET)
            |(0 << CTRLR0_WAIT_CYCLES_OFFSET);
    }
    else
    {
        val = TRANS_TYPE_STANDARD
            |(CTRLR0_INST_L_8bit << CTRLR0_INST_L_OFFSET)
            |(addr_len << CTRLR0_ADDR_L_OFFSET)
            |(0 << CTRLR0_WAIT_CYCLES_OFFSET);
    }

    ospi_writel(ospi_cfg, spi_ctrlr0, val);
    spi_enable(ospi_cfg);
}

/**
  \fn      void ospi_send_blocking(ospi_flash_cfg_t *ospi_cfg, uint32_t data)
  \brief   Send the last byte of data/command after sending address and remaining bytes of command/ data using ospi_push()
  \param[in] ospi_cfg : OSPI configuration structure
  \param[in] data : Last byte of data
  \return  none
*/
void ospi_send_blocking(ospi_flash_cfg_t *ospi_cfg, uint32_t data)
{
    ospi_writel(ospi_cfg, data_reg, data);
    ospi_writel(ospi_cfg, ser, ospi_cfg->ser);

    while ((ospi_readl(ospi_cfg, sr) & (SR_TF_EMPTY|SR_BUSY)) != SR_TF_EMPTY);
}

/**
  \fn        void ospi_push(ospi_flash_cfg_t *ospi_cfg, uint32_t data)
  \brief     Send the bytes of address/command/data except the last byte
  \param[in] ospi_cfg : OSPI configuration structure
  \param[in] data : address / command / data
  \return    none
*/
void ospi_push(ospi_flash_cfg_t *ospi_cfg, uint32_t data)
{
    ospi_writel(ospi_cfg, data_reg, data);
}

/**
  \fn        void ospi_recv_blocking(ospi_flash_cfg_t *ospi_cfg, uint32_t command)
  \brief     Send the command and read the data into RX buffer
  \param[in] ospi_cfg : OSPI configuration structure
  \param[in] command : Flash command
  \return    none
*/
void ospi_recv_blocking(ospi_flash_cfg_t *ospi_cfg, uint32_t command, uint8_t *buffer)
{
    uint32_t val;

    ospi_writel(ospi_cfg, data_reg, command);
    ospi_writel(ospi_cfg, ser, ospi_cfg->ser);

    ospi_cfg->rx_cnt = 0;

    while (ospi_cfg->rx_cnt < ospi_cfg->rx_req)
    {
        while(ospi_readl(ospi_cfg, rxflr) > 0)
        {
            val = ospi_readl(ospi_cfg, data_reg);
            *buffer++ = (uint8_t) val;
            ospi_cfg->rx_cnt++;
    	}
    }
}


/**
  \fn        void ospi_xip_enter(ospi_flash_cfg_t *ospi_cfg)
  \brief     To enter the XIP mode, if the Flash boots up in SDR / DDR mode.
  \param[in] ospi_cfg : OSPI configuration structure
  \return    none
*/
void ospi_xip_enter(ospi_flash_cfg_t *ospi_cfg, uint16_t incr_command, uint16_t wrap_command)
{
    uint32_t val;

    spi_disable(ospi_cfg);

    val = CTRLR0_IS_MST
        |(OCTAL << CTRLR0_SPI_FRF_OFFSET)
        |(0 << CTRLR0_SCPOL_OFFSET)
        |(0 << CTRLR0_SCPH_OFFSET)
        |(0 << CTRLR0_SSTE_OFFSET)
        |(TMOD_RO << CTRLR0_TMOD_OFFSET)
        |(CTRLR0_DFS_16bit << CTRLR0_DFS_OFFSET);

    ospi_writel(ospi_cfg, ctrlr0, val);

    val = (OCTAL << XIP_CTRL_FRF_OFFSET)
            | (0x2 << XIP_CTRL_TRANS_TYPE_OFFSET)
            | (ADDR_L32bit << XIP_CTRL_ADDR_L_OFFSET)
            | (INST_L8bit << XIP_CTRL_INST_L_OFFSET)
            | (0x0 << XIP_CTRL_MD_BITS_EN_OFFSET)
            | (ospi_cfg->wait_cycles << XIP_CTRL_WAIT_CYCLES_OFFSET)
            | (0x1 << XIP_CTRL_DFC_HC_OFFSET)
            | (0x1 << XIP_CTRL_DDR_EN_OFFSET)
            | (0x0 << XIP_CTRL_INST_DDR_EN_OFFSET)
            | (0x1 << XIP_CTRL_RXDS_EN_OFFSET)
            | (0x1 << XIP_CTRL_INST_EN_OFFSET)
            | (0x0 << XIP_CTRL_CONT_XFER_EN_OFFSET)
            | (0x0 << XIP_CTRL_HYPERBUS_EN_OFFSET)
            | (0x0 << XIP_CTRL_RXDS_SIG_EN)
            | (0x0 << XIP_CTRL_XIP_MBL_OFFSET)
            | (0x0 << XIP_PREFETCH_EN_OFFSET)
            | (0x0 << XIP_CTRL_RXDS_VL_EN_OFFSET);

    ospi_writel(ospi_cfg, xip_ctrl, val);

    ospi_writel(ospi_cfg, rx_sample_dly, 0);
    ospi_cfg->aes_regs->aes_rxds_delay = 11;

    ospi_writel(ospi_cfg, xip_mode_bits, 0x0);
    ospi_writel(ospi_cfg, xip_incr_inst, incr_command);
    ospi_writel(ospi_cfg, xip_wrap_inst, wrap_command);
    ospi_writel(ospi_cfg, xip_ser, ospi_cfg->ser);

    spi_enable(ospi_cfg);
    ospi_xip_enable(ospi_cfg);
}

/**
  \fn        void ospi_flash_exit_non_volatile_xip(ospi_flash_cfg_t *ospi_cfg, uint16_t incr_command, uint16_t wrap_command)
  \brief     If the Flash boots in XIP mode, this function will be called to exit the non volatile XIP
  \param[in] ospi_cfg : OSPI configuration structure
  \param[in] incr_command : Command to be used in INCR transaction
  \param[in] wrap_command : Command to be used in WRAP transaction
  \return    none
*/
void ospi_xip_exit(ospi_flash_cfg_t *ospi_cfg, uint16_t incr_command, uint16_t wrap_command)
{
    uint32_t val;

    spi_disable(ospi_cfg);

    val = CTRLR0_IS_MST
        |(OCTAL << CTRLR0_SPI_FRF_OFFSET)
        |(0 << CTRLR0_SCPOL_OFFSET)
        |(0 << CTRLR0_SCPH_OFFSET)
        |(0 << CTRLR0_SSTE_OFFSET)
        |(TMOD_RO << CTRLR0_TMOD_OFFSET)
        |(CTRLR0_DFS_32bit << CTRLR0_DFS_OFFSET);

    ospi_writel(ospi_cfg, ctrlr0, val);

    val = TRANS_TYPE_FRF_DEFINED
        |((ospi_cfg->ddr_en) << CTRLR0_SPI_DDR_EN_OFFSET)
        |(2 << CTRLR0_XIP_MBL_OFFSET)
        |(1 << CTRLR0_XIP_DFS_HC_OFFSET)
        |(1 << CTRLR0_XIP_INST_EN_OFFSET)
        |(CTRLR0_INST_L_8bit << CTRLR0_INST_L_OFFSET)
        |(ospi_cfg->addrlen) << (CTRLR0_ADDR_L_OFFSET)
        |(ospi_cfg->wait_cycles << CTRLR0_WAIT_CYCLES_OFFSET);

    ospi_writel(ospi_cfg, spi_ctrlr0, val);

    ospi_writel(ospi_cfg, xip_mode_bits, 0x1);
    ospi_writel(ospi_cfg, xip_incr_inst, incr_command);
    ospi_writel(ospi_cfg, xip_wrap_inst, wrap_command);
    ospi_writel(ospi_cfg, xip_ser, ospi_cfg->ser);
    ospi_writel(ospi_cfg, ser, ospi_cfg->ser);
    ospi_writel(ospi_cfg, xip_cnt_time_out, 100);

    spi_enable(ospi_cfg);

    ospi_xip_enable(ospi_cfg);
    ospi_xip_disable(ospi_cfg);
}

/**
  \fn        void ospi_init(ospi_flash_cfg_t *ospi_cfg)
  \brief     This function initializes the OSPI
  \param[in] ospi_cfg : OSPI configuration structure
  \return    none
*/
void ospi_init(ospi_flash_cfg_t *ospi_cfg)
{
    ospi_xip_disable(ospi_cfg);
    spi_disable(ospi_cfg);
    ospi_writel(ospi_cfg, ser, 0);
    ospi_writel(ospi_cfg, rx_sample_dly, OSPI_XIP_RX_SAMPLE_DELAY);
    ospi_writel(ospi_cfg, txd_drive_edge, OSPI_XIP_DDR_DRIVE_EDGE);
    ospi_cfg->aes_regs->aes_rxds_delay = OSPI_XIP_RXDS_DELAY;
    spi_set_clk(ospi_cfg, (GetSystemAXIClock() / ospi_cfg->ospi_clock));
    spi_enable(ospi_cfg);
}
