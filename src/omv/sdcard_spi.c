/* Copyright (c) 2010, Martin Thomas, ChaN All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in
 the documentation and/or other materials provided with the
 distribution.
 * Neither the name of the copyright holders nor the names of
 contributors may be used to endorse or promote products derived
 from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE. */

#include "mp.h"
#include <stdbool.h>
#include STM32_HAL_H
#include "ffconf.h"
#include "diskio.h"
#include "pincfg.h"
#include "systick.h"
#include "sdcard.h"

/* Definitions for MMC/SDC command */
#define CMD0    (0x40+0)    /* GO_IDLE_STATE */
#define CMD1    (0x40+1)    /* SEND_OP_COND (MMC) */
#define ACMD41  (0xC0+41)   /* SEND_OP_COND (SDC) */
#define CMD8    (0x40+8)    /* SEND_IF_COND */
#define CMD9    (0x40+9)    /* SEND_CSD */
#define CMD10   (0x40+10)   /* SEND_CID */
#define CMD12   (0x40+12)   /* STOP_TRANSMISSION */
#define ACMD13  (0xC0+13)   /* SD_STATUS (SDC) */
#define CMD16   (0x40+16)   /* SET_BLOCKLEN */
#define CMD17   (0x40+17)   /* READ_SINGLE_BLOCK */
#define CMD18   (0x40+18)   /* READ_MULTIPLE_BLOCK */
#define CMD23   (0x40+23)   /* SET_BLOCK_COUNT (MMC) */
#define ACMD23  (0xC0+23)   /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24   (0x40+24)   /* WRITE_BLOCK */
#define CMD25   (0x40+25)   /* WRITE_MULTIPLE_BLOCK */
#define CMD55   (0x40+55)   /* APP_CMD */
#define CMD58   (0x40+58)   /* READ_OCR */

/* MMC card type flags (MMC_GET_TYPE) */
#define CT_MMC		0x01		/* MMC ver 3 */
#define CT_SD1		0x02		/* SD ver 1 */
#define CT_SD2		0x04		/* SD ver 2 */
#define CT_SDC		(CT_SD1|CT_SD2)	/* SD */
#define CT_BLOCK	0x08		/* Block addressing */

#define SD_TIMEOUT          (100000)
#define SPI_TIMEOUT         (100)  /* in ms */

/*--------------------------------------------------------------------------
  Module Private Functions and Variables
  ---------------------------------------------------------------------------*/
static BYTE CardType;            /* Card type flags */
static const DWORD socket_state_mask_cp = (1 << 0);
static volatile DSTATUS Stat = STA_NOINIT;    /* Disk status */

static SPI_HandleTypeDef SPIHandle;
DRESULT sdcard_ioctl(BYTE drv, BYTE ctrl, void *buff);

bool sdcard_is_present(void)
{
    return (HAL_GPIO_ReadPin(SD_CD_PORT, SD_CD_PIN)==GPIO_PIN_RESET);
}

/*-----------------------------------------------------------------------*/
/* Transmit/Receive a byte to MMC via SPI  (Platform dependent)          */
/*-----------------------------------------------------------------------*/
static BYTE spi_send(BYTE out)
{
    if (HAL_SPI_TransmitReceive(&SPIHandle, &out, &out, 1, SPI_TIMEOUT) != HAL_OK) {
        BREAK();
    }
    return out;
}

static bool spi_send_buff(const BYTE *buff, uint32_t size)
{
    mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
    bool res = (HAL_SPI_Transmit(&SPIHandle, (void*)buff, size, SPI_TIMEOUT) == HAL_OK);
    MICROPY_END_ATOMIC_SECTION(atomic_state);
    return res;
}

#define spi_recv() spi_send(0xFF)

static bool spi_recv_buff(BYTE *buff, uint32_t size)
{
    mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
    do {
        *buff++ = spi_recv();
    } while (--size);
    MICROPY_END_ATOMIC_SECTION(atomic_state);
    return true;
}

/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/
static BYTE wait_ready(void)
{
    BYTE res;
    volatile int timeout = SD_TIMEOUT;

    spi_recv();
    do
        res = spi_recv();
    while ((res != 0xFF) && timeout--);

    return res;
}

