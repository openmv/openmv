/*******************************************************************************
  This module contains WINC1500 SPI protocol bus APIs implementation.

  File Name:
    nmspi.c

  Summary:
    This module contains WINC1500 SPI protocol bus APIs implementation.

  Description:
    This module contains WINC1500 SPI protocol bus APIs implementation.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2022, Microchip Technology Inc., and its subsidiaries. All rights reserved.

The software and documentation is provided by microchip and its contributors
"as is" and any express, implied or statutory warranties, including, but not
limited to, the implied warranties of merchantability, fitness for a particular
purpose and non-infringement of third party intellectual property rights are
disclaimed to the fullest extent permitted by law. In no event shall microchip
or its contributors be liable for any direct, indirect, incidental, special,
exemplary, or consequential damages (including, but not limited to, procurement
of substitute goods or services; loss of use, data, or profits; or business
interruption) however caused and on any theory of liability, whether in contract,
strict liability, or tort (including negligence or otherwise) arising in any way
out of the use of the software and documentation, even if advised of the
possibility of such damage.

Except as expressly permitted hereunder and subject to the applicable license terms
for any third-party software incorporated in the software and any applicable open
source software license terms, no license or other rights, whether express or
implied, are granted under any patent or other intellectual property rights of
Microchip or any third party.
*/

#include "osal/osal.h"
#include "nm_common.h"

#include "nmspi.h"
#include "nmasic.h"
#include "wdrv_winc_common.h"
#include "wdrv_winc_spi.h"

#define NMI_PERIPH_REG_BASE 0x1000
#define NMI_INTR_REG_BASE (NMI_PERIPH_REG_BASE+0xa00)
#define NMI_CHIPID  (NMI_PERIPH_REG_BASE)
#define NMI_PIN_MUX_0 (NMI_PERIPH_REG_BASE + 0x408)
#define NMI_INTR_ENABLE (NMI_INTR_REG_BASE)

#define NMI_SPI_REG_BASE 0xe800
#define NMI_SPI_CTL (NMI_SPI_REG_BASE)
#define NMI_SPI_MASTER_DMA_ADDR (NMI_SPI_REG_BASE+0x4)
#define NMI_SPI_MASTER_DMA_COUNT (NMI_SPI_REG_BASE+0x8)
#define NMI_SPI_SLAVE_DMA_ADDR (NMI_SPI_REG_BASE+0xc)
#define NMI_SPI_SLAVE_DMA_COUNT (NMI_SPI_REG_BASE+0x10)
#define NMI_SPI_TX_MODE (NMI_SPI_REG_BASE+0x20)
#define NMI_SPI_PROTOCOL_CONFIG (NMI_SPI_REG_BASE+0x24)
#define NMI_SPI_INTR_CTL (NMI_SPI_REG_BASE+0x2c)
#define NMI_SPI_MISC_CTRL (NMI_SPI_REG_BASE+0x48)

#define NMI_SPI_PROTOCOL_OFFSET (NMI_SPI_PROTOCOL_CONFIG-NMI_SPI_REG_BASE)

#define SPI_BASE                NMI_SPI_REG_BASE

//#define CMD_DMA_WRITE           0xc1
//#define CMD_DMA_READ            0xc2
#define CMD_INTERNAL_WRITE      0xc3
#define CMD_INTERNAL_READ       0xc4
//#define CMD_TERMINATE           0xc5
//#define CMD_REPEAT              0xc6
#define CMD_DMA_EXT_WRITE       0xc7
#define CMD_DMA_EXT_READ        0xc8
#define CMD_SINGLE_WRITE        0xc9
#define CMD_SINGLE_READ         0xca
#define CMD_RESET               0xcf

#define N_OK                     0
#define N_FAIL                  -1
#define N_RESET                 -2
#define N_RETRY                 -3

#define SPI_RESP_RETRY_COUNT    (10)
#define SPI_RETRY_COUNT         (10)
#define DATA_PKT_SZ_256         256
#define DATA_PKT_SZ_512         512
#define DATA_PKT_SZ_1K          1024
#define DATA_PKT_SZ_4K          (4 * 1024)
#define DATA_PKT_SZ_8K          (8 * 1024)
#define DATA_PKT_SZ             DATA_PKT_SZ_8K

