/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     arx3a0_camera_sensor_testapp.c
 * @author   Chandra Bhushan Singh
 * @email    chandrabhushan.singh@alifsemi.com
 * @version  V1.0.0
 * @date     07-Sept-2023
 * @brief    TestApp to verify ARX3A0 Camera Sensor with
 *            FREERTOS as an Operating System.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

//* System Includes */
#include <stdio.h>

/* Cpi Driver */
#include "Driver_CPI.h"
#include "RTE_Components.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */

/* PINMUX Driver */
#include "pinconf.h"

/* SE Services */
#include "se_services_port.h"

/*RTOS Includes */
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

/* Camera  Driver instance 0 */
extern ARM_DRIVER_CPI Driver_CPI;
static ARM_DRIVER_CPI *CAMERAdrv = &Driver_CPI;

/*Define for FreeRTOS*/
#define STACK_SIZE                      1024
#define TIMER_SERVICE_TASK_STACK_SIZE   configTIMER_TASK_STACK_DEPTH
#define IDLE_TASK_STACK_SIZE            configMINIMAL_STACK_SIZE

StackType_t IdleStack[2 * IDLE_TASK_STACK_SIZE];
StaticTask_t IdleTcb;
StackType_t TimerStack[2 * TIMER_SERVICE_TASK_STACK_SIZE];
StaticTask_t TimerTcb;

/* Thread id of thread */
TaskHandle_t camera_xHandle;

/****************************** FreeRTOS functions **********************/

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer = &IdleTcb;
    *ppxIdleTaskStackBuffer = IdleStack;
    *pulIdleTaskStackSize = IDLE_TASK_STACK_SIZE;
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    (void) pxTask;

    for (;;);
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
    *ppxTimerTaskTCBBuffer = &TimerTcb;
    *ppxTimerTaskStackBuffer = TimerStack;
    *pulTimerTaskStackSize = TIMER_SERVICE_TASK_STACK_SIZE;
}

void vApplicationIdleHook(void)
{
    for (;;);
}

/*****************Only for FreeRTOS use *************************/

/* @Note: ARX3A0 Camera Sensor configurations
 *        are directly borrowed from ARX3A0 Camera Sensor drivers,
 *        for detail refer ARX3A0 driver.
 *
 * Selected ARX3A0 Camera Sensor configurations:
 *   - Interface     : MIPI CSI2
 *   - Resolution    : 560X560
 *   - Output Format : RAW Bayer10
 */

/* ARX3A0 Camera Sensor Resolution. */
#define ARX3A0_CAMERA_RESOLUTION_560x560           0
#define ARX3A0_CAMERA_RESOLUTION                   ARX3A0_CAMERA_RESOLUTION_560x560

#if (ARX3A0_CAMERA_RESOLUTION == ARX3A0_CAMERA_RESOLUTION_560x560)
#define FRAME_WIDTH                               (560)
#define FRAME_HEIGHT                              (560)
#endif

/* Allocate Camera frame buffer memory using memory pool section in
 *  Linker script (sct scatter) file.
 */

/* pool size for Camera frame buffer:
 *  which will be frame width x frame height
 */
#define FRAMEBUFFER_POOL_SIZE                     ((FRAME_WIDTH) * (FRAME_HEIGHT))

/* pool area for Camera frame buffer.
 *  Allocated in the "camera_frame_buf" section.
 */
uint8_t framebuffer_pool[FRAMEBUFFER_POOL_SIZE] \
        __attribute__((section(".bss.camera_frame_buf")));

/* (optional)
 * if required convert captured image data format to any other image format.
 *
 *  - for ARX3A0 Camera sensor,
 *    selected Bayer output format:
 *    in-order to get the color image,
 *    Bayer format must be converted in to RGB format.
 *    User can use below provided
 *    "Open-Source" code for Bayer to RGB Conversion
 *    which uses DC1394 library.
 */
/* Enable image conversion Bayer to RGB. */
#define IMAGE_CONVERSION_BAYER_TO_RGB_EN         0

/* Check if image conversion Bayer to RGB is Enabled? */
#if IMAGE_CONVERSION_BAYER_TO_RGB_EN

/* @Note: Bayer to RGB configurations
 *        are directly borrowed from "Open-Source" code for
 *        Bayer to RGB Conversion, for detail refer bayer2rgb.c.
 *
 * Selected Bayer to RGB configurations:
 *   - converted image format : tiff
 *   - bpp bit per pixel      : 8-bit
 */
