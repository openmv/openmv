
/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**
 * @file mhu_driver.h
 * @brief Header file for MHU driver module
 * @par
 * @defgroup host_services host_services
 */

#ifndef __MHU_DRIVER_H__
#define __MHU_DRIVER_H__

/******************************************************************************
 *  I N C L U D E   F I L E S
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "mhu.h"

/*******************************************************************************
 *  M A C R O   D E F I N E S
 ******************************************************************************/

/*
 * Register bit definitions. The name are as in the SSE700 TRM
 */

// Response configuration (RESP_CFG)
#define MHU_NR_RESP    0x1

// Access request (ACCESS_REQUEST)
#define MHU_ACC_REQ    0x1

// Access ready (ACCESS_READY)
#define MHU_ACC_RDY    0x1

// Channel interrupt status
#define MHU_CH_INT_ST  0x1

// Channel interrupt status (CH_INT_ST)
// Channel interrupt clear (CH_INT_CLR)
// Channel interrupt clear (CH_INT_EN)
#define MHU_CH_CLR     0x1

// Interrupt enable (INT_EN)
#define MHU_CHCOMB     0x4
#define MHU_R2NR       0x2
#define MHU_NR2R       0x1

#define MHU_NUMBER_MAX       10
#define MHU_CHANNELS         2

/*******************************************************************************
 *  T Y P E D E F S
 ******************************************************************************/

typedef struct
{
  volatile const uint32_t    CH_ST;      //0x00
  uint32_t                   Reserved0;  //0x04
  uint32_t                   Reserved1;  //0x08
  volatile uint32_t          CH_SET;     //0x0C
  volatile const uint32_t    CH_INT_ST;  //0x10
  volatile uint32_t          CH_INT_CLR; //0x14
  volatile uint32_t          CH_INT_EN;  //0x18
  uint32_t                   Reserved2;  //0x1C
} MHU_sender_channel_window_t;

typedef struct
{
  MHU_sender_channel_window_t    CHANNEL[MHU_NUMBER_OF_CHANNELS_MAX]; //0x0-0xF7C
  volatile const uint32_t        MHU_CFG;
  volatile uint32_t              RESP_CFG;
  volatile uint32_t              ACCESS_REQUEST;
  volatile const uint32_t        ACCESS_READY;
  volatile const uint32_t        INT_ST;
  volatile uint32_t              INT_CLR;
  volatile uint32_t              INT_EN;
  uint32_t                       Reserved0;
  volatile const uint32_t        CH_INT_ST0;
  volatile const uint32_t        CH_INT_ST1;
  volatile const uint32_t        CH_INT_ST2;
  volatile const uint32_t        CH_INT_ST3;
  uint32_t                       Reserved1[6];
  volatile const uint32_t        IIDR;
  volatile const uint32_t        AIDR;
  volatile const uint32_t        PID4;
  volatile const uint32_t        Reserved2[3];
  volatile const uint32_t        PID0;
  volatile const uint32_t        PID1;
  volatile const uint32_t        PID2;
  volatile const uint32_t        PID3;
  volatile const uint32_t        CID0;
  volatile const uint32_t        CID1;
  volatile const uint32_t        CID2;
  volatile const uint32_t        CID3;
} MHU_sender_frame_register_t;

typedef struct {
  volatile const uint32_t    CH_ST;      //0x00
  volatile const uint32_t    CH_ST_MASKED;  //0x04
  volatile uint32_t          CH_CLR;     //0x08
  uint32_t                   Reserved0;  //0x0C
  volatile const uint32_t    CH_MASK_ST; //0x10
  volatile uint32_t          CH_MSK_SET; //0x14
  volatile uint32_t          CH_MSK_CLR; //0x18
  uint32_t                   Reserved1;  //0x1C
} MHU_receiver_channel_window_t;

typedef struct {
  MHU_receiver_channel_window_t    CHANNEL[MHU_NUMBER_OF_CHANNELS_MAX];
  volatile const uint32_t          MHU_CFG;
  uint32_t                         Reserved0[3];
  volatile const uint32_t          INT_ST;
  volatile uint32_t                INT_CLR;
  volatile uint32_t                INT_EN;
  uint32_t                         Reserved1;
  volatile const uint32_t          CH_INT_ST0;
  volatile const uint32_t          CH_INT_ST1;
  volatile const uint32_t          CH_INT_ST2;
  volatile const uint32_t          CH_INT_ST3;
  volatile const uint32_t          Reserved2[6];
  volatile const uint32_t          IIDR;
  volatile const uint32_t          AIDR;
  volatile const uint32_t          PID4;
  volatile const uint32_t          Reserved3[3];
  volatile const uint32_t          PID0;
  volatile const uint32_t          PID1;
  volatile const uint32_t          PID2;
  volatile const uint32_t          PID3;
  volatile const uint32_t          CID0;
  volatile const uint32_t          CID1;
  volatile const uint32_t          CID2;
  volatile const uint32_t          CID3;
} MHU_receiver_frame_register_t;

typedef enum
{
  MHU_SENDER_INTR_NR2R = 0,
  MHU_SENDER_INTR_R2NR,
  MHU_SENDER_INTR_CHCOMB,
} MHU_sender_intr_source_t;