static uint8_t gu8Crc_off = 0;

static OSAL_MUTEX_HANDLE_TYPE s_spiLock;

static inline int8_t spi_read(uint8_t *b, uint16_t sz)
{
    if (true == WDRV_WINC_SPIReceive((unsigned char *const) b, sz))
    {
        return N_OK;
    }

    return N_FAIL;
}

static inline int8_t spi_write(uint8_t *b, uint16_t sz)
{
    if (true == WDRV_WINC_SPISend((unsigned char *const) b, sz))
    {
        return N_OK;
    }

    return N_FAIL;
}

/********************************************

    Crc7

********************************************/

static const uint8_t crc7_syndrome_table[256] = {
    0x00, 0x09, 0x12, 0x1b, 0x24, 0x2d, 0x36, 0x3f,
    0x48, 0x41, 0x5a, 0x53, 0x6c, 0x65, 0x7e, 0x77,
    0x19, 0x10, 0x0b, 0x02, 0x3d, 0x34, 0x2f, 0x26,
    0x51, 0x58, 0x43, 0x4a, 0x75, 0x7c, 0x67, 0x6e,
    0x32, 0x3b, 0x20, 0x29, 0x16, 0x1f, 0x04, 0x0d,
    0x7a, 0x73, 0x68, 0x61, 0x5e, 0x57, 0x4c, 0x45,
    0x2b, 0x22, 0x39, 0x30, 0x0f, 0x06, 0x1d, 0x14,
    0x63, 0x6a, 0x71, 0x78, 0x47, 0x4e, 0x55, 0x5c,
    0x64, 0x6d, 0x76, 0x7f, 0x40, 0x49, 0x52, 0x5b,
    0x2c, 0x25, 0x3e, 0x37, 0x08, 0x01, 0x1a, 0x13,
    0x7d, 0x74, 0x6f, 0x66, 0x59, 0x50, 0x4b, 0x42,
    0x35, 0x3c, 0x27, 0x2e, 0x11, 0x18, 0x03, 0x0a,
    0x56, 0x5f, 0x44, 0x4d, 0x72, 0x7b, 0x60, 0x69,
    0x1e, 0x17, 0x0c, 0x05, 0x3a, 0x33, 0x28, 0x21,
    0x4f, 0x46, 0x5d, 0x54, 0x6b, 0x62, 0x79, 0x70,
    0x07, 0x0e, 0x15, 0x1c, 0x23, 0x2a, 0x31, 0x38,
    0x41, 0x48, 0x53, 0x5a, 0x65, 0x6c, 0x77, 0x7e,
    0x09, 0x00, 0x1b, 0x12, 0x2d, 0x24, 0x3f, 0x36,
    0x58, 0x51, 0x4a, 0x43, 0x7c, 0x75, 0x6e, 0x67,
    0x10, 0x19, 0x02, 0x0b, 0x34, 0x3d, 0x26, 0x2f,
    0x73, 0x7a, 0x61, 0x68, 0x57, 0x5e, 0x45, 0x4c,
    0x3b, 0x32, 0x29, 0x20, 0x1f, 0x16, 0x0d, 0x04,
    0x6a, 0x63, 0x78, 0x71, 0x4e, 0x47, 0x5c, 0x55,
    0x22, 0x2b, 0x30, 0x39, 0x06, 0x0f, 0x14, 0x1d,
    0x25, 0x2c, 0x37, 0x3e, 0x01, 0x08, 0x13, 0x1a,
    0x6d, 0x64, 0x7f, 0x76, 0x49, 0x40, 0x5b, 0x52,
    0x3c, 0x35, 0x2e, 0x27, 0x18, 0x11, 0x0a, 0x03,
    0x74, 0x7d, 0x66, 0x6f, 0x50, 0x59, 0x42, 0x4b,
    0x17, 0x1e, 0x05, 0x0c, 0x33, 0x3a, 0x21, 0x28,
    0x5f, 0x56, 0x4d, 0x44, 0x7b, 0x72, 0x69, 0x60,
    0x0e, 0x07, 0x1c, 0x15, 0x2a, 0x23, 0x38, 0x31,
    0x46, 0x4f, 0x54, 0x5d, 0x62, 0x6b, 0x70, 0x79
};

