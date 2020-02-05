/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
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
#include "sensor.h"
#include "framebuffer.h"
#include "omv_boardconfig.h"

extern sensor_t sensor;
USBD_HandleTypeDef hUsbDeviceFS;
extern volatile uint8_t g_uvc_stream_status;
extern struct uvc_streaming_control videoCommitControl;

void __attribute__((noreturn)) __fatal_error()
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
    __fatal_error();
    return NULL;
}

NORETURN void fb_alloc_fail()
{
    __fatal_error();
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
    __fatal_error();
}

void *mp_obj_new_exception_msg(const void *exc_type, const char *msg)
{
    return NULL;
}

const void *mp_type_MemoryError = NULL;

const void *mp_sys_stdout_print = NULL;

static uint8_t frame_index = 0;
static uint8_t format_index = 0;

static uint8_t uvc_header[2] = { 2, 0 };
static uint8_t packet[VIDEO_PACKET_SIZE];
uint32_t packet_size = VIDEO_PACKET_SIZE-2;

bool streaming_cb(image_t *image)
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

    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;

    GPIO_InitStructure.Pin = OMV_BOOTLDR_LED_PIN;
    HAL_GPIO_Init(OMV_BOOTLDR_LED_PORT, &GPIO_InitStructure);
    HAL_GPIO_WritePin(OMV_BOOTLDR_LED_PORT, OMV_BOOTLDR_LED_PIN, GPIO_PIN_SET);

    // Re-enable IRQs (disabled by bootloader)
    __enable_irq();

    #ifdef OMV_SDRAM_SIZE
    if (!sdram_init()) {
        __fatal_error();
    }
    #if (OMV_SDRAM_TEST == 1)
    if (!sdram_test(false)) {
        __fatal_error();
    }
    #endif
    #endif

    sensor_init0();
    fb_alloc_init0();

    // Initialize the sensor
    if (sensor_init() != 0) {
        __fatal_error();
    }
    
    sensor_reset();

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
                        sensor_set_pixformat(PIXFORMAT_YUV422);
                        break;
                    case VS_FMT_INDEX(GREY):
                        sensor_set_pixformat(PIXFORMAT_GRAYSCALE);
                        break;
                    case VS_FMT_INDEX(RGB565):
                        sensor_set_pixformat(PIXFORMAT_RGB565);
                        break;
                    default:
                        break;
                }

                switch (videoCommitControl.bFrameIndex) {
                    case VS_FRAME_INDEX_1:
                        sensor_set_framesize(FRAMESIZE_QQQVGA);
                        break;
                    case VS_FRAME_INDEX_2:
                        sensor_set_framesize(FRAMESIZE_QQVGA);
                        break;
                    case VS_FRAME_INDEX_3:
                        sensor_set_framesize(FRAMESIZE_QVGA);
                        break;
                    case VS_FRAME_INDEX_4:
                        sensor_set_framesize(FRAMESIZE_VGA);
                        break;
                    default:
                        break;
                }

                frame_index = videoCommitControl.bFrameIndex;
                format_index = videoCommitControl.bFormatIndex;
            }

            image_t image;
            image.pixels = NULL;
            sensor.snapshot(&sensor, &image, streaming_cb);
        }
    }
}
