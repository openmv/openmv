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
 * @file     Driver_FLASH.c
 * @author   Khushboo Singh
 * @email    khushboo.singh@alifsemi.com
 * @version  V1.0.0
 * @date     21-Oct-2022
 * @brief    ISSI FLASH Driver.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include "Driver_Flash.h"
#include "Driver_OSPI.h"
#include "RTE_Device.h"
#include "RTE_Components.h"
#include "IS25WX256.h"
#include CMSIS_device_header

#if !(RTE_ISSI_FLASH)
#error "ISSI Flash driver is not enabled in RTE_Device.h"
#endif

#if !(RTE_Drivers_ISSI_FLASH)
#error "ISSI Flash driver is not enabled in RTE_Components.h"
#endif

#define ARM_FLASH_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,0) /* driver version */


#ifndef DRIVER_FLASH_NUM
#define DRIVER_FLASH_NUM                                         1         /* Default driver number */
#endif

#ifndef DRIVER_OSPI_NUM
#define DRIVER_OSPI_NUM                                          RTE_ISSI_FLASH_OSPI_DRV_NUM
#endif

#ifndef DRIVER_OSPI_BUS_SPEED
#define DRIVER_OSPI_BUS_SPEED                                    RTE_ISSI_FLASH_OSPI_BUS_SPEED
#endif

/* SPI Data Flash Commands */
#define CMD_WRITE_VOL_CONFIG                                    (0x81U)
#define CMD_READ_DATA                                           (0x7CU)
#define CMD_READ_STATUS                                         (0x05U)
#define CMD_WRITE_ENABLE                                        (0x06U)
#define CMD_PAGE_PROGRAM                                        (0x84U)
#define CMD_READ_FLAG_STATUS                                    (0x70U)
#define CMD_SECTOR_ERASE                                        (0x21U)
#define CMD_BULK_ERASE                                          (0xC7U)

#define IO_MODE_ADDRESS                                         0x00000000U
#define WAIT_CYCLE_ADDRESS                                      0x00000001U
#define OCTAL_DDR_WO_DQS                                        (0xC7U)
#define OCTAL_DDR                                               (0xE7U)
#define DEFAULT_WAIT_CYCLES                                     RTE_ISSI_FLASH_WAIT_CYCLES

#define FLAG_STATUS_BUSY                                        0x80U
#define FLAG_STATUS_ERROR                                       0x30U

/* Flash Driver ISSI_Flags */
#define FLASH_INIT                                              (0x01U)
#define FLASH_POWER                                             (0x02U)


/* SPI Driver */
extern ARM_DRIVER_OSPI ARM_Driver_OSPI_(DRIVER_OSPI_NUM);
static ARM_DRIVER_OSPI * ptrOSPI = &ARM_Driver_OSPI_(DRIVER_OSPI_NUM);

/* SPI Bus Speed */
#define OSPI_BUS_SPEED                                           ((uint32_t)DRIVER_OSPI_BUS_SPEED)
#define OSPI_MAX_RX_COUNT                                        256

/* Flash Information */
ARM_FLASH_INFO ISSI_FlashInfo = {
    NULL,
    FLASH_ISSI_SECTOR_COUNT,
    FLASH_ISSI_SECTOR_SIZE,
    FLASH_ISSI_PAGE_SIZE,
    FLASH_ISSI_PROGRAM_UNIT,
    FLASH_ISSI_ERASED_VALUE,
#if (ARM_FLASH_API_VERSION > 0x201U)
    { 0U, 0U, 0U }
#endif
};

/* Flash Status */
static ARM_FLASH_STATUS ISSI_FlashStatus;

/* Flag to track the driver state */
static uint8_t ISSI_Flags;

/* Flag to monitor OSPI events */
static volatile uint32_t issi_event_flag;

/* Driver Version */
const ARM_DRIVER_VERSION DriverVersion = {
    ARM_FLASH_API_VERSION,
    ARM_FLASH_DRV_VERSION
};

/* Driver Capabilities */
const ARM_FLASH_CAPABILITIES DriverCapabilities = {
    0U,                                 /* event_ready */
    1U,                                 /* data_width = 0:8-bit, 1:16-bit, 2:32-bit */
    1U,                                 /* erase_chip */
#if (ARM_FLASH_API_VERSION > 0x200U)
    0U                                  /* reserved */
#endif
};

