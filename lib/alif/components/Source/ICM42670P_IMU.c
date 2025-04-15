/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/******************************************************************************
 * @file     ICM42670P_IMU.c
 * @author   Shreehari H K
 * @email    shreehari.hk@alifsemi.com
 * @version  V1.0.0
 * @date     02-July-2024
 * @brief    Driver for Inertial Measurement Unit ICM42670P
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

#include "RTE_Components.h"
#include "system_utils.h"

/* IMU Driver */
#include "Driver_IMU.h"

/* I3C Driver */
#include "Driver_I3C.h"

#if !defined (RTE_Drivers_ICM42670P)
    #error "IMU ICM42670P is not configured in RTE_Components.h"
#endif

#define ARM_IMU_DRV_VERSION  ARM_DRIVER_VERSION_MAJOR_MINOR(1, 2)

/* Timeout in Microsec */
#define IMU_I3C_TIMEOUT_US              (100000)

#define ICM42670P_UPPER_DATA_BYTE_Pos   (8)

/* Register offest */
#define ICM42670P_REG_IDX_SIZE          (1)

/* ICM42670P Driver status */
#define ICM42670P_DRIVER_INITIALIZED    (1 << 0U)
#define ICM42670P_DRIVER_POWERED        (1 << 1U)

/* Target Slave Address */
#define ICM42670P_DEFAULT_ADDR          (0x68)

/* Target Slave's Who Am I Reg addr and it's value */
#define ICM42670P_WHO_AM_I_REG          (0x75)
#define ICM42670P_REG_WHO_AM_I_VAL      (0x67)

/* Target Slave  */
#define ICM42670P_PWR_MGMT0_REG         (0x1F)
#define ICM42670P_PWR_MGMT0_VAL         (0xFU)

/*  Data Registers */
#define ICM42670P_TEMP_DATA1_REG        (0x09)
#define ICM42670P_TEMP_DATA_SIZE        (0x02)
#define ICM42670P_ACCEL_DATA1_REG       (0x0B)
#define ICM42670P_ACCEL_DATA_SIZE       (0x06)
#define ICM42670P_GYRO_DATA1_REG        (0x11)
#define ICM42670P_GYRO_DATA_SIZE        (0x06)

/* Data registers offset */
#define ICM42670P_TEMP_DATA1_OFFSET     (0x00)
#define ICM42670P_TEMP_DATA0_OFFSET     (0x01)
#define ICM42670P_ACCEL_DATA_X1_OFFSET  (0x00)
#define ICM42670P_ACCEL_DATA_X0_OFFSET  (0x01)
#define ICM42670P_ACCEL_DATA_Y1_OFFSET  (0x02)
#define ICM42670P_ACCEL_DATA_Y0_OFFSET  (0x03)
#define ICM42670P_ACCEL_DATA_Z1_OFFSET  (0x04)
#define ICM42670P_ACCEL_DATA_Z0_OFFSET  (0x05)
#define ICM42670P_GYRO_DATA_X1_OFFSET   (0x00)
#define ICM42670P_GYRO_DATA_X0_OFFSET   (0x01)
#define ICM42670P_GYRO_DATA_Y1_OFFSET   (0x02)
#define ICM42670P_GYRO_DATA_Y0_OFFSET   (0x03)
#define ICM42670P_GYRO_DATA_Z1_OFFSET   (0x04)
#define ICM42670P_GYRO_DATA_Z0_OFFSET   (0x05)

/* Configuration Registers */
#define ICM42670P_GYRO_CONFIG0_REG      (0x20)
#define ICM42670P_GYRO_CONFIG0_VAL      (0x06)
#define ICM42670P_GYRO_CONFIG1_REG      (0x23)
#define ICM42670P_GYRO_CONFIG1_VAL      (0x01)
#define ICM42670P_ACCEL_CONFIG0_REG     (0x21)
#define ICM42670P_ACCEL_CONFIG0_VAL     (0x06)
#define ICM42670P_ACCEL_CONFIG1_REG     (0x24)
#define ICM42670P_ACCEL_CONFIG1_VAL     (0x41)
#define ICM42670P_TEMP_CONFIG0_REG      (0x22)
#define ICM42670P_TEMP_CONFIG0_VAL      (0x00)

