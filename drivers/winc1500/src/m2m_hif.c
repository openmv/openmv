/*******************************************************************************
  File Name:
    m2m_hif.c

  Summary:
    This module contains M2M host interface API implementations.

  Description:
    This module contains M2M host interface API implementations.
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

#include "nm_common.h"
#include "nmbus.h"
#include "nm_bsp.h"
#include "m2m_hif.h"
#include "m2m_types.h"
#include "nmasic.h"
#include "m2m_periph.h"
#include "osal/osal.h"

#define NMI_AHB_DATA_MEM_BASE  0x30000
#define NMI_AHB_SHARE_MEM_BASE 0xd0000

#define WIFI_HOST_RCV_CTRL_0    (0x1070)
#define WIFI_HOST_RCV_CTRL_1    (0x1084)
#define WIFI_HOST_RCV_CTRL_2    (0x1078)
#define WIFI_HOST_RCV_CTRL_3    (0x106c)
#define WIFI_HOST_RCV_CTRL_4    (0x150400)
#define WIFI_HOST_RCV_CTRL_5    (0x1088)

static OSAL_SEM_HANDLE_TYPE hifSemaphore;

typedef struct {
    uint8_t u8ChipMode;
    uint8_t u8ChipSleep;
    uint8_t u8HifRXDone;
    uint8_t u8Interrupt;
    uint32_t u32RxAddr;
    uint32_t u32RxSize;
    tpfHifCallBack pfWifiCb;
    tpfHifCallBack pfIpCb;
    tpfHifCallBack pfOtaCb;
    tpfHifCallBack pfSigmaCb;
    tpfHifCallBack pfHifCb;
    tpfHifCallBack pfCryptoCb;
    tpfHifCallBack pfSslCb;
}tstrHifContext;

volatile tstrHifContext gstrHifCxt;

static int8_t hif_set_rx_done(void)
{
    uint32_t reg;
    int8_t ret = M2M_SUCCESS;

    gstrHifCxt.u8HifRXDone = 0;
    ret = nm_read_reg_with_ret(WIFI_HOST_RCV_CTRL_0,&reg);
    if(ret != M2M_SUCCESS)goto ERR1;
    /* Set RX Done */
    reg |= NBIT1;
    ret = nm_write_reg(WIFI_HOST_RCV_CTRL_0,reg);
    if(ret != M2M_SUCCESS)goto ERR1;
ERR1:
    return ret;
}
/**
*   @fn         static void m2m_hif_cb(uint8_t u8OpCode, uint16_t u16DataSize, uint32_t u32Addr)
*   @brief      WiFi call back function
*   @param [in] u8OpCode
*                   HIF Opcode type.
*   @param [in] u16DataSize
*                   HIF data length.
*   @param [in] u32Addr
*                   HIF address.
*   @author
*   @date
*   @version    1.0
*/
static void m2m_hif_cb(uint8_t u8OpCode, uint16_t u16DataSize, uint32_t u32Addr)
{
}
/**
*   @fn     int8_t hif_chip_wake(void);
*   @brief  To Wakeup the chip.
*   @return     The function shall return ZERO for successful operation and a negative value otherwise.
*/

int8_t hif_chip_wake(void)
{
    int8_t ret = M2M_SUCCESS;
    if(gstrHifCxt.u8HifRXDone)
    {
        /*chip already wake for the rx not done no need to send wake request*/
        return ret;
    }
    if(gstrHifCxt.u8ChipSleep == 0)
    {
        if(gstrHifCxt.u8ChipMode != M2M_NO_PS)
        {
            ret = chip_wake();
            if(ret != M2M_SUCCESS)goto ERR1;
        }
        else
        {
        }
    }
    gstrHifCxt.u8ChipSleep++;
ERR1:
    return ret;
}
/*!
@fn \
    void hif_set_sleep_mode(uint8_t u8Pstype);

@brief
    Set the sleep mode of the HIF layer.

@param [in] u8Pstype
                Sleep mode.

@return
    The function SHALL return 0 for success and a negative value otherwise.
*/

void hif_set_sleep_mode(uint8_t u8Pstype)
{
    gstrHifCxt.u8ChipMode = u8Pstype;
}
/*!
@fn \
    uint8_t hif_get_sleep_mode(void);

@brief
    Get the sleep mode of the HIF layer.

@return
    The function SHALL return the sleep mode of the HIF layer.
*/

uint8_t hif_get_sleep_mode(void)
{
    return gstrHifCxt.u8ChipMode;
}