/**
  \fn          void spi_callback_event(uint32_t event)
  \brief       Call back api called from OSPI.
  \return      none
**/
static void spi_callback_event(uint32_t event)
{
    issi_event_flag = event;
}

/**
  \fn          ARM_DRIVER_VERSION ARM_Flash_GetVersion (void)
  \brief       Get driver version.
  \return      \ref ARM_DRIVER_VERSION
**/
static ARM_DRIVER_VERSION ARM_Flash_GetVersion (void)
{
    return DriverVersion;
}

/**
  \fn          ARM_FLASH_CAPABILITIES ARM_Flash_GetCapabilities (void)
  \brief       Get driver capabilities.
  \return      \ref ARM_FLASH_CAPABILITIES
*/
static ARM_FLASH_CAPABILITIES ARM_Flash_GetCapabilities (void)
{
    return DriverCapabilities;
}

/**
  \fn          static int32_t ControlSlaveSelect(bool enable)
  \brief       Helper function to control the slave select line
  \param[in]   enable : True/False to enable or disable the SS line
  \return      \ref execution_status
*/
static int32_t ControlSlaveSelect(bool enable)
{
    int32_t status;
    uint32_t arg;

    if (enable)
    {
        arg = ARM_OSPI_SS_ACTIVE;
    }
    else
    {
        arg = ARM_OSPI_SS_INACTIVE;
    }

    do {
        status = ptrOSPI->Control(ARM_OSPI_CONTROL_SS, arg);
    } while (status == ARM_DRIVER_ERROR_BUSY);

    return status;
}