/*-----------------------------------------------------------------------*/
/* SD_DESELECT the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/
static void release_spi(void)
{
    SD_DESELECT();
    spi_recv();
}

/*-----------------------------------------------------------------------*/
/* Receive a data packet from MMC                                        */
/*-----------------------------------------------------------------------*/
static bool rcvr_datablock(BYTE *buff, UINT btr)
{
    BYTE token;
    volatile int timeout = SD_TIMEOUT;
    do {                            /* Wait for data packet in timeout of 100ms */
        token = spi_recv();
    } while ((token == 0xFF) && timeout--);

    if(token != 0xFE) {
        return false;    /* If not valid data token, return with error */
    }

    spi_recv_buff(buff, btr);

    /* Discard CRC */
    spi_recv();
    spi_recv();
    return true;
}

/*-----------------------------------------------------------------------*/
/* Send a data packet to MMC                                             */
/*-----------------------------------------------------------------------*/
static bool xmit_datablock(const BYTE *buff, BYTE token)
{
    BYTE resp;

    if (wait_ready() != 0xFF) {
        return false;
    }

    /* transmit data token */
    spi_send(token);

    if (token != 0xFD) { /* Is data token */
        spi_send_buff(buff, SDCARD_BLOCK_SIZE);

        /* CRC (Dummy) */
        spi_send(0xFF);
        spi_send(0xFF);

        /* Receive data response */
        resp = spi_recv();

        /* If not accepted, return with error */
        if ((resp & 0x1F) != 0x05)
            return false;
    }

    return true;
}

static BYTE send_cmd(BYTE cmd, DWORD arg)
{
    BYTE n, res;

    if (cmd & 0x80) {    /* ACMD<n> is the command sequence of CMD55-CMD<n> */
        cmd &= 0x7F;
        res = send_cmd(CMD55, 0);
        if (res > 1) return res;
    }

    /* SD_SELECT the card and wait for ready */
    SD_DESELECT();
    SD_SELECT();
    if (wait_ready() != 0xFF) {
        return 0xFF;
    }

    /* Send command packet */
    spi_send(cmd);                      /* Start + Command index */
    spi_send((BYTE)(arg >> 24));        /* Argument[31..24] */
    spi_send((BYTE)(arg >> 16));        /* Argument[23..16] */
    spi_send((BYTE)(arg >> 8));         /* Argument[15..8] */
    spi_send((BYTE)arg);                /* Argument[7..0] */
    n = 0x01;                           /* Dummy CRC + Stop */
    if (cmd == CMD0) n = 0x95;          /* Valid CRC for CMD0(0) */
    if (cmd == CMD8) n = 0x87;          /* Valid CRC for CMD8(0x1AA) */
    spi_send(n);

    /* Receive command response */
    if (cmd == CMD12) spi_recv();       /* Skip a stuff byte when stop reading */

    n = 10;                             /* Wait for a valid response in timeout of 10 attempts */
    do
        res = spi_recv();
    while ((res & 0x80) && --n);

    return res;            /* Return with the response value */
}


void sdcard_hw_init(uint32_t baudrate)
{
    /* SPI configuration */
    SPIHandle.Instance               = SD_SPI;
    SPIHandle.Init.Mode              = SPI_MODE_MASTER;
    SPIHandle.Init.Direction         = SPI_DIRECTION_2LINES;
    SPIHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
    SPIHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
    SPIHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
    SPIHandle.Init.NSS               = SPI_NSS_SOFT;
    SPIHandle.Init.BaudRatePrescaler = baudrate;
    SPIHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    SPIHandle.Init.TIMode            = SPI_TIMODE_DISABLED;
    SPIHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
    SPIHandle.Init.CRCPolynomial     = 7;

    /* Initialize the SPI */
    if (HAL_SPI_Init(&SPIHandle) != HAL_OK) {
        /* Initialization Error */
        BREAK();
    } else {
        // TODO is this needed ?
        uint8_t buf[1];
        // TODO use while ?
        HAL_SPI_Receive(&SPIHandle, buf, sizeof(buf), SD_TIMEOUT);
    }
}

