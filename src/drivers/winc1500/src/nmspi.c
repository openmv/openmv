/**
 *
 * \file
 *
 * \brief This module contains NMC1000 SPI protocol bus APIs implementation.
 *
 * Copyright (c) 2016-2021 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
#include "common/include/nm_common.h"

#ifdef CONF_WINC_USE_SPI

#include "bus_wrapper/include/nm_bus_wrapper.h"
#include "driver/include/nmspi.h"

#define NMI_PERIPH_REG_BASE 0x1000
#define NMI_INTR_REG_BASE (NMI_PERIPH_REG_BASE+0xa00)
#define NMI_CHIPID	(NMI_PERIPH_REG_BASE)
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

#define CMD_DMA_WRITE			0xc1
#define CMD_DMA_READ			0xc2
#define CMD_INTERNAL_WRITE		0xc3
#define CMD_INTERNAL_READ		0xc4
#define CMD_TERMINATE			0xc5
#define CMD_REPEAT				0xc6
#define CMD_DMA_EXT_WRITE		0xc7
#define CMD_DMA_EXT_READ		0xc8
#define CMD_SINGLE_WRITE		0xc9
#define CMD_SINGLE_READ			0xca
#define CMD_RESET				0xcf

#define N_OK                     0
#define N_FAIL                  -1
#define N_RESET                 -2
#define N_RETRY                 -3

#define SPI_RESP_RETRY_COUNT 	(10)
#define SPI_RETRY_COUNT			(10)
#define DATA_PKT_SZ_256 		256
#define DATA_PKT_SZ_512			512
#define DATA_PKT_SZ_1K			1024
#define DATA_PKT_SZ_4K			(4 * 1024)
#define DATA_PKT_SZ_8K			(8 * 1024)
#define DATA_PKT_SZ				DATA_PKT_SZ_8K

static uint8 	gu8Crc_off	=   0;

static inline sint8 nmi_spi_read(uint8 *b, uint16 sz)
{
    return nm_spi_rw(NULL, b, sz);
}
static inline sint8 nmi_spi_write(uint8 *b, uint16 sz)
{
    return nm_spi_rw(b, NULL, sz);
}
static sint8 nmi_spi_writeread(uint8 *bw, uint8 *br, uint16 sz)
{
    return nm_spi_rw(bw, br, sz);
}

/********************************************

	Crc7

********************************************/

