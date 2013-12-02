#include "sccb.h"
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>

#define SCCB_SIOC_PIN       GPIO_Pin_8
#define SCCB_SIOD_PIN       GPIO_Pin_9

#define SCCB_PORT           GPIOB
#define SCCB_PORT_CLOCK     RCC_AHB1Periph_GPIOB

#define SCCB_SIOC_H()    	GPIO_SetBits(GPIOB, SCCB_SIOC_PIN)  
#define SCCB_SIOC_L()       GPIO_ResetBits(GPIOB, SCCB_SIOC_PIN)
   
#define SCCB_SIOD_H()    	GPIO_SetBits(GPIOB, SCCB_SIOD_PIN)  
#define SCCB_SIOD_L()     	GPIO_ResetBits(GPIOB, SCCB_SIOD_PIN)

#define SCCB_SIOD_READ()     GPIO_ReadInputDataBit(GPIOB, SCCB_SIOD_PIN)
#define SCCB_SIOD_WRITE(bit) GPIO_WriteBit(SCCB_PORT, SCCB_SIOD_PIN, bit);

#define SLAVE_ID_ADDRESS    (0x60)
#define ACK 0
#define NACK 1

static void delay(void)
{
   volatile uint32_t d = 10;
   while(d--) {
   }
}

void SCCB_Init()
{
   GPIO_InitTypeDef GPIO_InitStructure;

   /* Enable the software I2C Clock */
   RCC_AHB1PeriphClockCmd(SCCB_PORT_CLOCK, ENABLE);

   /* Configure the SDA and SCL pins */
   GPIO_InitStructure.GPIO_Pin = SCCB_SIOC_PIN | SCCB_SIOD_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(SCCB_PORT, &GPIO_InitStructure);

   SCCB_SIOC_H();
   SCCB_SIOD_H();
   delay();
}


void sda_input()
{
   GPIO_InitTypeDef GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Pin = SCCB_SIOD_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(SCCB_PORT, &GPIO_InitStructure);
}

void sda_output()
{
   GPIO_InitTypeDef GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Pin = SCCB_SIOD_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(SCCB_PORT, &GPIO_InitStructure);
}

static void SCCB_Start(void)
{
    /* The start of data transmission occurs when
       SIO_D is driven low while SIO_C is high */
    SCCB_SIOD_H();
    delay();

    SCCB_SIOD_L();
    delay();

    SCCB_SIOC_L();
    delay();
}

static void SCCB_Stop(void)
{
    /* The stop of data transmission occurs when
       SIO_D is driven high while SIO_C is high */
    SCCB_SIOD_L();
    delay();

    SCCB_SIOC_H();
    delay();

    SCCB_SIOD_H();
    delay();
}

static uint8_t SCCB_ReadByte(char ack)
{
   uint8_t data = 0;
   char i;

   sda_input();
   for(i = 0; i < 8; i++) {
      SCCB_SIOC_H();
      delay();

      data |= SCCB_SIOD_READ()&0x01;
      if(i != 7)
         data <<= 1;

      SCCB_SIOC_L();
      delay();
   }
   sda_output();

   // issue the ack
   SCCB_SIOD_H();
   delay();

   SCCB_SIOC_H();  
   delay();

   SCCB_SIOC_L();
   delay();

   return data;
}

static char SCCB_WriteByte(uint8_t data)
{
    char i;
    /* shift the 8 data bits */
    for (i=0; i<8; i++) {
        if (data & 0x80) {
            SCCB_SIOD_H();
        } else {
            SCCB_SIOD_L();
        }
        data <<= 1;
        delay();

        SCCB_SIOC_H();
        delay();

        SCCB_SIOC_L();
        delay();
   }

   SCCB_SIOD_H();
   delay();

   SCCB_SIOC_H();
   delay();
  
   sda_input();
   i = SCCB_SIOD_READ();
   sda_output();

   SCCB_SIOC_L();
   delay();

   return i;
}

// returns ack state, 0 means acknowledged
uint8_t SCCB_Write(uint8_t reg_address, uint8_t data)
{
   uint8_t result = 0;

   /* 3-phase write transmission cycle */
   SCCB_Start();
   result |= SCCB_WriteByte(SLAVE_ID_ADDRESS); 
   result |= SCCB_WriteByte(reg_address);
   result |= SCCB_WriteByte(data); 
   SCCB_Stop();

   return result;
}

uint8_t SCCB_Read(uint8_t reg_addr)
{
   uint8_t data;

   /* 2-phase write transmission cycle */

   SCCB_Start();
   SCCB_WriteByte(SLAVE_ID_ADDRESS);
   SCCB_WriteByte(reg_addr); // id address
   SCCB_Stop();

   /* 2-phase read transmission cycle */
   SCCB_Start();
   SCCB_WriteByte(SLAVE_ID_ADDRESS | 0x01);
   data = SCCB_ReadByte(NACK);
   SCCB_Stop();

   return data;
}