void sdcard_init(void)
{
    BYTE n, cmd, ty, ocr[4];
    volatile int timeout = SD_TIMEOUT;

    sdcard_hw_init(SPI_BAUDRATEPRESCALER_256);
    systick_sleep(250);

    for (n = 10; n; n--) {
        /* 80 dummy clocks */
        spi_recv();
    }

    ty = 0;
    if (send_cmd(CMD0, 0) == 1) {            /* Enter Idle state */
        if (send_cmd(CMD8, 0x1AA) == 1) {    /* SDHC */
            for (n = 0; n < 4; n++) {
                ocr[n] = spi_recv();            /* Get trailing return value of R7 response */
            }
            if (ocr[2] == 0x01 && ocr[3] == 0xAA) {                 /* The card can work at VDD range of 2.7-3.6V */
                while (--timeout && send_cmd(ACMD41, 1UL << 30));   /* Wait for leaving idle state (ACMD41 with HCS bit) */
                if (timeout && send_cmd(CMD58, 0) == 0) {           /* Check CCS bit in the OCR */
                    for (n = 0; n < 4; n++) {
                        ocr[n] = spi_recv();
                    }
                    ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
                }
            }
        } else {                            /* SDSC or MMC */
            if (send_cmd(ACMD41, 0) <= 1) {
                ty = CT_SD1; cmd = ACMD41;    /* SDSC */
            } else {
                ty = CT_MMC; cmd = CMD1;    /* MMC */
            }
            while (--timeout && send_cmd(cmd, 0));      /* Wait for leaving idle state */
            if (!timeout || send_cmd(CMD16, SDCARD_BLOCK_SIZE) != 0)  /* Set R/W block length */
                ty = 0;
        }
    }

    CardType = ty;
    release_spi();

    if (ty) {
        /* Initialization succeeded */
        Stat &= ~STA_NOINIT; /* Clear STA_NOINIT */
        sdcard_hw_init(SPI_BAUDRATEPRESCALER_2);
    } else {
        /* Initialization failed */
        BREAK();
    }
}

bool sdcard_power_on(void)
{
    return true;
}

void sdcard_power_off(void)
{

}

mp_uint_t sdcard_read_blocks(uint8_t *buff, uint32_t sector, uint32_t count)
{
    if (Stat & STA_NOINIT) {
        return false;
    }

    if (!(CardType & CT_BLOCK)) {
        sector *= SDCARD_BLOCK_SIZE;    /* Convert to byte address if needed */
    }

    if (count == 1) {    /* Single block read */
        if (send_cmd(CMD17, sector) == 0)    { /* READ_SINGLE_BLOCK */
            if (rcvr_datablock(buff, SDCARD_BLOCK_SIZE)) {
                count = 0;
            }
        }
    } else {            /* Multiple block read */
        if (send_cmd(CMD18, sector) == 0) {    /* READ_MULTIPLE_BLOCK */
            do {
                if (!rcvr_datablock(buff, SDCARD_BLOCK_SIZE)) {
                    break;
                }
                buff += SDCARD_BLOCK_SIZE;
            } while (--count);
            send_cmd(CMD12, 0);                /* STOP_TRANSMISSION */
        }
    }
    release_spi();
    return count ? true: false;
}