/**
  \fn          int32_t ReadStatusReg (uint32_t command, uint32_t *stat)
  \brief       Read status register to check the status of flash device.
  \param[in]   command : Status register/ Flag Status Register
  \return      \ref execution_status
**/
static int32_t ReadStatusReg (uint8_t command, uint8_t *stat)
{
    int32_t status = ARM_DRIVER_OK;
    uint32_t cmd[3];

    status = ptrOSPI->Control(ARM_OSPI_MODE_MASTER |
                    ARM_OSPI_DATA_BITS(8) |
                    ARM_OSPI_SS_MASTER_SW,
                    OSPI_BUS_SPEED);

    if (status != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Select Slave */
    status = ControlSlaveSelect(true);
    if (status != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Set command */
    cmd[0] = command;

    status = ptrOSPI->Control(ARM_OSPI_SET_ADDR_LENGTH_WAIT_CYCLE, (ARM_OSPI_ADDR_LENGTH_0_BITS << ARM_OSPI_ADDR_LENGTH_POS) | (8 << ARM_OSPI_WAIT_CYCLE_POS));
    if (status != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Send command and receive register value */
    status = ptrOSPI->Transfer (&cmd[0], &cmd[1], 2U);
    if (status != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    while (!issi_event_flag)
    {
         __WFE();
    }

    if (!(issi_event_flag & ARM_OSPI_EVENT_TRANSFER_COMPLETE))
    {
        ControlSlaveSelect(false);
        issi_event_flag = 0;
        return ARM_DRIVER_ERROR;
    }
    issi_event_flag = 0;

    *stat = (uint8_t) cmd[1];

    status = ControlSlaveSelect(false);

    return status;
}

/**
  \fn          int32_t SetWriteEnable (void)
  \brief       Set write enable before writing to flash.
  \param[in]   none
  \return      \ref execution_status
**/
static int32_t SetWriteEnable (void)
{
    int32_t status;
    uint32_t cmd;
    uint8_t val;

    status = ptrOSPI->Control(ARM_OSPI_MODE_MASTER |
             ARM_OSPI_DATA_BITS(8) |
             ARM_OSPI_SS_MASTER_SW,
             OSPI_BUS_SPEED);

    if (status != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    issi_event_flag = 0;

    /* Select slave */
    status = ControlSlaveSelect(true);
    if (status != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    cmd = CMD_WRITE_ENABLE;

    status = ptrOSPI->Send(&cmd, 1U);
    if (status != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    while (!issi_event_flag)
    {
         __WFE();
    }

    if (!(issi_event_flag & ARM_OSPI_EVENT_TRANSFER_COMPLETE))
    {
        ControlSlaveSelect(false);
        issi_event_flag = 0;
        return ARM_DRIVER_ERROR;
    }
    issi_event_flag = 0;

    status = ControlSlaveSelect(false);

    /* Reading status register will work only when FLASH is switched to Octal mode (after PowerControl(ARM_POWER_FULL)) */
    if (ISSI_Flags & FLASH_POWER)
    {
        status = ReadStatusReg(CMD_READ_STATUS, &val);

        if((status == ARM_DRIVER_OK) && ((val & 0x02) == 0))
        {
            return ARM_DRIVER_ERROR;
        }
    }
    return status;
}


/**
  \fn          int32_t ARM_Flash_Initialize (ARM_Flash_SignalEvent_t cb_event)
  \brief       Initialize the Flash Interface.
  \param[in]   cb_event  Pointer to \ref ARM_Flash_SignalEvent
  \return      \ref execution_status
**/
 static int32_t ARM_Flash_Initialize (ARM_Flash_SignalEvent_t cb_event)
 {
    int32_t status;
    (void)cb_event;

    ISSI_FlashStatus.busy  = 0U;
    ISSI_FlashStatus.error = 0U;

    status = ptrOSPI->Initialize(spi_callback_event);

    if (status != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    ISSI_Flags |= FLASH_INIT;

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t ARM_Flash_Uninitialize (void)
  \brief       De-initialize the Flash Interface.
  \return      \ref execution_status
**/
static int32_t ARM_Flash_Uninitialize (void)
{
    ISSI_Flags = 0U;
    return ptrOSPI->Uninitialize();
}

/**
  \fn          int32_t ARM_Flash_PowerControl (ARM_POWER_STATE state)
  \brief       Control the Flash interface power.
  \param[in]   state  Power state
  \return      \ref execution_status
**/

static int32_t ARM_Flash_PowerControl (ARM_POWER_STATE state)
{
    int32_t status;
    uint32_t cmd[5];

    switch (state)
    {
        case ARM_POWER_OFF:
        {
            ISSI_Flags &= ~FLASH_POWER;
            ISSI_FlashStatus.busy  = 0U;
            ISSI_FlashStatus.error = 0U;

            return ptrOSPI->PowerControl(ARM_POWER_OFF);
        }

        case ARM_POWER_FULL:
        {

            if ((ISSI_Flags & FLASH_INIT) == 0U)
            {
                return ARM_DRIVER_ERROR;
            }

            if ((ISSI_Flags & FLASH_POWER) == 0U)
            {
                status = ptrOSPI->PowerControl(ARM_POWER_FULL);

                if (status != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }

                status = ptrOSPI->Control(ARM_OSPI_MODE_MASTER |
                                 ARM_OSPI_DATA_BITS(8) |
                                 ARM_OSPI_SS_MASTER_SW,
                                 OSPI_BUS_SPEED);

                if (status != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }

                /* Prepare command and address for setting flash in octal mode */
                cmd[0] = CMD_WRITE_VOL_CONFIG;
                cmd[1] = (uint8_t)(IO_MODE_ADDRESS >> 16);
                cmd[2] = (uint8_t)(IO_MODE_ADDRESS >> 8);
                cmd[3] = (uint8_t)(IO_MODE_ADDRESS >> 0);
                cmd[4] = OCTAL_DDR;

                status = ptrOSPI->Control(ARM_OSPI_SET_ADDR_LENGTH_WAIT_CYCLE, (ARM_OSPI_ADDR_LENGTH_0_BITS << ARM_OSPI_ADDR_LENGTH_POS) | (0 << ARM_OSPI_WAIT_CYCLE_POS));

                if (status != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }

                status = ptrOSPI->Control(ARM_OSPI_SET_FRAME_FORMAT, ARM_OSPI_FRF_STANDRAD);

                if (status != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }

                status = ptrOSPI->Control(ARM_OSPI_SET_DDR_MODE, ARM_OSPI_DDR_DISABLE);

                if (status != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }

                status = SetWriteEnable();

                if (status != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }

                status = ControlSlaveSelect(true);

                if (status != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }

                status = ptrOSPI->Control(ARM_OSPI_SET_ADDR_LENGTH_WAIT_CYCLE, (ARM_OSPI_ADDR_LENGTH_24_BITS << ARM_OSPI_ADDR_LENGTH_POS) | (0 << ARM_OSPI_WAIT_CYCLE_POS));

                if (status != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }

                status = ptrOSPI->Control(ARM_OSPI_SET_FRAME_FORMAT, ARM_OSPI_FRF_STANDRAD);

                if (status != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }

                status = ptrOSPI->Control(ARM_OSPI_SET_DDR_MODE, ARM_OSPI_DDR_DISABLE);

                if (status != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }

                status = ptrOSPI->Send(cmd, 5);

                if (status != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }

                while (!issi_event_flag)
                {
                     __WFE();
                }

                if (!(issi_event_flag & ARM_OSPI_EVENT_TRANSFER_COMPLETE))
                {
                    ControlSlaveSelect(false);
                    issi_event_flag = 0;
                    return ARM_DRIVER_ERROR;
                }

                issi_event_flag = 0;

                status = ControlSlaveSelect(false);

                if (status != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }

                /* Prepare buffer with command and address to configure default wait cycles */
                cmd[0] = CMD_WRITE_VOL_CONFIG;
                cmd[1] = WAIT_CYCLE_ADDRESS;
                cmd[2] = DEFAULT_WAIT_CYCLES;
                cmd[3] = DEFAULT_WAIT_CYCLES;

                status = ptrOSPI->Control(ARM_OSPI_SET_ADDR_LENGTH_WAIT_CYCLE, (ARM_OSPI_ADDR_LENGTH_0_BITS << ARM_OSPI_ADDR_LENGTH_POS) | (0 << ARM_OSPI_WAIT_CYCLE_POS));
                if (status != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }

                status = ptrOSPI->Control(ARM_OSPI_SET_FRAME_FORMAT, ARM_OSPI_FRF_OCTAL);

                if (status != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }

                status = ptrOSPI->Control(ARM_OSPI_SET_DDR_MODE, ARM_OSPI_DDR_ENABLE);

                if (status != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }

                status = SetWriteEnable();

                if (status != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }

                status = ControlSlaveSelect(true);

                if (status != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }

                status = ptrOSPI->Control(ARM_OSPI_SET_ADDR_LENGTH_WAIT_CYCLE, (ARM_OSPI_ADDR_LENGTH_32_BITS << ARM_OSPI_ADDR_LENGTH_POS) | (0 << ARM_OSPI_WAIT_CYCLE_POS));

                if (status != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }

                status = ptrOSPI->Send(cmd, 4);

                if (status != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }

                while (!issi_event_flag)
                {
                     __WFE();
                }

                if (!(issi_event_flag & ARM_OSPI_EVENT_TRANSFER_COMPLETE))
                {
                    ControlSlaveSelect(false);
                    issi_event_flag = 0;
                    return ARM_DRIVER_ERROR;
                }

                issi_event_flag = 0;

                status = ControlSlaveSelect(false);

                if (status != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }

                ISSI_FlashStatus.busy  = 0U;
                ISSI_FlashStatus.error = 0U;

                ISSI_Flags |= FLASH_POWER;
            }
            return ARM_DRIVER_OK;
        }

        case ARM_POWER_LOW:
            return ARM_DRIVER_ERROR_UNSUPPORTED;

        default:
            return ARM_DRIVER_ERROR;
    }
}

/**
  \fn          int32_t ARM_Flash_ReadData (uint32_t addr, void *data, uint32_t cnt)
  \brief       Read data from Flash.
  \param[in]   addr  Data address.
  \param[out]  data  Pointer to a buffer storing the data read from Flash.
  \param[in]   cnt   Number of data items to read.
  \return      number of data items read or \ref execution_status
**/
static int32_t ARM_Flash_ReadData (uint32_t addr, void *data, uint32_t cnt)
{
    uint32_t cmd[2], data_cnt;
    uint16_t *data_ptr;
    int32_t  status, num = 0;

    if ((addr > (FLASH_ISSI_SECTOR_COUNT * FLASH_ISSI_SECTOR_SIZE)) || (data == NULL) || ((addr + cnt) > (FLASH_ISSI_SECTOR_COUNT * FLASH_ISSI_SECTOR_SIZE)))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    data_ptr = (uint16_t *) data;

    status = ptrOSPI->Control(ARM_OSPI_SET_ADDR_LENGTH_WAIT_CYCLE, (ARM_OSPI_ADDR_LENGTH_0_BITS << ARM_OSPI_ADDR_LENGTH_POS) | (0 << ARM_OSPI_WAIT_CYCLE_POS));

    if (status != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    status = SetWriteEnable();

    if (status != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    status = ptrOSPI->Control(ARM_OSPI_SET_ADDR_LENGTH_WAIT_CYCLE, (ARM_OSPI_ADDR_LENGTH_32_BITS << ARM_OSPI_ADDR_LENGTH_POS) | (16 << ARM_OSPI_WAIT_CYCLE_POS));

    if (status != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Switch to 16 bit mode for reading data */
    status = ptrOSPI->Control(ARM_OSPI_MODE_MASTER |
                ARM_OSPI_DATA_BITS(16) |
                ARM_OSPI_SS_MASTER_SW,
                OSPI_BUS_SPEED);

    if (status != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    while(cnt)
    {
        status = ControlSlaveSelect(true);

        if (status != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }

        /* At frequency > 2.5 MHz, max no. of frames that can be read by OSPI is 256 (RX_FIFO_DEPTH) */
        data_cnt = OSPI_MAX_RX_COUNT;

        if (data_cnt > cnt)
        {
            data_cnt = cnt;
        }

        /* Prepare command with address */
        cmd[0] = CMD_READ_DATA;
        cmd[1] = addr;

        status = ptrOSPI->Transfer(cmd, data_ptr, data_cnt);

        if (status != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }

        while (!issi_event_flag)
        {
             __WFE();
        }

        if (!(issi_event_flag & ARM_OSPI_EVENT_TRANSFER_COMPLETE))
        {
            ControlSlaveSelect(false);
            issi_event_flag = 0;
            return ARM_DRIVER_ERROR;
        }

        issi_event_flag = 0;

        status = ControlSlaveSelect(false);

        if (status != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }

        /* For 16 bit frames, update address by data_cnt * 2*/
        addr += (data_cnt * 2);
        cnt -= data_cnt;
        data_ptr += data_cnt;
        num += data_cnt;
    }

    status = num;
    return status;
}

/* temporary buffer used in ProgramData below */
static uint32_t cmd[261];

/**
  \fn          int32_t ARM_Flash_ProgramData (uint32_t addr, const void *data, uint32_t cnt)
  \brief       Program data to Flash.
  \param[in]   addr  Data address.
  \param[in]   data  Pointer to a buffer containing the data to be programmed to Flash.
  \param[in]   cnt   Number of data items to program.
  \return      number of data items programmed or \ref execution_status
**/
static int32_t ARM_Flash_ProgramData (uint32_t addr, const void *data, uint32_t cnt)
{
    const uint16_t *data_ptr;
    uint8_t val = 0;
    int32_t status = ARM_DRIVER_OK;
    uint32_t num = 0, data_cnt, index, iter = 0, it = 0;

    if ((addr > (FLASH_ISSI_SECTOR_COUNT * FLASH_ISSI_SECTOR_SIZE)) || (data == NULL) || ((addr + cnt) > (FLASH_ISSI_SECTOR_COUNT * FLASH_ISSI_SECTOR_SIZE)))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    data_ptr = data;

    while (cnt)
    {
        ISSI_FlashStatus.busy  = 1U;
        ISSI_FlashStatus.error = 0U;

        status = ptrOSPI->Control(ARM_OSPI_SET_ADDR_LENGTH_WAIT_CYCLE, (ARM_OSPI_ADDR_LENGTH_0_BITS << ARM_OSPI_ADDR_LENGTH_POS) | (0 << ARM_OSPI_WAIT_CYCLE_POS));

        if (status != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }

        status = SetWriteEnable();

        if (status != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }

        data_cnt = (FLASH_ISSI_PAGE_SIZE - (addr % FLASH_ISSI_PAGE_SIZE)) >> 1;

        if (data_cnt > cnt)
        {
            data_cnt = cnt;
        }


        /* Prepare command with address */
        cmd[0] = CMD_PAGE_PROGRAM;
        cmd[1] = addr;

        index = 2;

        for ( it = 0; it < data_cnt; it++)
        {
            cmd[index++] = data_ptr[iter++];
        }

        status = ptrOSPI->Control(ARM_OSPI_SET_ADDR_LENGTH_WAIT_CYCLE, (ARM_OSPI_ADDR_LENGTH_32_BITS << ARM_OSPI_ADDR_LENGTH_POS) | (0 << ARM_OSPI_WAIT_CYCLE_POS));

        if (status != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }

        /* Switch to 16 bit mode for program data */
        status = ptrOSPI->Control(ARM_OSPI_MODE_MASTER |
                 ARM_OSPI_DATA_BITS(16) |
                 ARM_OSPI_SS_MASTER_SW,
                 OSPI_BUS_SPEED);

        if (status != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }

        status = ControlSlaveSelect(true);

        if (status != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }

        do {
            /* Send data along with the command and address bytes (+2) */
            status = ptrOSPI->Send(cmd, (data_cnt + 2));
        } while (status == ARM_DRIVER_ERROR_BUSY);

        if (status != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }

        while (!issi_event_flag)
        {
             __WFE();
        }

        if (!(issi_event_flag & ARM_OSPI_EVENT_TRANSFER_COMPLETE))
        {
            ControlSlaveSelect(false);
            issi_event_flag = 0;
            return ARM_DRIVER_ERROR;
        }

        issi_event_flag = 0;

        /* For 16 bit data frames, increment the byte address with 2 * data_cnt programmed */
        addr += (data_cnt * 2);
        num  += data_cnt;
        cnt  -= data_cnt;

        status = ControlSlaveSelect(false);

        if (status != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }

        /* Read status until device ready */
        do
        {
            status = ReadStatusReg(CMD_READ_FLAG_STATUS, &val);

            if (status != ARM_DRIVER_OK)
            {
                break;
            }

            /* Check ISSI_Flags Status register value */
            if ((val & FLAG_STATUS_BUSY) != 0U)
            {
                ISSI_FlashStatus.busy = 0U;
            }

            /* Check Program and Erase bits */
            if ((val & FLAG_STATUS_ERROR) == 0U)
            {
                ISSI_FlashStatus.error = 0U;
            }
            else
            {
                ISSI_FlashStatus.error = 1U;
            }
        }while ((val & FLAG_STATUS_BUSY) == 0);
    }

    status = ControlSlaveSelect(false);
    if (status != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Number of data items programmed */
    status = (int32_t)num;
    return status;
}

/**
  \fn          int32_t ARM_Flash_EraseSector (uint32_t addr)
  \brief       Erase Flash Sector.
  \param[in]   addr  Sector address
  \return      \ref execution_status
**/
static int32_t ARM_Flash_EraseSector (uint32_t addr)
{
    int32_t status;
    uint8_t num;
    uint32_t cmd[2];

    if (addr > (FLASH_ISSI_SECTOR_COUNT * FLASH_ISSI_SECTOR_SIZE))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    status = ptrOSPI->Control(ARM_OSPI_SET_ADDR_LENGTH_WAIT_CYCLE, (ARM_OSPI_ADDR_LENGTH_0_BITS << ARM_OSPI_ADDR_LENGTH_POS) | (0 << ARM_OSPI_WAIT_CYCLE_POS));

    if (status != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    status = SetWriteEnable();

    if (status == ARM_DRIVER_OK)
    {
        ISSI_FlashStatus.busy  = 1U;
        ISSI_FlashStatus.error = 0U;

        status = ptrOSPI->Control(ARM_OSPI_MODE_MASTER |
                    ARM_OSPI_DATA_BITS(8) |
                    ARM_OSPI_SS_MASTER_SW,
                    OSPI_BUS_SPEED);

        if (status != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }

        /* Select Slave */
        status = ControlSlaveSelect(true);

        if (status == ARM_DRIVER_OK)
        {
            /* Prepare command with address */
            cmd[0] = CMD_SECTOR_ERASE;
            cmd[1] = addr;


            status = ptrOSPI->Control(ARM_OSPI_SET_ADDR_LENGTH_WAIT_CYCLE, (ARM_OSPI_ADDR_LENGTH_32_BITS << ARM_OSPI_ADDR_LENGTH_POS) | (0 << ARM_OSPI_WAIT_CYCLE_POS));

            if (status != ARM_DRIVER_OK)
            {
                return ARM_DRIVER_ERROR;
            }

            status = ptrOSPI->Send(cmd, 2U);

            if (status != ARM_DRIVER_OK)
            {
                return ARM_DRIVER_ERROR;
            }

            while (!issi_event_flag)
            {
                 __WFE();
            }

            if (!(issi_event_flag & ARM_OSPI_EVENT_TRANSFER_COMPLETE))
            {
                ControlSlaveSelect(false);
                issi_event_flag = 0;
                return ARM_DRIVER_ERROR;
            }

            issi_event_flag = 0;

            do
            {
                status = ReadStatusReg(CMD_READ_FLAG_STATUS, &num);
                if (status != ARM_DRIVER_OK)
                {
                    break;
                }

                /* Check ISSI_Flags Status register value */
                if ((num & FLAG_STATUS_BUSY) != 0U)
                {
                    ISSI_FlashStatus.busy = 0U;
                }

                /* Check Program and Erase bits */
                if ((num & FLAG_STATUS_ERROR) == 0U)
                {
                    ISSI_FlashStatus.error = 0U;
                }
                else
                {
                    ISSI_FlashStatus.error = 1U;
                }
            } while ((num & FLAG_STATUS_BUSY) == 0);
        }

        status = ControlSlaveSelect(false);
    }
    return status;
}


 /**
  \fn          int32_t ARM_Flash_EraseChip (void)
  \brief       Erase complete Flash.
               Optional function for faster full chip erase.
  \return      \ref execution_status
**/
static int32_t ARM_Flash_EraseChip (void)
{
    uint32_t cmd;
    uint8_t num;
    int32_t status;

    ISSI_FlashStatus.busy  = 1U;
    ISSI_FlashStatus.error = 0U;

    status = ptrOSPI->Control(ARM_OSPI_SET_ADDR_LENGTH_WAIT_CYCLE, (ARM_OSPI_ADDR_LENGTH_0_BITS << ARM_OSPI_ADDR_LENGTH_POS) | (0 << ARM_OSPI_WAIT_CYCLE_POS));

    if (status != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    status = SetWriteEnable();

    if (status == ARM_DRIVER_OK)
    {

        status = ptrOSPI->Control(ARM_OSPI_MODE_MASTER |
                    ARM_OSPI_DATA_BITS(8) |
                    ARM_OSPI_SS_MASTER_SW,
                    OSPI_BUS_SPEED);

        if (status != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }

        /* Select Slave */
        status = ControlSlaveSelect(true);

        if (status == ARM_DRIVER_OK)
        {
            /* Prepare command */
            cmd = CMD_BULK_ERASE;

            status = ptrOSPI->Control(ARM_OSPI_SET_ADDR_LENGTH_WAIT_CYCLE, (ARM_OSPI_ADDR_LENGTH_0_BITS << ARM_OSPI_ADDR_LENGTH_POS) | (0 << ARM_OSPI_WAIT_CYCLE_POS));

            if (status != ARM_DRIVER_OK)
            {
                return ARM_DRIVER_ERROR;
            }

            status = ptrOSPI->Send(&cmd, 1U);

            if (status != ARM_DRIVER_OK)
            {
                return ARM_DRIVER_ERROR;
            }

            while (!issi_event_flag)
            {
                 __WFE();
            }

            if (!(issi_event_flag & ARM_OSPI_EVENT_TRANSFER_COMPLETE))
            {
                ControlSlaveSelect(false);
                issi_event_flag = 0;
                return ARM_DRIVER_ERROR;
            }

            issi_event_flag = 0;

            do
            {
                status = ReadStatusReg(CMD_READ_FLAG_STATUS, &num);
                if (status != ARM_DRIVER_OK)
                {
                    break;
                }

                /* Check ISSI_Flags Status register value */
                if ((num & FLAG_STATUS_BUSY) != 0U)
                {
                    ISSI_FlashStatus.busy = 0U;
                }

                /* Check Program and Erase bits */
                if ((num & FLAG_STATUS_ERROR) == 0U)
                {
                    ISSI_FlashStatus.error = 0U;
                }
                else
                {
                    ISSI_FlashStatus.error = 1U;
                }
            } while ((num & FLAG_STATUS_BUSY) == 0);
        }
        status = ControlSlaveSelect(false);
    }
    return status;
}

/**
  \fn          ARM_FLASH_STATUS ARM_Flash_GetStatus (void)
  \brief       Get Flash status.
  \return      Flash status \ref ARM_FLASH_STATUS
**/
static ARM_FLASH_STATUS ARM_Flash_GetStatus (void)
{
    uint8_t val;

    if (ISSI_FlashStatus.busy == 1U)
    {
        /* Read flag status register */
        if (ReadStatusReg (CMD_READ_FLAG_STATUS, &val) == ARM_DRIVER_OK)
        {
            /* Check "Program or erase controller" bit */
            if ((val & FLAG_STATUS_BUSY) != 0U)
            {
                ISSI_FlashStatus.busy = 0U;
            }

            /* Check Erase and Program bits */
            if ((val & FLAG_STATUS_ERROR) == 0U)
            {
                ISSI_FlashStatus.error = 0U;
            }
            else
            {
                ISSI_FlashStatus.error = 1U;
            }
        }
        else
        {
            ISSI_FlashStatus.error = 1U;
        }
    }
    return ISSI_FlashStatus;
}

/**
  \fn          ARM_FLASH_INFO * ARM_Flash_GetInfo (void)
  \brief       Get Flash information.
  \return      Pointer to Flash information \ref ARM_FLASH_INFO
**/
static ARM_FLASH_INFO * ARM_Flash_GetInfo (void)
{
    return &ISSI_FlashInfo;
}


static ARM_FLASH_INFO * GetInfo (void)
{
    return ARM_Flash_GetInfo();
}

static ARM_DRIVER_VERSION GetVersion(void)
{
    return ARM_Flash_GetVersion();
}

static ARM_FLASH_CAPABILITIES GetCapabilities(void)
{
    return ARM_Flash_GetCapabilities();
}

static ARM_FLASH_STATUS GetStatus (void)
{
    return ARM_Flash_GetStatus();
}

static int32_t EraseChip (void)
{
    return ARM_Flash_EraseChip();
}

static int32_t EraseSector (uint32_t addr)
{
    return ARM_Flash_EraseSector (addr);
}

static int32_t ProgramData (uint32_t addr, const void *data, uint32_t cnt)
{
    return ARM_Flash_ProgramData (addr, data, cnt);
}

static int32_t ReadData (uint32_t addr, void *data, uint32_t cnt)
{
    return ARM_Flash_ReadData (addr, data, cnt);
}

static int32_t PowerControl (ARM_POWER_STATE state)
{
    return ARM_Flash_PowerControl (state);
}

static int32_t Initialize (ARM_Flash_SignalEvent_t cb_event)
{
    return ARM_Flash_Initialize (cb_event);
}

static int32_t Uninitialize (void)
{
    return ARM_Flash_Uninitialize ();
}

/* Flash Driver Control Block */
extern
ARM_DRIVER_FLASH ARM_Driver_Flash_(DRIVER_FLASH_NUM);
ARM_DRIVER_FLASH ARM_Driver_Flash_(DRIVER_FLASH_NUM) = {
    GetVersion,
    GetCapabilities,
    Initialize,
    Uninitialize,
    PowerControl,
    ReadData,
    ProgramData,
    EraseSector,
    EraseChip,
    GetStatus,
    GetInfo
};