#define TIFF_HDR_NUM_ENTRY                       8
#define TIFF_HDR_SIZE                            10 + TIFF_HDR_NUM_ENTRY * 12

/* bpp bit per pixel
 *  Valid parameters are:
 *   -  8-bit
 *   - 16-bit
 */
#define BITS_PER_PIXEL_8_BIT                     8
#define BITS_PER_PIXEL                           BITS_PER_PIXEL_8_BIT

/* pool size for Camera frame buffer for Bayer to RGB conversion:
 *   which will be frame width x frame height x (bpp / 8) * 3 + tiff header(106 Bytes).
 */
#define BAYER_TO_RGB_BUFFER_POOL_SIZE   \
    ( (FRAME_WIDTH) * (FRAME_HEIGHT) * (BITS_PER_PIXEL / 8) * 3 + TIFF_HDR_SIZE )

/* pool area for Camera frame buffer for Bayer to RGB conversion.
 *  Allocated in the "camera_frame_bayer_to_rgb_buf" section.
 */
uint8_t bayer_to_rgb_buffer_pool[BAYER_TO_RGB_BUFFER_POOL_SIZE] \
        __attribute__((section(".bss.camera_frame_bayer_to_rgb_buf")));

/* Optional:
 *  Camera Image Conversions
 */
typedef enum {
    BAYER_TO_RGB_CONVERSION   = (1 << 0),
}IMAGE_CONVERSION;

#endif /* end of IMAGE_CONVERSION_BAYER_TO_RGB_EN */

/* Camera callback events */
typedef enum {
    CAM_CB_EVENT_CAPTURE_STOPPED      = (1 << 0),
    CAM_CB_EVENT_ERROR                = (1 << 1)
}CAMERA_CB_EVENTS;

/**
  \fn          void camera_callback(uint32_t event)
  \brief       Camera isr callback
  \param[in]   event: Camera Event
  \return      none
  */
void camera_callback(uint32_t event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult = pdFALSE;

    if(event & ARM_CPI_EVENT_CAMERA_CAPTURE_STOPPED)
    {
        /* Transfer Success: Capture Stop detected, Wake-up Thread. */
        xResult = xTaskNotifyFromISR(camera_xHandle,CAM_CB_EVENT_CAPTURE_STOPPED,eSetBits, &xHigherPriorityTaskWoken);
        if (xResult == pdTRUE)
        {
            portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
        }
    }

    if(event & ARM_CPI_EVENT_ERR_CAMERA_INPUT_FIFO_OVERRUN)
    {
        /* Transfer Error: Received FIFO over-run, Wake-up Thread. */
        xResult = xTaskNotifyFromISR(camera_xHandle,CAM_CB_EVENT_ERROR,eSetBits, &xHigherPriorityTaskWoken);
        if (xResult == pdTRUE)
        {
            portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
        }
    }

    if(event & ARM_CPI_EVENT_ERR_CAMERA_OUTPUT_FIFO_OVERRUN)
    {
        /* Transfer Error: Received FIFO over-run, Wake-up Thread. */
        xResult = xTaskNotifyFromISR(camera_xHandle,CAM_CB_EVENT_ERROR,eSetBits, &xHigherPriorityTaskWoken);
        if (xResult == pdTRUE)
        {
            portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
        }
    }

    if(event & ARM_CPI_EVENT_ERR_HARDWARE)
    {
        /* Transfer Error: Received Hardware error, Wake-up Thread. */
        xResult = xTaskNotifyFromISR(camera_xHandle,CAM_CB_EVENT_ERROR,eSetBits, &xHigherPriorityTaskWoken);
        if (xResult == pdTRUE)
        {
            portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
        }
    }
}

/**
  \fn          int32_t i3c_pinmux(void)
  \brief       i3c hardware pin initialization:
                  - PIN-MUX configuration
                  - PIN-PAD configuration
  \param[in]   none
  \return      0:success; -1:failure
  */