static uint8_t crc7_byte(uint8_t crc, uint8_t data)
{
    return crc7_syndrome_table[(crc << 1) ^ data];
}

static uint8_t crc7(uint8_t crc, const uint8_t *buffer, uint32_t len)
{
    while (len--)
    {
        crc = crc7_byte(crc, *buffer++);
    }
    return crc;
}

/********************************************

    Spi protocol Function

********************************************/

static int8_t spi_cmd(uint8_t cmd, uint32_t adr, uint32_t u32data, uint32_t sz, uint8_t clockless)
{
    uint8_t bc[9];
    uint8_t len = 5;

    bc[0] = cmd;
    switch (cmd)
    {
        case CMD_SINGLE_READ:               /* single word (4 bytes) read */
            bc[1] = (uint8_t)(adr >> 16);
            bc[2] = (uint8_t)(adr >> 8);
            bc[3] = (uint8_t)adr;
            len = 5;
            break;

        case CMD_INTERNAL_READ:         /* internal register read */
            bc[1] = (uint8_t)(adr >> 8);
            if(clockless)
            {
                bc[1] |= (1 << 7);
            }
            bc[2] = (uint8_t)adr;
            bc[3] = 0x00;
            len = 5;
            break;
#ifdef CMD_TERMINATE
        case CMD_TERMINATE:                 /* termination */
            bc[1] = 0x00;
            bc[2] = 0x00;
            bc[3] = 0x00;
            len = 5;
            break;
#endif
#ifdef CMD_REPEAT
        case CMD_REPEAT:                        /* repeat */
            bc[1] = 0x00;
            bc[2] = 0x00;
            bc[3] = 0x00;
            len = 5;
            break;
#endif
        case CMD_RESET:                         /* reset */
            bc[1] = 0xff;
            bc[2] = 0xff;
            bc[3] = 0xff;
            len = 5;
            break;
#if defined(CMD_DMA_WRITE) || defined(CMD_DMA_READ)
        case CMD_DMA_WRITE:                 /* dma write */
        case CMD_DMA_READ:                  /* dma read */
            bc[1] = (uint8_t)(adr >> 16);
            bc[2] = (uint8_t)(adr >> 8);
            bc[3] = (uint8_t)adr;
            bc[4] = (uint8_t)(sz >> 8);
            bc[5] = (uint8_t)(sz);
            len = 7;
            break;
#endif
        case CMD_DMA_EXT_WRITE:     /* dma extended write */
        case CMD_DMA_EXT_READ:          /* dma extended read */
            bc[1] = (uint8_t)(adr >> 16);
            bc[2] = (uint8_t)(adr >> 8);
            bc[3] = (uint8_t)adr;
            bc[4] = (uint8_t)(sz >> 16);
            bc[5] = (uint8_t)(sz >> 8);
            bc[6] = (uint8_t)(sz);
            len = 8;
            break;

        case CMD_INTERNAL_WRITE:        /* internal register write */
            bc[1] = (uint8_t)(adr >> 8);
            if(clockless)
            {
                bc[1] |= (1 << 7);
            }
            bc[2] = (uint8_t)(adr);
            bc[3] = (uint8_t)(u32data >> 24);
            bc[4] = (uint8_t)(u32data >> 16);
            bc[5] = (uint8_t)(u32data >> 8);
            bc[6] = (uint8_t)(u32data);
            len = 8;
            break;

        case CMD_SINGLE_WRITE:          /* single word write */
            bc[1] = (uint8_t)(adr >> 16);
            bc[2] = (uint8_t)(adr >> 8);
            bc[3] = (uint8_t)(adr);
            bc[4] = (uint8_t)(u32data >> 24);
            bc[5] = (uint8_t)(u32data >> 16);
            bc[6] = (uint8_t)(u32data >> 8);
            bc[7] = (uint8_t)(u32data);
            len = 9;
            break;

        default:
            return N_FAIL;
    }

    if (gu8Crc_off == 0)
    {
        bc[len-1] = (crc7(0x7f, (const uint8_t *)&bc[0], len-1)) << 1;
    }
    else
    {
        len -= 1;
    }

    if (N_OK != spi_write(bc, len))
    {
        M2M_ERR("[spi_cmd]: Failed cmd write, bus error...\r\n");
        return N_FAIL;
    }

    return N_OK;
}