/**
*   @fn     int8_t hif_chip_sleep_sc(void);
*   @brief  To clear the chip sleep but keep the chip sleep
*    @return        The function shall return ZERO for successful operation and a negative value otherwise.
*/

int8_t hif_chip_sleep_sc(void)
{
    if(gstrHifCxt.u8ChipSleep >= 1)
    {
        gstrHifCxt.u8ChipSleep--;
    }
    return M2M_SUCCESS;
}
/**
*   @fn     int8_t hif_chip_sleep(void);
*   @brief  To make the chip sleep.
*    @return        The function shall return ZERO for successful operation and a negative value otherwise.
*/

int8_t hif_chip_sleep(void)
{
    int8_t ret = M2M_SUCCESS;

    if(gstrHifCxt.u8ChipSleep >= 1)
    {
        gstrHifCxt.u8ChipSleep--;
    }

    if(gstrHifCxt.u8ChipSleep == 0)
    {
        if(gstrHifCxt.u8ChipMode != M2M_NO_PS)
        {
            ret = chip_sleep();
            if(ret != M2M_SUCCESS)goto ERR1;
        }
        else
        {
        }
    }
ERR1:
    return ret;
}
/**
*   @fn     int8_t hif_init(void * arg);
*   @brief  To initialize HIF layer.
*   @param [in] arg
*               Pointer to the arguments.
*   @return     The function shall return ZERO for successful operation and a negative value otherwise.
*/

int8_t hif_init(void * arg)
{
    memset((uint8_t*)&gstrHifCxt,0,sizeof(tstrHifContext));

    if (OSAL_RESULT_TRUE != OSAL_SEM_Create(&hifSemaphore, OSAL_SEM_TYPE_BINARY, 1, 1))
        return M2M_ERR_INIT;

    hif_register_cb(M2M_REQ_GROUP_HIF,m2m_hif_cb);
    return M2M_SUCCESS;
}
/**
*   @fn     int8_t hif_deinit(void * arg);
*   @brief  To De-initialize HIF layer.
*   @param [in] arg
*               Pointer to the arguments.
*   @return     The function shall return ZERO for successful operation and a negative value otherwise.
*/
int8_t hif_deinit(void * arg)
{
    int8_t ret = M2M_SUCCESS;
    ret = hif_chip_wake();
    memset((uint8_t*)&gstrHifCxt,0,sizeof(tstrHifContext));
    return ret;
}
/**
*   @fn     int8_t hif_send(uint8_t u8Gid,uint8_t u8Opcode,uint8_t *pu8CtrlBuf,uint16_t u16CtrlBufSize,
                       uint8_t *pu8DataBuf,uint16_t u16DataSize, uint16_t u16DataOffset)
*   @brief  Send packet using host interface.

*   @param [in] u8Gid
*               Group ID.
*   @param [in] u8Opcode
*               Operation ID.
*   @param [in] pu8CtrlBuf
*               Pointer to the Control buffer.
*   @param [in] u16CtrlBufSize
                Control buffer size.
*   @param [in] u16DataOffset
                Packet Data offset.
*   @param [in] pu8DataBuf
*               Packet buffer Allocated by the caller.
*   @param [in] u16DataSize
                Packet buffer size (including the HIF header).
*    @return        The function shall return ZERO for successful operation and a negative value otherwise.
*/

