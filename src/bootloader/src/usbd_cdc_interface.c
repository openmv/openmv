#include <stdint.h>
#include "flash.h"
#include "usbdev/usbd_cdc.h"
#include "omv_boardconfig.h"
#include "omv_bootconfig.h"
#include "qspif.h"

#define APP_RX_DATA_SIZE    (2048)
#define APP_TX_DATA_SIZE    (2048)

USBD_CDC_LineCodingTypeDef LineCoding =
{
    115200, /* baud rate*/
    0x00,   /* stop bits-1*/
    0x00,   /* parity - none*/
    0x08    /* nb. of bits 8*/
};

uint32_t BuffLength;
uint32_t UserTxBufPtrIn = 0;    /* Increment this pointer or roll it back to
                                   start address when data are received over USART */
uint32_t UserTxBufPtrOut = 0;   /* Increment this pointer or roll it back to
                                   start address when data are sent over USB */
uint8_t UserRxBuffer[APP_RX_DATA_SIZE];/* Received Data over USB are stored in this buffer */
uint8_t UserTxBuffer[APP_TX_DATA_SIZE];/* Received Data over UART (CDC interface) are stored in this buffer */

static volatile uint8_t ide_connected = 0;
static volatile uint8_t vcp_connected = 0;

#define FLASH_BUF_SIZE  (CDC_DATA_MAX_PACKET_SIZE)
static volatile uint32_t flash_buf_idx=0;
static volatile uint8_t  flash_buf[FLASH_BUF_SIZE];
static const    uint32_t flash_layout[3] = OMV_BOOT_FLASH_LAYOUT;
#if defined(OMV_BOOT_QSPIF_LAYOUT)
#define QSPIF_BUF_SIZE  OMV_BOOT_QSPIF_PAGE_SIZE
static volatile uint32_t qspif_buf_idx=0;
static volatile uint8_t  qspif_buf[QSPIF_BUF_SIZE];
static const    uint32_t qspif_layout[3] = OMV_BOOT_QSPIF_LAYOUT;
#else
static const    uint32_t qspif_layout[3] = {0, 0, 0};
#endif
static const    uint32_t bootloader_version = OMV_BOOT_VERSION;

/* USB handler declaration */
extern USBD_HandleTypeDef  USBD_Device;

