/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * main function.
 */
#include STM32_HAL_H
#include <stdbool.h>
#include "sdram.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_uvc.h"
#include "usbd_uvc_if.h"
#include "omv_i2c.h"
#include "omv_csi.h"
#include "framebuffer.h"
#include "omv_boardconfig.h"
#if OMV_ENABLE_BOOTLOADER
#include "omv_bootconfig.h"
#endif

#if defined(I2C1)
I2C_HandleTypeDef I2CHandle1;
#endif
#if defined(I2C2)
I2C_HandleTypeDef I2CHandle2;
#endif
#if defined(I2C3)
I2C_HandleTypeDef I2CHandle3;
#endif
#if defined(I2C4)
I2C_HandleTypeDef I2CHandle4;
#endif

#if defined(SPI1)
SPI_HandleTypeDef SPIHandle1;
#endif
#if defined(SPI2)
SPI_HandleTypeDef SPIHandle2;
#endif
#if defined(SPI3)
SPI_HandleTypeDef SPIHandle3;
#endif
#if defined(SPI4)
SPI_HandleTypeDef SPIHandle4;
#endif
#if defined(SPI5)
SPI_HandleTypeDef SPIHandle5;
#endif
#if defined(SPI6)
SPI_HandleTypeDef SPIHandle6;
#endif

DMA_HandleTypeDef *dma_handle[16];

extern omv_csi_t csi;
USBD_HandleTypeDef hUsbDeviceFS;
extern volatile uint8_t g_uvc_stream_status;
extern struct uvc_streaming_control videoCommitControl;

void mp_hal_delay_ms(uint32_t Delay)
{
    HAL_Delay(Delay);
}

mp_uint_t mp_hal_ticks_ms(void)
{
    return HAL_GetTick();
}

void __attribute__((noreturn)) __fatal_error(const char *error)
{
    while (1) {
    }
}

void __attribute__((noreturn)) mp_raise_msg_varg(const void *err, ...)
{
    while (1) {
    }
}

#ifdef STACK_PROTECTOR
uint32_t __stack_chk_guard=0xDEADBEEF;

void __attribute__((noreturn)) __stack_chk_fail(void)
{
    __asm__ volatile ("BKPT");
    while (1) {
    }
}
#endif

void *xrealloc(void *mem, uint32_t size)
{
    // Will never be called.
    __fatal_error("");
    return NULL;
}

NORETURN void fb_alloc_fail()
{
    __fatal_error("");
}

int puts(const char *s) {
    return 0;
}

int printf(const char *fmt, ...)
{
    return 0;
}

NORETURN void nlr_jump(void *val)
{
    __fatal_error("");
}

void *mp_obj_new_exception_msg(const void *exc_type, const char *msg)
{
    return NULL;
}

void mp_handle_pending(bool x) {
}

const void *mp_type_MemoryError = NULL;

const void *mp_sys_stdout_print = NULL;

static uint8_t frame_index = 0;
static uint8_t format_index = 0;

static uint8_t uvc_header[2] = { 2, 0 };
static uint8_t packet[VIDEO_PACKET_SIZE];
uint32_t packet_size = VIDEO_PACKET_SIZE-2;

bool process_frame(image_t *image)
{
    uint32_t xfer_size = 0;
    uint32_t xfer_bytes = 0;
    uint8_t *dst = packet + 2;

    xfer_size = image->w * image->h * image->bpp;

    while (xfer_bytes < xfer_size) {
        packet[0] = uvc_header[0];
        packet[1] = uvc_header[1];

        switch (videoCommitControl.bFormatIndex) {
            case VS_FMT_INDEX(GREY): {
                for (int i=0; i<packet_size; i++, xfer_bytes+=image->bpp) {
                    dst[i+0] = image->pixels[xfer_bytes+0];
                }
                break;
            }
            case VS_FMT_INDEX(YUYV):
            case VS_FMT_INDEX(RGB565): {
                for (int i=0; i<packet_size; i+=2, xfer_bytes+=2) {
                    dst[i+0] = image->pixels[xfer_bytes+1];
                    dst[i+1] = image->pixels[xfer_bytes+0];
                }
                break;
            }
            default:
                break;
        }

        if (xfer_bytes == xfer_size) {
            packet[1] |= 0x2;    // Flag end of frame
            uvc_header[1] ^= 1;  // Toggle bit 0 for next new frame
        }

        while (UVC_Transmit_FS(packet, VIDEO_PACKET_SIZE) != USBD_OK) {
            __WFI();
        }
    }

    if (g_uvc_stream_status != 2 ||
            frame_index != videoCommitControl.bFrameIndex ||
            format_index != videoCommitControl.bFormatIndex) {
        return false;
    }

    return true;
}

int main()
{
    HAL_Init();

    #if OMV_ENABLE_BOOTLOADER
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;

    GPIO_InitStructure.Pin = OMV_BOOT_LED_PIN;
    HAL_GPIO_Init(OMV_BOOT_LED_PORT, &GPIO_InitStructure);
    HAL_GPIO_WritePin(OMV_BOOT_LED_PORT, OMV_BOOT_LED_PIN, GPIO_PIN_SET);
    #endif

    // Re-enable IRQs (disabled by bootloader)
    __enable_irq();

    #ifdef OMV_SDRAM_SIZE
    if (!sdram_init()) {
        __fatal_error("");
    }
    #endif

    fb_alloc_init0();
    framebuffer_init0();
    omv_csi_init0();

    // Initialize the csi
    if (omv_csi_init() != 0) {
        __fatal_error("");
    }

    omv_csi_reset();

    /* Init Device Library */
    USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS);

    /* Add Supported Class */
    USBD_RegisterClass(&hUsbDeviceFS, &USBD_UVC);

    /* Add Interface Class */
    USBD_UVC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS);

    USBD_Start(&hUsbDeviceFS);

    while (true) {
        while (g_uvc_stream_status == 2) {
            if (frame_index != videoCommitControl.bFrameIndex ||
                format_index != videoCommitControl.bFormatIndex) {
                switch (videoCommitControl.bFormatIndex) {
                    case VS_FMT_INDEX(YUYV):
                        omv_csi_set_pixformat(PIXFORMAT_YUV422);
                        break;
                    case VS_FMT_INDEX(GREY):
                        omv_csi_set_pixformat(PIXFORMAT_GRAYSCALE);
                        break;
                    case VS_FMT_INDEX(RGB565):
                        omv_csi_set_pixformat(PIXFORMAT_RGB565);
                        break;
                    default:
                        break;
                }

                switch (videoCommitControl.bFrameIndex) {
                    case VS_FRAME_INDEX_1:
                        omv_csi_set_framesize(OMV_CSI_FRAMESIZE_QQQVGA);
                        break;
                    case VS_FRAME_INDEX_2:
                        omv_csi_set_framesize(OMV_CSI_FRAMESIZE_QQVGA);
                        break;
                    case VS_FRAME_INDEX_3:
                        omv_csi_set_framesize(OMV_CSI_FRAMESIZE_QVGA);
                        break;
                    case VS_FRAME_INDEX_4:
                        omv_csi_set_framesize(OMV_CSI_FRAMESIZE_VGA);
                        break;
                    default:
                        break;
                }

                frame_index = videoCommitControl.bFrameIndex;
                format_index = videoCommitControl.bFormatIndex;
            }

            image_t image = {0};
            do {
                csi.snapshot(&csi, &image, 0);
            } while (process_frame(&image));
        }
    }
}