int8_t hif_send(uint8_t u8Gid,uint8_t u8Opcode,uint8_t *pu8CtrlBuf,uint16_t u16CtrlBufSize,
               uint8_t *pu8DataBuf,uint16_t u16DataSize, uint16_t u16DataOffset)
{
    int8_t     ret = M2M_ERR_SEND;
    tstrHifHdr strHif;

    while (OSAL_RESULT_FALSE == OSAL_SEM_Pend(&hifSemaphore, OSAL_WAIT_FOREVER))
    {
    }

    strHif.u8Opcode     = u8Opcode&(~NBIT7);
    strHif.u8Gid        = u8Gid;
    strHif.u16Length    = M2M_HIF_HDR_OFFSET;
    if(pu8DataBuf != NULL)
    {
        strHif.u16Length += u16DataOffset + u16DataSize;
    }
    else
    {
        strHif.u16Length += u16CtrlBufSize;
    }
    if (strHif.u16Length <= M2M_HIF_MAX_PACKET_SIZE)
    {
    ret = hif_chip_wake();
        if(ret == M2M_SUCCESS)
        {
            volatile uint32_t reg, dma_addr = 0;
            volatile uint16_t cnt = 0;

            reg = 0UL;
            reg |= (uint32_t)u8Gid;
            reg |= ((uint32_t)u8Opcode<<8);
            reg |= ((uint32_t)strHif.u16Length<<16);
            ret = nm_write_reg(NMI_STATE_REG,reg);
            if(M2M_SUCCESS != ret) goto ERR1;

            reg = 0UL;
            reg |= NBIT1;
            ret = nm_write_reg(WIFI_HOST_RCV_CTRL_2, reg);
            if(M2M_SUCCESS != ret) goto ERR1;

            dma_addr = 0;

            for(cnt = 0; cnt < 1000; cnt ++)
            {
                ret = nm_read_reg_with_ret(WIFI_HOST_RCV_CTRL_2,(uint32_t *)&reg);
                if(ret != M2M_SUCCESS) break;
                /*
                 * If it takes too long to get a response, the slow down to
                 * avoid back-to-back register read operations.
                 */
                if(cnt >= 500) {
                    if(cnt < 501) {
                        M2M_INFO("Slowing down...\r\n");
                    }
                    nm_sleep(1);
                }
                if (!(reg & NBIT1))
                {
                    ret = nm_read_reg_with_ret(WIFI_HOST_RCV_CTRL_4,(uint32_t *)&dma_addr);
                    if(ret != M2M_SUCCESS) {
                        /*in case of read error clear the DMA address and return error*/
                        dma_addr = 0;
                        goto ERR1;
                    }
                    /*in case of success break */
                    break;
                }
            }

            if (dma_addr != 0)
            {
                volatile uint32_t u32CurrAddr;
                u32CurrAddr = dma_addr;
                strHif.u16Length=NM_BSP_B_L_16(strHif.u16Length);
                ret = nm_write_block(u32CurrAddr, (uint8_t*)&strHif, M2M_HIF_HDR_OFFSET);
                if(M2M_SUCCESS != ret) goto ERR1;
                u32CurrAddr += M2M_HIF_HDR_OFFSET;
                if(pu8CtrlBuf != NULL)
                {
                    ret = nm_write_block(u32CurrAddr, pu8CtrlBuf, u16CtrlBufSize);
                    if(M2M_SUCCESS != ret) goto ERR1;
                    u32CurrAddr += u16CtrlBufSize;
                }
                if(pu8DataBuf != NULL)
                {
                    u32CurrAddr += (u16DataOffset - u16CtrlBufSize);
                    ret = nm_write_block(u32CurrAddr, pu8DataBuf, u16DataSize);
                    if(M2M_SUCCESS != ret) goto ERR1;
                    u32CurrAddr += u16DataSize;
                }

                reg = dma_addr << 2;
                reg |= NBIT1;
                ret = nm_write_reg(WIFI_HOST_RCV_CTRL_3, reg);
                if(M2M_SUCCESS != ret) goto ERR1;
            }
            else
            {
                ret = hif_chip_sleep();
                M2M_DBG("Failed to alloc rx size %d\r\n", ret);
                ret = M2M_ERR_MEM_ALLOC;
                goto ERR2;
            }

        }
        else
        {
            M2M_ERR("(HIF)Failed to wakeup the chip\r\n");
            goto ERR2;
        }
    }
    else
    {
        M2M_ERR("HIF message length (%d) exceeds max length (%d)\r\n",strHif.u16Length, M2M_HIF_MAX_PACKET_SIZE);
        ret = M2M_ERR_SEND;
        goto ERR2;
    }
    /*actual sleep ret = M2M_SUCCESS*/
    ret = hif_chip_sleep();
    OSAL_SEM_Post(&hifSemaphore);
    return ret;
ERR1:
    /*reset the count but no actual sleep as it already bus error*/
    hif_chip_sleep_sc();
ERR2:
    OSAL_SEM_Post(&hifSemaphore);
    /*logical error*/
    return ret;
}
/**
*   @fn     hif_isr
*   @brief  Host interface interrupt service routine
*   @author M. Abdelmawla
*   @date   15 July 2012
*   @return 1 in case of interrupt received else 0 will be returned
*   @version    1.0
*/
static int8_t hif_isr(void)
{
    int8_t ret = M2M_SUCCESS;
    uint32_t reg;
    tstrHifHdr strHif;

    while (OSAL_RESULT_FALSE == OSAL_SEM_Pend(&hifSemaphore, OSAL_WAIT_FOREVER))
    {
    }

    ret = nm_read_reg_with_ret(WIFI_HOST_RCV_CTRL_0, &reg);
    if(M2M_SUCCESS == ret)
    {
        if(reg & 0x1)   /* New interrupt has been received */
        {
            uint16_t size;

            /*Clearing RX interrupt*/
            reg &= ~NBIT0;
            ret = nm_write_reg(WIFI_HOST_RCV_CTRL_0,reg);
            if(ret != M2M_SUCCESS)goto ERR1;
            gstrHifCxt.u8HifRXDone = 1;
            size = (uint16_t)((reg >> 2) & 0xfff);
            if (size > 0) {
                uint32_t address = 0;
                /**
                start bus transfer
                **/
                ret = nm_read_reg_with_ret(WIFI_HOST_RCV_CTRL_1, &address);
                if(M2M_SUCCESS != ret)
                {
                    M2M_ERR("(hif) WIFI_HOST_RCV_CTRL_1 bus fail\r\n");
                    goto ERR1;
                }
                gstrHifCxt.u32RxAddr = address;
                gstrHifCxt.u32RxSize = size;
                ret = nm_read_block(address, (uint8_t*)&strHif, sizeof(tstrHifHdr));
                strHif.u16Length = NM_BSP_B_L_16(strHif.u16Length);
                if(M2M_SUCCESS != ret)
                {
                    M2M_ERR("(hif) address bus fail\r\n");
                    goto ERR1;
                }
                if(strHif.u16Length != size)
                {
                    if((size - strHif.u16Length) > 4)
                    {
                        M2M_ERR("(hif) Corrupted packet Size = %u <L = %u, G = %u, OP = %02X>\r\n",
                            size, strHif.u16Length, strHif.u8Gid, strHif.u8Opcode);
                        ret = M2M_ERR_BUS_FAIL;
                        goto ERR1;
                    }
                }

                OSAL_SEM_Post(&hifSemaphore);

                if(M2M_REQ_GROUP_WIFI == strHif.u8Gid)
                {
                    if(gstrHifCxt.pfWifiCb)
                        gstrHifCxt.pfWifiCb(strHif.u8Opcode,strHif.u16Length - M2M_HIF_HDR_OFFSET, address + M2M_HIF_HDR_OFFSET);
                    else
                        M2M_ERR("WIFI callback is not registered\r\n");
                }
                else if(M2M_REQ_GROUP_IP == strHif.u8Gid)
                {
                    if(gstrHifCxt.pfIpCb)
                        gstrHifCxt.pfIpCb(strHif.u8Opcode,strHif.u16Length - M2M_HIF_HDR_OFFSET, address + M2M_HIF_HDR_OFFSET);
                    else
                        M2M_ERR("Socket callback is not registered\r\n");
                }
                else if(M2M_REQ_GROUP_OTA == strHif.u8Gid)
                {
                    if(gstrHifCxt.pfOtaCb)
                        gstrHifCxt.pfOtaCb(strHif.u8Opcode,strHif.u16Length - M2M_HIF_HDR_OFFSET, address + M2M_HIF_HDR_OFFSET);
                    else
                        M2M_ERR("Ota callback is not registered\r\n");
                }
                else if(M2M_REQ_GROUP_CRYPTO == strHif.u8Gid)
                {
                    if(gstrHifCxt.pfCryptoCb)
                        gstrHifCxt.pfCryptoCb(strHif.u8Opcode,strHif.u16Length - M2M_HIF_HDR_OFFSET, address + M2M_HIF_HDR_OFFSET);
                    else
                        M2M_ERR("Crypto callback is not registered\r\n");
                }
                else if(M2M_REQ_GROUP_SIGMA == strHif.u8Gid)
                {
                    if(gstrHifCxt.pfSigmaCb)
                        gstrHifCxt.pfSigmaCb(strHif.u8Opcode,strHif.u16Length - M2M_HIF_HDR_OFFSET, address + M2M_HIF_HDR_OFFSET);
                    else
                        M2M_ERR("Sigma callback is not registered\r\n");
                }
                else if(M2M_REQ_GROUP_SSL == strHif.u8Gid)
                {
                    if(gstrHifCxt.pfSslCb)
                        gstrHifCxt.pfSslCb(strHif.u8Opcode,strHif.u16Length - M2M_HIF_HDR_OFFSET, address + M2M_HIF_HDR_OFFSET);
                    else
                        M2M_ERR("SSL callback is not registered\r\n");
                }
                else
                {
                    M2M_ERR("(hif) invalid group ID\r\n");
                    return M2M_ERR_BUS_FAIL;
                }
                if(gstrHifCxt.u8HifRXDone)
                {
                    M2M_ERR("(hif) host app didn't set RX Done <%u><%X>\r\n", strHif.u8Gid, strHif.u8Opcode);
                    ret = hif_set_rx_done();
                    if(ret != M2M_SUCCESS)
                        return ret;
                }

                return M2M_SUCCESS;
            }
            else
            {
                M2M_ERR("(hif) Wrong size\r\n");
                ret = M2M_ERR_RCV;
                goto ERR1;
            }
        }
        else
        {
            M2M_ERR("(hif) False interrupt %lx\r\n",reg);
            goto ERR1;
        }
    }
    else
    {
        M2M_ERR("(hif) Failed to read interrupt reg\r\n");
        goto ERR1;
    }

ERR1:
    OSAL_SEM_Post(&hifSemaphore);
    return ret;
}