/* Memory selection registers */
#define ICM42670P_BLK_SEL_W_REG         (0x79)
#define ICM42670P_BLK_SEL_W_VAL         (0x0)
#define ICM42670P_MADDR_W_REG           (0x7A)
#define ICM42670P_M_W_REG               (0x7B)

#define ICM42670P_BLK_SEL_R_REG         (0x7C)
#define ICM42670P_BLK_SEL_R_VAL         (0x0)
#define ICM42670P_MADDR_R_REG           (0x7D)
#define ICM42670P_M_R_REG               (0x7E)

/* Interrupt config and src register*/
#define ICM42670P_INTF_CONFIG6_REG      (0x23)    /* IBI configuration Register */
#define ICM42670P_INTF_CONFIG6_VAL      (0x14)

#define ICM42670P_INT_SOURCE8_REG       (0x31)    /* IBI Interrupt source Register  */
#define ICM42670P_INT_SOURCE8_VAL       (0x8)     /* IBI for data availability */

#define ICM42670P_ACCEL_CALIB_VAL       (2048U)   /*Calibration for full scale output selection of +-16g */
#define ICM42670P_GYRO_CALIB_VAL        (16.4)    /*Calibration for full scale output selection of +-2kdps */

#define ICM42670P_ACCEL_VAL(x)          ((x * 1000) / (ICM42670P_ACCEL_CALIB_VAL)) /* Acceleration value in mg*/
#define ICM42670P_GYRO_VAL(x)           ((x * 1000) / (ICM42670P_GYRO_CALIB_VAL))  /* Gyro value in mdps */
#define ICM42670P_TEMPERATURE(x)        ((x / 128.0) + 25) /* Temp value in C */

/* ICM42670P driver Info variable */
static struct ICM42670P_DRV_INFO
{
    uint8_t                    state;              /* Driver state                   */
    uint8_t                    target_addr;        /* Target slave's dynamic address */
    uint8_t                    reserved[2];        /* Reserved                       */
    volatile uint32_t          imu_i3c_event;      /* I3C Event status               */
    volatile ARM_IMU_STATUS    status;             /* Driver status                  */
} icm42670p_drv_info;

/* Driver version*/
static const ARM_DRIVER_VERSION DriverVersion =
{
    ARM_IMU_API_VERSION,
    ARM_IMU_DRV_VERSION
};

/* I3C driver */
extern ARM_DRIVER_I3C Driver_I3C;
static ARM_DRIVER_I3C *I3C_Driver = &Driver_I3C;

/* Driver Capabilities */
static const ARM_IMU_CAPABILITIES DriverCapabilities =
{
    1,  /* Supports Accelerometer data */
    1,  /* Supports Gyroscope data */
    0,  /* Doesn't support Magnetometer data */
    1,  /* Supports Temperature sens data */
    0   /* reserved (must be zero)*/
};

/**
  \fn          ARM_DRIVER_VERSION ARM_IMU_GetVersion(void)
  \brief       Gets IMU driver version.
  \return      \ref ARM_DRIVER_VERSION.
*/
static ARM_DRIVER_VERSION ARM_IMU_GetVersion(void)
{
    return DriverVersion;
}

/**
  \fn          ARM_IMU_CAPABILITIES ARM_IMU_GetCapabilities(void)
  \brief       Gets IMU driver capabilities.
  \return      \ref ARM_IMU_CAPABILITIES.
*/
static ARM_IMU_CAPABILITIES ARM_IMU_GetCapabilities(void)
{
    return DriverCapabilities;
}

/**
  \fn          ARM_IMU_STATUS ARM_IMU_GetStatus(void)
  \brief       Gets IMU driver status.
  \return      \ref ARM_IMU_STATUS.
*/
static ARM_IMU_STATUS ARM_IMU_GetStatus(void)
{
    return icm42670p_drv_info.status;
}

/**
  \fn           void IMU_I3CCallBack(uint32_t event)
  \brief        IMU driver I3C callback event.
  \param[in]    event: I3C Event.
  \return       None
*/
void IMU_I3CCallBack(uint32_t event)
{
    icm42670p_drv_info.imu_i3c_event |= event;

    if(icm42670p_drv_info.imu_i3c_event & ARM_I3C_EVENT_IBI_SLV_INTR_REQ)
    {
        /* Sets Data Rcvd to true */
        icm42670p_drv_info.status.data_rcvd = 1U;
    }
}

