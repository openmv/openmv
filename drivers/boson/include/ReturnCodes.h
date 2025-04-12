//  /////////////////////////////////////////////////////
//  // DO NOT EDIT.  This is a machine generated file. //
//  /////////////////////////////////////////////////////
/******************************************************************************/
/*                                                                            */
/*  Copyright (C) 2018, FLIR Systems                                          */
/*  All rights reserved.                                                      */
/*                                                                            */
/*  This document is controlled to FLIR Technology Level 2. The information   */
/*  contained in this document pertains to a dual use product controlled for  */
/*  export by the Export Administration Regulations (EAR). Diversion contrary */
/*  to US law is prohibited. US Department of Commerce authorization is not   */
/*  required prior to export or transfer to foreign persons or parties unless */
/*  otherwise prohibited.                                                     */
/*                                                                            */
/******************************************************************************/

#ifndef FLR_RESULT_CODES_H
#define FLR_RESULT_CODES_H

enum _returnCodes {
    MAX_ERR_CODE = 0xFFFFFFFF, // 65535
    
    R_SUCCESS                                = 0U, // 0x00000000
    R_UART_UNSPECIFIED_FAILURE               = 1U, // 0x00000001
    R_UART_PORT_FAILURE                      = 2U, // 0x00000002
    R_UART_RECEIVE_TIMEOUT                   = 3U, // 0x00000003
    R_UART_PORT_ALREADY_OPEN                 = 4U, // 0x00000004
    
    R_SDK_API_UNSPECIFIED_FAILURE            = 272U, // 0x00000110
    R_SDK_API_NOT_DEFINED                    = 273U, // 0x00000111
    R_SDK_PKG_UNSPECIFIED_FAILURE            = 288U, // 0x00000120
    R_SDK_PKG_BUFFER_OVERFLOW                = 303U, // 0x0000012F
    R_SDK_DSPCH_UNSPECIFIED_FAILURE          = 304U, // 0x00000130
    R_SDK_DSPCH_SEQUENCE_MISMATCH            = 305U, // 0x00000131
    R_SDK_DSPCH_ID_MISMATCH                  = 306U, // 0x00000132
    R_SDK_DSPCH_MALFORMED_STATUS             = 307U, // 0x00000133
    R_SDK_TX_UNSPECIFIED_FAILURE             = 320U, // 0x00000140
    R_CAM_RX_UNSPECIFIED_FAILURE             = 336U, // 0x00000150
    R_CAM_DSPCH_UNSPECIFIED_FAILURE          = 352U, // 0x00000160
    R_CAM_DSPCH_BAD_CMD_ID                   = 353U, // 0x00000161
    R_CAM_DSPCH_BAD_PAYLOAD_STATUS           = 354U, // 0x00000162
    R_CAM_PKG_UNSPECIFIED_FAILURE            = 368U, // 0x00000170
    R_CAM_PKG_INSUFFICIENT_BYTES             = 381U, // 0x0000017D
    R_CAM_PKG_EXCESS_BYTES                   = 382U, // 0x0000017E
    R_CAM_PKG_BUFFER_OVERFLOW                = 383U, // 0x0000017F
    R_CAM_API_UNSPECIFIED_FAILURE            = 384U, // 0x00000180
    R_CAM_API_INVALID_INPUT                  = 385U, // 0x00000181
    R_CAM_TX_UNSPECIFIED_FAILURE             = 400U, // 0x00000190
    R_API_RX_UNSPECIFIED_FAILURE             = 416U, // 0x000001A0
    R_CAM_FEATURE_NOT_ENABLED                = 432U, // 0x000001B0
    
    FLR_OK                                   = 0U, // 0x00000000
    FLR_COMM_OK                              = 0U, // 0x00000000
    