/**
*   @fn     hif_handle_isr(void)
*   @brief  Handle interrupt received from NMC1500 firmware.
*   @return     The function SHALL return 0 for success and a negative value otherwise.
*/

int8_t hif_handle_isr(void)
{
    int8_t ret = M2M_SUCCESS;

    ret = hif_isr();
    if (M2M_SUCCESS != ret)
    {
        M2M_ERR("(hif) Fail to handle interrupt %d try again..\r\n",ret);
    }

    return ret;
}
/*
*   @fn     hif_receive
*   @brief  Host interface interrupt service routine
*   @param [in] u32Addr
*               Receive start address
*   @param [out]    pu8Buf
*               Pointer to receive buffer. Allocated by the caller
*   @param [in] u16Sz
*               Receive buffer size
*   @param [in] isDone
*               If you don't need any more packets send True otherwise send false
*   @return     The function shall return ZERO for successful operation and a negative value otherwise.
*/
int8_t hif_receive(uint32_t u32Addr, uint8_t *pu8Buf, uint16_t u16Sz, uint8_t isDone)
{
    int8_t ret = M2M_SUCCESS;
    if((u32Addr == 0)||(pu8Buf == NULL) || (u16Sz == 0))
    {
        if(isDone)
        {
            /* set RX done */
            ret = hif_set_rx_done();
        }
        else
        {
            ret = M2M_ERR_FAIL;
            M2M_ERR(" hif_receive: Invalid argument\r\n");
        }
        goto ERR1;
    }

    if(u16Sz > gstrHifCxt.u32RxSize)
    {
        ret = M2M_ERR_FAIL;
        M2M_ERR("APP Requested Size is larger than the received buffer size <%u><%lu>\r\n", u16Sz, gstrHifCxt.u32RxSize);
        goto ERR1;
    }
    if((u32Addr < gstrHifCxt.u32RxAddr)||((u32Addr + u16Sz)>(gstrHifCxt.u32RxAddr + gstrHifCxt.u32RxSize)))
    {
        ret = M2M_ERR_FAIL;
        M2M_ERR("APP Requested Address beyond the received buffer address and length\r\n");
        goto ERR1;
    }

    /* Receive the payload */
    ret = nm_read_block(u32Addr, pu8Buf, u16Sz);
    if(ret != M2M_SUCCESS)goto ERR1;

    /* check if this is the last packet */
    if((((gstrHifCxt.u32RxAddr + gstrHifCxt.u32RxSize) - (u32Addr + u16Sz)) <= 0) || isDone)
    {
        /* set RX done */
        ret = hif_set_rx_done();
    }

ERR1:
    return ret;
}