typedef enum
{
  MHU_CHANNEL_0,
  MHU_CHANNEL_1,
  MHU_CHANNEL_2,
  MHU_CHANNEL_3,
  MHU_CHANNEL_4,
  MHU_CHANNEL_5,
  MHU_CHANNEL_6,
  MHU_CHANNEL_7,
  MHU_CHANNEL_8,
  MHU_CHANNEL_9,

  MHU_CHANNEL_10,
  MHU_CHANNEL_11,
  MHU_CHANNEL_12,
  MHU_CHANNEL_13,
  MHU_CHANNEL_14,
  MHU_CHANNEL_15,
  MHU_CHANNEL_16,
  MHU_CHANNEL_17,
  MHU_CHANNEL_18,
  MHU_CHANNEL_19,

  MHU_CHANNEL_20,
  MHU_CHANNEL_21,
  MHU_CHANNEL_22,
  MHU_CHANNEL_23,
  MHU_CHANNEL_24,
  MHU_CHANNEL_25,
  MHU_CHANNEL_26,
  MHU_CHANNEL_27,
  MHU_CHANNEL_28,
  MHU_CHANNEL_29,

  MHU_CHANNEL_30,
  MHU_CHANNEL_31,
  MHU_CHANNEL_32,
  MHU_CHANNEL_33,
  MHU_CHANNEL_34,
  MHU_CHANNEL_35,
  MHU_CHANNEL_36,
  MHU_CHANNEL_37,
  MHU_CHANNEL_38,
  MHU_CHANNEL_39,

  MHU_CHANNEL_40,
  MHU_CHANNEL_41,
  MHU_CHANNEL_42,
  MHU_CHANNEL_43,
  MHU_CHANNEL_44,
  MHU_CHANNEL_45,
  MHU_CHANNEL_46,
  MHU_CHANNEL_47,
  MHU_CHANNEL_48,
  MHU_CHANNEL_49,

  MHU_CHANNEL_50,
  MHU_CHANNEL_51,
  MHU_CHANNEL_52,
  MHU_CHANNEL_53,
  MHU_CHANNEL_54,
  MHU_CHANNEL_55,
  MHU_CHANNEL_56,
  MHU_CHANNEL_57,
  MHU_CHANNEL_58,
  MHU_CHANNEL_59,

  MHU_CHANNEL_60,
  MHU_CHANNEL_61,
  MHU_CHANNEL_62,
  MHU_CHANNEL_63,
  MHU_CHANNEL_64,
  MHU_CHANNEL_65,
  MHU_CHANNEL_66,
  MHU_CHANNEL_67,
  MHU_CHANNEL_68,
  MHU_CHANNEL_69,

  MHU_CHANNEL_70,
  MHU_CHANNEL_71,
  MHU_CHANNEL_72,
  MHU_CHANNEL_73,
  MHU_CHANNEL_74,
  MHU_CHANNEL_75,
  MHU_CHANNEL_76,
  MHU_CHANNEL_77,
  MHU_CHANNEL_78,
  MHU_CHANNEL_79,

  MHU_CHANNEL_80,
  MHU_CHANNEL_81,
  MHU_CHANNEL_82,
  MHU_CHANNEL_83,
  MHU_CHANNEL_84,
  MHU_CHANNEL_85,
  MHU_CHANNEL_86,
  MHU_CHANNEL_87,
  MHU_CHANNEL_88,
  MHU_CHANNEL_89,

  MHU_CHANNEL_90,
  MHU_CHANNEL_91,
  MHU_CHANNEL_92,
  MHU_CHANNEL_93,
  MHU_CHANNEL_94,
  MHU_CHANNEL_95,
  MHU_CHANNEL_96,
  MHU_CHANNEL_97,
  MHU_CHANNEL_98,
  MHU_CHANNEL_99,

  MHU_CHANNEL_100,
  MHU_CHANNEL_101,
  MHU_CHANNEL_102,
  MHU_CHANNEL_103,
  MHU_CHANNEL_104,
  MHU_CHANNEL_105,
  MHU_CHANNEL_106,
  MHU_CHANNEL_107,
  MHU_CHANNEL_108,
  MHU_CHANNEL_109,

  MHU_CHANNEL_110,
  MHU_CHANNEL_111,
  MHU_CHANNEL_112,
  MHU_CHANNEL_113,
  MHU_CHANNEL_114,
  MHU_CHANNEL_115,
  MHU_CHANNEL_116,
  MHU_CHANNEL_117,
  MHU_CHANNEL_118,
  MHU_CHANNEL_119,

  MHU_CHANNEL_120,
  MHU_CHANNEL_121,
  MHU_CHANNEL_122,
  MHU_CHANNEL_123,

  MHU_CHANNEL_MAX,
} mhu_channel_number_t;

typedef enum
{
  MHU_CHANNEL_COMBINED_CH0_31,
  MHU_CHANNEL_COMBINED_CH32_63,
  MHU_CHANNEL_COMBINED_CH64_95,
  MHU_CHANNEL_COMBINED_CH96_123,
} mhu_channel_combined_group_t;

/*******************************************************************************
 *  F U N C T I O N   P R O T O T Y P E S
 ******************************************************************************/

typedef void (*MHU_sender_callback) (void);
void MHU_sender_initialize(uint32_t sender_frame_base_address_list[],
                           uint32_t sender_frame_count,
                           MHU_send_msg_acked_callback_t msg_acked_callback);
	
void MHU_send_message_irq_handler(uint32_t sender_id);

mhu_send_status_t MHU_send_message(uint32_t sender_id,
                                   uint32_t channel_number,
                                   uint32_t message_data);

void MHU_receiver_initialize(
    uint32_t receiver_frame_base_address_list[],
    uint32_t receiver_frame_count,
    MHU_rx_msg_callback_t callback);
void MHU_receive_message_irq_handler(uint32_t receiver_id);

#endif /* __MHU_DRIVER_H__ */
