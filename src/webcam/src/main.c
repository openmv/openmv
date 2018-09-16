#include STM32_HAL_H
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

int main()
{
    HAL_Init();

    // Re-enable IRQs (disabled by bootloader)
    __enable_irq();


    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;

    GPIO_InitStructure.Pin = OMV_BOOTLDR_LED_PIN;
    HAL_GPIO_Init(OMV_BOOTLDR_LED_PORT, &GPIO_InitStructure);
    HAL_GPIO_WritePin(OMV_BOOTLDR_LED_PORT, OMV_BOOTLDR_LED_PIN, GPIO_PIN_SET);

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

    image_t image = {0};
    uint32_t frame_size = 0;
    uint8_t  frame_index = 0;
    uint8_t  format_index = 0;
    uint32_t packet_size = VIDEO_PACKET_SIZE;
    static uint8_t uvc_header[2] = { 2, 0 };
    static uint8_t packet[VIDEO_PACKET_SIZE];

    while (1) {
        uint32_t idx = 0;
        while (g_uvc_stream_status == 2) {
            uint32_t pidx = 0;
            packet[pidx++] = uvc_header[0];
            packet[pidx++] = uvc_header[1];

            if (image.bpp == 0) {
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

                // capture new frame
                sensor.snapshot(&sensor, &image);
                frame_size =  image.w * image.h * image.bpp;
            }

            switch (videoCommitControl.bFormatIndex) {
                case VS_FMT_INDEX(YUYV): {
                    for (; pidx<packet_size && idx<frame_size; idx+=2, pidx+=2) {
                        packet[pidx+0] = image.pixels[idx+1];
                        packet[pidx+1] = image.pixels[idx+0];
                    }
                    break;
                }
                case VS_FMT_INDEX(GREY): {
                    for (; pidx<packet_size && idx<frame_size; idx++, pidx++) {
                        packet[pidx] = image.pixels[idx];
                    }
                    break;
                }
                case VS_FMT_INDEX(RGB565): {
                    for (; pidx<packet_size && idx<frame_size; idx+=2, pidx+=2) {
                        packet[pidx+0] = image.pixels[idx+1];
                        packet[pidx+1] = image.pixels[idx+0];
                    }
                    break;
                }
                default:
                    break;
            }

            if (idx == frame_size) {
                packet[1] |= 0x2;    // Flag end of frame
                uvc_header[1] ^= 1;  // Toggle bit 0 for next new frame
                idx = image.bpp = 0; // Reset image.
            }

            while (UVC_Transmit_FS(packet, pidx) != USBD_OK) {
                __WFI();
            }
        }
    }
}