    FLR_ERROR                                = 513U, // 0x00000201
    FLR_NOT_READY                            = 514U, // 0x00000202
    FLR_RANGE_ERROR                          = 515U, // 0x00000203
    FLR_CHECKSUM_ERROR                       = 516U, // 0x00000204
    FLR_BAD_ARG_POINTER_ERROR                = 517U, // 0x00000205
    FLR_DATA_SIZE_ERROR                      = 518U, // 0x00000206
    FLR_UNDEFINED_FUNCTION_ERROR             = 519U, // 0x00000207
    FLR_ILLEGAL_ADDRESS_ERROR                = 520U, // 0x00000208
    FLR_BAD_OUT_TYPE                         = 521U, // 0x00000209
    FLR_BAD_OUT_INTERFACE                    = 522U, // 0x0000020A
    FLR_DEPRECATED_FUNCTION_ERROR            = 523U, // 0x0000020B
    FLR_COMM_PORT_NOT_OPEN                   = 613U, // 0x00000265
    FLR_COMM_INVALID_PORT_ERROR              = 614U, // 0x00000266
    FLR_COMM_RANGE_ERROR                     = 615U, // 0x00000267
    FLR_ERROR_CREATING_COMM                  = 616U, // 0x00000268
    FLR_ERROR_STARTING_COMM                  = 617U, // 0x00000269
    FLR_ERROR_CLOSING_COMM                   = 618U, // 0x0000026A
    FLR_COMM_CHECKSUM_ERROR                  = 619U, // 0x0000026B
    FLR_COMM_NO_DEV                          = 620U, // 0x0000026C
    FLR_COMM_TIMEOUT_ERROR                   = 621U, // 0x0000026D
    FLR_COMM_ERROR_WRITING_COMM              = 621U, // 0x0000026D
    FLR_COMM_ERROR_READING_COMM              = 622U, // 0x0000026E
    FLR_COMM_COUNT_ERROR                     = 623U, // 0x0000026F
    FLR_OPERATION_CANCELED                   = 638U, // 0x0000027E
    FLR_UNDEFINED_ERROR_CODE                 = 639U, // 0x0000027F
    FLR_LEN_NOT_SUBBLOCK_BOUNDARY            = 640U, // 0x00000280
    FLR_CONFIG_ERROR                         = 641U, // 0x00000281
    FLR_I2C_ERROR                            = 642U, // 0x00000282
    FLR_CAM_BUSY                             = 643U, // 0x00000283
    FLR_HEATER_ERROR                         = 644U, // 0x00000284
    FLR_WINDOW_ERROR                         = 645U, // 0x00000285
    FLR_VBATT_ERROR                          = 646U, // 0x00000286
    R_SYM_UNSPECIFIED_FAILURE                = 768U, // 0x00000300
    R_SYM_INVALID_POSITION_ERROR             = 769U, // 0x00000301
    FLR_RES_NOT_AVAILABLE                    = 800U, // 0x00000320
    FLR_RES_NOT_IMPLEMENTED                  = 801U, // 0x00000321
    FLR_RES_RANGE_ERROR                      = 802U, // 0x00000322
    FLR_SYSTEMINIT_XX_ERROR                  = 900U, // 0x00000384
    FLR_SDIO_XX_ERROR                        = 1000U, // 0x000003E8
    FLR_STOR_SD_XX_ERROR                     = 1100U, // 0x0000044C
    FLR_USB_VIDEO_XX_ERROR                   = 1200U, // 0x000004B0
    FLR_USB_CDC_XX_ERROR                     = 1300U, // 0x00000514
    FLR_USB_MSD_XX_ERROR                     = 1400U, // 0x00000578
    FLR_NET_XX_ERROR                         = 1500U, // 0x000005DC
    FLR_BT_XX_ERROR                          = 1600U, // 0x00000640
    FLR_FLASH_XX_ERROR                       = 1700U, // 0x000006A4
    FLR_FLASH_ERASE_ERROR                    = 1701U, // 0x000006A5
    FLR_FLASH_WRITE_ERROR                    = 1702U, // 0x000006A6
    FLR_FLASH_READ_ERROR                     = 1703U, // 0x000006A7
    FLR_FLASH_BUSY_ERROR                     = 1704U, // 0x000006A8
    FLR_FLASH_ADDRESS_ERROR                  = 1705U, // 0x000006A9
    FLR_FLASH_RANGE_ERROR                    = 1706U, // 0x000006AA
    FLR_FLASH_ACCESS_ERROR                   = 1707U, // 0x000006AB
    FLR_FLASH_OPERATION_RETRY_ERROR          = 1708U, // 0x000006AC
    FLR_FLASH_UNKNOWN_ERROR                  = 1709U, // 0x000006AD
    FLR_FLASHHDR_ERASED                      = 1800U, // 0x00000708
    FLR_FLASHHDR_PARTIAL_WRITE               = 1801U, // 0x00000709
    FLR_FLASHHDR_WRONG_FOOTER_ID             = 1802U, // 0x0000070A
    FLR_FLASHHDR_WRONG_FOOTER_METADATA       = 1803U, // 0x0000070B
    FLR_FLASHHDR_WRONG_FOOTER_TYPE           = 1804U, // 0x0000070C
    FLR_FLASHHDR_WRONG_HEADER_SIZE           = 1805U, // 0x0000070D
    FLR_FLASHHDR_FOOTER_CRC_ERROR            = 1806U, // 0x0000070E
    FLR_UNKNOWN_PROBE_MODEL                  = 1900U, // 0x0000076C
};
typedef enum _returnCodes FLR_RESULT;

#endif
