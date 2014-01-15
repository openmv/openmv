#include "sccb.h"
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_syscfg.h>
#include <stm32f4xx_misc.h>
#include <stm32f4xx_i2c.h>

/* I2C defs */ 
#define I2Cx               I2C1
#define I2C_CLOCK          RCC_APB1Periph_I2C1
#define I2C_FREQ           (30000)
#define I2C_SLAVE_ADDR     (0x60)
#define I2C_MAX_TIMEOUT    (10000)

/* I2C GPIO defs */
#define I2C_GPIO_PORT       GPIOB
#define I2C_GPIO_CLOCK      RCC_AHB1Periph_GPIOB
#define I2C_GPIO_AF         GPIO_AF_I2C1
#define I2C_GPIO_SCL_PIN    GPIO_Pin_8
#define I2C_GPIO_SDA_PIN    GPIO_Pin_9
#define I2C_GPIO_SCL_SRC    GPIO_PinSource8
#define I2C_GPIO_SDA_SRC    GPIO_PinSource9

void SCCB_Init()
{
    I2C_InitTypeDef  I2C_InitStruct;
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable I2C/GPIO clocks */
    RCC_APB1PeriphClockCmd(I2C_CLOCK, ENABLE);
    RCC_AHB1PeriphClockCmd(I2C_GPIO_CLOCK, ENABLE); 

    /* Connect I2C GPIOs to AF4 */
    GPIO_PinAFConfig(I2C_GPIO_PORT, I2C_GPIO_SCL_SRC, I2C_GPIO_AF);
    GPIO_PinAFConfig(I2C_GPIO_PORT, I2C_GPIO_SDA_SRC, I2C_GPIO_AF);

    /* Configure I2C GPIOs */  
    GPIO_InitStructure.GPIO_Pin   = I2C_GPIO_SCL_PIN|I2C_GPIO_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(I2C_GPIO_PORT, &GPIO_InitStructure);

    /* Configure I2Cx */
    I2C_DeInit(I2Cx);
      
    /* Set the I2C structure parameters */
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_OwnAddress1 = 0xFE;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStruct.I2C_ClockSpeed = 30000;

    /* Initialize the I2C peripheral w/ selected parameters */
    I2C_Init(I2Cx, &I2C_InitStruct);

    /* Enable the I2C peripheral */
    I2C_Cmd(I2Cx, ENABLE);
}

uint8_t SCCB_Write(uint8_t addr, uint8_t data)
{
    uint32_t timeout = I2C_MAX_TIMEOUT;

    /* Generate the Start Condition */
    I2C_GenerateSTART(I2Cx, ENABLE);

    /* Test on I2Cx EV5 and clear it */
    timeout = I2C_MAX_TIMEOUT; /* Initialize timeout value */
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
    {
      /* If the timeout delay is exeeded, exit with error code */
      if ((timeout--) == 0) return 0xFF;
    }
     
    /* Send DCMI selcted device slave Address for write */
    I2C_Send7bitAddress(I2Cx, I2C_SLAVE_ADDR, I2C_Direction_Transmitter);

    /* Test on I2Cx EV6 and clear it */
    timeout = I2C_MAX_TIMEOUT; /* Initialize timeout value */
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
      /* If the timeout delay is exeeded, exit with error code */
      if ((timeout--) == 0) return 0xFF;
    }

    /* Send I2Cx location address LSB */
    I2C_SendData(I2Cx, (uint8_t)(addr));

    /* Test on I2Cx EV8 and clear it */
    timeout = I2C_MAX_TIMEOUT; /* Initialize timeout value */
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
      /* If the timeout delay is exeeded, exit with error code */
      if ((timeout--) == 0) return 0xFF;
    }

    /* Send Data */
    I2C_SendData(I2Cx, data);

    /* Test on I2Cx EV8 and clear it */
    timeout = I2C_MAX_TIMEOUT; /* Initialize timeout value */
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
      /* If the timeout delay is exeeded, exit with error code */
      if ((timeout--) == 0) return 0xFF;
    }  

    /* Send I2Cx STOP Condition */
    I2C_GenerateSTOP(I2Cx, ENABLE);

    /* If operation is OK, return 0 */
    return 0;
}

uint8_t SCCB_Read(uint8_t addr)
{
    uint32_t timeout = I2C_MAX_TIMEOUT;
    uint8_t data = 0;

    /* Generate the Start Condition */
    I2C_GenerateSTART(I2Cx, ENABLE);

    /* Test on I2Cx EV5 and clear it */
    timeout = I2C_MAX_TIMEOUT; /* Initialize timeout value */
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
    {
      /* If the timeout delay is exeeded, exit with error code */
      if ((timeout--) == 0) return 0xFF;
    }

    /* Send DCMI selcted device slave Address for write */
    I2C_Send7bitAddress(I2Cx, I2C_SLAVE_ADDR, I2C_Direction_Transmitter);

    /* Test on I2Cx EV6 and clear it */
    timeout = I2C_MAX_TIMEOUT; /* Initialize timeout value */
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
      /* If the timeout delay is exeeded, exit with error code */
      if ((timeout--) == 0) return 0xFF;
    }

    /* Send I2Cx location address LSB */
    I2C_SendData(I2Cx, (uint8_t)(addr));

    /* Test on I2Cx EV8 and clear it */
    timeout = I2C_MAX_TIMEOUT; /* Initialize timeout value */
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
      /* If the timeout delay is exeeded, exit with error code */
      if ((timeout--) == 0) return 0xFF;
    } 

    /* Prepare Stop after receiving data */
    I2C_GenerateSTOP(I2Cx, ENABLE);

    /* Clear AF flag if arised */
    I2Cx->SR1 |= (uint16_t)0x0400;

    /* Generate the Start Condition */
    I2C_GenerateSTART(I2Cx, ENABLE);

    /* Test on I2Cx EV6 and clear it */
    timeout = I2C_MAX_TIMEOUT; /* Initialize timeout value */
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
    {
      /* If the timeout delay is exeeded, exit with error code */
      if ((timeout--) == 0) return 0xFF;
    } 

    /* Send DCMI selcted device slave Address for write */
    I2C_Send7bitAddress(I2Cx, I2C_SLAVE_ADDR, I2C_Direction_Receiver);

    /* Test on I2Cx EV6 and clear it */
    timeout = I2C_MAX_TIMEOUT; /* Initialize timeout value */
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    {
      /* If the timeout delay is exeeded, exit with error code */
      if ((timeout--) == 0) return 0xFF;
    }

    /* Prepare an NACK for the next data received */
    I2C_AcknowledgeConfig(I2Cx, DISABLE);

    /* Test on I2Cx EV7 and clear it */
    timeout = I2C_MAX_TIMEOUT; /* Initialize timeout value */
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
    {
      /* If the timeout delay is exeeded, exit with error code */
      if ((timeout--) == 0) return 0xFF;
    }

    /* Prepare Stop after receiving data */
    I2C_GenerateSTOP(I2Cx, ENABLE);

    /* Receive the Data */
    data = I2C_ReceiveData(I2Cx);

    /* return the read data */
    return data;
}