int32_t i3c_pinmux(void)
{
    int32_t ret;

    /* Configure GPIO Pin : P7_6 as i3c_sda_d
     * Pad function: PADCTRL_READ_ENABLE |
     *               PADCTRL_DRIVER_DISABLED_PULL_UP
     */
    ret = pinconf_set(PORT_7, PIN_2, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE |
            PADCTRL_DRIVER_DISABLED_PULL_UP);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: i3c PINMUX and PINPAD failed.\r\n");
        return -1;
    }

    /* Configure GPIO Pin : P7_7 as i3c_scl_d
     * Pad function: PADCTRL_READ_ENABLE
     *               PADCTRL_DRIVER_DISABLED_PULL_UP
     */
    ret = pinconf_set(PORT_7, PIN_3, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE |
            PADCTRL_DRIVER_DISABLED_PULL_UP);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: i3c PINMUX and PINPAD failed.\r\n");
        return -1;
    }

    return 0;
}

/**
  \fn          int32_t camera_pinmux(void)
  \brief       Camera hardware pin initialization:
                   - PIN-MUX configuration
  \param[in]   none
  \return      0:success; -1:failure
  */
int32_t camera_pinmux(void)
{
    int32_t ret;

    ret = pinconf_set(PORT_0, PIN_3, PINMUX_ALTERNATE_FUNCTION_6, 0);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: Camera Pin-Mux failed.\r\n");
        return -1;
    }

    return 0;
}

/**
  \fn          int32_t hardware_init(void)
  \brief       - i3c hardware pin initialization:
                   - PIN-MUX configuration
                   - PIN-PAD configuration
               - Camera hardware pin initialization:
                   - PIN-MUX configuration
  \param[in]   none
  \return      0:success; -1:failure
  */
int32_t hardware_init(void)
{
    int32_t ret;

    /* i3c pinmux. */
    ret = i3c_pinmux();
    if(ret != 0)
    {
        printf("\r\n Error in i3c pinmux.\r\n");
        return -1;
    }

    /* Camera pinmux. */
    ret = camera_pinmux();
    if(ret != 0)
    {
        printf("\r\n Error in Camera pinmux.\r\n");
        return -1;
    }

    return 0;
}

/* Check if image conversion Bayer to RGB is Enabled? */
#if IMAGE_CONVERSION_BAYER_TO_RGB_EN
/**
  \fn          int32_t camera_image_conversion(IMAGE_CONVERSION image_conversion,
                                               uint8_t  *src,
                                               uint8_t  *dest,
                                               uint32_t  frame_width,
                                               uint32_t  frame_height)
  \brief       Convert image data from one format to any other image format.
                - Supported conversions
                - Bayer(RAW) to RGB Conversion
                - User can use below provided
                  "Open-Source" Bayer to RGB Conversion code
                  which uses DC1394 library.
                - This code will,
                - Add header for tiff image format
                - Convert RAW Bayer to RGB depending on
                - bpp bit per pixel 8/16 bit
                - DC1394 Color Filter
                - DC1394 Bayer interpolation methods
                - Output image size will be
                - width x height x (bpp / 8) x 3 + tiff header(106 Bytes)
  \param[in]   image_conversion : image conversion methods \ref IMAGE_CONVERSION
  \param[in]   src              : Source address, Pointer to already available
                                  image data Address
  \param[in]   dest             : Destination address,Pointer to Address,
                                  where converted image data will be stored.
  \param[in]   frame_width      : image frame width
  \param[in]   frame_height     : image frame height
  \return      success          :  0
  \return      failure          : -1
  */
int32_t camera_image_conversion(IMAGE_CONVERSION  image_conversion,
                                uint8_t *src, uint8_t *dest, uint32_t frame_width,
                                uint32_t frame_height)
{
    /* Bayer to RGB Conversion. */
    extern int32_t bayer_to_RGB(uint8_t  *src,   uint8_t  *dest,   \
                                uint32_t  width, uint32_t  height);

    int32_t ret = 0;

    switch(image_conversion)
    {
        case BAYER_TO_RGB_CONVERSION:
        {
            printf("\r\n Start Bayer to RGB Conversion: \r\n");
            printf("\t Frame Buffer Addr: 0x%X \r\n \t Bayer_to_RGB Addr: 0x%X\n", \
                   (uint32_t) src, (uint32_t) dest);
            ret = bayer_to_RGB(src, dest, frame_width, frame_height);
            if(ret != 0)
            {
                printf("\r\n Error: CAMERA image conversion: Bayer to RGB failed.\r\n");
                return -1;
            }
            break;
        }

        default:
        {
            return -1;
        }
    }

    return 0;
}
#endif /* end of IMAGE_CONVERSION_BAYER_TO_RGB_EN */