static int8_t spi_cmd_rsp(uint8_t cmd, uint8_t clockless)
{
    uint8_t rsp;
    int8_t s8RetryCnt;

    /**
        Command/Control response
    **/
    if ((cmd == CMD_RESET)
#ifdef CMD_TERMINATE
        || (cmd == CMD_TERMINATE)
#endif
#ifdef CMD_REPEAT
        || (cmd == CMD_REPEAT)
#endif
        )
    {
        if (N_OK != spi_read(&rsp, 1))
        {
            return N_FAIL;
        }
    }

    /* wait for response */
    s8RetryCnt = SPI_RESP_RETRY_COUNT;
    do
    {
        if (N_OK != spi_read(&rsp, 1))
        {
            M2M_ERR("[spi_cmd_rsp]: Failed cmd response read, bus error...\r\n");
            return N_FAIL;
        }
    }
    while((rsp != cmd) && (!clockless) && (s8RetryCnt-- > 0));

    if (s8RetryCnt < 0)
    {
        M2M_ERR("[spi_cmd_rsp]: Failed cmd response read\n");
        return N_FAIL;
    }
    /**
        State response
    **/
    /* wait for response */
    s8RetryCnt = SPI_RESP_RETRY_COUNT;
    do
    {
        if (N_OK != spi_read(&rsp, 1))
        {
            M2M_ERR("[spi_cmd_rsp]: Failed cmd response read, bus error...\r\n");
            return N_FAIL;
        }
    }
    while((rsp != 0x00) && (!clockless) && (s8RetryCnt-- > 0));

    if (s8RetryCnt < 0)
    {
        M2M_ERR("[spi_cmd_rsp]: Failed cmd response read\n");
        return N_FAIL;
    }

    return N_OK;
}

static void spi_reset(void)
{
    nm_sleep(1);
    (void)spi_cmd(CMD_RESET, 0, 0, 0, 0);
    (void)spi_cmd_rsp(CMD_RESET, 0);
    nm_sleep(1);
}

/********************************************

    Spi Internal Read/Write Function

********************************************/

static int8_t spi_data_read(uint8_t *b, uint16_t sz,uint8_t clockless)
{
    int16_t retry, ix, nbytes;
    int8_t result = N_OK;
    uint8_t crc[2];
    uint8_t rsp;

    /**
        Data
    **/
    ix = 0;
    do {
        if (sz <= DATA_PKT_SZ)
        {
            nbytes = sz;
        }
        else
        {
            nbytes = DATA_PKT_SZ;
        }

        /**
            Data Response header
        **/
        retry = SPI_RESP_RETRY_COUNT;
        do
        {
            if (N_OK != spi_read(&rsp, 1))
            {
                M2M_ERR("[spi_data_read]: Failed data response read, bus error...\r\n");
                result = N_FAIL;
                break;
            }
            if ((rsp & 0xf0) == 0xf0)
            {
                break;
            }
        }
        while (retry--);

        if (result == N_FAIL)
        {
            break;
        }

        if ((retry <= 0) && (!clockless))
        {
            M2M_ERR("[spi_data_read]: Failed data response read...(%02x)\r\n", rsp);
            result = N_FAIL;
            break;
        }

        /**
            Read bytes
        **/
        if (N_OK != spi_read(&b[ix], nbytes))
        {
            M2M_ERR("[spi_data_read]: Failed data block read, bus error...\r\n");
            result = N_FAIL;
            break;
        }
        if(!clockless)
        {
            /**
            Read Crc
            **/
            if (gu8Crc_off == 0)
            {
                if (N_OK != spi_read(crc, 2))
                {
                    M2M_ERR("[spi_data_read]: Failed data block CRC read, bus error...\r\n");
                    result = N_FAIL;
                    break;
                }
            }
        }
        ix += nbytes;
        sz -= nbytes;

    } while (sz);

    return result;
}

