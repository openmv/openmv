/*******************************************************************************
  File Name:
    m2m_hif.h

  Summary:
    This module contains M2M host interface API definitions.

  Description:
    This module contains M2M host interface API definitions.
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
// DOM-IGNORE-END

#ifndef _M2M_HIF_
#define _M2M_HIF_

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
INCLUDES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

#include "nm_common.h"

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
MACROS
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

#define M2M_HIF_MAX_PACKET_SIZE      (1600 - 4)
/*!< Maximum size of the buffer could be transferred between Host and Firmware.
*/

#define M2M_HIF_HDR_OFFSET (sizeof(tstrHifHdr) + 4)

/**
*   @struct     tstrHifHdr
*   @brief      Structure to hold HIF header
*/
typedef struct
{
    uint8_t   u8Gid;        /*!< Group ID */
    uint8_t   u8Opcode; /*!< OP code */
    uint16_t  u16Length;    /*!< Payload length */
}tstrHifHdr;

#ifdef __cplusplus
     extern "C" {
#endif

/*!
@typedef typedef void (*tpfHifCallBack)(uint8_t u8OpCode, uint16_t u16DataSize, uint32_t u32Addr);
@brief  used to point to Wi-Fi call back function depend on Arduino project or other projects.
@param [in] u8OpCode
                HIF Opcode type.
@param [in] u16DataSize
                HIF data length.
@param [in] u32Addr
                HIF address.
@param [in] grp
                HIF group type.
*/
typedef void (*tpfHifCallBack)(uint8_t u8OpCode, uint16_t u16DataSize, uint32_t u32Addr);
/**
*   @fn         int8_t hif_init(void * arg);
*   @brief
                To initialize HIF layer.
*   @param [in] arg
*               Pointer to the arguments.
*   @return
                The function shall return ZERO for successful operation and a negative value otherwise.
*/
int8_t hif_init(void * arg);
/**
*   @fn         int8_t hif_deinit(void * arg);
*   @brief
                To Deinitialize HIF layer.
*   @param [in] arg
*               Pointer to the arguments.
*    @return
                The function shall return ZERO for successful operation and a negative value otherwise.
*/
int8_t hif_deinit(void * arg);
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
*    @return    The function shall return ZERO for successful operation and a negative value otherwise.
*/
int8_t hif_send(uint8_t u8Gid,uint8_t u8Opcode,uint8_t *pu8CtrlBuf,uint16_t u16CtrlBufSize,
                       uint8_t *pu8DataBuf,uint16_t u16DataSize, uint16_t u16DataOffset);
/**
*   @fn     hif_receive
*   @brief  Host interface interrupt service routine
*   @param [in] u32Addr
*               Receive start address
*   @param [out] pu8Buf
*               Pointer to receive buffer. Allocated by the caller
*   @param [in]  u16Sz
*               Receive buffer size
*   @param [in] isDone
*               If you don't need any more packets send True otherwise send false
*   @return
                The function shall return ZERO for successful operation and a negative value otherwise.
*/

int8_t hif_receive(uint32_t u32Addr, uint8_t *pu8Buf, uint16_t u16Sz, uint8_t isDone);
/**
*   @fn         hif_register_cb
*   @brief
                To set Callback function for every  Component.

*   @param [in] u8Grp
*               Group to which the Callback function should be set.

*   @param [in] fn
*               function to be set to the specified group.
*   @return
                The function shall return ZERO for successful operation and a negative value otherwise.
*/
int8_t hif_register_cb(uint8_t u8Grp,tpfHifCallBack fn);
/**
*   @fn     int8_t hif_chip_sleep(void);
*   @brief
                To make the chip sleep.
*   @return
                The function shall return ZERO for successful operation and a negative value otherwise.
*/
int8_t hif_chip_sleep(void);
/**
*   @fn     int8_t hif_chip_sleep_sc(void);
*   @brief
                To clear the chip count only but keep the chip awake
*   @return
                The function shall return ZERO for successful operation and a negative value otherwise.
*/
int8_t hif_chip_sleep_sc(void);
/**
*   @fn     int8_t hif_chip_wake(void);
*   @brief
            To Wakeup the chip.
*   @return
            The function shall return ZERO for successful operation and a negative value otherwise.
*/

int8_t hif_chip_wake(void);
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

void hif_set_sleep_mode(uint8_t u8Pstype);
/*!
@fn \
    uint8_t hif_get_sleep_mode(void);

@brief
    Get the sleep mode of the HIF layer.

@return
    The function SHALL return the sleep mode of the HIF layer.
*/

uint8_t hif_get_sleep_mode(void);

/**
*   @fn     hif_handle_isr(void)
*   @brief
            Handle interrupt received from NMC1500 firmware.
*   @return
            The function SHALL return 0 for success and a negative value otherwise.
*/
int8_t hif_handle_isr(void);

#ifdef __cplusplus
}
#endif
#endif