static const uint8 crc7_syndrome_table[256] = {
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


static inline uint8 crc7_byte(uint8 crc, uint8 data)
{
	return crc7_syndrome_table[(crc << 1) ^ data];
}

static inline uint8 crc7(uint8 crc, const uint8 *buffer, uint32 len)
{
	while (len--)
		crc = crc7_byte(crc, *buffer++);
	return crc;
}

/********************************************

	Spi protocol Function

********************************************/

static sint8 spi_cmd(uint8 cmd, uint32 adr, uint32 u32data, uint32 sz,uint8 clockless)
{
	uint8 bc[9];
	uint8 len = 5;
	sint8 result = N_OK;

	bc[0] = cmd;
	switch (cmd) {
	case CMD_SINGLE_READ:				/* single word (4 bytes) read */
		bc[1] = (uint8)(adr >> 16);
		bc[2] = (uint8)(adr >> 8);
		bc[3] = (uint8)adr;
		len = 5;
		break;
	case CMD_INTERNAL_READ:			/* internal register read */
		bc[1] = (uint8)(adr >> 8);
		if(clockless)  bc[1] |= (1 << 7);
		bc[2] = (uint8)adr;
		bc[3] = 0x00;
		len = 5;
		break;
#if defined(CMD_TERMINATE)
	case CMD_TERMINATE:					/* termination */
		bc[1] = 0x00;
		bc[2] = 0x00;
		bc[3] = 0x00;
		len = 5;
		break;
#endif
#if defined(CMD_REPEAT)
	case CMD_REPEAT:						/* repeat */
		bc[1] = 0x00;
		bc[2] = 0x00;
		bc[3] = 0x00;
		len = 5;
		break;
#endif
	case CMD_RESET:							/* reset */
		bc[1] = 0xff;
		bc[2] = 0xff;
		bc[3] = 0xff;
		len = 5;
		break;
#if defined(CMD_DMA_WRITE) || defined(CMD_DMA_READ)
	case CMD_DMA_WRITE:					/* dma write */
	case CMD_DMA_READ:					/* dma read */
		bc[1] = (uint8)(adr >> 16);
		bc[2] = (uint8)(adr >> 8);
		bc[3] = (uint8)adr;
		bc[4] = (uint8)(sz >> 8);
		bc[5] = (uint8)(sz);
		len = 7;
		break;
#endif
	case CMD_DMA_EXT_WRITE:		/* dma extended write */
	case CMD_DMA_EXT_READ:			/* dma extended read */
		bc[1] = (uint8)(adr >> 16);
		bc[2] = (uint8)(adr >> 8);
		bc[3] = (uint8)adr;
		bc[4] = (uint8)(sz >> 16);
		bc[5] = (uint8)(sz >> 8);
		bc[6] = (uint8)(sz);
		len = 8;
		break;
	case CMD_INTERNAL_WRITE:		/* internal register write */
		bc[1] = (uint8)(adr >> 8);
		if(clockless)  bc[1] |= (1 << 7);
		bc[2] = (uint8)(adr);
		bc[3] = (uint8)(u32data >> 24);
		bc[4] = (uint8)(u32data >> 16);
		bc[5] = (uint8)(u32data >> 8);
		bc[6] = (uint8)(u32data);
		len = 8;
		break;
	case CMD_SINGLE_WRITE:			/* single word write */
		bc[1] = (uint8)(adr >> 16);
		bc[2] = (uint8)(adr >> 8);
		bc[3] = (uint8)(adr);
		bc[4] = (uint8)(u32data >> 24);
		bc[5] = (uint8)(u32data >> 16);
		bc[6] = (uint8)(u32data >> 8);
		bc[7] = (uint8)(u32data);
		len = 9;
		break;
	default:
		result = N_FAIL;
		break;
	}

    if(result == N_OK) {
		if (!gu8Crc_off)
			bc[len-1] = (crc7(0x7f, (const uint8 *)&bc[0], len-1)) << 1;
		else
			len-=1;

		if (M2M_SUCCESS != nmi_spi_write(bc, len)) {
			M2M_ERR("[nmi spi]: Failed cmd write, bus error...\n");
			result = N_FAIL;
		}
	}

	return result;
}

static sint8 spi_data_rsp(uint8 cmd)
{
	uint8 len;
	uint8 rsp[3];
	sint8 result = N_OK;

    if (!gu8Crc_off)
		len = 2;
	else
		len = 3;

	if (M2M_SUCCESS != nmi_spi_read(&rsp[0], len)) {
		M2M_ERR("[nmi spi]: Failed bus error...\n");
		result = N_FAIL;
		goto _fail_;
	}
		
	if((rsp[len-1] != 0)||(rsp[len-2] != 0xC3))
	{
		M2M_ERR("[nmi spi]: Failed data response read, %x %x %x\n",rsp[0],rsp[1],rsp[2]);
		result = N_FAIL;
		goto _fail_;
	}
_fail_:

	return result;
}

static sint8 spi_cmd_rsp(uint8 cmd)
{
	uint8 rsp;
	sint8 result = N_OK;
	sint8 s8RetryCnt;

	/**
		Command/Control response
	**/
#if defined(CMD_TERMINATE)
    if(cmd == CMD_TERMINATE) {
        if(M2M_SUCCESS != nmi_spi_read(&rsp, 1)) {
            result = N_FAIL;
            goto _fail_;
        }
    }
#endif
#if defined(CMD_REPEAT)
    if(cmd == CMD_REPEAT) {
		if (M2M_SUCCESS != nmi_spi_read(&rsp, 1)) {
			result = N_FAIL;
			goto _fail_;
		}
	}
#endif

	/* wait for response */
	s8RetryCnt = SPI_RESP_RETRY_COUNT;
	do
	{
		if (M2M_SUCCESS != nmi_spi_read(&rsp, 1)) {
			M2M_ERR("[nmi spi]: Failed cmd response read, bus error...\n");
			result = N_FAIL;
			goto _fail_;
		}
	} while((rsp != cmd) && (s8RetryCnt-- >0));

	/**
		State response
	**/
	/* wait for response */
	s8RetryCnt = SPI_RESP_RETRY_COUNT;
	do
	{
		if (M2M_SUCCESS != nmi_spi_read(&rsp, 1)) {
			M2M_ERR("[nmi spi]: Failed cmd response read, bus error...\n");
			result = N_FAIL;
			goto _fail_;
		}
	} while((rsp != 0x00) && (s8RetryCnt-- >0));

_fail_:

	return result;
}

sint8 nm_spi_reset(void)
{
	//M2M_INFO("Reset Spi\n");
	spi_cmd(CMD_RESET, 0, 0, 0, 0);

	if(spi_cmd_rsp(CMD_RESET) != N_OK) {
		// Reset command failed, need to send repeated 1's until reset occurs
		uint8 w_buf[8] = {0xFF};
		uint8 r_buf[8];
		M2M_ERR("[nmi spi]: Failed rst cmd response\n");
		nmi_spi_writeread(w_buf, r_buf, 8);
		if(r_buf[7] != 0xFF)
		{
			M2M_ERR("[nmi spi]: Failed repeated reset\n");
			return N_FAIL;
		}
	}
	return N_OK;
}

static sint8 spi_data_read(uint8 *b, uint16 sz,uint8 clockless)
{
	sint16 retry, ix, nbytes;
	sint8 result = N_OK;
	uint8 crc[2];
	uint8 rsp;

	/**
		Data
	**/
	ix = 0;
	do {
		if (sz <= DATA_PKT_SZ)
			nbytes = sz;
		else
			nbytes = DATA_PKT_SZ;

		/**
			Data Response header
		**/
		retry = SPI_RESP_RETRY_COUNT;
		do {
			if (M2M_SUCCESS != nmi_spi_read(&rsp, 1)) {
				M2M_ERR("[nmi spi]: Failed data response read, bus error...\n");
				result = N_FAIL;
				break;
			}
            if((rsp & 0xf0) == 0xf0)
				break;
		} while (retry--);

		if (result == N_FAIL)
			break;

		if (retry <= 0) {
			M2M_ERR("[nmi spi]: Failed data response read...(%02x)\n", rsp);
			result = N_FAIL;
			break;
		}

		/**
			Read bytes
		**/
		if (M2M_SUCCESS != nmi_spi_read(&b[ix], nbytes)) {
			M2M_ERR("[nmi spi]: Failed data block read, bus error...\n");
			result = N_FAIL;
			break;
		}
		if(!clockless)
		{
			/**
			Read Crc
			**/
			if (!gu8Crc_off) {
				if (M2M_SUCCESS != nmi_spi_read(crc, 2)) {
					M2M_ERR("[nmi spi]: Failed data block crc read, bus error...\n");
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

static sint8 spi_data_write(uint8 *b, uint16 sz)
{
    sint16 ix = 0;
	uint16 nbytes;
    sint8 result = N_OK;
	uint8 cmd, order, crc[2] = {0};
	//uint8 rsp;

	/**
		Data
	**/
	do {
		if (sz <= DATA_PKT_SZ)
			nbytes = sz;
		else
			nbytes = DATA_PKT_SZ;

		/**
			Write command
		**/
		cmd = 0xf0;
		if (ix == 0)  {
			if (sz <= DATA_PKT_SZ)
				order = 0x3;
			else
				order = 0x1;
		} else {
			if (sz <= DATA_PKT_SZ)
				order = 0x3;
			else
				order = 0x2;
		}
		cmd |= order;
		if (M2M_SUCCESS != nmi_spi_write(&cmd, 1)) {
			M2M_ERR("[nmi spi]: Failed data block cmd write, bus error...\n");
			result = N_FAIL;
			break;
		}

		/**
			Write data
		**/
		if (M2M_SUCCESS != nmi_spi_write(&b[ix], nbytes)) {
			M2M_ERR("[nmi spi]: Failed data block write, bus error...\n");
			result = N_FAIL;
			break;
		}

		/**
			Write Crc
		**/
		if (!gu8Crc_off) {
			if (M2M_SUCCESS != nmi_spi_write(crc, 2)) {
				M2M_ERR("[nmi spi]: Failed data block crc write, bus error...\n");
				result = N_FAIL;
				break;
			}
		}

		ix += nbytes;
		sz -= nbytes;
	} while (sz);


	return result;
}

/********************************************

	Spi Internal Read/Write Function

********************************************/

/********************************************

	Spi interfaces

********************************************/

/**
 *  @fn         nm_spi_write_reg
 *  @brief      Write register
 *  @param[in]  u32Addr
 *                  Register address
 *  @param[in]  u32Val
 *                  Value to be written to the register
 *  @return     @ref M2M_SUCCESS in case of success and @ref M2M_ERR_BUS_FAIL in case of failure
 */
sint8 nm_spi_write_reg(uint32 addr, uint32 u32data)
{
	uint8 retry = SPI_RETRY_COUNT;
	sint8 result = N_OK;
	uint8 cmd = CMD_SINGLE_WRITE;
	uint8 clockless = 0;
	
_RETRY_:	
	if (addr <= 0x30)
	{
		/**
		NMC1000 clockless registers.
		**/
		cmd = CMD_INTERNAL_WRITE;
		clockless = 1;
	}

	result = spi_cmd(cmd, addr, u32data, 4, clockless);
	if (result != N_OK) {
		M2M_ERR("[nmi spi]: Failed cmd, write reg (%08x)...\n", (unsigned int)addr);
		goto _FAIL_;
	}

	result = spi_cmd_rsp(cmd);
	if (result != N_OK) {
		M2M_ERR("[nmi spi]: Failed cmd response, write reg (%08x)...\n", (unsigned int)addr);
		goto _FAIL_;
	}
_FAIL_:
	if(result != N_OK)
	{
		nm_bsp_sleep(1);
		spi_cmd(CMD_RESET, 0, 0, 0, 0);
		spi_cmd_rsp(CMD_RESET);
		M2M_ERR("Reset and retry %d %x %x\n",retry,addr,u32data);
		nm_bsp_sleep(1);
		retry--;
		if(retry) goto _RETRY_;
	}

	return result;
}

static sint8 nm_spi_write(uint32 addr, uint8 *buf, uint16 size)
{
	sint8 result;
	uint8 retry = SPI_RETRY_COUNT;
	uint8 cmd = CMD_DMA_EXT_WRITE;


_RETRY_:
	/**
		Command
	**/
	//Workaround hardware problem with single byte transfers over SPI bus
	if (size == 1)
		size = 2;

	result = spi_cmd(cmd, addr, 0, size,0);
	if (result != N_OK) {
		M2M_ERR("[nmi spi]: Failed cmd, write block (%08x)...\n", (unsigned int)addr);
		goto _FAIL_;
	}

	result = spi_cmd_rsp(cmd);
	if (result != N_OK) {
		M2M_ERR("[nmi spi ]: Failed cmd response, write block (%08x)...\n", (unsigned int)addr);
		goto _FAIL_;
	}

	/**
		Data
	**/
	result = spi_data_write(buf, size);
	if (result != N_OK) {
		M2M_ERR("[nmi spi]: Failed block data write...\n");
		goto _FAIL_;
	}
	/**
		Data RESP
	**/
	result = spi_data_rsp(cmd);
	if (result != N_OK) {
		M2M_ERR("[nmi spi]: Failed block data write...\n");
		goto _FAIL_;
	}
	
_FAIL_:
	if(result != N_OK)
	{
		nm_bsp_sleep(1);
		spi_cmd(CMD_RESET, 0, 0, 0, 0);
		spi_cmd_rsp(CMD_RESET);
		M2M_ERR("Reset and retry %d %x %d\n",retry,addr,size);
		nm_bsp_sleep(1);
		retry--;
		if(retry) goto _RETRY_;
	}


	return result;
}

/**
 *  @fn         nm_spi_read_reg_with_ret
 *  @brief      Read register with error code return
 *  @param[in]  u32Addr
 *                  Register address
 *  @param[out] pu32RetVal
 *                  Pointer to u32 variable used to return the read value
 *  @return     @ref M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
 */
sint8 nm_spi_read_reg_with_ret(uint32 addr, uint32 *u32data)
{
	uint8 retry = SPI_RETRY_COUNT;
	volatile sint8 result = N_OK;
	uint8 cmd = CMD_SINGLE_READ;
	uint8 tmp[4];
	uint8 clockless = 0;

_RETRY_:

	if (addr <= 0xff)
	{
		/**
		NMC1000 clockless registers.
		**/
		cmd = CMD_INTERNAL_READ;
		clockless = 1;
	}

	result = spi_cmd(cmd, addr, 0, 4, clockless);
	if (result != N_OK) {
		M2M_ERR("[nmi spi]: Failed cmd, read reg (%08x)...\n", (unsigned int)addr);
		goto _FAIL_;
	}

	result = spi_cmd_rsp(cmd);
	if (result != N_OK) {
		M2M_ERR("[nmi spi]: Failed cmd response, read reg (%08x)...\n", (unsigned int)addr);
		goto _FAIL_;
	}

	/* to avoid endianness issues */
	result = spi_data_read(&tmp[0], 4, clockless);
	if (result != N_OK) {
		M2M_ERR("[nmi spi]: Failed data read...\n");
		goto _FAIL_;
	}

	*u32data = tmp[0] |
		((uint32)tmp[1] << 8) |
		((uint32)tmp[2] << 16) |
		((uint32)tmp[3] << 24);
		
_FAIL_:
	if(result != N_OK)
	{
		nm_bsp_sleep(1);
		spi_cmd(CMD_RESET, 0, 0, 0, 0);
		spi_cmd_rsp(CMD_RESET);
		M2M_ERR("Reset and retry %d %lx\n",retry,addr);
		nm_bsp_sleep(1);
		retry--;
		if(retry) goto _RETRY_;
	}
		
	return result;
}

static sint8 nm_spi_read(uint32 addr, uint8 *buf, uint16 size)
{
	uint8 cmd = CMD_DMA_EXT_READ;
	sint8 result;
	uint8 retry = SPI_RETRY_COUNT;
	uint8 tmp[2];
	uint8 single_byte_workaround = 0;

_RETRY_:

	/**
		Command
	**/
	if (size == 1)
	{
		//Workaround hardware problem with single byte transfers over SPI bus
		size = 2;
		single_byte_workaround = 1;
	}
	result = spi_cmd(cmd, addr, 0, size,0);
	if (result != N_OK) {
		M2M_ERR("[nmi spi]: Failed cmd, read block (%08x)...\n", (unsigned int)addr);
		goto _FAIL_;
	}

	result = spi_cmd_rsp(cmd);
	if (result != N_OK) {
		M2M_ERR("[nmi spi]: Failed cmd response, read block (%08x)...\n", (unsigned int)addr);
		goto _FAIL_;
	}

	/**
		Data
	**/
	if (single_byte_workaround)
	{
		result = spi_data_read(tmp, size,0);
		buf[0] = tmp[0];
	}
	else
		result = spi_data_read(buf, size,0);

	if (result != N_OK) {
		M2M_ERR("[nmi spi]: Failed block data read...\n");
		goto _FAIL_;
	}

_FAIL_:
	if(result != N_OK)
	{
		nm_bsp_sleep(1);
		spi_cmd(CMD_RESET, 0, 0, 0, 0);
		spi_cmd_rsp(CMD_RESET);
		M2M_ERR("Reset and retry %d %lx %d\n",retry,addr,size);
		nm_bsp_sleep(1);
		retry--;
		if(retry) goto _RETRY_;
	}

	return result;
}

/********************************************

	Bus interfaces

********************************************/

static void spi_init_pkt_sz(void)
{
	uint32 val32;

	/* Make sure SPI max. packet size fits the defined DATA_PKT_SZ.  */
	val32 = nm_spi_read_reg(SPI_BASE+0x24);
	val32 &= ~(0x7 << 4);
	switch(DATA_PKT_SZ)
	{
        case 256:
            val32 |= (0 << 4);
            break;
        case 512:
            val32 |= (1 << 4);
            break;
        case 1024:
            val32 |= (2 << 4);
            break;
        case 2048:
            val32 |= (3 << 4);
            break;
        case 4096:
            val32 |= (4 << 4);
            break;
        case 8192:
            val32 |= (5 << 4);
            break;
	}
	nm_spi_write_reg(SPI_BASE+0x24, val32);
}

/**
*	@fn		nm_spi_init
*	@brief	Initialize the SPI
 *  @return     @ref M2M_SUCCESS in case of success and @ref M2M_ERR_BUS_FAIL in case of failure
*/
sint8 nm_spi_init(void)
{
	uint32 chipid;
	uint32 reg = 0;
	

	/**
		configure protocol
	**/
	gu8Crc_off = 0;

    if(nm_spi_read_reg_with_ret(NMI_SPI_PROTOCOL_CONFIG, &reg) != M2M_SUCCESS) {
		/* Read failed. Try with CRC off. This might happen when module
		is removed but chip isn't reset*/
		gu8Crc_off = 1;
		M2M_ERR("[nmi spi]: Failed internal read protocol with CRC on, retrying with CRC off...\n");
        if(nm_spi_read_reg_with_ret(NMI_SPI_PROTOCOL_CONFIG, &reg) != M2M_SUCCESS) {
			// Read failed with both CRC on and off, something went bad
			M2M_ERR( "[nmi spi]: Failed internal read protocol...\n");
            return M2M_ERR_BUS_FAIL;
		}
	}
	if(gu8Crc_off == 0)
	{
		reg &= ~0xc;	/* disable crc checking */
		reg &= ~0x70;
		reg |= (0x5 << 4);
        if(nm_spi_write_reg(NMI_SPI_PROTOCOL_CONFIG, reg) != M2M_SUCCESS) {
			M2M_ERR( "[nmi spi]: Failed internal write protocol reg...\n");
            return M2M_ERR_BUS_FAIL;
		}
		gu8Crc_off = 1;
	}

	/**
		make sure can read back chip id correctly
	**/
    if(nm_spi_read_reg_with_ret(0x1000, &chipid) != M2M_SUCCESS) {
		M2M_ERR("[nmi spi]: Fail cmd read chip id...\n");
		return M2M_ERR_BUS_FAIL;
	}

	M2M_DBG("[nmi spi]: chipid (%08x)\n", (unsigned int)chipid);
	spi_init_pkt_sz();


	return M2M_SUCCESS;
}

/**
*	@fn		nm_spi_init
*	@brief	DeInitialize the SPI 
 *  @return     @ref M2M_SUCCESS in case of success and @ref M2M_ERR_BUS_FAIL in case of failure
*/ 
sint8 nm_spi_deinit(void)
{
	gu8Crc_off = 0;
	return M2M_SUCCESS;
}

/*
*	@fn		nm_spi_read_reg
*	@brief	Read register
*	@param [in]	u32Addr
*				Register address
*	@return	Register value
*/
uint32 nm_spi_read_reg(uint32 u32Addr)
{
	uint32 u32Val;

    nm_spi_read_reg_with_ret(u32Addr, &u32Val);

	return u32Val;
}

/*
*	@fn		nm_spi_read_block
*	@brief	Read block of data
*	@param [in]	u32Addr
*				Start address
*	@param [out]	puBuf
*				Pointer to a buffer used to return the read data
*	@param [in]	u16Sz
*				Number of bytes to read. The buffer size must be >= u16Sz
*	@return	M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*	@author	M. Abdelmawla
*	@date	11 July 2012
*	@version	1.0
*/
sint8 nm_spi_read_block(uint32 u32Addr, uint8 *puBuf, uint16 u16Sz)
{
	sint8 s8Ret;

	s8Ret = nm_spi_read(u32Addr, puBuf, u16Sz);

	if(N_OK == s8Ret) s8Ret = M2M_SUCCESS;
	else s8Ret = M2M_ERR_BUS_FAIL;

	return s8Ret;
}

/*
*	@fn		nm_spi_write_block
*	@brief	Write block of data
*	@param [in]	u32Addr
*				Start address
*	@param [in]	puBuf
*				Pointer to the buffer holding the data to be written
*	@param [in]	u16Sz
*				Number of bytes to write. The buffer size must be >= u16Sz
*	@return	M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*	@author	M. Abdelmawla
*	@date	11 July 2012
*	@version	1.0
*/
sint8 nm_spi_write_block(uint32 u32Addr, uint8 *puBuf, uint16 u16Sz)
{
	sint8 s8Ret;

	s8Ret = nm_spi_write(u32Addr, puBuf, u16Sz);

	if(N_OK == s8Ret) s8Ret = M2M_SUCCESS;
	else s8Ret = M2M_ERR_BUS_FAIL;

	return s8Ret;
}

#endif