static int8_t spi_data_write(uint8_t *b, uint16_t sz)
{
    int16_t ix = 0;
    uint16_t nbytes;
    int8_t result = N_OK;
    uint8_t cmd, order, crc[2] = {0};
    //uint8_t rsp;

    /**
        Data
    **/
    do
    {
        if (sz <= DATA_PKT_SZ)
        {
            nbytes = sz;
        }
        else
        {
            nbytes = DATA_PKT_SZ;
        }

        /**
            Write command
        **/
        cmd = 0xf0;
        if (ix == 0)
        {
            if (sz <= DATA_PKT_SZ)
            {
                order = 0x3;
            }
            else
            {
                order = 0x1;
            }
        }
        else
        {
            if (sz <= DATA_PKT_SZ)
            {
                order = 0x3;
            }
            else
            {
                order = 0x2;
            }
        }

        cmd |= order;
        if (N_OK != spi_write(&cmd, 1))
        {
            M2M_ERR("[spi_data_write]: Failed data block cmd write, bus error...\r\n");
            result = N_FAIL;
            break;
        }

        /**
            Write data
        **/
        if (N_OK != spi_write(&b[ix], nbytes))
        {
            M2M_ERR("[spi_data_write]: Failed data block write, bus error...\r\n");
            result = N_FAIL;
            break;
        }

        /**
            Write Crc
        **/
        if (gu8Crc_off == 0)
        {
            if (N_OK != spi_write(crc, 2))
            {
                M2M_ERR("[spi_data_write]: Failed data block CRC write, bus error...\r\n");
                result = N_FAIL;
                break;
            }
        }

        ix += nbytes;
        sz -= nbytes;
    }
    while (sz);

    return result;
}

/********************************************

    Spi interfaces

********************************************/

static int8_t spi_write_reg(uint32_t u32Addr, uint32_t u32Val)
{
    uint8_t cmd = CMD_SINGLE_WRITE;
    uint8_t clockless = 0;

    if (u32Addr <= 0x30)
    {
        /**
        NMC1000 clockless registers.
        **/
        cmd = CMD_INTERNAL_WRITE;
        clockless = 1;
    }

    if (spi_cmd(cmd, u32Addr, u32Val, 4, clockless) != N_OK)
    {
        M2M_ERR("[spi_write_reg]: Failed cmd, write reg (%08" PRIx32 ")...\r\n", u32Addr);
        return N_FAIL;
    }

    if (rNMI_GLB_RESET == u32Addr)
    {
        return N_OK;
    }

    if (spi_cmd_rsp(cmd, clockless) != N_OK)
    {
        M2M_ERR("[spi_write_reg]: Failed cmd response, write reg (%08" PRIx32 ")...\r\n", u32Addr);
        return N_FAIL;
    }

    return N_OK;
}

static int8_t spi_write_block(uint32_t u32Addr, uint8_t *puBuf, uint16_t u16Sz)
{
    uint8_t len;
    uint8_t rsp[3];

    /**
        Command
    **/
    if (spi_cmd(CMD_DMA_EXT_WRITE, u32Addr, 0, u16Sz, 0) != N_OK)
    {
        M2M_ERR("[spi_write_block]: Failed cmd, write block (%08" PRIx32 ")...\r\n", u32Addr);
        return N_FAIL;
    }

    if (spi_cmd_rsp(CMD_DMA_EXT_WRITE, 0) != N_OK)
    {
        M2M_ERR("[spi_write_block]: Failed cmd response, write block (%08" PRIx32 ")...\r\n", u32Addr);
        return N_FAIL;
    }

    /**
        Data
    **/
    if (spi_data_write(puBuf, u16Sz) != N_OK)
    {
        M2M_ERR("[spi_write_block]: Failed block data write...\r\n");
        return N_FAIL;
    }
    /**
        Data RESP
    **/
    if (gu8Crc_off == 0)
    {
        len = 2;
    }
    else
    {
        len = 3;
    }

    if (N_OK != spi_read(&rsp[0], len))
    {
        M2M_ERR("[spi_write_block]: Failed bus error...\r\n");
        return N_FAIL;
    }

    if((rsp[len-1] != 0) || (rsp[len-2] != 0xC3))
    {
        M2M_ERR("[spi_write_block]: Failed data response read, %x %x %x\r\n", rsp[0], rsp[1], rsp[2]);
        return N_FAIL;
    }

    return N_OK;
}