/**
  \fn           int32_t IMU_ResetDynAddr(void)
  \brief        Resets all slaves' dynamic address
  \return       \ref execution_status.
*/
static int32_t IMU_ResetDynAddr(void)
{
    int32_t ret;

    ARM_I3C_CMD i3c_cmd   = {0};

    /* Reset slave address */
    i3c_cmd.rw            = 0U;
    i3c_cmd.cmd_id        = I3C_CCC_RSTDAA(true);
    i3c_cmd.len           = 0U;
    i3c_cmd.addr          = 0;

    ret = I3C_Driver->MasterSendCommand(&i3c_cmd);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* wait for callback event. */
    while(!((icm42670p_drv_info.imu_i3c_event & ARM_I3C_EVENT_TRANSFER_DONE) ||
            (icm42670p_drv_info.imu_i3c_event & ARM_I3C_EVENT_TRANSFER_ERROR)));
    if(icm42670p_drv_info.imu_i3c_event & ARM_I3C_EVENT_TRANSFER_ERROR)
    {
        return ARM_DRIVER_ERROR;
    }

    icm42670p_drv_info.imu_i3c_event = 0U;

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t IMU_SetDynAddr(void)
  \brief        Assigns Dynamic address to IMU.
  \return       \ref execution_status.
*/
static int32_t IMU_SetDynAddr(void)
{
    int32_t ret;

    /* I3C CCC (Common Command Codes) */
    ARM_I3C_CMD i3c_cmd   = {0};

    i3c_cmd.rw            = 0U;
    i3c_cmd.cmd_id        = I3C_CCC_SETDASA;
    i3c_cmd.len           = 1U;

    /* Assign IMU's Static address */
    i3c_cmd.addr          = ICM42670P_DEFAULT_ADDR;

    /* Assign Dynamic address */
    ret = I3C_Driver->MasterAssignDA(&i3c_cmd);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* wait for callback event. */
    while(!((icm42670p_drv_info.imu_i3c_event & ARM_I3C_EVENT_TRANSFER_DONE) ||
            (icm42670p_drv_info.imu_i3c_event & ARM_I3C_EVENT_TRANSFER_ERROR)));
    if(icm42670p_drv_info.imu_i3c_event & ARM_I3C_EVENT_TRANSFER_ERROR)
    {
        return ARM_DRIVER_ERROR;
    }

    icm42670p_drv_info.imu_i3c_event = 0U;

    /* Fetch the assigned dynamic address */
    ret = I3C_Driver->GetSlaveDynAddr(ICM42670P_DEFAULT_ADDR,
                                      &icm42670p_drv_info.target_addr);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t IMU_Init(void)
  \brief       Initializes IMU driver.
               This function will
                 - initialize i3c instance.
  \return      \ref Execution status.
*/
static int32_t IMU_Init(void)
{
    int32_t ret;
    ARM_DRIVER_VERSION version;

    /* Get i3c driver version. */
    version = I3C_Driver->GetVersion();

    if((version.api < ARM_DRIVER_VERSION_MAJOR_MINOR(7U, 0U))       ||
       (version.drv < ARM_DRIVER_VERSION_MAJOR_MINOR(7U, 0U)))
    {
        return ARM_DRIVER_ERROR;
    }

    ret = I3C_Driver->Initialize(IMU_I3CCallBack);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    ret = I3C_Driver->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t IMU_Write(uint8_t tar_addr, uint16_t reg_addr,
                                 uint8_t *reg_data, uint8_t len)
  \brief       Writes data to IMU registers.
  \param[in]   tar_addr : I3C slave address.
  \param[in]   reg_addr : Register address.
  \param[in]   reg_data : pointer to source data buf.
  \param[in]   len      : Number of bytes to write.
  \return      \ref Execution status.
*/
static int32_t IMU_Write(uint8_t tar_addr, uint16_t reg_addr,
                         uint8_t *reg_data, uint8_t len)

{
    int32_t  ret     = 0U;
    uint8_t  iter    = 0U;
    uint32_t counter = 0U;
    uint8_t  tx_buf[4];

    /* Store register's address in 0th index */
    tx_buf[0]        = reg_addr;

    if(len)
    {
        for(iter = 0U; iter < len; iter++)
        {
            tx_buf[iter + ICM42670P_REG_IDX_SIZE] = reg_data[iter];
        }
    }

    icm42670p_drv_info.imu_i3c_event = 0U;

    /* Send msg to slave */
    ret = I3C_Driver->MasterTransmit(tar_addr,
                                     tx_buf,
                                     (len + ICM42670P_REG_IDX_SIZE));
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* wait for callback event. */
    while(!((icm42670p_drv_info.imu_i3c_event & ARM_I3C_EVENT_TRANSFER_DONE) ||
            (icm42670p_drv_info.imu_i3c_event & ARM_I3C_EVENT_TRANSFER_ERROR)))
    {
        if(counter++ < IMU_I3C_TIMEOUT_US)
        {
            sys_busy_loop_us(1);
        }
        else
        {
            return ARM_DRIVER_ERROR;
        }
    }
    if(icm42670p_drv_info.imu_i3c_event & ARM_I3C_EVENT_TRANSFER_ERROR)
    {
        return ARM_DRIVER_ERROR;
    }

    icm42670p_drv_info.imu_i3c_event = 0U;

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t IMU_Read(uint8_t tar_addr, uint16_t reg_addr,
                                uint8_t *reg_data, uint8_t len)
  \brief       Reads data from IMU registers.
  \param[in]   tar_addr : I3C slave address.
  \param[in]   reg_addr : Register address.
  \param[in]   reg_data : pointer to destination buf.
  \param[in]   len      : Number of bytes to read.
  \return      \ref Execution status.
*/
static int32_t IMU_Read(uint8_t tar_addr, uint16_t reg_addr,
                        uint8_t *reg_data, uint8_t len)
{
    int32_t  ret     = 0U;
    uint32_t counter = 0U;

    /* Send register address */
    ret = IMU_Write(tar_addr, reg_addr, NULL, 0U);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* Receive data from slave */
    ret = I3C_Driver->MasterReceive(tar_addr, reg_data, len);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* wait for callback event. */
    while(!((icm42670p_drv_info.imu_i3c_event &
             ARM_I3C_EVENT_TRANSFER_DONE) ||
            (icm42670p_drv_info.imu_i3c_event &
             ARM_I3C_EVENT_TRANSFER_ERROR)))
    {
        if(counter++ < IMU_I3C_TIMEOUT_US)
        {
            sys_busy_loop_us(1);
        }
        else
        {
            return ARM_DRIVER_ERROR;
        }
    }
    if(icm42670p_drv_info.imu_i3c_event & ARM_I3C_EVENT_TRANSFER_ERROR)
    {
        return ARM_DRIVER_ERROR;
    }
    icm42670p_drv_info.imu_i3c_event = 0U;

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t IMU_AccelConfig(void)
  \brief       Configures Accelerometer of IMU driver.
  \return      \ref Execution status.
*/
static int32_t IMU_AccelConfig(void)
{
    uint8_t data[4];

    /* Configure Accelerometer */
    data[0] = ICM42670P_ACCEL_CONFIG0_VAL;
    IMU_Write(icm42670p_drv_info.target_addr,
              ICM42670P_ACCEL_CONFIG0_REG, data, 1U);

    sys_busy_loop_us(10);

    data[0] = 0;
    IMU_Read(icm42670p_drv_info.target_addr,
             ICM42670P_ACCEL_CONFIG0_REG, data, 1U);

    if(data[0] != ICM42670P_ACCEL_CONFIG0_VAL)
    {
        return ARM_DRIVER_ERROR;
    }

    data[0] = ICM42670P_ACCEL_CONFIG1_VAL;
    IMU_Write(icm42670p_drv_info.target_addr,
              ICM42670P_ACCEL_CONFIG1_REG, data, 1U);

    sys_busy_loop_us(10);

    data[0] = 0;
    IMU_Read(icm42670p_drv_info.target_addr,
             ICM42670P_ACCEL_CONFIG1_REG, data, 1U);

    if(data[0] != ICM42670P_ACCEL_CONFIG1_VAL)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t IMU_GyroConfig(void)
  \brief       Configures Gyroscope of IMU driver.
  \return      \ref Execution status.
*/
static int32_t IMU_GyroConfig(void)
{
    uint8_t data[4];

    /* Configure Gyroscope */
    data[0] = ICM42670P_GYRO_CONFIG0_VAL;
    IMU_Write(icm42670p_drv_info.target_addr,
              ICM42670P_GYRO_CONFIG0_REG, data, 1U);

    sys_busy_loop_us(10);

    data[0] = 0;
    IMU_Read(icm42670p_drv_info.target_addr,
             ICM42670P_GYRO_CONFIG0_REG, data, 1U);

    if(data[0] != ICM42670P_GYRO_CONFIG0_VAL)
    {
        return ARM_DRIVER_ERROR;
    }

    data[0] = ICM42670P_GYRO_CONFIG1_VAL;
    IMU_Write(icm42670p_drv_info.target_addr,
              ICM42670P_GYRO_CONFIG1_REG, data, 1U);

    sys_busy_loop_us(10);

    data[0] = 0;
    IMU_Read(icm42670p_drv_info.target_addr,
             ICM42670P_GYRO_CONFIG1_REG, data, 1U);

    if(data[0] != ICM42670P_GYRO_CONFIG1_VAL)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t IMU_TempConfig(void)
  \brief       Configures Temp sensor of IMU driver.
  \return      \ref Execution status.
*/
static int32_t IMU_TempConfig(void)
{
    uint8_t data[4];

    /* Configure Temperature sensor */
    data[0] = ICM42670P_TEMP_CONFIG0_VAL;
    IMU_Write(icm42670p_drv_info.target_addr,
              ICM42670P_TEMP_CONFIG0_REG, data, 1U);

    sys_busy_loop_us(10);

    data[0] = 0;
    IMU_Read(icm42670p_drv_info.target_addr,
             ICM42670P_TEMP_CONFIG0_REG, data, 1U);

    if(data[0] != ICM42670P_TEMP_CONFIG0_VAL)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t IMU_IBIConfig(void)
  \brief       Configures In-Band-Interrupt of IMU.
  \return      \ref Execution status.
*/
static int32_t IMU_IBIConfig(void)
{
    uint8_t data[4];

    /* Below code is for IBI for data availability */
    data[0] = ICM42670P_BLK_SEL_W_VAL;
    IMU_Write(icm42670p_drv_info.target_addr,
              ICM42670P_BLK_SEL_W_REG, data, 1U);

    data[0] = ICM42670P_INT_SOURCE8_REG;
    IMU_Write(icm42670p_drv_info.target_addr,
              ICM42670P_MADDR_W_REG, data, 1U);

    data[0] = ICM42670P_INT_SOURCE8_VAL;
    IMU_Write(icm42670p_drv_info.target_addr,
              ICM42670P_M_W_REG, data, 1U);

    sys_busy_loop_us(10);

    data[0] = ICM42670P_BLK_SEL_R_VAL;
    IMU_Write(icm42670p_drv_info.target_addr,
              ICM42670P_BLK_SEL_R_REG, data, 1U);

    data[0] = ICM42670P_INT_SOURCE8_REG;
    IMU_Write(icm42670p_drv_info.target_addr,
              ICM42670P_MADDR_R_REG, data, 1U);

    sys_busy_loop_us(10);
    data[0] = 0;
    IMU_Read(icm42670p_drv_info.target_addr,
             ICM42670P_M_R_REG, data, 1U);

    if(data[0] != ICM42670P_INT_SOURCE8_VAL)
    {
        return ARM_DRIVER_ERROR;
    }
    sys_busy_loop_us(10);

    /* Below code is for for Enabling IBI */
    data[0] = ICM42670P_BLK_SEL_W_VAL;
    IMU_Write(icm42670p_drv_info.target_addr,
              ICM42670P_BLK_SEL_W_REG, data, 1U);

    data[0] = ICM42670P_INTF_CONFIG6_REG;
    IMU_Write(icm42670p_drv_info.target_addr,
              ICM42670P_MADDR_W_REG, data, 1U);

    data[0] = ICM42670P_INTF_CONFIG6_VAL;
    IMU_Write(icm42670p_drv_info.target_addr,
              ICM42670P_M_W_REG, data, 1U);

    sys_busy_loop_us(10);

    data[0] = ICM42670P_BLK_SEL_R_VAL;
    IMU_Write(icm42670p_drv_info.target_addr,
              ICM42670P_BLK_SEL_R_REG, data, 1U);

    data[0] = ICM42670P_INTF_CONFIG6_REG;
    IMU_Write(icm42670p_drv_info.target_addr,
              ICM42670P_MADDR_R_REG, data, 1U);

    sys_busy_loop_us(10);

    data[0] = 0;
    IMU_Read(icm42670p_drv_info.target_addr,
             ICM42670P_M_R_REG, data, 1U);

    if(data[0] != ICM42670P_INTF_CONFIG6_VAL)
    {
        return ARM_DRIVER_ERROR;
    }

    sys_busy_loop_us(10);

    return ARM_DRIVER_OK;
}
/**
  \fn          int32_t IMU_Config(void)
  \brief       Configures IMU driver.
  \return      \ref Execution status.
*/
static int32_t IMU_Config(void)
{
    int32_t ret;
    uint8_t data[4];

    /* Powers up ICM42670 */
    data[0] = ICM42670P_PWR_MGMT0_VAL;
    IMU_Write(icm42670p_drv_info.target_addr,
              ICM42670P_PWR_MGMT0_REG, data, 1U);

    sys_busy_loop_us(10);

    data[0] = 0;
    IMU_Read(icm42670p_drv_info.target_addr,
             ICM42670P_PWR_MGMT0_REG, data, 1U);

    if(data[0] != ICM42670P_PWR_MGMT0_VAL)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Invokes Accelerometer configuration */
    ret = IMU_AccelConfig();
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* Invokes Gyroscope configuration */
    ret = IMU_GyroConfig();
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* Invokes Temperature sensor configuration */
    ret = IMU_TempConfig();
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* Invokes IBI configuration */
    ret = IMU_IBIConfig();
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t IMU_Setup(void)
  \brief       Sets up IMU driver.
               This function will
                 - Sets up i3c instance.
  \return      \ref Execution status.
*/
static int32_t IMU_Setup(void)
{
    int32_t ret;
    uint8_t data[4];

    /* Initializes I3C master */
    ret = I3C_Driver->Control(I3C_MASTER_INIT, 0);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* i3c Speed Mode Configuration: Slow mode*/
    ret = I3C_Driver->Control(I3C_MASTER_SET_BUS_MODE,
                              I3C_BUS_SLOW_MODE);

    /* Rejects Hot-Join request */
    ret = I3C_Driver->Control(I3C_MASTER_SETUP_HOT_JOIN_ACCEPTANCE, 0);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* Rejects Master request */
    ret = I3C_Driver->Control(I3C_MASTER_SETUP_MR_ACCEPTANCE, 0);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* Accepts Slave Interrupt request */
    ret = I3C_Driver->Control(I3C_MASTER_SETUP_SIR_ACCEPTANCE, 1);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* Resets IMU's address */
    ret = IMU_ResetDynAddr();
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* Dynamic Addresses the IMU */
    ret = IMU_SetDynAddr();
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* i3c Speed Mode Configuration: Normal mode*/
    ret = I3C_Driver->Control(I3C_MASTER_SET_BUS_MODE,
                              I3C_BUS_NORMAL_MODE);

    /* Reads Chip ID */
    IMU_Read(icm42670p_drv_info.target_addr, ICM42670P_WHO_AM_I_REG, data, 1U);

    if(data[0] != ICM42670P_REG_WHO_AM_I_VAL)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Configures the slave */
    ret = IMU_Config();
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t IMU_GetAccelData(ARM_IMU_COORDINATES *accel_data)
  \brief       Gets Accelerometer data from IMU driver.
  \param[in]   accel_data : Accelerometer data
  \return      \ref Execution status.
*/
static int32_t IMU_GetAccelData(ARM_IMU_COORDINATES *accel_data)
{
    ARM_IMU_COORDINATES data;
    uint8_t             buf[6];

    /* Reads Acceleromter data */
    IMU_Read(icm42670p_drv_info.target_addr,
             ICM42670P_ACCEL_DATA1_REG,
             buf,
             ICM42670P_ACCEL_DATA_SIZE);

    /* Processes Accelerometer data */
    data.x        = (buf[ICM42670P_ACCEL_DATA_X1_OFFSET] <<
                     ICM42670P_UPPER_DATA_BYTE_Pos);
    data.x       |= buf[ICM42670P_ACCEL_DATA_X0_OFFSET];
    data.y        = (buf[ICM42670P_ACCEL_DATA_Y1_OFFSET] <<
                     ICM42670P_UPPER_DATA_BYTE_Pos);
    data.y       |= buf[ICM42670P_ACCEL_DATA_Y0_OFFSET];
    data.z        = (buf[ICM42670P_ACCEL_DATA_Z1_OFFSET] <<
                     ICM42670P_UPPER_DATA_BYTE_Pos);
    data.z       |= buf[ICM42670P_ACCEL_DATA_Z0_OFFSET];

    accel_data->x = ICM42670P_ACCEL_VAL(data.x);
    accel_data->y = ICM42670P_ACCEL_VAL(data.y);
    accel_data->z = ICM42670P_ACCEL_VAL(data.z);

    /* Resets data received status */
    icm42670p_drv_info.status.data_rcvd  = 0U;

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t IMU_GetGyroData(ARM_IMU_COORDINATES *gyro_data)
  \brief       Gets Gyroscope data from IMU driver.
  \param[in]   gyro_data : Gyroscope data
  \return      \ref Execution status.
*/
static int32_t IMU_GetGyroData(ARM_IMU_COORDINATES *gyro_data)
{
    ARM_IMU_COORDINATES data;
    uint8_t             buf[6];

    /* Reads Gyroscope data */
    IMU_Read(icm42670p_drv_info.target_addr,
             ICM42670P_GYRO_DATA1_REG,
             buf,
             ICM42670P_GYRO_DATA_SIZE);

    /* Processes Gyroscope data */
    data.x       = (buf[ICM42670P_GYRO_DATA_X1_OFFSET] <<
                    ICM42670P_UPPER_DATA_BYTE_Pos);
    data.x      |= buf[ICM42670P_GYRO_DATA_X0_OFFSET];
    data.y       = (buf[ICM42670P_GYRO_DATA_Y1_OFFSET] <<
                    ICM42670P_UPPER_DATA_BYTE_Pos);
    data.y      |= buf[ICM42670P_GYRO_DATA_Y0_OFFSET];
    data.z       = (buf[ICM42670P_GYRO_DATA_Z1_OFFSET] <<
                    ICM42670P_UPPER_DATA_BYTE_Pos);
    data.z      |= buf[ICM42670P_GYRO_DATA_Z0_OFFSET];

    gyro_data->x = ICM42670P_GYRO_VAL(data.x);
    gyro_data->y = ICM42670P_GYRO_VAL(data.y);
    gyro_data->z = ICM42670P_GYRO_VAL(data.z);

    /* Resets data received status */
    icm42670p_drv_info.status.data_rcvd  = 0U;

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t IMU_GetTempData(float *temp_data)
  \brief       Gets Temperature sensor data from IMU driver.
  \param[in]   temp_data : Temperature sensor data
  \return      \ref Execution status.
*/
static int32_t IMU_GetTempData(float *temp_data)
{
    int16_t ltemp;
    uint8_t buf[2];

    /* Reads Temperature Sensor data */
    IMU_Read(icm42670p_drv_info.target_addr,
             ICM42670P_TEMP_DATA1_REG,
             buf,
             ICM42670P_TEMP_DATA_SIZE);

    /* Processes Temp data */
    ltemp      = (buf[ICM42670P_TEMP_DATA1_OFFSET] <<
                  ICM42670P_UPPER_DATA_BYTE_Pos);
    ltemp     |= buf[ICM42670P_TEMP_DATA0_OFFSET];
    *temp_data = ICM42670P_TEMPERATURE(ltemp);

    /* Resets data received status */
    icm42670p_drv_info.status.data_rcvd  = 0U;

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t IMU_Deinit(void)
  \brief       De-initializes IMU driver.
  \return      \ref Execution status.
*/
static int32_t IMU_Deinit(void)
{
    int32_t ret;

    ret = I3C_Driver->PowerControl(ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    ret = I3C_Driver->Uninitialize();
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t  ARM_IMU_Initialize (void)
  \brief       Initializes IMU Interface.
  \param[in]   none.
  \return      \ref execution_status.
*/
static int32_t ARM_IMU_Initialize(void)
{
    int32_t ret;

    if(icm42670p_drv_info.state & ICM42670P_DRIVER_INITIALIZED)
    {
        return ARM_DRIVER_OK;
    }

    ret = IMU_Init();
    if (ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* Initializes driver state and data received status */
    icm42670p_drv_info.state            = ICM42670P_DRIVER_INITIALIZED;
    icm42670p_drv_info.status.data_rcvd = 0U;

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t  ARM_IMU_Uninitialize(void)
  \brief       Uninitializes IMU Interface.
  \param[in]   none.
  \return      \ref execution_status.
*/
static int32_t ARM_IMU_Uninitialize(void)
{
    int32_t ret;

    ret = IMU_Deinit();
    if (ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* Resets driver state and data received status */
    icm42670p_drv_info.state            = 0U;
    icm42670p_drv_info.status.data_rcvd = 0U;

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t  ARM_IMU_PowerControl(ARM_POWER_STATE state)
  \brief       Controls power for IMU Interface.
  \param[in]   state : Power state
  \return      \ref execution_status.
*/
static int32_t ARM_IMU_PowerControl(ARM_POWER_STATE state)
{
    int32_t ret;

    /* Returns error if not initialized */
    if(!(icm42670p_drv_info.state & ICM42670P_DRIVER_INITIALIZED))
    {
        return ARM_DRIVER_ERROR;
    }

    switch (state)
    {
        case ARM_POWER_OFF:
            if(!(icm42670p_drv_info.state & ICM42670P_DRIVER_POWERED))
            {
                return ARM_DRIVER_OK;
            }

            /* Sets state to Powered Down */
            icm42670p_drv_info.state &= ~ICM42670P_DRIVER_POWERED;
            break;

         case ARM_POWER_FULL:
             if(icm42670p_drv_info.state & ICM42670P_DRIVER_POWERED)
             {
                 return ARM_DRIVER_OK;
             }

             /* Sets up IMU */
             ret = IMU_Setup();
             if(ret != ARM_DRIVER_OK)
             {
                 return ret;
             }

             /* Sets state to Powered Up */
             icm42670p_drv_info.state |= ICM42670P_DRIVER_POWERED;
             break;

         case ARM_POWER_LOW:
         default:
         {
             return ARM_DRIVER_ERROR_UNSUPPORTED;
         }
    }
    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t ARM_IMU_Control(uint32_t control, uint32_t arg)
  \brief        Control ICM42670P Slave
  \param[in]    control  : Operation
  \param[in]    arg      : Argument of operation
  \return       \ref execution_status
*/
static int32_t ARM_IMU_Control(uint32_t control, uint32_t arg)
{
    ARM_IMU_COORDINATES *ptr;
    float *temp_data;

    switch(control)
    {
        case IMU_GET_ACCELEROMETER_DATA:
            if(!arg)
            {
                return ARM_DRIVER_ERROR;
            }
            ptr = (ARM_IMU_COORDINATES*)arg;
            IMU_GetAccelData(ptr);
            break;

        case IMU_GET_GYROSCOPE_DATA:
            if(!arg)
            {
                return ARM_DRIVER_ERROR;
            }
            ptr = (ARM_IMU_COORDINATES*)arg;
            IMU_GetGyroData(ptr);
            break;

        case IMU_GET_TEMPERATURE_DATA:
            if(!arg)
            {
                return ARM_DRIVER_ERROR;
            }
            temp_data = (float*)arg;
            IMU_GetTempData(temp_data);
            break;

        case IMU_GET_MAGNETOMETER_DATA:
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    return ARM_DRIVER_OK;
}

extern ARM_DRIVER_IMU ICM42670P;

ARM_DRIVER_IMU ICM42670P =
{
    ARM_IMU_GetVersion,
    ARM_IMU_GetCapabilities,
    ARM_IMU_GetStatus,
    ARM_IMU_Initialize,
    ARM_IMU_Uninitialize,
    ARM_IMU_PowerControl,
    ARM_IMU_Control
};