mp_uint_t sdcard_write_blocks(const uint8_t *buff, uint32_t sector, uint32_t count)
{
    if (Stat & STA_NOINIT) {
        return false;
    }

    if (!(CardType & CT_BLOCK)) {
        sector *= SDCARD_BLOCK_SIZE;    /* Convert to byte address if needed */
    }

    if (count == 1) {    /* Single block write */
        if ((send_cmd(CMD24, sector) == 0)    /* WRITE_BLOCK */
                && xmit_datablock(buff, 0xFE))
            count = 0;
    } else {            /* Multiple block write */
        if (CardType & CT_SDC) {
            send_cmd(ACMD23, count);
        }

        if (send_cmd(CMD25, sector) == 0) {    /* WRITE_MULTIPLE_BLOCK */
            do {
                if (!xmit_datablock(buff, 0xFC)) break;
                buff += SDCARD_BLOCK_SIZE;
            } while (--count);
            if (!xmit_datablock(0, 0xFD))    /* STOP_TRAN token */
                count = 1;
        }
    }
    release_spi();
    return count ? true: false;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

uint64_t sdcard_get_capacity_in_bytes(void)
{
    uint32_t sec;
    sdcard_ioctl(0, GET_SECTOR_COUNT, &sec);
    return sec * SDCARD_BLOCK_SIZE;
}

DRESULT sdcard_ioctl (
        BYTE drv,        /* Physical drive number (0) */
        BYTE ctrl,        /* Control code */
        void *buff        /* Buffer to send/receive control data */
        )
{
    DRESULT res;
    BYTE n, csd[16], *ptr = buff;
    WORD csize;

    if (drv) return RES_PARERR;

    res = RES_ERROR;

    if (Stat & STA_NOINIT) return RES_NOTRDY;

    switch (ctrl) {
        case CTRL_SYNC :        /* Make sure that no pending write process */
            SD_SELECT();
            if (wait_ready() == 0xFF)
                res = RES_OK;
            break;

        case GET_SECTOR_COUNT :    /* Get number of sectors on the disk (DWORD) */
            if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
                if ((csd[0] >> 6) == 1) {    /* SDC version 2.00 */
                    csize = csd[9] + ((WORD)csd[8] << 8) + 1;
                    *(DWORD*)buff = (DWORD)csize << 10;
                } else {                    /* SDC version 1.XX or MMC*/
                    n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
                    csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
                    *(DWORD*)buff = (DWORD)csize << (n - 9);
                }
                res = RES_OK;
            }
            break;

        case GET_SECTOR_SIZE :    /* Get R/W sector size (WORD) */
            *(WORD*)buff = SDCARD_BLOCK_SIZE;
            res = RES_OK;
            break;

        case GET_BLOCK_SIZE :    /* Get erase block size in unit of sector (DWORD) */
            if (CardType & CT_SD2) {    /* SDC version 2.00 */
                if (send_cmd(ACMD13, 0) == 0) {    /* Read SD status */
                    spi_recv();
                    if (rcvr_datablock(csd, 16)) {                /* Read partial block */
                        for (n = 64 - 16; n; n--) spi_recv();    /* Purge trailing data */
                        *(DWORD*)buff = 16UL << (csd[10] >> 4);
                        res = RES_OK;
                    }
                }
            } else {                    /* SDC version 1.XX or MMC */
                if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {    /* Read CSD */
                    if (CardType & CT_SD1) {    /* SDC version 1.XX */
                        *(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
                    } else {                    /* MMC */
                        *(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
                    }
                    res = RES_OK;
                }
            }
            break;

        case MMC_GET_TYPE :        /* Get card type flags (1 byte) */
            *ptr = CardType;
            res = RES_OK;
            break;

        case MMC_GET_CSD :        /* Receive CSD as a data block (16 bytes) */
            if (send_cmd(CMD9, 0) == 0        /* READ_CSD */
                    && rcvr_datablock(ptr, 16))
                res = RES_OK;
            break;

        case MMC_GET_CID :        /* Receive CID as a data block (16 bytes) */
            if (send_cmd(CMD10, 0) == 0        /* READ_CID */
                    && rcvr_datablock(ptr, 16))
                res = RES_OK;
            break;

        case MMC_GET_OCR :        /* Receive OCR as an R3 resp (4 bytes) */
            if (send_cmd(CMD58, 0) == 0) {    /* READ_OCR */
                for (n = 4; n; n--) *ptr++ = spi_recv();
                res = RES_OK;
            }
            break;

        case MMC_GET_SDSTAT :    /* Receive SD status as a data block (64 bytes) */
            if (send_cmd(ACMD13, 0) == 0) {    /* SD_STATUS */
                spi_recv();
                if (rcvr_datablock(ptr, 64))
                    res = RES_OK;
            }
            break;

        default:
            res = RES_PARERR;
    }

    release_spi();

    return res;
}