static int8_t spi_read_reg(uint32_t u32Addr, uint32_t* pu32RetVal)
{
    uint8_t cmd = CMD_SINGLE_READ;
    uint8_t tmp[4];
    uint8_t clockless = 0;

    if (u32Addr <= 0xff)
    {
        /**
        NMC1000 clockless registers.
        **/
        cmd = CMD_INTERNAL_READ;
        clockless = 1;
    }

    if (spi_cmd(cmd, u32Addr, 0, 4, clockless) != N_OK)
    {
        M2M_ERR("[spi_read_reg]: Failed cmd, read reg (%08" PRIx32 ")...\r\n", u32Addr);
        return N_FAIL;
    }

    if (spi_cmd_rsp(cmd, clockless) != N_OK)
    {
        M2M_ERR("[spi_read_reg]: Failed cmd response, read reg (%08" PRIx32 ")...\r\n", u32Addr);
        return N_FAIL;
    }

    /* to avoid endianess issues */
    if (spi_data_read(&tmp[0], 4, clockless) != N_OK)
    {
        M2M_ERR("[spi_read_reg]: Failed data read...\r\n");
        return N_FAIL;
    }

    *pu32RetVal = ((uint32_t)tmp[0])       |
                  ((uint32_t)tmp[1] << 8)  |
                  ((uint32_t)tmp[2] << 16) |
                  ((uint32_t)tmp[3] << 24);

    return N_OK;
}

static int8_t spi_read_block(uint32_t u32Addr, uint8_t *puBuf, uint16_t u16Sz)
{
    /**
        Command
    **/
    if (spi_cmd(CMD_DMA_EXT_READ, u32Addr, 0, u16Sz, 0) != N_OK)
    {
        M2M_ERR("[spi_read_block]: Failed cmd, read block (%08" PRIx32 ")...\r\n", u32Addr);
        return N_FAIL;
    }

    if (spi_cmd_rsp(CMD_DMA_EXT_READ, 0) != N_OK)
    {
        M2M_ERR("[spi_read_block]: Failed cmd response, read block (%08" PRIx32 ")...\r\n", u32Addr);
        return N_FAIL;
    }

    /**
        Data
    **/
    if (spi_data_read(puBuf, u16Sz, 0) != N_OK)
    {
        M2M_ERR("[spi_read_block]: Failed block data read...\r\n");
        return N_FAIL;
    }

    return N_OK;
}

static void spi_init_pkt_sz(void)
{
    uint32_t val32;

    /* Make sure SPI max. packet size fits the defined DATA_PKT_SZ.  */
    val32 = nm_spi_read_reg(SPI_BASE+0x24);
    val32 &= ~(0x7 << 4);
    switch(DATA_PKT_SZ)
    {
        case 256:  val32 |= (0 << 4); break;
        case 512:  val32 |= (1 << 4); break;
        case 1024: val32 |= (2 << 4); break;
        case 2048: val32 |= (3 << 4); break;
        case 4096: val32 |= (4 << 4); break;
        case 8192: val32 |= (5 << 4); break;
        default: return;
    }
    (void)nm_spi_write_reg(SPI_BASE+0x24, val32);
}

/********************************************

    Bus interfaces

********************************************/

int8_t nm_spi_reset(void)
{
    if (OSAL_RESULT_TRUE != OSAL_MUTEX_Lock(&s_spiLock, OSAL_WAIT_FOREVER))
    {
        return M2M_ERR_BUS_FAIL;
    }

    (void)spi_cmd(CMD_RESET, 0, 0, 0, 0);
    (void)spi_cmd_rsp(CMD_RESET, 0);

    (void)OSAL_MUTEX_Unlock(&s_spiLock);

    return M2M_SUCCESS;
}

void nm_spi_lock_init(void)
{
    OSAL_MUTEX_Create(&s_spiLock);
}