/**
  \fn          void camera_demo_thread_entry(void *pvParameters)
  \brief       TestApp to verify ARX3A0 Camera Sensor with
               FREERTOS as an Operating System.
               This demo thread does:
                    - initialize i3c and Camera hardware pins
                      using PinMux Driver;
                    - initialize DPHY Tx.
                    - initialize Camera driver
                    - capture one frame
                    - captured data will be stored in to allocated
                      frame buffer address
                    - stop Camera capture
                    - (optional)
                    -if required convert captured image format
                     in to any other image format
                    - for ARX3A0 Camera sensor,
                         - selected Bayer output format;
                         - in-order to get the color image,
                           Bayer format must be converted in to RGB format.
                         - User can use below provided
                           "Open-Source" code for Bayer to RGB Conversion
                           which uses DC1394 library.
                    - dump captured/converted image data from memory address
                      using any debugger
                    - display image
  @param       pvParameters.
  \return      none
  */
void camera_demo_thread_entry(void *pvParameters)
{
    int32_t ret            = 0;
    uint32_t actual_events = 0;
    uint32_t service_error_code;
    uint32_t error_code;
    run_profile_t runp = {0};
    ARM_DRIVER_VERSION version;

    printf("\r\n \t\t >>> ARX3A0 Camera Sensor demo with FreeRTOS is starting up!!! <<< \r\n");


    /* Allocated memory address for
     *   - Camera frame buffer and
     *   - (Optional) Camera frame buffer for Bayer to RGB Conversion
     */
    printf("\n \t frame buffer        pool size: 0x%0X  pool addr: 0x%0X \r\n ", \
            FRAMEBUFFER_POOL_SIZE, (uint32_t) framebuffer_pool);

#if IMAGE_CONVERSION_BAYER_TO_RGB_EN
    printf("\n \t bayer_to_rgb buffer pool size: 0x%0X  pool addr: 0x%0X \r\n ", \
            BAYER_TO_RGB_BUFFER_POOL_SIZE, (uint32_t) bayer_to_rgb_buffer_pool);
#endif

    /* Initialize i3c and Camera hardware pins using PinMux Driver. */
    ret = hardware_init();
    if(ret != 0)
    {
        printf("\r\n Error: CAMERA Hardware Initialize failed.\r\n");
        return;
    }

    /* Initialize the SE services */
    se_services_port_init();

    /* Enable MIPI Clocks */
    error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_CLK_100M, true, &service_error_code);
    if(error_code != SERVICES_REQ_SUCCESS)
    {
        printf("SE: MIPI 100MHz clock enable = %d\n", error_code);
        return;
    }

    error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_HFOSC, true, &service_error_code);
    if(error_code != SERVICES_REQ_SUCCESS)
    {
        printf("SE: MIPI 38.4Mhz(HFOSC) clock enable = %d\n", error_code);
        goto error_disable_100mhz_clk;
    }

    /* Get the current run configuration from SE */
    error_code = SERVICES_get_run_cfg(se_services_s_handle,
                                      &runp,
                                      &service_error_code);
    if(error_code)
    {
        printf("\r\nSE: get_run_cfg error = %d\n", error_code);
        goto error_disable_hfosc_clk;
    }

    /*
     * Note:
     * This demo uses a specific profile setting that only enables the
     * items it needs. For example, it only requests the RAM regions and
     * peripheral power that are relevant for this demo. If you want to adapt
     * this example for your own use case, you should adjust the profile setting
     * accordingly. You can either add any additional items that you need, or
     * remove the request altogether to use the default setting that turns on
     * almost everything.
     */

    runp.memory_blocks = MRAM_MASK | SRAM0_MASK;

    runp.phy_pwr_gating = MIPI_PLL_DPHY_MASK | MIPI_TX_DPHY_MASK | MIPI_RX_DPHY_MASK | LDO_PHY_MASK;

    /* Set the new run configuration */
    error_code = SERVICES_set_run_cfg(se_services_s_handle,
                                      &runp,
                                      &service_error_code);
    if(error_code)
    {
        printf("\r\nSE: set_run_cfg error = %d\n", error_code);
        goto error_disable_hfosc_clk;
    }

    version = CAMERAdrv->GetVersion();
    printf("\r\n Camera driver version api:0x%X driver:0x%X \r\n",version.api, version.drv);

    ret = CAMERAdrv->Initialize(camera_callback);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: CAMERA Initialize failed.\r\n");
        goto error_disable_hfosc_clk;
    }

    /* Power up Camera peripheral */
    ret = CAMERAdrv->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: CAMERA Power Up failed.\r\n");
        goto error_uninitialize_camera;
    }

    /* Control configuration for camera controller */
    ret = CAMERAdrv->Control(CPI_CONFIGURE, 0);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: CPI Configuration failed.\r\n");
        goto error_uninitialize_camera;
    }

    /* Control configuration for camera sensor */
    ret = CAMERAdrv->Control(CPI_CAMERA_SENSOR_CONFIGURE, 0);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: CAMERA SENSOR Configuration failed.\r\n");
        goto error_poweroff_camera;
    }

    /*Control configuration for camera events */
    ret = CAMERAdrv->Control(CPI_EVENTS_CONFIGURE, \
                             ARM_CPI_EVENT_CAMERA_CAPTURE_STOPPED | \
                             ARM_CPI_EVENT_ERR_CAMERA_INPUT_FIFO_OVERRUN | \
                             ARM_CPI_EVENT_ERR_CAMERA_OUTPUT_FIFO_OVERRUN | \
                             ARM_CPI_EVENT_ERR_HARDWARE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: CAMERA SENSOR Event Configuration failed.\r\n");
        goto error_poweroff_camera;
    }

    printf("\r\n Let's Start Capturing Camera Frame...\r\n");
    ret = CAMERAdrv->CaptureFrame(framebuffer_pool);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: CAMERA Capture Frame failed.\r\n");
        goto error_poweroff_camera;
    }

    /* wait till any event to comes in isr callback */
    xTaskNotifyWait(NULL, CAM_CB_EVENT_CAPTURE_STOPPED | CAM_CB_EVENT_ERROR, &actual_events, portMAX_DELAY);

    if(!(actual_events & CAM_CB_EVENT_CAPTURE_STOPPED) && (actual_events & CAM_CB_EVENT_ERROR))
    {
        /* Error: Camera Capture Frame failed. */
        printf("\r\n \t\t >> Error: CAMERA Capture Frame failed. \r\n");
        goto error_poweroff_camera;
    }

    /* Okay, we have received Success: Camera Capture Frame stop detected.
     * now stop Camera Capture.
     */
    ret = CAMERAdrv->Stop();
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: CAMERA stop Capture failed.\r\n");
        goto error_poweroff_camera;
    }

    /* (optional)
     * if required convert captured image data format to any other image format.
     *  - for ARX3A0 Camera sensor,
     *     selected Bayer output format:
     *      in-order to get the color image,
     *       Bayer format must be converted in to RGB format.
     *       User can use below provided
     *        "Open-Source" code for Bayer to RGB Conversion
     *        which uses DC1394 library.
     */
    /* Check if image conversion Bayer to RGB is Enabled? */
