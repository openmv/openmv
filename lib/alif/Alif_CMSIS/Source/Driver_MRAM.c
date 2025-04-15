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
 * @file     Driver_MRAM.c
 * @author   Tanay Rami
 * @email    tanay@alifsemi.com
 * @version  V1.0.0
 * @date     01-June-2023
 * @brief    ARM CMSIS-Driver for MRAM(On-Chip NVM(Non-Volatile Memory)).
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

/* System Includes */
#include "string.h"

/* Project Includes */
#include "Driver_MRAM_Private.h"
#include "mram.h"

#if !(RTE_MRAM)
#error "MRAM is not enabled in the RTE_Device.h"
#endif

#if (defined(RTE_Drivers_MRAM) && !RTE_MRAM)
#error "MRAM not configured in RTE_Device.h!"
#endif

#define ARM_MRAM_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0) /* driver version */

/* MRAM Device Resources. */
static MRAM_RESOURCES mram =
{
    .state  = {0}
};

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_MRAM_API_VERSION,
    ARM_MRAM_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_MRAM_CAPABILITIES DriverCapabilities = {
    0, /* data_width = 0:128-bit  */
    0  /* reserved (must be zero) */
};


/**
  \fn          ARM_DRIVER_VERSION MRAM_GetVersion(void)
  \brief       Get driver version.
  \return      \ref ARM_DRIVER_VERSION
*/
static ARM_DRIVER_VERSION MRAM_GetVersion(void)
{
    return DriverVersion;
}

/**
  \fn          ARM_MRAM_CAPABILITIES MRAM_GetCapabilities(void)
  \brief       Get driver capabilities.
  \return      \ref ARM_MRAM_CAPABILITIES
*/
static ARM_MRAM_CAPABILITIES MRAM_GetCapabilities(void)
{
    return DriverCapabilities;
}

