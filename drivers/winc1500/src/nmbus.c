/*******************************************************************************
  This module contains WINC1500 bus APIs implementation.

  File Name:
    nmbus.c

  Summary:
    This module contains WINC1500 bus APIs implementation.

  Description:
    This module contains WINC1500 bus APIs implementation.
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

#include "nmbus.h"
#include "nm_bus_wrapper.h"
#include "nmspi.h"

#define MAX_TRX_CFG_SZ      8

// OpenMV: the bus capabilities and the type come from the bus wrapper, which is
// where the Harmony build would declare them too.

/*
*   OpenMV: the Harmony build stubs these out here because its WDRV layer owns
*   the SPI. Ours is a real bus wrapper, so use it instead of shadowing it.
*/

/**
*   @fn     nm_bus_iface_init
*   @brief  Initialize bus interface
*   @return M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*   @author M. Abdelmawla
*   @date   11 July 2012
*   @version    1.0
*/
int8_t nm_bus_iface_init(void *pvInitVal)
{
    int8_t ret = M2M_SUCCESS;
    ret = nm_bus_init(pvInitVal);
    return ret;
}

/**
*   @fn     nm_bus_iface_deinit
*   @brief  Deinitialize bus interface
*   @return M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*   @author Samer Sarhan
*   @date   07 April 2014
*   @version    1.0
*/
int8_t nm_bus_iface_deinit(void)
{
    int8_t ret = M2M_SUCCESS;
    ret = nm_bus_deinit();

    return ret;
}

/**
*   @fn     nm_bus_reset
*   @brief  reset bus interface
*   @return M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*   @version    1.0
*/
int8_t nm_bus_reset(void)
{
    return nm_spi_reset();
}

/**
*   @fn     nm_bus_iface_reconfigure
*   @brief  reconfigure bus interface
*   @return M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*   @author Viswanathan Murugesan
*   @date   22 Oct 2014
*   @version    1.0
*/
int8_t nm_bus_iface_reconfigure(void *ptr)
{
    return M2M_SUCCESS;
}

/*
*   @fn     nm_read_reg
*   @brief  Read register
*   @param [in] u32Addr
*               Register address
*   @return Register value
*   @author M. Abdelmawla
*   @date   11 July 2012
*   @version    1.0
*/
uint32_t nm_read_reg(uint32_t u32Addr)
{
    return nm_spi_read_reg(u32Addr);
}

/*
*   @fn     nm_read_reg_with_ret
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
int8_t nm_read_reg_with_ret(uint32_t u32Addr, uint32_t* pu32RetVal)
{
    return nm_spi_read_reg_with_ret(u32Addr,pu32RetVal);
}

/*
*   @fn     nm_write_reg
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
int8_t nm_write_reg(uint32_t u32Addr, uint32_t u32Val)
{
    return nm_spi_write_reg(u32Addr,u32Val);
}

static int8_t p_nm_read_block(uint32_t u32Addr, uint8_t *puBuf, uint16_t u16Sz)
{
    return nm_spi_read_block(u32Addr,puBuf,u16Sz);
}
/*
*   @fn     nm_read_block
*   @brief  Read block of data
*   @param [in] u32Addr
*               Start address
*   @param [out]    puBuf
*               Pointer to a buffer used to return the read data
*   @param [in] u32Sz
*               Number of bytes to read. The buffer size must be >= u32Sz
*   @return M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*   @author M. Abdelmawla
*   @date   11 July 2012
*   @version    1.0
*/
int8_t nm_read_block(uint32_t u32Addr, uint8_t *puBuf, uint32_t u32Sz)
{
    uint16_t u16MaxTrxSz = egstrNmBusCapabilities.u16MaxTrxSz - MAX_TRX_CFG_SZ;
    uint32_t off = 0;
    int8_t s8Ret = M2M_SUCCESS;

    for(;;)
    {
        if(u32Sz <= u16MaxTrxSz)
        {
            s8Ret += p_nm_read_block(u32Addr, &puBuf[off], (uint16_t)u32Sz);
            break;
        }
        else
        {
            s8Ret += p_nm_read_block(u32Addr, &puBuf[off], u16MaxTrxSz);
            if(M2M_SUCCESS != s8Ret) break;
            u32Sz -= u16MaxTrxSz;
            off += u16MaxTrxSz;
            u32Addr += u16MaxTrxSz;
        }
    }

    return s8Ret;
}

static int8_t p_nm_write_block(uint32_t u32Addr, uint8_t *puBuf, uint16_t u16Sz)
{
    return nm_spi_write_block(u32Addr,puBuf,u16Sz);
}
/**
*   @fn     nm_write_block
*   @brief  Write block of data
*   @param [in] u32Addr
*               Start address
*   @param [in] puBuf
*               Pointer to the buffer holding the data to be written
*   @param [in] u32Sz
*               Number of bytes to write. The buffer size must be >= u32Sz
*   @return M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*   @author M. Abdelmawla
*   @date   11 July 2012
*   @version    1.0
*/
int8_t nm_write_block(uint32_t u32Addr, uint8_t *puBuf, uint32_t u32Sz)
{
    uint16_t u16MaxTrxSz = egstrNmBusCapabilities.u16MaxTrxSz - MAX_TRX_CFG_SZ;
    uint32_t off = 0;
    int8_t s8Ret = M2M_SUCCESS;

    for(;;)
    {
        if(u32Sz <= u16MaxTrxSz)
        {
            s8Ret += p_nm_write_block(u32Addr, &puBuf[off], (uint16_t)u32Sz);
            break;
        }
        else
        {
            s8Ret += p_nm_write_block(u32Addr, &puBuf[off], u16MaxTrxSz);
            if(M2M_SUCCESS != s8Ret) break;
            u32Sz -= u16MaxTrxSz;
            off += u16MaxTrxSz;
            u32Addr += u16MaxTrxSz;
        }
    }

    return s8Ret;
}

//DOM-IGNORE-END