/*
*   @fn     nm_spi_init
*   @brief  Initialize the SPI
*   @return M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*   @author M. Abdelmawla
*   @date   11 July 2012
*   @version    1.0
*/
int8_t nm_spi_init(void)
{
    uint32_t chipid;
    uint32_t reg = 0;

    /**
        configure protocol
    **/
    gu8Crc_off = 0;

    if (nm_spi_read_reg_with_ret(NMI_SPI_PROTOCOL_CONFIG, &reg) != M2M_SUCCESS)
    {
        /* Read failed. Try with CRC off. This might happen when module
        is removed but chip isn't reset*/
        gu8Crc_off = 1;
        M2M_ERR("[nm_spi_init]: Failed internal read protocol with CRC on, retrying with CRC off...\r\n");

        if (nm_spi_read_reg_with_ret(NMI_SPI_PROTOCOL_CONFIG, &reg) != M2M_SUCCESS)
        {
            // Reaad failed with both CRC on and off, something went bad
            M2M_ERR("[nm_spi_init]: Failed internal read protocol...\r\n");

            return M2M_ERR_BUS_FAIL;
        }
    }
    if(gu8Crc_off == 0)
    {
        reg &= ~0xc;    /* disable CRC checking */
        reg &= ~0x70;
        reg |= (0x5 << 4);

        if (nm_spi_write_reg(NMI_SPI_PROTOCOL_CONFIG, reg) != M2M_SUCCESS)
        {
            M2M_ERR("[nm_spi_init]: Failed internal write protocol reg...\r\n");

            return M2M_ERR_BUS_FAIL;
        }

        gu8Crc_off = 1;
    }

    /**
        make sure can read back chip id correctly
    **/
    if (nm_spi_read_reg_with_ret(0x1000, &chipid) != M2M_SUCCESS)
    {
        M2M_ERR("[nm_spi_init]: Fail cmd read chip id...\r\n");
        return M2M_ERR_BUS_FAIL;
    }

    M2M_DBG("[nm_spi_init]: chipid (%08x)\r\n", (unsigned int)chipid);
    spi_init_pkt_sz();

    return M2M_SUCCESS;
}

/*
*   @fn     nm_spi_deinit
*   @brief  DeInitialize the SPI
*   @return M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*   @author Samer Sarhan
*   @date   27 Feb 2015
*   @version    1.0
*/
int8_t nm_spi_deinit(void)
{
    gu8Crc_off = 0;
    OSAL_MUTEX_Delete(&s_spiLock);
    return M2M_SUCCESS;
}

/*
*   @fn     nm_spi_read_reg
*   @brief  Read register
*   @param [in] u32Addr
*               Register address
*   @return Register value
*   @author M. Abdelmawla
*   @date   11 July 2012
*   @version    1.0
*/
uint32_t nm_spi_read_reg(uint32_t u32Addr)
{
    uint32_t u32Val = 0;

    (void)nm_spi_read_reg_with_ret(u32Addr, &u32Val);

    return u32Val;
}

/*
*   @fn     nm_spi_read_reg_with_ret
*   @brief  Read register with error code return
*   @param [in] u32Addr
*               Register address
*   @param [out]    pu32RetVal
*               Pointer to u32 variable used to return the read value
*   @return M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*   @author M. Abdelmawla
*   @date   11 July 2012
*   @version    1.0
*/
int8_t nm_spi_read_reg_with_ret(uint32_t u32Addr, uint32_t* pu32RetVal)
{
    uint8_t retry = SPI_RETRY_COUNT;

    if (OSAL_RESULT_TRUE != OSAL_MUTEX_Lock(&s_spiLock, OSAL_WAIT_FOREVER))
    {
        return M2M_ERR_BUS_FAIL;
    }

    while(retry-- > 0)
    {
        if (spi_read_reg(u32Addr, pu32RetVal) == N_OK)
        {
            (void)OSAL_MUTEX_Unlock(&s_spiLock);

            return M2M_SUCCESS;
        }

        M2M_ERR("Reset and retry %d %" PRIx32 "\r\n", retry, u32Addr);
        spi_reset();
    }

    (void)OSAL_MUTEX_Unlock(&s_spiLock);

    return M2M_ERR_BUS_FAIL;
}