/**
  \fn          int32_t MRAM_Initialize(void)
  \brief       Initialize the MRAM Interface.
  \return      \ref execution_status
*/
static int32_t MRAM_Initialize(void)
{
    if (mram.state.initialized == 1)
        return ARM_DRIVER_OK;

    /* Validate User MRAM size. */
    if (MRAM_USER_SIZE > MRAM_SIZE)
        return ARM_DRIVER_ERROR;

    /* set the state as initialized. */
    mram.state.initialized = 1;

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t MRAM_Uninitialize(void)
  \brief       De-initialize the MRAM Interface.
  \return      \ref execution_status
*/
static int32_t MRAM_Uninitialize(void)
{
    mram.state.initialized = 0;

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t MRAM_PowerControl(ARM_POWER_STATE  state)
  \brief       Control the MRAM interface power.
  \param[in]   state  Power state
  \return      \ref execution_status
*/
static int32_t MRAM_PowerControl(ARM_POWER_STATE  state)
{
    switch (state)
    {
    case ARM_POWER_OFF:
        /* Reset the power state. */
        mram.state.powered = 0;
        break;

    case ARM_POWER_LOW:
        break;

    case ARM_POWER_FULL:
        if (mram.state.initialized == 0U)
          return ARM_DRIVER_ERROR;

        if (mram.state.powered)
          break;

        /* Set the state as powered */
        mram.state.powered = 1;
        break;

    default:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t MRAM_ReadData(uint32_t addr, void *data, uint32_t cnt)
  \brief       Read data from MRAM.
  \param[in]   addr   MRAM address-offset.
  \param[out]  data   Pointer to a buffer storing the data read from MRAM.
  \param[in]   cnt    Number of data items to read.
  \return      number of data items read or \ref execution_status
*/
static int32_t MRAM_ReadData(uint32_t addr, void *data, uint32_t cnt)
{
    if(mram.state.powered == 0U)
        return ARM_DRIVER_ERROR;

    /* validate MRAM address and count. */
    if(addr > MRAM_USER_SIZE)
        return ARM_DRIVER_ERROR_PARAMETER;

    if((addr + cnt) > MRAM_USER_SIZE)
        return ARM_DRIVER_ERROR_PARAMETER;

    /* addr is MRAM address-offset,
     * so add MRAM Base-address to it. */
    addr += MRAM_BASE;

    /* read from MRAM */
    mram_read(data, (void*)addr, cnt);

    return cnt;
}

/**
  \fn          void MRAM_write(uint8_t *p_dst, const uint8_t *p_src)
  \brief       write 128-bit data from source to destination(MRAM).
  \Note        It is CRITICAL that this code running on the Application core
                contains the following:
                 - H/W Limitations with Rev A silicon:
                    The code that is performing the MRAM writes
                     CANNOT be located and executing from MRAM.
                     I.E. the code must be executing from SRAM.
                    No code can be running from MRAM when you want to
                     write to the MRAM.
                 - The code should disable interrupts around the code
                    where the MRAM writes and cache flush occur.
                 - The code should include the function / Intrinsic “__DSB()”
                    to make sure all the data writes are flushed out
                    and have occurred.
  \param[out]  p_dst   Pointer to destination address.
  \param[in]   p_src   Pointer to source address.
  \return      none
*/
static void MRAM_write(uint8_t *p_dst, const uint8_t *p_src)
{
    /* write 128bit to MRAM. */
    mram_write_128bit(p_dst, p_src);

    /* clean/flush Dcache. */
    RTSS_CleanDCache_by_Addr((uint32_t *)p_dst, MRAM_SECTOR_SIZE);
}

/**
  \fn          int32_t MRAM_ProgramData(uint32_t addr, const void *data, uint32_t cnt)
  \brief       Program data to MRAM.
  \param[in]   addr   MRAM address-offset.
  \param[in]   data   Pointer to a buffer containing the data to be programmed to MRAM.
  \param[in]   cnt    Number of data items to program.
  \return      number of data items programmed or \ref execution_status
*/
static int32_t MRAM_ProgramData(uint32_t addr, const void *data, uint32_t cnt)
{
    if(mram.state.powered == 0U)
        return ARM_DRIVER_ERROR;

    /* validate MRAM address and count. */
    if(addr > MRAM_USER_SIZE)
        return ARM_DRIVER_ERROR_PARAMETER;

    if((addr + cnt) > MRAM_USER_SIZE)
        return ARM_DRIVER_ERROR_PARAMETER;

    /* addr is MRAM address-offset,
     * so add MRAM Base-address to it. */
    addr += MRAM_BASE;

    /* check address with aligned to 16-Bytes.*/
    uint32_t aligned_addr   = addr & MRAM_ADDR_ALIGN_MASK;
    uint8_t *p_aligned_addr = (uint8_t *) aligned_addr;
    uint8_t *p_data         = (uint8_t *) data;
    uint32_t count          = cnt;

    /* use temporary buffer to store data in case of unaligned memory.*/
    uint8_t temp_buff[MRAM_SECTOR_SIZE] = {0}; /* 128-Bit */

    /* is MRAM address aligned to 16-Byte? */
    if(addr != aligned_addr)
    {
        /* unaligned MRAM address:
         *  - make it to nearest aligned 16-Byte address,
         *     by writing only unaligned bytes.
         */

        uint8_t offset           = addr & (~MRAM_ADDR_ALIGN_MASK);
        uint8_t unaligned_bytes  = MRAM_SECTOR_SIZE - offset;

        /* is unaligned bytes more than remaining count? */
        if(unaligned_bytes > count)
        {
            /* then take only remaining count. */
            unaligned_bytes = count;
        }

        /* copy original one sector data from MRAM address to buffer. */
        memcpy(temp_buff, p_aligned_addr, MRAM_SECTOR_SIZE);

        /* overwrite buffer with new data as per offset and unaligned bytes. */
        memcpy(temp_buff + offset, p_data, unaligned_bytes);

        /* now, copy 128bit from buffer to MRAM. */
        MRAM_write(p_aligned_addr, temp_buff);

        p_aligned_addr += MRAM_SECTOR_SIZE;
        p_data         += unaligned_bytes;
        count          -= unaligned_bytes;
    }

    uint32_t sector_cnt    = count / MRAM_SECTOR_SIZE;
    uint8_t unaligned_cnt  = count % MRAM_SECTOR_SIZE;

    /* program 128-bit to absolute sector. */
    while(sector_cnt--)
    {
        /* as MRAM address is 16-byte aligned,
         * directly copy 128bit from source-data to MRAM. */
        MRAM_write(p_aligned_addr, p_data);

        p_aligned_addr += MRAM_SECTOR_SIZE;
        p_data         += MRAM_SECTOR_SIZE;
    }

    /* program remaining unaligned data. */
    if(unaligned_cnt)
    {
        /* copy original one sector data from MRAM address to buffer.*/
        memcpy(temp_buff, p_aligned_addr, MRAM_SECTOR_SIZE);

        /* overwrite buffer with new data as per remaining unaligned count.*/
        memcpy(temp_buff, p_data, unaligned_cnt);

        /* now, copy 128bit from buffer to MRAM. */
        MRAM_write(p_aligned_addr, temp_buff);
    }

    return cnt;
}

/**
  \fn          int32_t MRAM_EraseSector(uint32_t addr)
  \brief       Erase MRAM Sector.
  \param[in]   addr   MRAM Sector address-offset.
  \return      \ref execution_status
*/
static int32_t MRAM_EraseSector(uint32_t addr)
{
    /* Added to fix Warning:
     *  unused parameter [-Wunused-parameter] */
    ARG_UNUSED(addr);

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t MRAM_EraseChip(void)
  \brief       Erase complete MRAM.
               Optional function for faster full chip erase.
  \return      \ref execution_status
*/
static int32_t MRAM_EraseChip(void)
{
    return ARM_DRIVER_OK;
}
// End MRAM Interface


/* MRAM Driver Instance */
#if (RTE_MRAM)

/* MRAM Driver Control Block */
extern ARM_DRIVER_MRAM Driver_MRAM;
ARM_DRIVER_MRAM Driver_MRAM =
{
    MRAM_GetVersion,
    MRAM_GetCapabilities,
    MRAM_Initialize,
    MRAM_Uninitialize,
    MRAM_PowerControl,
    MRAM_ReadData,
    MRAM_ProgramData,
    MRAM_EraseSector,
    MRAM_EraseChip
};
#endif /* RTE_MRAM */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
