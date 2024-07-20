/**
  ******************************************************************************
  * @file    stm32100e_eval_cec.h
  * @author  MCD Application Team
  * @version V4.5.0
  * @date    07-March-2011
  * @brief   This file contains all the functions prototypes for the stm32100e_eval_cec
  *          firmware driver.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************  
  */ 
#ifndef __PY_LCD_CEC_H__
#define __PY_LCD_CEC_H__
int cec_init(void);
void cec_deinit(void);
int cec_send_frame(uint8_t InitiatorAddress, uint8_t FollowerAddress, uint8_t MessageLength, uint8_t *Message);
int cec_receive_frame(uint8_t *Message, size_t *MessageLength, uint8_t FollowerAddress, uint8_t *InitiatorAddress);
#endif // __PY_LCD_CEC_H__