/*
*   @fn     nm_spi_write_reg
*   @brief  write register
*   @param [in] u32Addr
*               Register address
*   @param [in] u32Val
*               Value to be written to the register
*   @return M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*   @author M. Abdelmawla
*   @date   11 July 2012
*   @version    1.0
*/
int8_t nm_spi_write_reg(uint32_t u32Addr, uint32_t u32Val)
{
    uint8_t retry = SPI_RETRY_COUNT;

    if (OSAL_RESULT_TRUE != OSAL_MUTEX_Lock(&s_spiLock, OSAL_WAIT_FOREVER))
    {
        return M2M_ERR_BUS_FAIL;
    }

    while(retry-- > 0)
    {
        if (spi_write_reg(u32Addr, u32Val) == N_OK)
        {
            (void)OSAL_MUTEX_Unlock(&s_spiLock);

            return M2M_SUCCESS;
        }

        M2M_ERR("Reset and retry %d %" PRIx32 " %" PRIx32 "\r\n", retry, u32Addr, u32Val);
        spi_reset();
    }

    (void)OSAL_MUTEX_Unlock(&s_spiLock);

    return M2M_ERR_BUS_FAIL;
}

/*
*   @fn     nm_spi_read_block
*   @brief  Read block of data
*   @param [in] u32Addr
*               Start address
*   @param [out]    puBuf
*               Pointer to a buffer used to return the read data
*   @param [in] u16Sz
*               Number of bytes to read. The buffer size must be >= u16Sz
*   @return M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*   @author M. Abdelmawla
*   @date   11 July 2012
*   @version    1.0
*/
int8_t nm_spi_read_block(uint32_t u32Addr, uint8_t *puBuf, uint16_t u16Sz)
{
    uint8_t retry = SPI_RETRY_COUNT;
    uint8_t tmpBuf[2] = {0,0};
    uint8_t *puTmpBuf;

    if (OSAL_RESULT_TRUE != OSAL_MUTEX_Lock(&s_spiLock, OSAL_WAIT_FOREVER))
    {
        return M2M_ERR_BUS_FAIL;
    }

    if (u16Sz == 1)
    {
        u16Sz = 2;
        puTmpBuf = tmpBuf;
    }
    else
    {
        puTmpBuf = puBuf;
    }

    while(retry-- > 0)
    {
        if (spi_read_block(u32Addr, puTmpBuf, u16Sz) == N_OK)
        {
            (void)OSAL_MUTEX_Unlock(&s_spiLock);

            if (puTmpBuf == tmpBuf)
            {
                *puBuf = *tmpBuf;
            }

            return M2M_SUCCESS;
        }

        M2M_ERR("Reset and retry %d %" PRIx32 " %d\r\n", retry, u32Addr, u16Sz);
        spi_reset();
    }

    (void)OSAL_MUTEX_Unlock(&s_spiLock);

    return M2M_ERR_BUS_FAIL;
}

/*
*   @fn     nm_spi_write_block
*   @brief  Write block of data
*   @param [in] u32Addr
*               Start address
*   @param [in] puBuf
*               Pointer to the buffer holding the data to be written
*   @param [in] u16Sz
*               Number of bytes to write. The buffer size must be >= u16Sz
*   @return M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*   @author M. Abdelmawla
*   @date   11 July 2012
*   @version    1.0
*/
int8_t nm_spi_write_block(uint32_t u32Addr, uint8_t *puBuf, uint16_t u16Sz)
{
    uint8_t retry = SPI_RETRY_COUNT;

    if (OSAL_RESULT_TRUE != OSAL_MUTEX_Lock(&s_spiLock, OSAL_WAIT_FOREVER))
    {
        return M2M_ERR_BUS_FAIL;
    }

    //Workaround hardware problem with single byte transfers over SPI bus
    if (u16Sz == 1)
    {
        u16Sz = 2;
    }

    while(retry-- > 0)
    {
        if (spi_write_block(u32Addr, puBuf, u16Sz) == N_OK)
        {
            (void)OSAL_MUTEX_Unlock(&s_spiLock);

            return M2M_SUCCESS;
        }

        M2M_ERR("Reset and retry %d %" PRIx32 " %d\r\n", retry, u32Addr, u16Sz);
        spi_reset();
    }

    (void)OSAL_MUTEX_Unlock(&s_spiLock);

    return M2M_ERR_BUS_FAIL;
}

//DOM-IGNORE-END