#if IMAGE_CONVERSION_BAYER_TO_RGB_EN
    ret = camera_image_conversion(BAYER_TO_RGB_CONVERSION,
            framebuffer_pool,
            bayer_to_rgb_buffer_pool,
            FRAME_WIDTH,
            FRAME_HEIGHT);
    if(ret != 0)
    {
        printf("\r\n Error: CAMERA image conversion failed.\r\n");
        return;
    }
#endif /* end of IMAGE_CONVERSION_BAYER_TO_RGB_EN */

    /* How to dump captured/converted image data from memory address?
     *  1)To dump memory using ARM DS(Development Studio) and Ulink Pro Debugger
     *
     *  Use below command in "Commands" tab:
     *   dump binary memory path_with_filename.fileformat starting_address ending_address
     *
     *   example:(update user directory name)
     *    dump binary memory /home/user/camera_dump/cam_image0_560p.bin 0x8000000 0x804C8FF
     *
     *   Bayer to RGB:
     *    dump binary memory /home/user/camera_dump/cam_image0_Bayer_to_RGB_560p.tif 0x8000000 0x80E5B69
     *
     *   2)To dump memory using Trace32
     *  Use below command in "Commands" tab:
     *   data.save.binary path_with_filename.fileformat starting_address--ending_address
     *
     *   example:(update user directory name)
     *    data.save.binary /home/user/camera_dump/cam_image0_560p.bin 0x8000000--0x804C8FF
     *
     *   Bayer to RGB:
     *    data.save.binary /home/user/camera_dump/cam_image0_Bayer_to_RGB_560p.tif 0x8000000--0x80E5B69
     *
     *   This commands will dump memory from staring address to ending address
     *   and store it in to given path with filename.
     *
     *
     */
    printf("\n To dump memory using ARM DS with Ulink Pro Debugger or Trace32 :");
    printf("\n  Use below commands in Commands tab: update user directory name \r\n");