/* Private function prototypes -----------------------------------------------*/
static int8_t CDC_Itf_Init(void);
static int8_t CDC_Itf_DeInit(void);
static int8_t CDC_Itf_Control(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Itf_Receive(uint8_t* pbuf, uint32_t *Len);

enum bootldr_cmd {
    BOOTLDR_START           = 0xABCD0001,
    BOOTLDR_RESET           = 0xABCD0002,
    BOOTLDR_FLASH_ERASE     = 0xABCD0004,
    BOOTLDR_FLASH_WRITE     = 0xABCD0008,
    BOOTLDR_FLASH_LAYOUT    = 0xABCD0010,
    BOOTLDR_QSPIF_ERASE     = 0xABCD1004,
    BOOTLDR_QSPIF_WRITE     = 0xABCD1008,
    BOOTLDR_QSPIF_LAYOUT    = 0xABCD1010,
    BOOTLDR_QSPIF_MEMTEST   = 0xABCD1020,
};

USBD_CDC_ItfTypeDef USBD_CDC_fops =
{
    CDC_Itf_Init,
    CDC_Itf_DeInit,
    CDC_Itf_Control,
    CDC_Itf_Receive
};

/**
 * @brief  CDC_Itf_Init
 *         Initializes the CDC media low layer
 * @param  None
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CDC_Itf_Init(void)
{
    // Set Application Buffers
    USBD_CDC_SetTxBuffer(&USBD_Device, UserTxBuffer, 0);
    USBD_CDC_SetRxBuffer(&USBD_Device, UserRxBuffer);

    return (USBD_OK);
}

/**
 * @brief  CDC_Itf_DeInit
 *         DeInitializes the CDC media low layer
 * @param  None
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CDC_Itf_DeInit(void)
{
    return USBD_OK;
}

/**
 * @brief  CDC_Itf_Control
 *         Manage the CDC class requests
 * @param  Cmd: Command code
 * @param  Buf: Buffer containing command data (request parameters)
 * @param  Len: Number of data to be sent (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CDC_Itf_Control(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{ 
    switch (cmd) {
        case CDC_SEND_ENCAPSULATED_COMMAND:
            /* Add your code here */
            break;

        case CDC_GET_ENCAPSULATED_RESPONSE:
            /* Add your code here */
            break;

        case CDC_SET_COMM_FEATURE:
            /* Add your code here */
            break;

        case CDC_GET_COMM_FEATURE:
            /* Add your code here */
            break;

        case CDC_CLEAR_COMM_FEATURE:
            /* Add your code here */
            break;

        case CDC_SET_LINE_CODING:
            LineCoding.bitrate = (uint32_t) (pbuf[0] | (pbuf[1] << 8) |
                                            (pbuf[2] << 16) | (pbuf[3] << 24));
            LineCoding.format     = pbuf[4];
            LineCoding.paritytype = pbuf[5];
            LineCoding.datatype   = pbuf[6];
            break;

        case CDC_GET_LINE_CODING:
            pbuf[0] = (uint8_t)(LineCoding.bitrate);
            pbuf[1] = (uint8_t)(LineCoding.bitrate >> 8);
            pbuf[2] = (uint8_t)(LineCoding.bitrate >> 16);
            pbuf[3] = (uint8_t)(LineCoding.bitrate >> 24);
            pbuf[4] = LineCoding.format;
            pbuf[5] = LineCoding.paritytype;
            pbuf[6] = LineCoding.datatype;
            break;

        case CDC_SET_CONTROL_LINE_STATE:
            vcp_connected = length & 1; // wValue is passed in Len (bit of a hack)
            break;

        case CDC_SEND_BREAK:
            /* Add your code here */
            break;

        default:
            break;
    }

    return (USBD_OK);
}

// This function is called to process outgoing data.  We hook directly into the
// SOF (start of frame) callback so that it is called exactly at the time it is
// needed (reducing latency), and often enough (increasing bandwidth).
void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
{
    uint32_t buffptr;
    uint32_t buffsize;

    if(UserTxBufPtrOut != UserTxBufPtrIn) {
        if (UserTxBufPtrOut > UserTxBufPtrIn) /* Rollback */ {
            buffsize = APP_RX_DATA_SIZE - UserTxBufPtrOut;
        } else {
            buffsize = UserTxBufPtrIn - UserTxBufPtrOut;
        }

        buffptr = UserTxBufPtrOut;
        USBD_CDC_SetTxBuffer(&USBD_Device, (uint8_t*)&UserTxBuffer[buffptr], buffsize);

        if (USBD_CDC_TransmitPacket(&USBD_Device) == USBD_OK) {
            UserTxBufPtrOut += buffsize;
            if (UserTxBufPtrOut == APP_RX_DATA_SIZE) {
                UserTxBufPtrOut = 0;
            }
        }
    }
}

void CDC_Tx(uint8_t *buf, uint32_t len)
{
    for (int i=0; i<len; i++) {
        UserTxBuffer[UserTxBufPtrIn++] = buf[i];

        /* To avoid buffer overflow */
        if(UserTxBufPtrIn == APP_RX_DATA_SIZE) {
            UserTxBufPtrIn = 0;
        }
    }
}