/**
*   @fn     hif_register_cb
*   @brief  To set Callback function for every component
*   @param [in] u8Grp
*               Group to which the Callback function should be set.
*   @param [in] fn
*               function to be set
*   @return     The function shall return ZERO for successful operation and a negative value otherwise.
*/

int8_t hif_register_cb(uint8_t u8Grp,tpfHifCallBack fn)
{
    int8_t ret = M2M_SUCCESS;
    switch(u8Grp)
    {
        case M2M_REQ_GROUP_IP:
            gstrHifCxt.pfIpCb = fn;
            break;
        case M2M_REQ_GROUP_WIFI:
            gstrHifCxt.pfWifiCb = fn;
            break;
        case M2M_REQ_GROUP_OTA:
            gstrHifCxt.pfOtaCb = fn;
            break;
        case M2M_REQ_GROUP_HIF:
            gstrHifCxt.pfHifCb = fn;
            break;
        case M2M_REQ_GROUP_CRYPTO:
            gstrHifCxt.pfCryptoCb = fn;
            break;
        case M2M_REQ_GROUP_SIGMA:
            gstrHifCxt.pfSigmaCb = fn;
            break;
        case M2M_REQ_GROUP_SSL:
            gstrHifCxt.pfSslCb = fn;
            break;
        default:
            M2M_ERR("GRp ? %d\r\n", u8Grp);
            ret = M2M_ERR_FAIL;
            break;
    }
    return ret;
}

//DOM-IGNORE-END