#if IMAGE_CONVERSION_BAYER_TO_RGB_EN
    printf("Ulink:\n   dump binary memory /home/user/camera_dump/cam_image0_Bayer_to_RGB_560p.tif 0x%X 0x%X \r\n", \
            (uint32_t) bayer_to_rgb_buffer_pool, (uint32_t) (bayer_to_rgb_buffer_pool + BAYER_TO_RGB_BUFFER_POOL_SIZE - 1));
    printf("T32:\n   data.save.binary /home/user/camera_dump/cam_image0_Bayer_to_RGB_560p.tif 0x%X--0x%X \r\n", \
            (uint32_t) bayer_to_rgb_buffer_pool, (uint32_t) (bayer_to_rgb_buffer_pool + BAYER_TO_RGB_BUFFER_POOL_SIZE - 1));

#else
    printf("Ulink:\n   dump binary memory /home/user/camera_dump/cam_image0_560p.bin 0x%X 0x%X \r\n", \
            (uint32_t) framebuffer_pool, (uint32_t) (framebuffer_pool + FRAMEBUFFER_POOL_SIZE - 1));
    printf("T32:\n   data.save.binary /home/user/camera_dump/cam_image0_560p.bin 0x%X--0x%X \r\n", \
            (uint32_t) framebuffer_pool, (uint32_t) (framebuffer_pool + FRAMEBUFFER_POOL_SIZE - 1));
#endif

    printf("\n  This command will dump memory from staring address to ending address \r");
    printf("\n  and store it in to given path with filename.\r\n\r\n");

    printf("\r\n\r\n XXX Camera demo thread is halting here! XXX...\r\n");
    printf("\r\n Now User can dump captured/converted image data from memory address using any debugger!!!\r\n");

error_poweroff_camera:
    /* Power off CAMERA peripheral */
    ret = CAMERAdrv->PowerControl(ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
        printf("\r\n Error: CAMERA Power OFF failed.\r\n");

error_uninitialize_camera:
    /* Un-initialize CAMERA driver */
    ret = CAMERAdrv->Uninitialize();
    if(ret != ARM_DRIVER_OK)
        printf("\r\n Error: CAMERA Uninitialize failed.\r\n");

error_disable_hfosc_clk:
    error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_HFOSC, false, &service_error_code);
    if(error_code != SERVICES_REQ_SUCCESS)
        printf("SE: MIPI 38.4Mhz(HFOSC)  clock disable = %d\n", error_code);

error_disable_100mhz_clk:
    error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_CLK_100M, false, &service_error_code);
    if(error_code != SERVICES_REQ_SUCCESS)
        printf("SE: MIPI 100MHz clock disable = %d\n", error_code);

    printf("\r\n XXX Camera demo thread is exiting XXX...\r\n");

    /* wait forever */
    while(1);
}

/*----------------------------------------------------------------------------
 *      Main: Initialize and start the FreeRTOS Kernel
 *---------------------------------------------------------------------------*/
int main( void )
{
    #if defined(RTE_Compiler_IO_STDOUT_User)
    int32_t ret;
    ret = stdout_init();
    if(ret != ARM_DRIVER_OK)
    {
        while(1)
        {
        }
    }
    #endif

    /* System Initialization */
    SystemCoreClockUpdate();

    /* Create application main thread */
    BaseType_t xReturned = xTaskCreate(camera_demo_thread_entry, "camera_demo_thread_entry",
                                       216, NULL,configMAX_PRIORITIES-1, &camera_xHandle);
    if(xReturned != pdPASS)
    {

        vTaskDelete(camera_xHandle);
        return -1;
     }

    /* Start thread execution */
    vTaskStartScheduler();
}

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