/**
 * @brief  CDC_Itf_DataRx
 *         Data received over USB OUT endpoint are sent over CDC interface
 *         through this function.
 * @param  Buf: Buffer of data to be transmitted
 * @param  Len: Number of data received (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CDC_Itf_Receive(uint8_t *Buf, uint32_t *Len)
{
    static volatile uint32_t flash_offset;
    #if defined(OMV_BOOT_QSPIF_LAYOUT)
    static volatile uint32_t qspif_offset=0;
    #endif

    uint32_t *cmd_buf = (uint32_t*) Buf;
    uint32_t cmd = *cmd_buf++;

    switch (cmd) {
        case BOOTLDR_START:
            flash_buf_idx = 0;
            #if defined(OMV_BOOT_QSPIF_LAYOUT)
            qspif_buf_idx = 0;
            #endif
            ide_connected = 1;
            flash_offset = MAIN_APP_ADDR;
            // Send back the bootloader version.
            CDC_Tx((uint8_t *) &bootloader_version, 4);
            break;

        case BOOTLDR_FLASH_LAYOUT:
            // Return flash layout (bootloader v2)
            CDC_Tx((uint8_t*) flash_layout, 12);
            break;

        case BOOTLDR_FLASH_ERASE: {
            uint32_t sector = *cmd_buf;
            flash_erase(sector);
            break;
        }

        case BOOTLDR_FLASH_WRITE: {
            uint8_t *buf =  Buf + 4;
            uint32_t len = *Len - 4;
            for (int i=0; i<len; i++) {
                flash_buf[flash_buf_idx++] = buf[i];
                if (flash_buf_idx == FLASH_BUF_SIZE) {
                    flash_buf_idx = 0;
                    flash_write((uint32_t*)flash_buf, flash_offset, FLASH_BUF_SIZE);
                    flash_offset += FLASH_BUF_SIZE;
                }
            }
            break; 
        }

        case BOOTLDR_QSPIF_LAYOUT:
            CDC_Tx((uint8_t*) qspif_layout, 12);
            break;

        case BOOTLDR_QSPIF_ERASE: {
            #if defined(OMV_BOOT_QSPIF_LAYOUT)
            uint32_t block = *cmd_buf;
            qspif_erase_block(block * OMV_BOOT_QSPIF_BLOCK_SIZE);
            #endif
            break;
        }

        case BOOTLDR_QSPIF_WRITE: {
            #if defined(OMV_BOOT_QSPIF_LAYOUT)
            uint8_t *buf =  Buf + 4;
            uint32_t len = *Len - 4;
            for (int i=0; i<len; i++) {
                qspif_buf[qspif_buf_idx++] = buf[i];
                if (qspif_buf_idx == QSPIF_BUF_SIZE) {
                    qspif_buf_idx = 0;
                    qspif_write((uint8_t*) qspif_buf, qspif_offset, QSPIF_BUF_SIZE);
                    qspif_offset += QSPIF_BUF_SIZE;
                }
            }
            #endif
            break;
        }

        case BOOTLDR_QSPIF_MEMTEST: {
            uint32_t qspif_test = 0;
            #if defined(OMV_BOOT_QSPIF_LAYOUT)
            qspif_test = (qspif_memory_test() == 0);
            #endif
            CDC_Tx((uint8_t*) &qspif_test, 4);
            break;
        }

        case BOOTLDR_RESET: {
            ide_connected = 0;
            if (flash_buf_idx) {
                // Pad and flush the last packet
                for (int i=flash_buf_idx; i<FLASH_BUF_SIZE; i++) {
                    flash_buf[i] = 0xFF;
                }
                flash_write((uint32_t*)flash_buf, flash_offset, FLASH_BUF_SIZE);
            }
            #if defined(OMV_BOOT_QSPIF_LAYOUT)
            if (qspif_buf_idx) {
                // Pad and flush the last packet
                for (int i=qspif_buf_idx; i<QSPIF_BUF_SIZE; i++) {
                    qspif_buf[i] = 0xFF;
                }
                qspif_write((uint8_t*) qspif_buf, qspif_offset, QSPIF_BUF_SIZE);
            }
            #endif
            break;
        }
    }

    // Initiate next USB packet transfer
    USBD_CDC_ReceivePacket(&USBD_Device);
    return USBD_OK;
}


uint8_t USBD_VCP_Connected(void)
{
    return vcp_connected;
}

uint8_t USBD_IDE_Connected(void)
{
    return ide_connected;
}
