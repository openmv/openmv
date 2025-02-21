/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

#ifndef __RTE_DEVICE_H
#define __RTE_DEVICE_H

// <e> MRAM (NVM (Non-Volatile Memory)) [Driver_MRAM]
// <i> Configuration settings for Driver_MRAM in component ::Drivers:MRAM
#define RTE_MRAM          1
#if RTE_MRAM
#define RTE_MRAM_SIZE     0x00580000
#endif
// </e> MRAM (NVM (Non-Volatile Memory)) [Driver_MRAM]

// <e> CPI (Camera) [Driver_CPI]
// <i> Configuration settings for Driver_CPI in component ::Drivers:CPI
#define RTE_CPI                                               1
#if RTE_CPI

// <o> CPI interrupt priority <0-255>
// <i> Defines CPI interrupt priority.
// <i> Default: 0
#define RTE_CPI_IRQ_PRI                                       0

// <o> CPI CSI halt enable/disable
//     <0=> disable
//     <1=> enable
// <i> Defines CPI CSI halt enable/disable.
// <i> Default: 0
#define RTE_CPI_CSI_HALT                                      0

// <o> CPI Row roundup
//     <0=> disable
//     <1=> enable
// <i> Defines CPI row roundup to 64 bit.
// <i> Default: 0
#define RTE_CPI_ROW_ROUNDUP                                   0

// <o> CPI FIFO read watermark
// <i> Defines FIFO read watermark.
// <i> Default: 0x8
#define RTE_CPI_FIFO_READ_WATERMARK                           0x8

// <o> CPI FIFO write watermark
// <i> Defines CPI FIFO write watermark.
// <i> Default: 0x18
#define RTE_CPI_FIFO_WRITE_WATERMARK                          0x18

// <e> MT9M114 [Driver_MT9M114]
// <o> Enable/Disable MT9M114 camera sensor
//     <0=> disable
//     <1=> enable
// <i> define if to enable or disable MT9M114 camera sensor
// <i> default: disable
#define RTE_MT9M114_CAMERA_SENSOR_CPI_ENABLE                  0

#if (RTE_MT9M114_CAMERA_SENSOR_CPI_ENABLE)

// <o> Select camera MT9M114 pixel clock polarity
//     <0=> not invert camera pixclk
//     <1=> invert camera pixclk
// <i> Defines camera MT9M114 pixel clock polarity
// <i> Default: not invert camera pixclk
#define RTE_MT9M114_CAMERA_SENSOR_CPI_PIXEL_CLK_POL           0

// <o> Select camera MT9M114 HSYNC polarity
//     <0=>  not invert HSYNC input
//     <1=>  invert HSYNC input
// <i> Defines camera MT9M114 HSYNC polarity
// <i> Default:  not invert HSYNC input
#define RTE_MT9M114_CAMERA_SENSOR_CPI_HSYNC_POL               0

// <o> Select camera MT9M114 VSYNC polarity
//     <0=> not invert VSYNC input
//     <1=> invert VSYNC input
// <i> Defines camera MT9M114 VSYNC polarity
// <i> Default:  not invert VSYNC input
#define RTE_MT9M114_CAMERA_SENSOR_CPI_VSYNC_POL               0

// <o> Select camera MT9M114 VSYNC wait
//     <0=> vsync wait disable
//     <1=> vsync wait enable
// <i> Defines camera MT9M114 VSYNC wait
// <i> Default: vsync wait disable
#define RTE_MT9M114_CAMERA_SENSOR_CPI_VSYNC_WAIT              0

// <o> Select camera MT9M114 VSYNC mode
//     <0=> sync enable
//     <1=> data enable
// <i> Defines camera MT9M114 VSYNC mode
// <i> Default: sync enable
#define RTE_MT9M114_CAMERA_SENSOR_CPI_VSYNC_MODE              0

// <o> Select video data mode
//     <0=> 1 bit
//     <1=> 2 bit
//     <2=> 4 bit
//     <3=> 8 bit
//     <4=> 16 bit
// <i> Defines video data mode
// <i> Default: 8 bit
#define RTE_MT9M114_CAMERA_SENSOR_CPI_DATA_MODE               3

// <o> Select Data Endianness
//     <0=> LSB First
//     <1=> MSB First
// <i> Select MSB/LSB
// <i> Default: LSB
#define RTE_MT9M114_CAMERA_SENSOR_CPI_DATA_ENDIANNESS         0

// <o> Select CODE10ON8
//     <0=> Disable
//     <1=> Enable
// <i> Defines transfer 10-bit coding over 8-bit data bus.
// <i> Default: 8 bit
#define RTE_MT9M114_CAMERA_SENSOR_CPI_CODE10ON8               0

// <o> Select camera MT9M114 data mask
//     <0=> 16 bit
//     <1=> 10 bit
//     <2=> 12 bit
//     <3=> 14 bit
// <i> Defines camera MT9M114 data mask
// <i> Default: 10 bit
#define RTE_MT9M114_CAMERA_SENSOR_CPI_DATA_MASK               1

// <o> select MT9M114 frame height
// <i> defines select MT9M114 frame height.
// <i> default: 480
#define RTE_MT9M114_CAMERA_SENSOR_FRAME_HEIGHT                480

// <o> select MT9M114 frame width
// <i> defines select MT9M114 frame width.
// <i> default: 640
#define RTE_MT9M114_CAMERA_SENSOR_FRAME_WIDTH                 640

// <o RTE_MT9M114_CAMERA_SENSOR_I2C_INSTANCE> Select camera sensor MT9M114 i2c instance
// <i> Defines camera sensor MT9M114 i2c instance
//     <0=>   I2C0
//     <1=>   I2C1
//     <2=>   I2C2
//     <3=>   I2C3
//     <I3C=> I2C OVER I3C
// <i> Default: 1
#define RTE_MT9M114_CAMERA_SENSOR_I2C_INSTANCE                1

#endif
// </e> MT9M114 [Driver_MT9M114]

#endif
// </e> CPI (Camera) [Driver_CPI]

// <e> LPCPI (Camera) [Driver_LPCPI]
// <i> Configuration settings for Driver_LPCPI in component ::Drivers:LPCPI
#define RTE_LPCPI                                             0
#if RTE_LPCPI

// <o> LPCPI interrupt priority <0-255>
// <i> Defines LPCPI interrupt priority.
// <i> Default: 0
#define RTE_LPCPI_IRQ_PRI                                     0

// <o> LPCPI Row roundup
//     <0=> disable
//     <1=> enable
// <i> Defines LPCPI row roundup to 64 bit.
// <i> Default: 0
#define RTE_LPCPI_ROW_ROUNDUP                                 0

// <o> LPCPI FIFO read watermark
// <i> Defines LPCPI FIFO read watermark.
// <i> Default: 0x8
#define RTE_LPCPI_FIFO_READ_WATERMARK                         0x8

// <o> LPCPI FIFO write watermark
// <i> Defines LPCPI FIFO write watermark.
// <i> Default: 0x18
#define RTE_LPCPI_FIFO_WRITE_WATERMARK                        0x18

// <e> MT9M114 [Driver_MT9M114]
// <o> Enable/Disable MT9M114 camera sensor
//     <0=> disable
//     <1=> enable
// <i> define if to enable or disable MT9M114 camera sensor
// <i> default: disable
#define RTE_MT9M114_CAMERA_SENSOR_LPCPI_ENABLE                1

#if (RTE_MT9M114_CAMERA_SENSOR_LPCPI_ENABLE)

// <o> Select camera MT9M114 pixel clock polarity
//     <0=> not invert camera pixclk
//     <1=> invert camera pixclk
// <i> Defines camera MT9M114 pixel clock polarity
// <i> Default: not invert camera pixclk
#define RTE_MT9M114_CAMERA_SENSOR_LPCPI_PIXEL_CLK_POL         0

// <o> Select camera MT9M114 HSYNC polarity
//     <0=>  not invert HSYNC input
//     <1=>  invert HSYNC input
// <i> Defines camera MT9M114 HSYNC polarity
// <i> Default:  not invert HSYNC input
#define RTE_MT9M114_CAMERA_SENSOR_LPCPI_HSYNC_POL             0

// <o> Select camera MT9M114 VSYNC polarity
//     <0=> not invert VSYNC input
//     <1=> invert VSYNC input
// <i> Defines camera MT9M114 VSYNC polarity
// <i> Default:  not invert VSYNC input
#define RTE_MT9M114_CAMERA_SENSOR_LPCPI_VSYNC_POL             0

// <o> Select camera MT9M114 VSYNC wait
//     <0=> vsync wait disable
//     <1=> vsync wait enable
// <i> Defines camera MT9M114 VSYNC wait
// <i> Default: vsync wait disable
#define RTE_MT9M114_CAMERA_SENSOR_LPCPI_VSYNC_WAIT            0

// <o> Select camera MT9M114 VSYNC mode
//     <0=> sync enable
//     <1=> data enable
// <i> Defines camera MT9M114 VSYNC mode
// <i> Default: sync enable
#define RTE_MT9M114_CAMERA_SENSOR_LPCPI_VSYNC_MODE            0

// <o> Select video data mode
//     <0=> 1 bit
//     <1=> 2 bit
//     <2=> 4 bit
//     <3=> 8 bit
// <i> Defines video data mode
// <i> Default: 8 bit
#define RTE_MT9M114_CAMERA_SENSOR_LPCPI_DATA_MODE             3

// <o> Select Data Endianness
//     <0=> LSB First
//     <1=> MSB First
// <i> Select MSB/LSB
// <i> Default: LSB
#define RTE_MT9M114_CAMERA_SENSOR_LPCPI_DATA_ENDIANNESS       0

// <o> Select CODE10ON8
//     <0=> Disable
//     <1=> Enable
// <i> Defines transfer 10-bit coding over 8-bit data bus.
// <i> Default: 8 bit
#define RTE_MT9M114_CAMERA_SENSOR_LPCPI_CODE10ON8             0

// <o> select MT9M114 frame height
// <i> defines select MT9M114 frame height.
// <i> default: 480
#define RTE_MT9M114_CAMERA_SENSOR_FRAME_HEIGHT                480

// <o> select MT9M114 frame width
// <i> defines select MT9M114 frame width.
// <i> default: 640
#define RTE_MT9M114_CAMERA_SENSOR_FRAME_WIDTH                 640

// <o RTE_MT9M114_CAMERA_SENSOR_I2C_INSTANCE> Select camera sensor MT9M114 i2c instance
// <i> Defines camera sensor MT9M114 i2c instance
//     <0=>   I2C0
//     <1=>   I2C1
//     <2=>   I2C2
//     <3=>   I2C3
//     <I3C=> I2C OVER I3C
// <i> Default: 1
#define RTE_MT9M114_CAMERA_SENSOR_I2C_INSTANCE                1

#endif
// </e> MT9M114 [Driver_MT9M114]

#endif
// </e> LPCPI (Camera) [Driver_LPCPI]

// <e> MIPI_CSI2 (mipi csi2) [Driver_MIPI_CSI2]
// <i> Configuration settings for Driver_MIPI_CSI2 in component ::Drivers:MIPI_CSI2
#define RTE_MIPI_CSI2                                           1
#if RTE_MIPI_CSI2

// <o> CSI pixel clock select
//     <0=>  400 MHz clock source (PLL_CLK1/2)
//     <1=>  480 MHz clock source (PLL_CLK3)
// <i> Defines CSI pixel clock select
// <i> Default: 400 MHz clock source (PLL_CLK1/2)
#define RTE_CSI2_PIX_CLK_SEL                                 0

// <o> select IPI mode
//     <0=> camera mode
//     <1=> controller mode
// <i> defines select IPI mode
// <i> default: camera mode
#define RTE_MIPI_CSI2_IPI_MODE                                  0

// <o> select memory flush
//     <0=> manual
//     <1=> auto
// <i> defines select memory flush
// <i> default: auto
#define RTE_MIPI_CSI2_MEMFLUSH                                  1

// <o> select sync event mode
//     <0=> not trigger by frame start
//     <1=> trigger by frame start
// <i> defines select sync event mode
// <i> default : not trigger by frame start
#define RTE_MIPI_CSI2_SYNC_ET_MODE                              0

// <o> select sync event select
//     <0=> auto
//     <1=> programmed
// <i> defines select sync event select
// <i> default : programmed
#define RTE_MIPI_CSI2_SYNC_ET_SEL                               1

// <o> embedded packets for IPI synchronization events
//     <0=> disable
//     <1=> enable
// <i> define if want to use embedded packets for IPI synchronization events
// <i> default: disable
#define RTE_MIPI_CSI2_EN_EMBEDDED                               0

// <o> blanking packets for IPI synchronization events
//     <0=> disable
//     <1=> enable
// <i> define if want to use blanking packets for IPI synchronization events
// <i> default: disable
#define RTE_MIPI_CSI2_EN_BLANKING                               0

// <o> null packets for IPI synchronization events
//     <0=> disable
//     <1=> enable
// <i> define if want to use null packets for IPI synchronization events
// <i> default: disable
#define RTE_MIPI_CSI2_EN_NULL                                   0

// <o> line start packets for IPI synchronization events
//     <0=> disable
//     <1=> enable
// <i> define if want to use line start packets for IPI synchronization events
// <i> default: disable
#define RTE_MIPI_CSI2_EN_LINE_START                             0

// <o> video packets for IPI synchronization events
//     <0=> disable
//     <1=> enable
// <i> define if want to use video packets for IPI synchronization events
// <i> default: enable
#define RTE_MIPI_CSI2_EN_VIDEO                                  1

// <o> datatype to overwrite
// <o> select CSI2 Data type to overwrite
//     <24=> YUV420_8
//     <25=> YUV420_10
//     <26=> YUV420_8_LEGACY
//     <28=> YUV420_8_SHIFT
//     <29=> YUV420_10_SHIFT
//     <30=> YUV422_8
//     <31=> YUV422_10
//     <32=> RGB444
//     <33=> RGB555
//     <34=> RGB565
//     <35=> RGB666
//     <36=> RGB888
//     <40=> RAW6
//     <41=> RAW7
//     <42=> RAW8
//     <43=> RAW10
//     <44=> RAW12
//     <45=> RAW14
//     <46=> RAW16
//     <48=> USER_DEFINED_1
//     <49=> USER_DEFINED_2
//     <50=> USER_DEFINED_3
//     <51=> USER_DEFINED_4
//     <52=> USER_DEFINED_5
//     <53=> USER_DEFINED_6
//     <54=> USER_DEFINED_7
//     <55=> USER_DEFINED_8
// <i> defines select CSI2 Data type to be overwrite
// <i> default: RAW10
#define RTE_MIPI_CSI2_EN_DT                                     43

// <o> datatype to overwrite with programmed datatype
// <i> define if want to use programmed datatype ignoring datatype of the header
// <i> default: 0
#define RTE_MIPI_CSI2_EN_DT_OVERWRITE                           0

// <o> Horizontal Synchronism Active Time range <0-2047>
// <i> Defines possible range for selecting horizontal sync active time
// <i> Default: 0
#define RTE_MIPI_CSI2_IPI_HSA_TIME              5

// <o> Horizontal Synchronism back porch Time range <0-2047>
// <i> Defines possible range for selecting horizontal sync back porch time
// <i> Default: 0
#define RTE_MIPI_CSI2_IPI_HBP_TIME              10

// <o> Horizontal sync delay Time range <0-2047>
// <i> Defines possible range for selecting horizontal sync delay time
// <i> Default: 560
#define RTE_MIPI_CSI2_IPI_HSD_TIME              280

// <o> Horizontal Active Time range <0-2047>
// <i> Defines possible range for selecting horizontal active time
// <i> Default: 560
#define RTE_MIPI_CSI2_IPI_HACTIVE_TIME          560

// <o> Vertical sync active period range <0-511>
// <i> Defines possible range for selecting vertical sync active period
// <i> Default: 0
#define RTE_MIPI_CSI2_IPI_VSA_LINE              4

// <o> Vertical back porch period range <0-511>
// <i> Defines possible range for selecting vertical back porch period
// <i> Default: 0
#define RTE_MIPI_CSI2_IPI_VBP_LINE              4

// <o> Vertical front porch period range <0-511>
// <i> Defines possible range for selecting vertical front porch period
// <i> Default: 0
#define RTE_MIPI_CSI2_IPI_VFP_LINE              4

// <o> Vertical active period range <0-8191>
// <i> Defines possible range for selecting vertical active period
// <i> Default: 560
#define RTE_MIPI_CSI2_IPI_VACTIVE_LINE          560

// <o> CSI2 interrupt priority <0-255>
// <i> Defines CSI2 interrupt priority.
// <i> Default: 0
#define RTE_MIPI_CSI2_IRQ_PRI                                   0

// <e> ARX3A0 [Driver_ARX3A0]
// <o> Enable/Disable ARX3A0 camera sensor
//     <0=> disable
//     <1=> enable
// <i> define if to enable or disable ARX3A0 camera sensor
// <i> default: enable
#define RTE_ARX3A0_CAMERA_SENSOR_CSI_ENABLE             1

#if (RTE_ARX3A0_CAMERA_SENSOR_CSI_ENABLE)

// <o> Select camera ARX3A0 frame per second
//     <5=>   5 FPS
//     <40=> 40 FPS
//     <60=> 60 FPS
//     <90=> 90 FPS
// <i> Defines camera ARX3A0 frame per second
// <i> Default: 90 FPS
#define RTE_ARX3A0_CAMERA_SENSOR_CSI_CFG_FPS                 90

// <o> Select camera ARX3A0 frequency
// <i> Defines camera ARX3A0 frequency
// <i> Default: 400000000
#define RTE_ARX3A0_CAMERA_SENSOR_CSI_FREQ                    400000000

// <o> select ARX3A0 CSI2 Data type
// <i> defines select CSI2 Data type
// <i> default: 43(RAW10)
#define RTE_ARX3A0_CAMERA_SENSOR_CSI_DATA_TYPE               43

// <o> select ARX3A0 clock mode
//     <0=> CONTINUOUS CLOCK_MODE
//     <1=> NON CONTINUOUS CLOCK MODE
// <i> defines ARX3A0 clock mode for mipi csi2
// <i> default: 1 (non continuous clock mode)
#define RTE_ARX3A0_CAMERA_SENSOR_CLOCK_MODE                  1

// <o> select ARX3A0 number of lanes in DPHY
// <i> defines select ARX3A0 number of lanes in DPHY.
// <i> default: 2 two lane
#define RTE_ARX3A0_CAMERA_SENSOR_CSI_N_LANES                 2

// <o> select ARX3A0 virtual channel ID
// <i> defines select ARX3A0 virtual channel ID.
// <i> default: 0
#define RTE_ARX3A0_CAMERA_SENSOR_CSI_VC_ID                   0

// <o> select ARX3A0 override CPI color mode
// <i> defines select ARX3A0 override CPI color mode.
// <i> default: 1 (Ensable)
#define RTE_ARX3A0_CAMERA_SENSOR_OVERRIDE_CPI_COLOR_MODE     1

// <o> select ARX3A0 CPI color mode
// <i> defines select ARX3A0 CPI color mode.
// <i> default: 2  (IPI-16 RAW 8)
#define RTE_ARX3A0_CAMERA_SENSOR_CPI_COLOR_MODE              2

// <o> select ARX3A0 frame height
// <i> defines select ARX3A0 frame height.
// <i> default: 560
#define RTE_ARX3A0_CAMERA_SENSOR_FRAME_HEIGHT                560

// <o> select ARX3A0 frame width
// <i> defines select ARX3A0 frame width.
// <i> default: 560
#define RTE_ARX3A0_CAMERA_SENSOR_FRAME_WIDTH                 560

// <o> Select camera sensor ARX3A0 CSI clock source division [Divisor] <2-511>
// <i> Defines camera sensor ARX3A0 CSI clock source division
// <i> Default: 20
#define RTE_ARX3A0_CAMERA_SENSOR_CSI_CLK_SCR_DIV             20

// <o> Select camera sensor ARX3A0 reset pin number
// <i> Defines camera sensor ARX3A0 reset pin number
// <i> Default: 1
#define RTE_ARX3A0_CAMERA_SENSOR_RESET_PIN_NO                1

// <o> Select camera sensor ARX3A0 reset GPIO port
// <i> Defines camera sensor ARX3A0 reset GPIO port
// <i> Default: 9
#define RTE_ARX3A0_CAMERA_SENSOR_RESET_GPIO_PORT             9

// <o> Select camera sensor ARX3A0 power pin number
// <i> Defines camera sensor ARX3A0 power pin number
// <i> Default: 5
#define RTE_ARX3A0_CAMERA_SENSOR_POWER_PIN_NO                5

// <o> Select camera sensor ARX3A0 power GPIO port
// <i> Defines camera sensor ARX3A0 power GPIO port
// <i> Default: 7
#define RTE_ARX3A0_CAMERA_SENSOR_POWER_GPIO_PORT             7

// <o RTE_ARX3A0_CAMERA_SENSOR_I2C_INSTANCE> Select camera sensor ARX3A0 i2c instance
// <i> Defines camera sensor ARX3A0 i2c instance
//     <0=>   I2C0
//     <1=>   I2C1
//     <2=>   I2C2
//     <3=>   I2C3
//     <I3C=> I2C OVER I3C
// <i> Default: 1
#define RTE_ARX3A0_CAMERA_SENSOR_I2C_INSTANCE                1

#endif
// </e> ARX3A0 [Driver_ARX3A0]

#endif
// </e> MIPI_CSI2 (mipi csi2) [Driver_MIPI_CSI2]

// <e> MIPI_DSI (mipi dsi) [Driver_MIPI_DSI]
// <i> Configuration settings for Driver_MIPI_DSI in component ::Drivers:MIPI_DSI
#define RTE_MIPI_DSI 1

#if RTE_MIPI_DSI

// <o> DPHY PLL input division factor
// <i> Defines DPHY PLL input division factor
// <i> Default: 3
#define RTE_MIPI_DSI_PLL_INPUT_DIV_FACTOR_N     3

// <o> DSI TX escape clock division value.
// <i> Defines DSI TX escape clock division value.
// <i> Default: 2
#define RTE_MIPI_DSI_TX_ESC_CLK_DIVISION        2

// <o> DPI Video Mode
//     <0=> NON_BURST SYNC_PULSES
//     <1=> NON_BURST SYNC_EVENTS
//     <2=> BURST MODE
// <i> Defines DPI Video Mode
// <i> Default:BURST MODE
#define RTE_MIPI_DSI_VID_MODE_TYPE              2

// <o> DSI Video number of Chunks
// <i> Defines Number of chunks used to transfer single video packet .
// <i> Default: 0
#define RTE_MIPI_DSI_VID_NUM_CHUNKS             0

// <o> DSI Size of NULL packet
// <i> Defines Size of the NULL packet.
// <i> Default: 0
#define RTE_MIPI_DSI_VID_NULL_SIZE              0

// <o> DSI IRQ priority <0-255>
// <i> Defines Interrupt priority.
// <i> Default: 0
#define RTE_MIPI_DSI_IRQ_PRI                    0

// <e> MIPI_DSI (ILI9806E_PANEL) [Driver_ILI9806E_PANEL]
// <o> DSI ILI9806E LCD PANEL
//     <0=> DISABLE
//     <1=> ENABLE
// <i> Defines DSI ILI9806E LCD PANEL
// <i> Default: ENABLE
#define RTE_MIPI_DSI_ILI9806E_PANEL  1

#if RTE_MIPI_DSI_ILI9806E_PANEL

// <o> DSI ILI9806E panel variant
//     <0=> E43RB_FW405
//     <1=> E43GB_MW405
//     <2=> E50RA_MW550
// <i> Defines ILI9806E panel variant
// <i> Default: E43RB_FW405
#define RTE_ILI9806E_PANEL_VARIANT             1

#if (RTE_ILI9806E_PANEL_VARIANT == 0)
#define RTE_ILI9806E_PANEL_E43RB_FW405_EN      1
#elif (RTE_ILI9806E_PANEL_VARIANT == 1)
#define RTE_ILI9806E_PANEL_E43GB_MW405_EN      1
#elif (RTE_ILI9806E_PANEL_VARIANT == 2)
#define RTE_ILI9806E_PANEL_E50RA_MW550_EN      1
#endif

// <o> ILI9806 LCD panel maximum bitrate in mbps
// <i> Defines ILI9806 LCD panel maximum bitrate in mbps.
// <i> Default: 500
#define RTE_ILI9806E_PANEL_MAX_BITRATE_MBPS          500

// <o> DSI color mode
//     <1=> 16bit
//     <4=> 18bit
//     <5=> 24bit
// <i> Defines Color mode for display panel
// <i> Default: 24bit
#define RTE_ILI9806E_PANEL_DSI_COLOR_MODE            1

// <o> DPHY Clock Mode
//     <0=> CONTINUOUS CLOCK_MODE
//     <1=> NON CONTINUOUS CLOCK MODE
// <i> Defines DPHY Clock Mode
// <i> Default: NON CONTINUOUS CLOCK MODE
#define RTE_ILI9806E_PANEL_DPHY_CLOCK_MODE           1

// <o> Number of data lanes
//     <1=> ONE
//     <2=> TWO
// <i> Defines Number of data lanes
// <i> Default: TWO
#define RTE_ILI9806E_PANEL_DSI_N_LANES               0x2

// <o> Virtual channel ID
// <i> Defines Virtual Channel ID
// <i> Default: 0
#define RTE_ILI9806E_PANEL_DSI_VC_ID                 0

// <o> ILI9806 LCD panel reset pin number
// <i> Defines ILI9806 LCD panel reset pin number.
// <i> Default: 5
#define RTE_ILI9806E_PANEL_RESET_PIN_NO              5

// <o> ILI9806 LCD panel reset pin GPIO port number
// <i> Defines ILI9806 LCD panel reset pin GPIO port number.
// <i> Default: 15
#define RTE_ILI9806E_PANEL_RESET_GPIO_PORT           15

// <o> ILI9806 LCD panel back light pin number
// <i> Defines ILI9806 LCD panel back light pin number.
// <i> Default: 1
#define RTE_ILI9806E_PANEL_BL_LED_PIN_NO             1

// <o> ILI9806 LCD panel back light pin GPIO port number
// <i> Defines ILI9806 LCD panel back light pin GPIO port number.
// <i> Default: 6
#define RTE_ILI9806E_PANEL_BL_LED_GPIO_PORT          6

// <e> MIPI_DSI (ILI9806E_PANEL_E43RB_FW405 | ILI9806E_PANEL_E43GB_MW405) [Driver_ILI9806E_PANEL]
#if (RTE_ILI9806E_PANEL_E43RB_FW405_EN || RTE_ILI9806E_PANEL_E43GB_MW405_EN)

// <o> Panel hsync time in pixels
// <i> Defines ILI9806 LCD panel hsync time in pixels.
// <i> Default: 4
#define RTE_PANEL_HSYNC_TIME            4

// <o> Panel FW405/MW405 hbp time in pixels
// <i> Defines ILI9806 LCD panel hbp time in pixels.
// <i> Default: 5
#define RTE_PANEL_HBP_TIME              5

// <o> Panel FW405/MW405 hfp time in pixels
// <i> Defines ILI9806 LCD panel hfp time in pixels.
// <i> Default: 5
#define RTE_PANEL_HFP_TIME              5

// <o> Panel FW405/MW405 hactive pixels
// <i> Defines ILI9806 LCD panel hactive pixels.
// <i> Default: 480
#define RTE_PANEL_HACTIVE_TIME          480

// <o> Panel FW405/MW405 vsync time in lines
// <i> Defines ILI9806 LCD panel vsync time in lines.
// <i> Default: 2
#define RTE_PANEL_VSYNC_LINE            2

// <o> Panel FW405/MW405 vbp time in lines
// <i> Defines ILI9806 LCD panel vbp time in lines.
// <i> Default: 10
#define RTE_PANEL_VBP_LINE              10

// <o> Panel FW405/MW405 vfp time in lines
// <i> Defines ILI9806 LCD panel vfp time in lines.
// <i> Default: 10
#define RTE_PANEL_VFP_LINE              10

// <o> Panel FW405/MW405 vactive lines
// <i> Defines ILI9806 LCD panel vactive lines.
// <i> Default: 800
#define RTE_PANEL_VACTIVE_LINE          800
// </e> MIPI_DSI (ILI9806E_PANEL_E43RB_FW405 | ILI9806E_PANEL_E43GB_MW405) [Driver_ILI9806E_PANEL]

// <e> MIPI_DSI (ILI9806E_PANEL_E50RA_MW550) [Driver_ILI9806E_PANEL]
#elif RTE_ILI9806E_PANEL_E50RA_MW550_EN

// <o> Panel MW550 hsync time in pixels
// <i> Defines ILI9806 LCD panel hsync time in pixels.
// <i> Default: 4
#define RTE_PANEL_HSYNC_TIME            4

// <o> Panel MW550 hbp time in pixels
// <i> Defines ILI9806 LCD panel hbp time in pixels.
// <i> Default: 30
#define RTE_PANEL_HBP_TIME              30

// <o> Panel MW550 hfp time in pixels
// <i> Defines ILI9806 LCD panel hfp time in pixels.
// <i> Default: 18
#define RTE_PANEL_HFP_TIME              18

// <o> Panel MW550 hactive pixels
// <i> Defines ILI9806 LCD panel hactive pixels.
// <i> Default: 480
#define RTE_PANEL_HACTIVE_TIME          480

// <o> Panel MW550 vsync time in lines
// <i> Defines ILI9806 LCD panel vsync time in lines.
// <i> Default: 4
#define RTE_PANEL_VSYNC_LINE            4

// <o> Panel MW550 vbp time in lines
// <i> Defines ILI9806 LCD panel vbp time in lines.
// <i> Default: 30
#define RTE_PANEL_VBP_LINE              30

// <o> Panel MW550 vfp time in lines
// <i> Defines ILI9806 LCD panel vfp time in lines.
// <i> Default: 20
#define RTE_PANEL_VFP_LINE              20

// <o> Panel MW550 vactive lines
// <i> Defines ILI9806 LCD panel vactive lines.
// <i> Default: 854
#define RTE_PANEL_VACTIVE_LINE          854
#endif
// </e> MIPI_DSI (ILI9806E_PANEL_E50RA_MW550) [Driver_ILI9806E_PANEL]

#endif
// </e> MIPI_DSI (ILI9806E_PANEL) [Driver_ILI9806E_PANEL]

#endif
// </e> MIPI_DSI (mipi dsi) [Driver_MIPI_DSI]

// <e> TOUCH_SCREEN (touch screen) [Driver_Touch_Screen]
// <i> Configuration settings for Driver_Touch_Screen in component ::Drivers:touch screen
#define RTE_TOUCH_SCREEN                 1

#if RTE_TOUCH_SCREEN

// <o> GT911 Touch screen
//     <0=> DISABLE
//     <1=> ENABLE
// <i> Defines GT911 Touch screen
// <i> Default: ENABLE
#define RTE_GT911                        1

#if RTE_GT911

// <o> select active touch points
//     <1=> 1
//     <2=> 2
//     <3=> 3
//     <4=> 4
//     <5=> 5
// <i> defines select active touch points
// <i> default: 5
#define RTE_ACTIVE_TOUCH_POINTS          5

// <o> GT911 Touch screen reset pin GPIO port number range <0-15>
// <i> Defines GT911 Touch screen reset pin GPIO port number.
// <i> Default: 4
#define RTE_GT911_TOUCH_RESET_GPIO_PORT   4

// <o> GT911 Touch screen reset pin number range <0-7>
// <i> Defines GT911 Touch screen reset pin number.
// <i> Default: 0
#define RTE_GT911_TOUCH_RESET_PIN_NO      0

// <o> GT911 Touch screen INT pin GPIO port number range <0-15>
// <i> Defines GT911 Touch screen INT pin GPIO port number.
// <i> Default: 9
#define RTE_GT911_TOUCH_INT_GPIO_PORT     9

// <o> GT911 Touch screen INT pin number range <0-7>
// <i> Defines GT911 Touch screen INT pin number.
// <i> Default: 4
#define RTE_GT911_TOUCH_INT_PIN_NO        4

// <o RTE_GT911_TOUCH_I2C_INSTANCE> Select GT911 Touchscreen i2c instance
// <i> Defines GT911 Touchscreen i2c instance
//     <0=>   I2C0
//     <1=>   I2C1
//     <2=>   I2C2
//     <3=>   I2C3
//     <I3C=> I2C OVER I3C
// <i> Default: 1
#define RTE_GT911_TOUCH_I2C_INSTANCE     1
#endif

#endif
// </e> TOUCH_SCREEN (touch screen) [Driver_Touch_Screen]

// <e> CDC200 (cdc200) [Driver_CDC200]
// <i> Configuration settings for Driver_CDC200 in component ::Drivers:CDC200
#define RTE_CDC200 1

#if RTE_CDC200

// <o> CDC200 IRQ priority <0-255>
// <i> Defines Interrupt priority.
// <i> Default: 0
#define RTE_CDC200_IRQ_PRI                   0

// <o> CDC200 clock select
//     <0=>  400 MHz clock source (PLL_CLK1/2)
//     <1=>  480 MHz clock source (PLL_CLK3)
// <i> Defines CDC200 clock select
// <i> Default: 400 MHz clock source (PLL_CLK1/2)
#define RTE_CDC200_CLK_SEL                   0

// <o> CDC200 background color red <0-255>
// <i> Defines CDC200  background color red.
// <i> Default: 0
#define RTE_CDC200_BGC_RED                   0

// <o> CDC200 background color green <0-255>
// <i> Defines CDC200  background color green.
// <i> Default: 0
#define RTE_CDC200_BGC_GREEN                 0

// <o> CDC200 background color blue <0-255>
// <i> Defines CDC200  background color blue.
// <i> Default: 0
#define RTE_CDC200_BGC_BLUE                  0

// <o> CDC200 pixel format
//     <0=> ARGB8888
//     <1=> RGB888
//     <2=> RGB565
//     <3=> RGBA8888
//     <6=> ARGB1555
//     <7=> ARGB4444
// <i> Defines CDC200 pixel format
// <i> Default: RGB888
#define RTE_CDC200_PIXEL_FORMAT              2

// <o> CDC200 Constant alpha <0-255>
// <i> Defines CDC200 constant alpha range from 0 (fully transparent) to 255 or 1.0 (fully opaque).
// <i> Default: 255
#define RTE_CDC200_CONSTANT_ALPHA            255

// <o> CDC200 blending factor
//     <0=> CONST_ALPHA
//     <1=> PIXEL_ALPHA_X_CONST_ALPHA
// <i> Defines CDC200 blending factor selection.
// <i> Default: PIXEL_ALPHA_X_CONST_ALPHA
#define RTE_CDC200_BLEND_FACTOR              1

// <o> CDC200 DPI interface FPS
// <i> Defines CDC200 DPI interface Framrate per second.
// <i> Default: 60
#define RTE_CDC200_DPI_FPS                   60

// <o> Parallel ILI6122 LCD PANEL
//     <0=> DISABLE
//     <1=> ENABLE
// <i> Defines Parallel ILI6122 LCD PANEL
// <i> Default: ENABLE
#define RTE_ILI6122_PANEL  0

#if RTE_ILI6122_PANEL

// <o> CDC HSYNC pin active state
//     <0=> ACTIVE LOW
//     <1=> ACTIVE HIGH
// <i> Defines CDC HSYNC pin active state
// <i> Default: ACTIVE LOW
#define RTE_ILI6122_PANEL_CDC_HSYNC_ACTIVE_LOW       0

// <o> CDC VSYNC pin active state
//     <0=> ACTIVE LOW
//     <1=> ACTIVE HIGH
// <i> Defines CDC VSYNC pin active state
// <i> Default: ACTIVE LOW
#define RTE_ILI6122_PANEL_CDC_VSYNC_ACTIVE_LOW       0

// <o> CDC Pixel Clock Output pin active state
//     <0=> FEED THROUGH
//     <1=> INVERTED
// <i> Defines CDC Pixel Clock pin active state
// <i> Default: FEED THROUGH
#define RTE_ILI6122_PANEL_CDC_PIXCLK_FEED_THROUGH    0

// <o> CDC BLANKING pin active state
//     <0=> ACTIVE LOW
//     <1=> ACTIVE HIGH
// <i> Defines CDC BLANKINg pin active state
// <i> Default: ACTIVE LOW
#define RTE_ILI6122_PANEL_CDC_BLANK_ACTIVE_LOW       0

// <o> Panel hsync time in pixels
// <i> Defines ILI6122 LCD panel hsync time in pixels.
// <i> Default: 1
#define RTE_PANEL_HSYNC_TIME                         1

// <o> Panel hbp time in pixels
// <i> Defines ILI6122 LCD panel hbp time in pixels.
// <i> Default: 46
#define RTE_PANEL_HBP_TIME                           46

// <o> Panel hfp time in pixels
// <i> Defines ILI6122 LCD panel hfp time in pixels.
// <i> Default: 210
#define RTE_PANEL_HFP_TIME                           210

// <o> Panel hactive pixels
// <i> Defines ILI6122 LCD panel hactive pixels.
// <i> Default: 800
#define RTE_PANEL_HACTIVE_TIME                       800

// <o> Panel vsync time in lines
// <i> Defines ILI6122 LCD panel vsync time in lines.
// <i> Default: 1
#define RTE_PANEL_VSYNC_LINE                         1

// <o> Panel vbp time in lines
// <i> Defines ILI6122 LCD panel vbp time in lines.
// <i> Default: 23
#define RTE_PANEL_VBP_LINE                           23

// <o> Panel vfp time in lines
// <i> Defines ILI6122 LCD panel vfp time in lines.
// <i> Default: 22
#define RTE_PANEL_VFP_LINE                           22

// <o> Panel vactive lines
// <i> Defines ILI6122 LCD panel vactive lines.
// <i> Default: 480
#define RTE_PANEL_VACTIVE_LINE                       480
#endif

#endif
// </e> CDC200 (cdc200) [Driver_CDC200]

// <e> I3C (Improved Inter-Integrated Circuit) [Driver_I3C]
// <i> Configuration settings for Driver_I3C in component ::Drivers:I3C
#define RTE_I3C   1
#if RTE_I3C
#define RTE_I3C_IRQ_PRI       0

// <e> I2C (Inter Integrated Circuit) [Driver_I2CI3C]
// <i> Configuration settings for Driver_I2CI3C in component ::Drivers:I2CI3C
#define RTE_I2CI3C            1
// </e> I2C (Inter Integrated Circuit) [Driver_I2CI3C]

// <o> I3C DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for I3C
// <i> Default: ENABLE
#define RTE_I3C_DMA_ENABLE   0

// <o> I3C DMA Selection
//    <0=> DMA2(M55-HE)
//    <1=> DMA0
// <i> Defines Select DMA0 for I3C. By default DMA0 will be considered
// <i> Default: DMA0
#define RTE_I3C_SELECT_DMA0  1

// <o> I3C DMA IRQ priority <0-255>
// <i> Defines I3C DMA Interrupt priority
// <i> Default: 0
#define RTE_I3C_DMA_IRQ_PRI  0

#endif
// </e> I3C (Improved Inter-Integrated Circuit) [Driver_I3C]


// <h> SPI (Serial Peripheral Interface)
// <e> SPI0 (Serial Peripheral Interface 0) [Driver_SPI0]
// <i> Configuration settings for Driver_SPI0 in component ::Drivers:SPI
#define RTE_SPI0    1

#ifdef RTE_SPI0

// <o> SPI0 IRQ priority <0-255>
// <i> Defines Interrupt priority for SPI0.
// <i> Default: 0
#define RTE_SPI0_IRQ_PRIORITY                   0

// <o> SPI0 TX FIFO Start level <0-15>
// <i> Defines TX FIFO transfer start level for SPI0.
// <i> Default: 0
#define RTE_SPI0_TX_FIFO_LEVEL_TO_START_TRANSFER 0

// <o> SPI0 load dummy data
//    <0=> DISABLE
//    <1=> ENABLE
// <i> load dummy data to start transfer for SPI0.
// <i> Default: 0
#define RTE_SPI0_TX_LOAD_DUMMY_TO_START_LEVEL   0

// <o> SPI0 TX FIFO threshold <0-15>
// <i> Defines TX FIFO threshold for SPI0.
// <i> Default: 0
#define RTE_SPI0_TX_FIFO_THRESHOLD              0

// <o> SPI0 RX FIFO threshold <0-15>
// <i> Defines RX FIFO threshold for SPI0.
// <i> Default: 0
#define RTE_SPI0_RX_FIFO_THRESHOLD              0

// <o> SPI0 slave selection
//    <0=> SS_0
//    <1=> SS_1
//    <2=> SS_2
//    <3=> SS_3
// <i> Defines slave selection for SPI0.
// <i> Default: 0
#define RTE_SPI0_CHIP_SELECTION_PIN             0

// <o> SPI0 RX SAMPLE DELAY <0-4>
// <i> Defines RX SAMPLE DELAY for SPI0.
// <i> Default: 0
#define RTE_SPI0_RX_SAMPLE_DELAY                0

// <o> SPI0 Master SS SW
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines use of SW control slave select.
// <i> Default: 0
#define RTE_SPI0_USE_MASTER_SS_SW               0

#if RTE_SPI0_USE_MASTER_SS_SW

// <o> SPI0 SW SS Port <0-15>
// <i> Defines SPI0 port of SS pin in software controlled mode.
// <i> Default: 0
#define RTE_SPI0_SW_SPI_PORT                    0

// <o> SPI0 SW SS Pin <0-7>
// <i> Defines SPI0 SS pin number in software controlled mode.
// <i> Default: 0
#define RTE_SPI0_SW_SPI_PIN                     0

// <o> SPI0 SW SS Polarity
//    <0=> ACTIVE LOW
//    <1=> ACTIVE HIGH
// <i> Defines SW controlled slave select polarity for SPI0.
// <i> Default: 0
#define RTE_SPI0_SW_SPI_SS_POLARITY             0
#endif //RTE_SPI0_USE_MASTER_SS_SW

// <o> SPI0 DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for SPI0
// <i> Default: DISABLE
#define RTE_SPI0_DMA_ENABLE                     0

// <o> SPI0 DMA IRQ priority <0-255>
// <i> Defines SPI0 DMA Interrupt priority
// <i> Default: 0
#define RTE_SPI0_DMA_IRQ_PRI                    0
#endif
// </e> SPI0 (Serial Peripheral Interface 0) [Driver_SPI0]


// <e> SPI1 (Serial Peripheral Interface 1) [Driver_SPI1]
// <i> Configuration settings for Driver_SPI1 in component ::Drivers:SPI
#define RTE_SPI1    1

#ifdef RTE_SPI1

// <o> SPI1 IRQ priority <0-255>
// <i> Defines Interrupt priority for SPI1.
// <i> Default: 0
#define RTE_SPI1_IRQ_PRIORITY                   0

// <o> SPI1 TX FIFO Start level <0-15>
// <i> Defines TX FIFO transfer start level for SPI1.
// <i> Default: 0
#define RTE_SPI1_TX_FIFO_LEVEL_TO_START_TRANSFER 0

// <o> SPI1 load dummy data
//    <0=> DISABLE
//    <1=> ENABLE
// <i> load dummy data to start transfer for SPI1.
// <i> Default: 0
#define RTE_SPI1_TX_LOAD_DUMMY_TO_START_LEVEL   0

// <o> SPI1 TX FIFO threshold <0-15>
// <i> Defines TX FIFO threshold for SPI1.
// <i> Default: 0
#define RTE_SPI1_TX_FIFO_THRESHOLD              0

// <o> SPI1 RX FIFO threshold <0-15>
// <i> Defines RX FIFO threshold for SPI1.
// <i> Default: 0
#define RTE_SPI1_RX_FIFO_THRESHOLD              0

// <o> SPI1 slave selection
//    <0=> SS_0
//    <1=> SS_1
//    <2=> SS_2
//    <3=> SS_3
// <i> Defines slave selection for SPI1.
// <i> Default: 0
#define RTE_SPI1_CHIP_SELECTION_PIN             0

// <o> SPI1 RX SAMPLE DELAY <0-4>
// <i> Defines RX SAMPLE DELAY for SPI1.
// <i> Default: 0
#define RTE_SPI1_RX_SAMPLE_DELAY                0

// <o> SPI1 Master SS SW
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines use of SW control slave select.
// <i> Default: 0
#define RTE_SPI1_USE_MASTER_SS_SW               0

#if RTE_SPI1_USE_MASTER_SS_SW

// <o> SPI1 SW SS Port <0-15>
// <i> Defines SPI1 port of SS pin in software controlled mode.
// <i> Default: 0
#define RTE_SPI1_SW_SPI_PORT                    0

// <o> SPI1 SW SS pin number <0-7>
// <i> Defines SPI1 SS pin number in software controlled mode.
// <i> Default: 0
#define RTE_SPI1_SW_SPI_PIN                     0

// <o> SPI1 SW SS Polarity
//    <0=> ACTIVE LOW
//    <1=> ACTIVE HIGH
// <i> Defines SW controlled slave select polarity for SPI1.
// <i> Default: 0
#define RTE_SPI1_SW_SPI_SS_POLARITY             0
#endif //RTE_SPI1_USE_MASTER_SS_SW

// <o> SPI1 DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for SPI1
// <i> Default: DISABLE
#define RTE_SPI1_DMA_ENABLE                     0

// <o> SPI1 DMA IRQ priority <0-255>
// <i> Defines SPI1 DMA Interrupt priority
// <i> Default: 0
#define RTE_SPI1_DMA_IRQ_PRI                    0
#endif
// </e> SPI1 (Serial Peripheral Interface 1) [Driver_SPI1]

// <e> SPI2 (Serial Peripheral Interface 2) [Driver_SPI2]
// <i> Configuration settings for Driver_SPI2 in component ::Drivers:SPI
#define RTE_SPI2    1

#ifdef RTE_SPI2

// <o> SPI2 IRQ priority <0-255>
// <i> Defines Interrupt priority for SPI2.
// <i> Default: 0
#define RTE_SPI2_IRQ_PRIORITY                   0

// <o> SPI2 TX FIFO Start level <0-15>
// <i> Defines TX FIFO transfer start level for SPI2.
// <i> Default: 0
#define RTE_SPI2_TX_FIFO_LEVEL_TO_START_TRANSFER 0

// <o> SPI2 load dummy data
//    <0=> DISABLE
//    <1=> ENABLE
// <i> load dummy data to start transfer for SPI2.
// <i> Default: 0
#define RTE_SPI2_TX_LOAD_DUMMY_TO_START_LEVEL   0

// <o> SPI2 TX FIFO threshold <0-15>
// <i> Defines TX FIFO threshold for SPI2.
// <i> Default: 0
#define RTE_SPI2_TX_FIFO_THRESHOLD              0

// <o> SPI2 RX FIFO threshold <0-15>
// <i> Defines RX FIFO threshold for SPI2.
// <i> Default: 0
#define RTE_SPI2_RX_FIFO_THRESHOLD              0

// <o> SPI2 slave selection
//    <0=> SS_0
//    <1=> SS_1
//    <2=> SS_2
//    <3=> SS_3
// <i> Defines slave selection for SPI2.
// <i> Default: 0
#define RTE_SPI2_CHIP_SELECTION_PIN             0

// <o> SPI2 RX SAMPLE DELAY <0-4>
// <i> Defines RX SAMPLE DELAY for SPI2.
// <i> Default: 0
#define RTE_SPI2_RX_SAMPLE_DELAY                0

// <o> SPI2 Master SS SW
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines use of SW control slave select.
// <i> Default: 0
#define RTE_SPI2_USE_MASTER_SS_SW               0

#if RTE_SPI2_USE_MASTER_SS_SW

// <o> SPI2 SW SS Port <0-15>
// <i> Defines SPI2 port of SS pin in software controlled mode
// <i> Default: 0
#define RTE_SPI2_SW_SPI_PORT                    0

// <o> SPI2 SW SS pin number <0-7>
// <i> Defines SPI2 SS pin number in software controlled mode
// <i> Default: 0
#define RTE_SPI2_SW_SPI_PIN                     0

// <o> SPI2 SW SS Polarity
//    <0=> ACTIVE LOW
//    <1=> ACTIVE HIGH
// <i> Defines SW controlled slave select polarity for SPI2.
// <i> Default: 0
#define RTE_SPI2_SW_SPI_SS_POLARITY             0
#endif //RTE_SPI2_USE_MASTER_SS_SW

// <o> SPI2 DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for SPI2
// <i> Default: DISABLE
#define RTE_SPI2_DMA_ENABLE                     0

// <o> SPI2 DMA IRQ priority <0-255>
// <i> Defines SPI2 DMA Interrupt priority
// <i> Default: 0
#define RTE_SPI2_DMA_IRQ_PRI                    0
#endif
// </e> SPI2 (Serial Peripheral Interface 2) [Driver_SPI2]


// <e> SPI3 (Serial Peripheral Interface 3) [Driver_SPI3]
// <i> Configuration settings for Driver_SPI3 in component ::Drivers:SPI
#define RTE_SPI3    1

#ifdef RTE_SPI3

// <o> SPI3 IRQ priority <0-255>
// <i> Defines Interrupt priority for SPI3.
// <i> Default: 0
#define RTE_SPI3_IRQ_PRIORITY                   0

// <o> SPI3 TX FIFO Start level <0-15>
// <i> Defines TX FIFO transfer start level for SPI3.
// <i> Default: 0
#define RTE_SPI3_TX_FIFO_LEVEL_TO_START_TRANSFER 0

// <o> SPI3 load dummy data
//    <0=> DISABLE
//    <1=> ENABLE
// <i> load dummy data to start transfer for SPI3.
// <i> Default: 0
#define RTE_SPI3_TX_LOAD_DUMMY_TO_START_LEVEL   0

// <o> SPI3 TX FIFO threshold <0-15>
// <i> Defines TX FIFO threshold for SPI3.
// <i> Default: 0
#define RTE_SPI3_TX_FIFO_THRESHOLD              0

// <o> SPI3 RX FIFO threshold <0-15>
// <i> Defines RX FIFO threshold for SPI3.
// <i> Default: 0
#define RTE_SPI3_RX_FIFO_THRESHOLD              0

// <o> SPI3 slave selection
//    <0=> SS_0
//    <1=> SS_1
//    <2=> SS_2
//    <3=> SS_3
// <i> Defines slave selection for SPI3.
// <i> Default: 0
#define RTE_SPI3_CHIP_SELECTION_PIN             1

// <o> SPI3 RX SAMPLE DELAY <0-4>
// <i> Defines RX SAMPLE DELAY for SPI3.
// <i> Default: 0
#define RTE_SPI3_RX_SAMPLE_DELAY                0

// <o> SPI3 Master SS SW
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines use of SW control slave select.
// <i> Default: 0
#define RTE_SPI3_USE_MASTER_SS_SW               0

#if RTE_SPI3_USE_MASTER_SS_SW

// <o> SPI3 SW SS Port <0-15>
// <i> Defines SPI3 port of SS pin in software controlled mode
// <i> Default: 0
#define RTE_SPI3_SW_SPI_PORT                    0

// <o> SPI3 SW SS pin number <0-7>
// <i> Defines SPI3 SS pin number in software controlled mode
// <i> Default: 0
#define RTE_SPI3_SW_SPI_PIN                     0

// <o> SPI3 SW SS Polarity
//    <0=> ACTIVE LOW
//    <1=> ACTIVE HIGH
// <i> Defines SW controlled slave select polarity for SPI3.
// <i> Default: 0
#define RTE_SPI3_SW_SPI_SS_POLARITY             0
#endif //RTE_SPI3_USE_MASTER_SS_SW

// <o> SPI3 DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for SPI3
// <i> Default: DISABLE
#define RTE_SPI3_DMA_ENABLE                     0

// <o> SPI3 DMA IRQ priority <0-255>
// <i> Defines SPI3 DMA Interrupt priority
// <i> Default: 0
#define RTE_SPI3_DMA_IRQ_PRI                    0
#endif
// </e> SPI3 (Serial Peripheral Interface 3) [Driver_SPI3]

// <e> LPSPI (Low Power Serial Peripheral Interface) [Driver_LPSPI]
// <i> Configuration settings for Driver_LPSPI in component ::Drivers:SPI
#if defined(M55_HE)

#define RTE_LPSPI    1

#ifdef RTE_LPSPI

// <o> LPSPI IRQ priority <0-255>
// <i> Defines Interrupt priority for LPSPI.
// <i> Default: 0
#define RTE_LPSPI_IRQ_PRIORITY                   0

// <o> LPSPI TX FIFO Start level <0-15>
// <i> Defines TX FIFO transfer start level for LPSPI.
// <i> Default: 0
#define RTE_LPSPI_TX_FIFO_LEVEL_TO_START_TRANSFER 0

// <o> LPSPI load dummy data
//    <0=> DISABLE
//    <1=> ENABLE
// <i> load dummy data to start transfer for LPSPI.
// <i> Default: 0
#define RTE_LPSPI_TX_LOAD_DUMMY_TO_START_LEVEL   0

// <o> LPSPI TX FIFO threshold <0-15>
// <i> Defines TX FIFO threshold for LPSPI.
// <i> Default: 0
#define RTE_LPSPI_TX_FIFO_THRESHOLD              0

// <o> LPSPI RX FIFO threshold <0-15>
// <i> Defines RX FIFO threshold for LPSPI.
// <i> Default: 0
#define RTE_LPSPI_RX_FIFO_THRESHOLD              0

// <o> LPSPI slave selection
//    <0=> SS_0
//    <1=> SS_1
//    <2=> SS_2
//    <3=> SS_3
// <i> Defines slave selection for LPSPI.
// <i> Default: 0
#define RTE_LPSPI_CHIP_SELECTION_PIN             0

// <o> LPSPI Master SS SW
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines use of SW control slave select.
// <i> Default: 0
#define RTE_LPSPI_USE_MASTER_SS_SW               0

#if RTE_LPSPI_USE_MASTER_SS_SW

// <o> LPSPI SW SS Port  <0-15>
// <i> Defines LPSPI port of SS pin in software controlled mode
// <i> Default: 0
#define RTE_LPSPI_SW_SPI_PORT                    0

// <o> LPSPI SW SS pin number <0-7>
// <i> Defines LPSPI SS pin number in software controlled mode
// <i> Default: 0
#define RTE_LPSPI_SW_SPI_PIN                     0

// <o> LPSPI SW SS Polarity
//    <0=> ACTIVE LOW
//    <1=> ACTIVE HIGH
// <i> Defines SW controlled slave select polarity for LPSPI.
// <i> Default: 0
#define RTE_LPSPI_SW_SPI_SS_POLARITY             0
#endif //RTE_LPSPI_USE_MASTER_SS_SW

// <o> LPSPI DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for LPSPI
// <i> Default: DISABLE
#define RTE_LPSPI_DMA_ENABLE                     0
#if RTE_LPSPI_DMA_ENABLE

// <o> LPSPI DMA Selection
//    <0=> DMA2
//    <1=> DMA0
// <i> Defines Select DMA0 for LPSPI. By default DMA2 will be considered
// <i> Default: DMA2
#define RTE_LPSPI_SELECT_DMA0                    0
#if RTE_LPSPI_SELECT_DMA0

// <o> LPSPI DMA0 Group Selection
//    <1=> DMA0_GROUP_1
//    <2=> DMA0_GROUP_2
// <i> Defines DMA0 Group for LPSPI. By default DMA0 Group 1 will be considered
// <i> Default: DMA0_GROUP_1
#define RTE_LPSPI_SELECT_DMA0_GROUP              1
#endif //RTE_LPSPI_SELECT_DMA0
#endif //RTE_LPSPI_DMA_ENABLE

// <o> LPSPI DMA IRQ priority <0-255>
// <i> Defines LPSPI DMA Interrupt priority
// <i> Default: 0
#define RTE_LPSPI_DMA_IRQ_PRI                    0
#endif  //RTE_LPSPI
#endif //M55_HE

// </e> LPSPI (Low Power Serial Peripheral Interface) [Driver_LPSPI]
// </h> SPI (Serial Peripheral Interface)


// <h> OSPI  (Octal Serial Peripheral Interface)
// <e> OSPI0 (Octal Serial Peripheral Interface 0) [Driver_OSPI]
// <i> Configuration settings for Driver_OSPI in component ::Drivers:OSPI
#define RTE_OSPI0                               1
#if RTE_OSPI0
// <o> OSPI0 IRQ priority <0-255>
// <i> Defines OSPI0 Interrupt priority
// <i> Default: 0
#define RTE_OSPI0_IRQ_PRIORITY                    0

// <o> OSPI0 Frame format
//    <0=> Standard
//    <1=> Dual
//    <2=> Quad
//    <3=> Octal
// <i> Defines OSPI0 Frame format
// <i> Default: Octal
#define RTE_OSPI0_SPI_FRAME_FORMAT                3

// <o> OSPI0 TX FIFO Start level <0-255>
// <i> Defines TX FIFO transfer start level for OSPI0
// <i> Default: 0
#define RTE_OSPI0_TX_FIFO_LEVEL_TO_START_TRANSFER 0

// <o> OSPI0 load dummy data
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Load dummy data to start transfer for OSPI0
// <i> Default: DISABLE
#define RTE_OSPI0_TX_LOAD_DUMMY_TO_START_LEVEL    0

// <o> OSPI0 TX FIFO threshold <0-255>
// <i> Defines TX FIFO threshold for OSPI0
// <i> Default: 64
#define RTE_OSPI0_TX_FIFO_THRESHOLD               64

// <o> OSPI0 RX FIFO threshold <0-255>
// <i> Defines RX FIFO threshold for OSPI0
// <i> Default: 0
#define RTE_OSPI0_RX_FIFO_THRESHOLD               0

// <o> OSPI0 DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Enable DMA for OSPI0
// <i> Default: DISABLE
#define RTE_OSPI0_DMA_ENABLE                      0
#if RTE_OSPI0_DMA_ENABLE

// <o> OSPI0 TX DMA Data level
// <i> Defines TX DMA Data level for OSPI0
// <i> Default: 64
#define RTE_OSPI0_TX_DMA_DATA_LEVEL               64

// <o> OSPI0 RX DMA Data level
// <i> Defines RX DMA Data level for OSPI0
// <i> Default: 15
#define RTE_OSPI0_RX_DMA_DATA_LEVEL               15

// <o> OSPI0 DMA IRQ priority <0-255>
// <i> Defines OSPI0 DMA Interrupt priority
// <i> Default: 0
#define RTE_OSPI0_DMA_IRQ_PRIORITY                0
#endif

// <o> OSPI0 slave selection
//    <0=> SS_0
//    <1=> SS_1
// <i> Defines the slave select line for OSPI0
// <i> Default: 0
#define RTE_OSPI0_CHIP_SELECTION_PIN              0

// <o> OSPI0 RX SAMPLE DELAY
// <i> Defines RX SAMPLE DELAY for OSPI0
// <i> Default: 0
#define RTE_OSPI0_RX_SAMPLE_DELAY                 0

// <o> OSPI0 DDR DRIVE EDGE
// <i> Defines DDR DRIVE EDGE for OSPI0
// <i> Default: 0
#define RTE_OSPI0_DDR_DRIVE_EDGE                  0

// <o> OSPI0 RXDS Delay <0-16>
// <i> Defines the delay applied to the RXDS signal of OSPI0
// <i> Default: 0
#define RTE_OSPI0_RXDS_DELAY                      0
#endif
// </e> OSPI0 (Octal Serial Peripheral Interface 0) [Driver_OSPI]

// <e> OSPI1 (Octal Serial Peripheral Interface 1) [Driver_OSPI]
// <i> Configuration settings for Driver_OSPI in component ::Drivers:OSPI
#define RTE_OSPI1                               1
#if RTE_OSPI1
// <o> OSPI1 IRQ priority <0-255>
// <i> Defines OSPI1 Interrupt priority
// <i> Default: 0
#define RTE_OSPI1_IRQ_PRIORITY                    0

// <o> OSPI1 Frame format
//    <0=> Standard
//    <1=> Dual
//    <2=> Quad
//    <3=> Octal
// <i> Defines OSPI0 Frame format
// <i> Default: Octal
#define RTE_OSPI1_SPI_FRAME_FORMAT                3

// <o> OSPI1 TX FIFO Start level <0-255>
// <i> Defines TX FIFO transfer start level for OSPI1
// <i> Default: 0
#define RTE_OSPI1_TX_FIFO_LEVEL_TO_START_TRANSFER 0

// <o> OSPI1 load dummy data
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Load dummy data to start transfer for OSPI1
// <i> Default: DISABLE
#define RTE_OSPI1_TX_LOAD_DUMMY_TO_START_LEVEL    0

// <o> OSPI1 TX FIFO threshold <0-255>
// <i> Defines TX FIFO threshold for OSPI1
// <i> Default: 64
#define RTE_OSPI1_TX_FIFO_THRESHOLD               64

// <o> OSPI1 RX FIFO threshold <0-255>
// <i> Defines RX FIFO threshold for OSPI1
// <i> Default: 0
#define RTE_OSPI1_RX_FIFO_THRESHOLD               0

// <o> OSPI1 DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Enable DMA for OSPI1
// <i> Default: DISABLE
#define RTE_OSPI1_DMA_ENABLE                      0
#if RTE_OSPI1_DMA_ENABLE

// <o> OSPI1 TX DMA Data level
// <i> Defines TX DMA Data level for OSPI1
// <i> Default: 130
#define RTE_OSPI1_TX_DMA_DATA_LEVEL               130

// <o> OSPI1 RX DMA Data level
// <i> Defines RX DMA Data level for OSPI1
// <i> Default: 15
#define RTE_OSPI1_RX_DMA_DATA_LEVEL               15

// <o> OSPI1 DMA IRQ priority <0-255>
// <i> Defines OSPI1 DMA Interrupt priority
// <i> Default: 0
#define RTE_OSPI1_DMA_IRQ_PRIORITY                0
#endif

// <o> OSPI1 slave selection
//    <0=> SS_0
//    <1=> SS_1
// <i> Defines the slave select line for OSPI1.
// <i> Default: 0
#define RTE_OSPI1_CHIP_SELECTION_PIN              0

// <o> OSPI1 RX SAMPLE DELAY
// <i> Defines RX SAMPLE DELAY for OSPI1
// <i> Default: 0
#define RTE_OSPI1_RX_SAMPLE_DELAY                 0

// <o> OSPI1 DDR DRIVE EDGE
// <i> Defines DDR DRIVE EDGE for OSPI1
// <i> Default: 1
#define RTE_OSPI1_DDR_DRIVE_EDGE                  1

// <o> OSPI1 RXDS Delay <0-16>
// <i> Defines the delay applied to the RXDS signal of OSPI1
// <i> Default: 11
#define RTE_OSPI1_RXDS_DELAY                      11
#endif
// </e> OSPI1 (Octal Serial Peripheral Interface 1) [Driver_OSPI]

//</h>
// <e> FLASH (ISSI FLASH) [Driver_Flash]
// <i> Configuration settings for Driver_Flash in component ::Drivers:Flash
#define RTE_ISSI_FLASH               1
#if RTE_ISSI_FLASH

// <o> ISSI FLASH OSPI Instance
// <i> Defines the OSPI controller instance the ISSI flash is connected to
// <i> Default: 1
#define RTE_ISSI_FLASH_OSPI_DRV_NUM             1

// <o> ISSI FLASH OSPI Bus speed
// <i> Defines the OSPI Bus speed
// <i> Default: 100000000
#define RTE_ISSI_FLASH_OSPI_BUS_SPEED           100000000
#endif
// </e> FLASH (ISSI FLASH) [Driver_Flash]


// <h> I2S  (Integrated Interchip Sound)
// <e> I2S0 (Integrated Interchip Sound 0) [Driver_SAI0]
// <i> Configuration settings for Driver_SAI0 in component ::Drivers:SAI
#define RTE_I2S0   1

#if RTE_I2S0

// <o> I2S0 WORD SELECT SIZE
//    <0=> WSS_CLOCK_CYCLES_16
//    <1=> WSS_CLOCK_CYCLES_24
//    <2=> WSS_CLOCK_CYCLES_32
// <i> Defines I2S0 size of word
// <i> Default: WSS_CLOCK_CYCLES_32
#define RTE_I2S0_WSS_CLOCK_CYCLES   2

// <o> I2S0 SCLK GATING
//    <0=> NO_CLOCK_GATING
//    <1=> SCLKG_CLOCK_CYCLES_12
//    <2=> SCLKG_CLOCK_CYCLES_16
//    <3=> SCLKG_CLOCK_CYCLES_20
//    <4=> SCLKG_CLOCK_CYCLES_24
// <i> Defines I2S0 SCLK Gating
// <i> Default: NO_CLOCK_GATING
#define RTE_I2S0_SCLKG_CLOCK_CYCLES   0

// <o> I2S0 RX FIFO TRIGGER LEVEL
//    <0=>  TRIGGER_LEVEL_1
//    <1=>  TRIGGER_LEVEL_2
//    <2=>  TRIGGER_LEVEL_3
//    <3=>  TRIGGER_LEVEL_4
//    <4=>  TRIGGER_LEVEL_5
//    <5=>  TRIGGER_LEVEL_6
//    <6=>  TRIGGER_LEVEL_7
//    <7=>  TRIGGER_LEVEL_8
//    <8=>  TRIGGER_LEVEL_9
//    <9=>  TRIGGER_LEVEL_10
//    <10=> TRIGGER_LEVEL_11
//    <11=> TRIGGER_LEVEL_12
//    <12=> TRIGGER_LEVEL_13
//    <13=> TRIGGER_LEVEL_14
//    <14=> TRIGGER_LEVEL_15
//    <15=> TRIGGER_LEVEL_16
// <i> Defines I2S0 receive fifo trigger level
// <i> Default: TRIGGER_LEVEL_8
#define RTE_I2S0_RX_TRIG_LVL   7

// <o> I2S0 TX FIFO TRIGGER LEVEL
//    <0=>  TRIGGER_LEVEL_1
//    <1=>  TRIGGER_LEVEL_2
//    <2=>  TRIGGER_LEVEL_3
//    <3=>  TRIGGER_LEVEL_4
//    <4=>  TRIGGER_LEVEL_5
//    <5=>  TRIGGER_LEVEL_6
//    <6=>  TRIGGER_LEVEL_7
//    <7=>  TRIGGER_LEVEL_8
//    <8=>  TRIGGER_LEVEL_9
//    <9=>  TRIGGER_LEVEL_10
//    <10=> TRIGGER_LEVEL_11
//    <11=> TRIGGER_LEVEL_12
//    <12=> TRIGGER_LEVEL_13
//    <13=> TRIGGER_LEVEL_14
//    <14=> TRIGGER_LEVEL_15
//    <15=> TRIGGER_LEVEL_16
// <i> Defines I2S0 transfer fifo trigger level
// <i> Default: TRIGGER_LEVEL_9
#define RTE_I2S0_TX_TRIG_LVL   8

// <o> I2S0 IRQ priority <0-255>
// <i> Defines I2S0 Interrupt priority
// <i> Default: 0
#define RTE_I2S0_IRQ_PRI   0

// <o> I2S0 DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for I2S0
// <i> Default: ENABLE
#define RTE_I2S0_DMA_ENABLE   0

// <o> I2S0 DMA IRQ priority <0-255>
// <i> Defines I2S0 DMA Interrupt priority
// <i> Default: 0
#define RTE_I2S0_DMA_IRQ_PRI   0

#endif
// </e> I2S0 (Integrated Interchip Sound 0) [Driver_SAI0]


// <e> I2S1 (Integrated Interchip Sound 1) [Driver_SAI1]
// <i> Configuration settings for Driver_SAI1 in component ::Drivers:SAI
#define RTE_I2S1   1

#if RTE_I2S1

// <o> I2S1 WORD SELECT SIZE
//    <0=> WSS_CLOCK_CYCLES_16
//    <1=> WSS_CLOCK_CYCLES_24
//    <2=> WSS_CLOCK_CYCLES_32
// <i> Defines I2S1 size of word
// <i> Default: WSS_CLOCK_CYCLES_32
#define RTE_I2S1_WSS_CLOCK_CYCLES   2

// <o> I2S1 SCLK GATING
//    <0=> NO_CLOCK_GATING
//    <1=> SCLKG_CLOCK_CYCLES_12
//    <2=> SCLKG_CLOCK_CYCLES_16
//    <3=> SCLKG_CLOCK_CYCLES_20
//    <4=> SCLKG_CLOCK_CYCLES_24
// <i> Defines I2S1 SCLK Gating
// <i> Default: NO_CLOCK_GATING
#define RTE_I2S1_SCLKG_CLOCK_CYCLES   0

// <o> I2S1 RX FIFO TRIGGER LEVEL
//    <0=>  TRIGGER_LEVEL_1
//    <1=>  TRIGGER_LEVEL_2
//    <2=>  TRIGGER_LEVEL_3
//    <3=>  TRIGGER_LEVEL_4
//    <4=>  TRIGGER_LEVEL_5
//    <5=>  TRIGGER_LEVEL_6
//    <6=>  TRIGGER_LEVEL_7
//    <7=>  TRIGGER_LEVEL_8
//    <8=>  TRIGGER_LEVEL_9
//    <9=>  TRIGGER_LEVEL_10
//    <10=> TRIGGER_LEVEL_11
//    <11=> TRIGGER_LEVEL_12
//    <12=> TRIGGER_LEVEL_13
//    <13=> TRIGGER_LEVEL_14
//    <14=> TRIGGER_LEVEL_15
//    <15=> TRIGGER_LEVEL_16
// <i> Defines I2S1 receive fifo trigger level
// <i> Default: TRIGGER_LEVEL_8
#define RTE_I2S1_RX_TRIG_LVL   7

// <o> I2S1 TX FIFO TRIGGER LEVEL
//    <0=>  TRIGGER_LEVEL_1
//    <1=>  TRIGGER_LEVEL_2
//    <2=>  TRIGGER_LEVEL_3
//    <3=>  TRIGGER_LEVEL_4
//    <4=>  TRIGGER_LEVEL_5
//    <5=>  TRIGGER_LEVEL_6
//    <6=>  TRIGGER_LEVEL_7
//    <7=>  TRIGGER_LEVEL_8
//    <8=>  TRIGGER_LEVEL_9
//    <9=>  TRIGGER_LEVEL_10
//    <10=> TRIGGER_LEVEL_11
//    <11=> TRIGGER_LEVEL_12
//    <12=> TRIGGER_LEVEL_13
//    <13=> TRIGGER_LEVEL_14
//    <14=> TRIGGER_LEVEL_15
//    <15=> TRIGGER_LEVEL_16
// <i> Defines I2S1 transfer fifo trigger level
// <i> Default: TRIGGER_LEVEL_9
#define RTE_I2S1_TX_TRIG_LVL   8

// <o> I2S1 IRQ priority <0-255>
// <i> Defines I2S1 Interrupt priority
// <i> Default: 1
#define RTE_I2S1_IRQ_PRI   1

// <o> I2S1 DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for I2S1
// <i> Default: ENABLE
#define RTE_I2S1_DMA_ENABLE   0

// <o> I2S1 DMA IRQ priority <0-255>
// <i> Defines I2S1 DMA Interrupt priority
// <i> Default: 0
#define RTE_I2S1_DMA_IRQ_PRI   0
#endif
// </e> I2S1 (Integrated Interchip Sound 1) [Driver_SAI1]


// <e> I2S2 (Integrated Interchip Sound 2) [Driver_SAI2]
// <i> Configuration settings for Driver_SAI2 in component ::Drivers:SAI
#define RTE_I2S2   1

#if RTE_I2S2

// <o> I2S2 WORD SELECT SIZE
//    <0=> WSS_CLOCK_CYCLES_16
//    <1=> WSS_CLOCK_CYCLES_24
//    <2=> WSS_CLOCK_CYCLES_32
// <i> Defines I2S2 size of word
// <i> Default: WSS_CLOCK_CYCLES_32
#define RTE_I2S2_WSS_CLOCK_CYCLES   2

// <o> I2S2 SCLK GATING
//    <0=> NO_CLOCK_GATING
//    <1=> SCLKG_CLOCK_CYCLES_12
//    <2=> SCLKG_CLOCK_CYCLES_16
//    <3=> SCLKG_CLOCK_CYCLES_20
//    <4=> SCLKG_CLOCK_CYCLES_24
// <i> Defines I2S2 SCLK Gating
// <i> Default: NO_CLOCK_GATING
#define RTE_I2S2_SCLKG_CLOCK_CYCLES   0

// <o> I2S2 RX FIFO TRIGGER LEVEL
//    <0=>  TRIGGER_LEVEL_1
//    <1=>  TRIGGER_LEVEL_2
//    <2=>  TRIGGER_LEVEL_3
//    <3=>  TRIGGER_LEVEL_4
//    <4=>  TRIGGER_LEVEL_5
//    <5=>  TRIGGER_LEVEL_6
//    <6=>  TRIGGER_LEVEL_7
//    <7=>  TRIGGER_LEVEL_8
//    <8=>  TRIGGER_LEVEL_9
//    <9=>  TRIGGER_LEVEL_10
//    <10=> TRIGGER_LEVEL_11
//    <11=> TRIGGER_LEVEL_12
//    <12=> TRIGGER_LEVEL_13
//    <13=> TRIGGER_LEVEL_14
//    <14=> TRIGGER_LEVEL_15
//    <15=> TRIGGER_LEVEL_16
// <i> Defines I2S2 receive fifo trigger level
// <i> Default: TRIGGER_LEVEL_8
#define RTE_I2S2_RX_TRIG_LVL   7

// <o> I2S2 TX FIFO TRIGGER LEVEL
//    <0=>  TRIGGER_LEVEL_1
//    <1=>  TRIGGER_LEVEL_2
//    <2=>  TRIGGER_LEVEL_3
//    <3=>  TRIGGER_LEVEL_4
//    <4=>  TRIGGER_LEVEL_5
//    <5=>  TRIGGER_LEVEL_6
//    <6=>  TRIGGER_LEVEL_7
//    <7=>  TRIGGER_LEVEL_8
//    <8=>  TRIGGER_LEVEL_9
//    <9=>  TRIGGER_LEVEL_10
//    <10=> TRIGGER_LEVEL_11
//    <11=> TRIGGER_LEVEL_12
//    <12=> TRIGGER_LEVEL_13
//    <13=> TRIGGER_LEVEL_14
//    <14=> TRIGGER_LEVEL_15
//    <15=> TRIGGER_LEVEL_16
// <i> Defines I2S2 transfer fifo trigger level
// <i> Default: TRIGGER_LEVEL_9
#define RTE_I2S2_TX_TRIG_LVL   8

// <o> I2S2 IRQ priority <0-255>
// <i> Defines I2S2 Interrupt priority
// <i> Default: 0
#define RTE_I2S2_IRQ_PRI   10

// <o> I2S2 DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for I2S2
// <i> Default: ENABLE
#define RTE_I2S2_DMA_ENABLE   0

// <o> I2S2 DMA IRQ priority <0-255>
// <i> Defines I2S2 DMA Interrupt priority
// <i> Default: 0
#define RTE_I2S2_DMA_IRQ_PRI   0
#endif
// </e> I2S2 (Integrated Interchip Sound 2) [Driver_SAI2]


// <e> I2S3 (Integrated Interchip Sound 3) [Driver_SAI3]
// <i> Configuration settings for Driver_SAI3 in component ::Drivers:SAI
#define RTE_I2S3   1

#if RTE_I2S3

// <o> I2S3 WORD SELECT SIZE
//    <0=> WSS_CLOCK_CYCLES_16
//    <1=> WSS_CLOCK_CYCLES_24
//    <2=> WSS_CLOCK_CYCLES_32
// <i> Defines I2S3 size of word
// <i> Default: WSS_CLOCK_CYCLES_32
#define RTE_I2S3_WSS_CLOCK_CYCLES   2

// <o> I2S3 SCLK GATING
//    <0=> NO_CLOCK_GATING
//    <1=> SCLKG_CLOCK_CYCLES_12
//    <2=> SCLKG_CLOCK_CYCLES_16
//    <3=> SCLKG_CLOCK_CYCLES_20
//    <4=> SCLKG_CLOCK_CYCLES_24
// <i> Defines I2S3 SCLK Gating
// <i> Default: NO_CLOCK_GATING
#define RTE_I2S3_SCLKG_CLOCK_CYCLES   0

// <o> I2S3 RX FIFO TRIGGER LEVEL
//    <0=>  TRIGGER_LEVEL_1
//    <1=>  TRIGGER_LEVEL_2
//    <2=>  TRIGGER_LEVEL_3
//    <3=>  TRIGGER_LEVEL_4
//    <4=>  TRIGGER_LEVEL_5
//    <5=>  TRIGGER_LEVEL_6
//    <6=>  TRIGGER_LEVEL_7
//    <7=>  TRIGGER_LEVEL_8
//    <8=>  TRIGGER_LEVEL_9
//    <9=>  TRIGGER_LEVEL_10
//    <10=> TRIGGER_LEVEL_11
//    <11=> TRIGGER_LEVEL_12
//    <12=> TRIGGER_LEVEL_13
//    <13=> TRIGGER_LEVEL_14
//    <14=> TRIGGER_LEVEL_15
//    <15=> TRIGGER_LEVEL_16
// <i> Defines I2S3 receive fifo trigger level
// <i> Default: TRIGGER_LEVEL_8
#define RTE_I2S3_RX_TRIG_LVL   7

// <o> I2S3 TX FIFO TRIGGER LEVEL
//    <0=>  TRIGGER_LEVEL_1
//    <1=>  TRIGGER_LEVEL_2
//    <2=>  TRIGGER_LEVEL_3
//    <3=>  TRIGGER_LEVEL_4
//    <4=>  TRIGGER_LEVEL_5
//    <5=>  TRIGGER_LEVEL_6
//    <6=>  TRIGGER_LEVEL_7
//    <7=>  TRIGGER_LEVEL_8
//    <8=>  TRIGGER_LEVEL_9
//    <9=>  TRIGGER_LEVEL_10
//    <10=> TRIGGER_LEVEL_11
//    <11=> TRIGGER_LEVEL_12
//    <12=> TRIGGER_LEVEL_13
//    <13=> TRIGGER_LEVEL_14
//    <14=> TRIGGER_LEVEL_15
//    <15=> TRIGGER_LEVEL_16
// <i> Defines I2S3 transfer fifo trigger level
// <i> Default: TRIGGER_LEVEL_9
#define RTE_I2S3_TX_TRIG_LVL   8

// <o> I2S3 IRQ priority <0-255>
// <i> Defines I2S3 Interrupt priority
// <i> Default: 0
#define RTE_I2S3_IRQ_PRI   0

// <o> I2S3 DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for I2S3
// <i> Default: ENABLE
#define RTE_I2S3_DMA_ENABLE   0

// <o> I2S3 DMA IRQ priority <0-255>
// <i> Defines I2S3 DMA Interrupt priority
// <i> Default: 0
#define RTE_I2S3_DMA_IRQ_PRI  RTE_I2S3_IRQ_PRI   0
#endif
// </e> I2S3 (Integrated Interchip Sound 3) [Driver_SAI3]


// <e> LPI2S (Low Power Integrated Interchip Sound) [Driver_SAILP]
// <i> Configuration settings for Driver_SAILP in component ::Drivers:SAI
#if defined(M55_HE)

#define RTE_LPI2S     1

#if RTE_LPI2S

// <o> LPI2S WORD SELECT SIZE
//    <0=> WSS_CLOCK_CYCLES_16
//    <1=> WSS_CLOCK_CYCLES_24
//    <2=> WSS_CLOCK_CYCLES_32
// <i> Defines LPI2S size of word
// <i> Default: WSS_CLOCK_CYCLES_32
#define RTE_LPI2S_WSS_CLOCK_CYCLES   2

// <o> LPI2S SCLK GATING
//    <0=> NO_CLOCK_GATING
//    <1=> SCLKG_CLOCK_CYCLES_12
//    <2=> SCLKG_CLOCK_CYCLES_16
//    <3=> SCLKG_CLOCK_CYCLES_20
//    <4=> SCLKG_CLOCK_CYCLES_24
// <i> Defines LPI2S SCLK Gating
// <i> Default: NO_CLOCK_GATING
#define RTE_LPI2S_SCLKG_CLOCK_CYCLES   0

// <o> LPI2S RX FIFO TRIGGER LEVEL
//    <0=>  TRIGGER_LEVEL_1
//    <1=>  TRIGGER_LEVEL_2
//    <2=>  TRIGGER_LEVEL_3
//    <3=>  TRIGGER_LEVEL_4
//    <4=>  TRIGGER_LEVEL_5
//    <5=>  TRIGGER_LEVEL_6
//    <6=>  TRIGGER_LEVEL_7
//    <7=>  TRIGGER_LEVEL_8
//    <8=>  TRIGGER_LEVEL_9
//    <9=>  TRIGGER_LEVEL_10
//    <10=> TRIGGER_LEVEL_11
//    <11=> TRIGGER_LEVEL_12
//    <12=> TRIGGER_LEVEL_13
//    <13=> TRIGGER_LEVEL_14
//    <14=> TRIGGER_LEVEL_15
//    <15=> TRIGGER_LEVEL_16
// <i> Defines LPI2S receive fifo trigger level
// <i> Default: TRIGGER_LEVEL_8
#define RTE_LPI2S_RX_TRIG_LVL   7

// <o> LPI2S TX FIFO TRIGGER LEVEL
//    <0=>  TRIGGER_LEVEL_1
//    <1=>  TRIGGER_LEVEL_2
//    <2=>  TRIGGER_LEVEL_3
//    <3=>  TRIGGER_LEVEL_4
//    <4=>  TRIGGER_LEVEL_5
//    <5=>  TRIGGER_LEVEL_6
//    <6=>  TRIGGER_LEVEL_7
//    <7=>  TRIGGER_LEVEL_8
//    <8=>  TRIGGER_LEVEL_9
//    <9=>  TRIGGER_LEVEL_10
//    <10=> TRIGGER_LEVEL_11
//    <11=> TRIGGER_LEVEL_12
//    <12=> TRIGGER_LEVEL_13
//    <13=> TRIGGER_LEVEL_14
//    <14=> TRIGGER_LEVEL_15
//    <15=> TRIGGER_LEVEL_16
// <i> Defines LPI2S transfer fifo trigger level
// <i> Default: TRIGGER_LEVEL_9
#define RTE_LPI2S_TX_TRIG_LVL   8

// <o> LPI2S IRQ priority <0-255>
// <i> Defines LPI2S Interrupt priority
// <i> Default: 1
#define RTE_LPI2S_IRQ_PRI   1

// <o> LPI2S DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for LPI2S
// <i> Default: ENABLE
#define RTE_LPI2S_DMA_ENABLE   0
#if RTE_LPI2S_DMA_ENABLE

// <o> LPI2S DMA Selection
//    <0=> DMA2
//    <1=> DMA0
// <i> Defines Select DMA0 for LPI2S. By default DMA2 will be considered
// <i> Default: DMA2
#define RTE_LPI2S_SELECT_DMA0   0
#endif

// <o> LPI2S DMA IRQ priority <0-255>
// <i> Defines LPI2S DMA Interrupt priority
// <i> Default: 0
#define RTE_LPI2S_DMA_IRQ_PRI   0
#endif //RTE_LPI2S
#endif //defined(M55_HE)

// </e> LPI2S (Low Power Integrated Interchip Sound) [Driver_SAILP]
// </h> I2S (Integrated Interchip Sound)

// <h> UART (Universal asynchronous receiver transmitter)
// <e> UART0 (Universal asynchronous receiver transmitter) [Driver_USART0]
// <i> Configuration settings for Driver_USART0 in component ::Drivers:USART
#define RTE_UART0   1

#if RTE_UART0

// <o> UART0 RX FIFO TRIGGER LEVEL
//    <0=>  UART_RX_ONE_CHAR_IN_FIFO
//    <1=>  UART_RX_FIFO_QUARTER_FULL
//    <2=>  UART_RX_FIFO_HALF_FULL
//    <3=>  UART_RX_FIFO_TWO_LESS_FULL
// <i> Defines UART0 receive fifo trigger level.
// <i> Default: UART_RX_ONE_CHAR_IN_FIFO
#define RTE_UART0_RX_TRIG_LVL   0

// <o> UART0 TX FIFO TRIGGER LEVEL
//    <0=>  UART_TX_FIFO_EMPTY
//    <1=>  UART_TX_FIFO_CHAR_2
//    <2=>  UART_TX_FIFO_QUARTER_FULL
//    <3=>  UART_TX_FIFO_HALF_FULL
// <i> Defines UART0 transmit fifo trigger level.
// <i> Default: UART_TX_FIFO_EMPTY
#define RTE_UART0_TX_TRIG_LVL   0

// <o> UART0 IRQ priority <0-255>
// <i> Defines Interrupt priority for UART0.
// <i> Default: 0
#define RTE_UART0_IRQ_PRI       0

// <o> UART0 CLK SOURCE
//    <0=> CLK_38.4MHz
//    <1=> CLK_100MHz
// <i> Defines UART0 clock source.
// <i> Default: CLK_100MHz
#define RTE_UART0_CLK_SOURCE    1

// <o> UART0 DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for UART0
// <i> Default: ENABLE
#define RTE_UART0_DMA_ENABLE   0

// <o> UART0 DMA IRQ priority <0-255>
// <i> Defines UART0 DMA Interrupt priority
// <i> Default: 0
#define RTE_UART0_DMA_IRQ_PRI  0

#endif //UART0

// </e> UART0 (Universal asynchronous receiver transmitter) [Driver_USART0]


// <e> UART1 (Universal asynchronous receiver transmitter) [Driver_USART1]
// <i> Configuration settings for Driver_USART1 in component ::Drivers:USART
#define RTE_UART1   1

#if RTE_UART1

// <o> UART1 RX FIFO TRIGGER LEVEL
//    <0=>  UART_RX_ONE_CHAR_IN_FIFO
//    <1=>  UART_RX_FIFO_QUARTER_FULL
//    <2=>  UART_RX_FIFO_HALF_FULL
//    <3=>  UART_RX_FIFO_TWO_LESS_FULL
// <i> Defines UART1 receive fifo trigger level.
// <i> Default: UART_RX_ONE_CHAR_IN_FIFO
#define RTE_UART1_RX_TRIG_LVL   0

// <o> UART1 TX FIFO TRIGGER LEVEL
//    <0=>  UART_TX_FIFO_EMPTY
//    <1=>  UART_TX_FIFO_CHAR_2
//    <2=>  UART_TX_FIFO_QUARTER_FULL
//    <3=>  UART_TX_FIFO_HALF_FULL
// <i> Defines UART1 transmit fifo trigger level.
// <i> Default: UART_TX_FIFO_EMPTY
#define RTE_UART1_TX_TRIG_LVL   0

// <o> UART1 IRQ priority <0-255>
// <i> Defines Interrupt priority for UART1.
// <i> Default: 0
#define RTE_UART1_IRQ_PRI       0

// <o> UART1 CLK SOURCE
//    <0=> CLK_38.4MHz
//    <1=> CLK_100MHz
// <i> Defines UART1 clock source.
// <i> Default: CLK_100MHz
#define RTE_UART1_CLK_SOURCE    1

// <o> UART1 DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for UART1
// <i> Default: ENABLE
#define RTE_UART1_DMA_ENABLE   0

// <o> UART1 DMA IRQ priority <0-255>
// <i> Defines UART1 DMA Interrupt priority
// <i> Default: 0
#define RTE_UART1_DMA_IRQ_PRI  0

#endif //UART1

// </e> UART1 (Universal asynchronous receiver transmitter) [Driver_USART1]


// <e> UART2 (Universal asynchronous receiver transmitter) [Driver_USART2]
// <i> Configuration settings for Driver_USART2 in component ::Drivers:USART
#define RTE_UART2   1

#if RTE_UART2

// <o> UART2 RX FIFO TRIGGER LEVEL
//    <0=>  UART_RX_ONE_CHAR_IN_FIFO
//    <1=>  UART_RX_FIFO_QUARTER_FULL
//    <2=>  UART_RX_FIFO_HALF_FULL
//    <3=>  UART_RX_FIFO_TWO_LESS_FULL
// <i> Defines UART2 receive fifo trigger level.
// <i> Default: UART_RX_ONE_CHAR_IN_FIFO
#define RTE_UART2_RX_TRIG_LVL   0

// <o> UART2 TX FIFO TRIGGER LEVEL
//    <0=>  UART_TX_FIFO_EMPTY
//    <1=>  UART_TX_FIFO_CHAR_2
//    <2=>  UART_TX_FIFO_QUARTER_FULL
//    <3=>  UART_TX_FIFO_HALF_FULL
// <i> Defines UART2 transmit fifo trigger level.
// <i> Default: UART_TX_FIFO_EMPTY
#define RTE_UART2_TX_TRIG_LVL   0

// <o> UART2 IRQ priority <0-255>
// <i> Defines Interrupt priority for UART2.
// <i> Default: 0
#define RTE_UART2_IRQ_PRI       0

// <o> UART2 CLK SOURCE
//    <0=> CLK_38.4MHz
//    <1=> CLK_100MHz
// <i> Defines UART2 clock source.
// <i> Default: CLK_38.4MHz
#define RTE_UART2_CLK_SOURCE    0

// <o> UART2 DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for UART2
// <i> Default: ENABLE
#define RTE_UART2_DMA_ENABLE   0

// <o> UART2 DMA IRQ priority <0-255>
// <i> Defines UART2 DMA Interrupt priority
// <i> Default: 0
#define RTE_UART2_DMA_IRQ_PRI  0

#endif //UART2

// </e> UART2 (Universal asynchronous receiver transmitter) [Driver_USART2]


// <e> UART3 (Universal asynchronous receiver transmitter) [Driver_USART3]
// <i> Configuration settings for Driver_USART3 in component ::Drivers:USART
#define RTE_UART3   1

#if RTE_UART3

// <o> UART3 RX FIFO TRIGGER LEVEL
//    <0=>  UART_RX_ONE_CHAR_IN_FIFO
//    <1=>  UART_RX_FIFO_QUARTER_FULL
//    <2=>  UART_RX_FIFO_HALF_FULL
//    <3=>  UART_RX_FIFO_TWO_LESS_FULL
// <i> Defines UART3 receive fifo trigger level.
// <i> Default: UART_RX_ONE_CHAR_IN_FIFO
#define RTE_UART3_RX_TRIG_LVL   0

// <o> UART3 TX FIFO TRIGGER LEVEL
//    <0=>  UART_TX_FIFO_EMPTY
//    <1=>  UART_TX_FIFO_CHAR_2
//    <2=>  UART_TX_FIFO_QUARTER_FULL
//    <3=>  UART_TX_FIFO_HALF_FULL
// <i> Defines UART3 transmit fifo trigger level.
// <i> Default: UART_TX_FIFO_EMPTY
#define RTE_UART3_TX_TRIG_LVL   0

// <o> UART3 IRQ priority <0-255>
// <i> Defines Interrupt priority for UART3.
// <i> Default: 0
#define RTE_UART3_IRQ_PRI       0

// <o> UART3 CLK SOURCE
//    <0=> CLK_38.4MHz
//    <1=> CLK_100MHz
// <i> Defines UART3 clock source.
// <i> Default: CLK_100MHz
#define RTE_UART3_CLK_SOURCE    1

// <o> UART3 DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for UART3
// <i> Default: ENABLE
#define RTE_UART3_DMA_ENABLE   0

// <o> UART3 DMA IRQ priority <0-255>
// <i> Defines UART3 DMA Interrupt priority
// <i> Default: 0
#define RTE_UART3_DMA_IRQ_PRI  0

#endif //UART3

// </e> UART3 (Universal asynchronous receiver transmitter) [Driver_USART3]


// <e> UART4 (Universal asynchronous receiver transmitter) [Driver_USART4]
// <i> Configuration settings for Driver_USART4 in component ::Drivers:USART
#define RTE_UART4   1

#if RTE_UART4

// <o> UART4 RX FIFO TRIGGER LEVEL
//    <0=>  UART_RX_ONE_CHAR_IN_FIFO
//    <1=>  UART_RX_FIFO_QUARTER_FULL
//    <2=>  UART_RX_FIFO_HALF_FULL
//    <3=>  UART_RX_FIFO_TWO_LESS_FULL
// <i> Defines UART4 receive fifo trigger level.
// <i> Default: UART_RX_ONE_CHAR_IN_FIFO
#define RTE_UART4_RX_TRIG_LVL   0

// <o> UART4 TX FIFO TRIGGER LEVEL
//    <0=>  UART_TX_FIFO_EMPTY
//    <1=>  UART_TX_FIFO_CHAR_2
//    <2=>  UART_TX_FIFO_QUARTER_FULL
//    <3=>  UART_TX_FIFO_HALF_FULL
// <i> Defines UART4 transmit fifo trigger level.
// <i> Default: UART_TX_FIFO_EMPTY
#define RTE_UART4_TX_TRIG_LVL   0

// <o> UART4 IRQ priority <0-255>
// <i> Defines Interrupt priority for UART4.
// <i> Default: 0
#define RTE_UART4_IRQ_PRI       0

// <o> UART4 CLK SOURCE
//    <0=> CLK_38.4MHz
//    <1=> CLK_100MHz
// <i> Defines UART4 clock source.
// <i> Default: CLK_100MHz
#define RTE_UART4_CLK_SOURCE    1

#define RTE_UART4_RS485_ENABLE                              0
#if RTE_UART4_RS485_ENABLE
#define RTE_UART4_RS485_TRANSFER_MODE                       2
#define RTE_UART4_RS485_DE_ASSERTION_TIME_8BIT              (0x01)
#define RTE_UART4_RS485_DE_DEASSERTION_TIME_8BIT            (0x01)
#define RTE_UART4_RS485_DE_TO_RE_TURN_AROUND_TIME_16BIT     (0x01)
#define RTE_UART4_RS485_RE_TO_DE_TURN_AROUND_TIME_16BIT     (0x03)
#endif

// <o> UART4 DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for UART4
// <i> Default: ENABLE
#define RTE_UART4_DMA_ENABLE   0

// <o> UART4 DMA Selection
//    <0=> DMA1(M55-HP)
//    <1=> DMA0
// <i> Defines Select DMA0 for UART4. By default DMA0 will be considered.
// <i> Default: DMA0
#define RTE_UART4_SELECT_DMA0  1

// <o> UART4 DMA IRQ priority <0-255>
// <i> Defines UART4 DMA Interrupt priority
// <i> Default: 0
#define RTE_UART4_DMA_IRQ_PRI  0

#endif //UART4

// </e> UART4 (Universal asynchronous receiver transmitter) [Driver_USART4]


// <e> UART5 (Universal asynchronous receiver transmitter) [Driver_USART5]
// <i> Configuration settings for Driver_USART5 in component ::Drivers:USART
#define RTE_UART5   1

#if RTE_UART5

// <o> UART5 RX FIFO TRIGGER LEVEL
//    <0=>  UART_RX_ONE_CHAR_IN_FIFO
//    <1=>  UART_RX_FIFO_QUARTER_FULL
//    <2=>  UART_RX_FIFO_HALF_FULL
//    <3=>  UART_RX_FIFO_TWO_LESS_FULL
// <i> Defines UART5 receive fifo trigger level.
// <i> Default: UART_RX_ONE_CHAR_IN_FIFO
#define RTE_UART5_RX_TRIG_LVL   0

// <o> UART5 TX FIFO TRIGGER LEVEL
//    <0=>  UART_TX_FIFO_EMPTY
//    <1=>  UART_TX_FIFO_CHAR_2
//    <2=>  UART_TX_FIFO_QUARTER_FULL
//    <3=>  UART_TX_FIFO_HALF_FULL
// <i> Defines UART5 transmit fifo trigger level.
// <i> Default: UART_TX_FIFO_EMPTY
#define RTE_UART5_TX_TRIG_LVL   0

// <o> UART5 IRQ priority <0-255>
// <i> Defines Interrupt priority for UART5.
// <i> Default: 0
#define RTE_UART5_IRQ_PRI       0

// <o> UART5 CLK SOURCE
//    <0=> CLK_38.4MHz
//    <1=> CLK_100MHz
// <i> Defines UART5 clock source.
// <i> Default: CLK_100MHz
#define RTE_UART5_CLK_SOURCE    1

#define RTE_UART5_RS485_ENABLE                              0
#if RTE_UART5_RS485_ENABLE
#define RTE_UART5_RS485_TRANSFER_MODE                       2
#define RTE_UART5_RS485_DE_ASSERTION_TIME_8BIT              (0x01)
#define RTE_UART5_RS485_DE_DEASSERTION_TIME_8BIT            (0x01)
#define RTE_UART5_RS485_DE_TO_RE_TURN_AROUND_TIME_16BIT     (0x01)
#define RTE_UART5_RS485_RE_TO_DE_TURN_AROUND_TIME_16BIT     (0x03)
#endif

// <o> UART5 DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for UART5
// <i> Default: ENABLE
#define RTE_UART5_DMA_ENABLE   0

// <o> UART5 DMA Selection
//    <0=> DMA1(M55-HP)
//    <1=> DMA0
// <i> Defines Select DMA0 for UART5. By default DMA0 will be considered
// <i> Default: DMA0
#define RTE_UART5_SELECT_DMA0  1

// <o> UART5 DMA IRQ priority <0-255>
// <i> Defines UART5 DMA Interrupt priority
// <i> Default: 0
#define RTE_UART5_DMA_IRQ_PRI  0

#endif //UART5

// </e> UART5 (Universal asynchronous receiver transmitter) [Driver_USART5]


// <e> UART6 (Universal asynchronous receiver transmitter) [Driver_USART6]
// <i> Configuration settings for Driver_USART6 in component ::Drivers:USART
#define RTE_UART6   1

#if RTE_UART6

// <o> UART6 RX FIFO TRIGGER LEVEL
//    <0=>  UART_RX_ONE_CHAR_IN_FIFO
//    <1=>  UART_RX_FIFO_QUARTER_FULL
//    <2=>  UART_RX_FIFO_HALF_FULL
//    <3=>  UART_RX_FIFO_TWO_LESS_FULL
// <i> Defines UART6 receive fifo trigger level.
// <i> Default: UART_RX_ONE_CHAR_IN_FIFO
#define RTE_UART6_RX_TRIG_LVL   0

// <o> UART6 TX FIFO TRIGGER LEVEL
//    <0=>  UART_TX_FIFO_EMPTY
//    <1=>  UART_TX_FIFO_CHAR_2
//    <2=>  UART_TX_FIFO_QUARTER_FULL
//    <3=>  UART_TX_FIFO_HALF_FULL
// <i> Defines UART6 transmit fifo trigger level.
// <i> Default: UART_TX_FIFO_EMPTY
#define RTE_UART6_TX_TRIG_LVL   0

// <o> UART6 IRQ priority <0-255>
// <i> Defines Interrupt priority for UART6.
// <i> Default: 0
#define RTE_UART6_IRQ_PRI       0

// <o> UART6 CLK SOURCE
//    <0=> CLK_38.4MHz
//    <1=> CLK_100MHz
// <i> Defines UART6 clock source.
// <i> Default: CLK_100MHz
#define RTE_UART6_CLK_SOURCE    1

#define RTE_UART6_RS485_ENABLE                              0
#if RTE_UART6_RS485_ENABLE
#define RTE_UART6_RS485_TRANSFER_MODE                       2
#define RTE_UART6_RS485_DE_ASSERTION_TIME_8BIT              (0x01)
#define RTE_UART6_RS485_DE_DEASSERTION_TIME_8BIT            (0x01)
#define RTE_UART6_RS485_DE_TO_RE_TURN_AROUND_TIME_16BIT     (0x01)
#define RTE_UART6_RS485_RE_TO_DE_TURN_AROUND_TIME_16BIT     (0x03)
#endif

// <o> UART6 DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for UART6
// <i> Default: ENABLE
#define RTE_UART6_DMA_ENABLE   0

// <o> UART6 DMA Selection
//    <0=> DMA1(M55-HP)
//    <1=> DMA0
// <i> Defines Select DMA0 for UART6. By default DMA0 will be considered
// <i> Default: DMA0
#define RTE_UART6_SELECT_DMA0  1

// <o> UART6 DMA IRQ priority <0-255>
// <i> Defines UART6 DMA Interrupt priority
// <i> Default: 0
#define RTE_UART6_DMA_IRQ_PRI  0

#endif //UART6

// </e> UART6 (Universal asynchronous receiver transmitter) [Driver_USART6]


// <e> UART7 (Universal asynchronous receiver transmitter) [Driver_USART7]
// <i> Configuration settings for Driver_USART7 in component ::Drivers:USART
#define RTE_UART7   1

#if RTE_UART7

// <o> UART7 RX FIFO TRIGGER LEVEL
//    <0=>  UART_RX_ONE_CHAR_IN_FIFO
//    <1=>  UART_RX_FIFO_QUARTER_FULL
//    <2=>  UART_RX_FIFO_HALF_FULL
//    <3=>  UART_RX_FIFO_TWO_LESS_FULL
// <i> Defines UART7 receive fifo trigger level.
// <i> Default: UART_RX_ONE_CHAR_IN_FIFO
#define RTE_UART7_RX_TRIG_LVL   0

// <o> UART7 TX FIFO TRIGGER LEVEL
//    <0=>  UART_TX_FIFO_EMPTY
//    <1=>  UART_TX_FIFO_CHAR_2
//    <2=>  UART_TX_FIFO_QUARTER_FULL
//    <3=>  UART_TX_FIFO_HALF_FULL
// <i> Defines UART7 transmit fifo trigger level.
// <i> Default: UART_TX_FIFO_EMPTY
#define RTE_UART7_TX_TRIG_LVL   0

// <o> UART7 IRQ priority <0-255>
// <i> Defines Interrupt priority for UART7.
// <i> Default: 0
#define RTE_UART7_IRQ_PRI       0

// <o> UART7 CLK SOURCE
//    <0=> CLK_38.4MHz
//    <1=> CLK_100MHz
// <i> Defines UART7 clock source.
// <i> Default: CLK_100MHz
#define RTE_UART7_CLK_SOURCE    1

#define RTE_UART7_RS485_ENABLE                              0
#if RTE_UART7_RS485_ENABLE
#define RTE_UART7_RS485_TRANSFER_MODE                       2
#define RTE_UART7_RS485_DE_ASSERTION_TIME_8BIT              (0x01)
#define RTE_UART7_RS485_DE_DEASSERTION_TIME_8BIT            (0x01)
#define RTE_UART7_RS485_DE_TO_RE_TURN_AROUND_TIME_16BIT     (0x01)
#define RTE_UART7_RS485_RE_TO_DE_TURN_AROUND_TIME_16BIT     (0x03)
#endif

// <o> UART7 DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for UART7
// <i> Default: ENABLE
#define RTE_UART7_DMA_ENABLE   0

// <o> UART7 DMA Selection
//    <0=> DMA1(M55-HP)
//    <1=> DMA0
// <i> Defines Select DMA0 for UART7. By default DMA0 will be considered
// <i> Default: DMA0
#define RTE_UART7_SELECT_DMA0  1

// <o> UART7 DMA IRQ priority <0-255>
// <i> Defines UART7 DMA Interrupt priority
// <i> Default: 0
#define RTE_UART7_DMA_IRQ_PRI  0

#endif //UART7

// </e> UART7 (Universal asynchronous receiver transmitter) [Driver_USART7]


// <e> LPUART (Low-Power Universal asynchronous receiver transmitter) [Driver_USARTLP]
// <i> Configuration settings for Driver_USARTLP in component ::Drivers:USART
#if defined(M55_HE)
#define RTE_LPUART   1
#endif

#if RTE_LPUART

// <o> LPUART RX FIFO TRIGGER LEVEL
//    <0=>  UART_RX_ONE_CHAR_IN_FIFO
//    <1=>  UART_RX_FIFO_QUARTER_FULL
//    <2=>  UART_RX_FIFO_HALF_FULL
//    <3=>  UART_RX_FIFO_TWO_LESS_FULL
// <i> Defines LPUART receive fifo trigger level.
// <i> Default: UART_RX_ONE_CHAR_IN_FIFO
#define RTE_LPUART_RX_TRIG_LVL   0

// <o> LPUART TX FIFO TRIGGER LEVEL
//    <0=>  UART_TX_FIFO_EMPTY
//    <1=>  UART_TX_FIFO_CHAR_2
//    <2=>  UART_TX_FIFO_QUARTER_FULL
//    <3=>  UART_TX_FIFO_HALF_FULL
// <i> Defines LPUART transmit fifo trigger level.
// <i> Default: UART_TX_FIFO_EMPTY
#define RTE_LPUART_TX_TRIG_LVL   0

// <o> LPUART IRQ priority <0-255>
// <i> Defines Interrupt priority for LPUART.
// <i> Default: 0
#define RTE_LPUART_IRQ_PRI       0

// <o> LPUART DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for LPUART
// <i> Default: ENABLE
#define RTE_LPUART_DMA_ENABLE    0

// <o> LPUART DMA Selection
//    <0=> DMA2
//    <1=> DMA0
// <i> Defines Select DMA0 for LPUART. By default DMA0 will be considered
// <i> Default: DMA0
#define RTE_LPUART_SELECT_DMA0   1

// <o> LPUART DMA IRQ priority <0-255>
// <i> Defines LPUART DMA Interrupt priority
// <i> Default: 0
#define RTE_LPUART_DMA_IRQ_PRI   0

#endif //RTE_LPUART

// </e> LPUART (Low-Power Universal asynchronous receiver transmitter) [Driver_USARTLP]
// </h> UART (Universal asynchronous receiver transmitter)


// <h> LPTIMER (Low power timer)
// <e> LPTIMER (Low power timer) [Driver_LPTIMER]
// <i> Configuration settings for Driver_LPTIMER in component ::Drivers:LPTIMER
#define RTE_LPTIMER    1

#if RTE_LPTIMER

// <e> LPTIMER CHANNEL 0
// <i> Configuration settings for Driver_LPTIMER channel 0 in component ::Drivers:LPTIMER
#define RTE_LPTIMER_CHANNEL0   1

// <o> CHANNEL0 IRQ priority <0-255>
// <i> Defines Interrupt priority for LPTIMER channel 0.
// <i> Default: 0
#define RTE_LPTIMER_CHANNEL0_IRQ_PRIORITY   (0)

// <o> CHANNEL0 FREE RUN feature
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines whether free run feature is not enabled/disabled for channel0.
// <i> Default: DISABLED
#define RTE_LPTIMER_CHANNEL0_FREE_RUN_MODE  (0)

// <o> CHANNEL0 clock source
//    <0=> CLK_32768Hz
//    <1=> CLK_128KHz
//    <2=> EXT_CLK
//    <3=> CLK_CASCADE
// <i> Defines clock source for channel0.
// <i> Default: CLK_32768Hz
#define RTE_LPTIMER_CHANNEL0_CLK_SRC        (0)
// </e> LPTIMER CHANNEL 0

// <e> LPTIMER CHANNEL 1
// <i> Configuration settings for Driver_LPTIMER channel 1 in component ::Drivers:LPTIMER
#define RTE_LPTIMER_CHANNEL1   1

// <o> CHANNEL1 IRQ priority <0-255>
// <i> Defines Interrupt priority for LPTIMER channel 1.
// <i> Default: 0
#define RTE_LPTIMER_CHANNEL1_IRQ_PRIORITY   (0)

// <o> CHANNEL1 FREE RUN feature
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines whether free run feature is not enabled/disabled for channel1.
// <i> Default: DISABLED
#define RTE_LPTIMER_CHANNEL1_FREE_RUN_MODE  (0)

// <o> CHANNEL1 clock source
//    <0=> CLK_32768Hz
//    <1=> CLK_128KHz
//    <2=> EXT_CLK
//    <3=> CLK_CASCADE
// <i> Defines clock source for channel1.
// <i> Default: CLK_32768Hz
#define RTE_LPTIMER_CHANNEL1_CLK_SRC        (0)
// </e> LPTIMER CHANNEL 1

// <e> LPTIMER CHANNEL 2
// <i> Configuration settings for Driver_LPTIMER channel 2 in component ::Drivers:LPTIMER
#define RTE_LPTIMER_CHANNEL2   1

// <o> CHANNEL2 IRQ priority <0-255>
// <i> Defines Interrupt priority for LPTIMER channel 2.
// <i> Default: 0
#define RTE_LPTIMER_CHANNEL2_IRQ_PRIORITY   (0)

// <o> CHANNEL2 FREE RUN feature
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines whether free run feature is not enabled/disabled for channel2.
// <i> Default: DISABLED
#define RTE_LPTIMER_CHANNEL2_FREE_RUN_MODE  (0)

// <o> CHANNEL2 clock source
//    <0=> CLK_32768Hz
//    <1=> CLK_128KHz
//    <2=> EXT_CLK
//    <3=> CLK_CASCADE
// <i> Defines clock source for channel2.
// <i> Default: CLK_32768Hz
#define RTE_LPTIMER_CHANNEL2_CLK_SRC        (0)
// </e> LPTIMER CHANNEL 2

// <e> LPTIMER CHANNEL 3
// <i> Configuration settings for Driver_LPTIMER channel 3 in component ::Drivers:LPTIMER
#define RTE_LPTIMER_CHANNEL3   1

// <o> CHANNEL3 IRQ priority <0-255>
// <i> Defines Interrupt priority for LPTIMER channel 3.
// <i> Default: 0
#define RTE_LPTIMER_CHANNEL3_IRQ_PRIORITY   (0)

// <o> CHANNEL3 FREE RUN feature
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines whether free run feature is not enabled/disabled for channel3.
// <i> Default: DISABLED
#define RTE_LPTIMER_CHANNEL3_FREE_RUN_MODE  (0)

// <o> CHANNEL3 clock source
//    <0=> CLK_32768Hz
//    <1=> CLK_128KHz
//    <2=> EXT_CLK
//    <3=> CLK_CASCADE
// <i> Defines clock source for channel3.
// <i> Default: CLK_32768Hz
#define RTE_LPTIMER_CHANNEL3_CLK_SRC        (0)
// </e> LPTIMER CHANNEL 3
#endif /* RTE_LPTIMER */
// </e> LPTIMER (Low power timer) [Driver_LPTIMER]
// </h> LPTIMER (Low power timer)


// <h> RTC (Real time clock)
// <e> RTC0 (Real time clock) [Driver_RTC0]
// <i> Configuration settings for Driver_RTC0 in component ::Drivers:RTC
#define RTE_RTC0                1

#if RTE_RTC0

// <o> RTC0 IRQ priority <0-255>
// <i> Defines Interrupt priority for RTC0.
// <i> Default: 0
#define RTE_RTC0_IRQ_PRI                    0
#endif
// </e> RTC0 (Real time clock) [Driver_RTC0]
// </h> RTC (Real time clock)


// <h> WDT (Watchdog Timer)
// <e> WDT0 (Watchdog Timer) [Driver_WDT0]
// <i> Configuration settings for Driver_WDT0 in component ::Drivers:WDT
#define RTE_WDT0                1
// </e> WDT0 (Watchdog Timer) [Driver_WDT0]
// </h> WDT (Watchdog Timer)


// <h> UTIMER (Universal Timer)
// <e> UTIMER (Universal Timer) [Driver_UTIMER]
// <i> Configuration settings for Driver_UTIMER in component ::Drivers:UTIMER
#define RTE_UTIMER   1

#if RTE_UTIMER

// <e> UTIMER CHANNEL 0
// <i> Configuration settings for UTIMER channel 0 in component ::Drivers:UTIMER
#define RTE_UTIMER_CHANNEL0   1

// <o> CHANNEL0 FIXED BUFFER
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Fixed Buffer features for UT channel 0.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL0_FIXED_BUFFER            0

// <o> CHANNEL0 DMA CONTROL
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines DMA feature control for UT channel 0.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL0_DMA_CONTROL            0

// <o> CHANNEL0 FAULT TYPE
//    <0=> LOW_UNTIL_CYCLE_END
//    <1=> LOW_UNTIL_COUNTER_STOP
// <i> Defines fault type for UT channel 0.
// <i> Default: LOW_UNTIL_COUNTER_STOP
#define RTE_UTIMER_CHANNEL0_FAULT_TYPE            0

// <o> CHANNEL0 TRIANGLE COUNTER BUFFER EVENT
//    <0=> BUF_AT_TROUGH
//    <1=> BUF_AT_TROUGH_N_CREST
// <i> Defines Buffer for triangle counter of UT channel 0.
// <i> Default: BUF_AT_TROUGH
#define RTE_UTIMER_CHANNEL0_BUF_TROUGH_N_CREST      0

// <o> CHANNEL0 DRIVER A
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver A state for UT channel 0.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL0_DRIVER_A                1

// <o> CHANNEL0 DRIVER B
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver B state for UT channel 0.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL0_DRIVER_B                0

// <o> CHANNEL0 DRIVER A START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A start state for UT channel 0.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL0_DRV_A_START_STATE       0

// <o> CHANNEL0 DRIVER A STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A stop state for UT channel 0.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL0_DRV_A_STOP_STATE        0

// <o> CHANNEL0 DRIVER A AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at compare match for UT channel 0.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL0_DRV_A_OP_AT_MATCH_COUNT  0

// <o> CHANNEL0 DRIVER A AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at cycle end for UT channel 0.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL0_DRV_A_OP_AT_CYCLE_END    0

// <o> CHANNEL0 DRIVER B START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B start state for UT channel 0.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL0_DRV_B_START_STATE       0

// <o> CHANNEL0 DRIVER B STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B stop state for UT channel 0.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL0_DRV_B_STOP_STATE        0

// <o> CHANNEL0 DRIVER B AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at compare match for UT channel 0.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL0_DRV_B_OP_AT_MATCH_COUNT  0

// <o> CHANNEL0 DRIVER B AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at cycle end for UT channel 0.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL0_DRV_B_OP_AT_CYCLE_END    0

// <o> CHANNEL0 BUFFER TRANSFER AT CREST
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at crest for UT channel 0.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL0_EVENT_AT_CREST          1

// <o> CHANNEL0 BUFFER TRANSFER AT TROUGH
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at trough for UT channel 0.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL0_EVENT_AT_TROUGH         0

// <o> CHANNEL0 BUFFERING TYPE
//    <0=> SINGLE BUFFER
//    <1=> DOUBLE BUFFER
// <i> Defines buffer type for UT channel 0.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL0_BUFFERING_TYPE          0

// <o> CHANNEL0 BUFFER OPERATION
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer operation for UT channel 0.
// <i> Default: ENABLE
#define RTE_UTIMER_CHANNEL0_BUFFER_OPERATION        0

// <o> CHANNEL0 CAPTURE BUFFER OPERATION FOR DRIVER A
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 0 capture mode.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL0_BUFFERING_TYPE_A        2

// <o> CHANNEL0 CAPTURE BUFFER OPERATION FOR DRIVER B
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 0 capture mode.
// <i> Default: NO BUFFER
#define RTE_UTIMER_CHANNEL0_BUFFERING_TYPE_B        0

// <o> CHANNEL0 CAPTURE A IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 0 capture A.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL0_CAPTURE_A_IRQ_PRIORITY  0

// <o> CHANNEL0 CAPTURE B IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 0 capture B.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL0_CAPTURE_B_IRQ_PRIORITY  0

// <o> CHANNEL0 CAPTURE C IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 0 capture C.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL0_CAPTURE_C_IRQ_PRIORITY  0

// <o> CHANNEL0 CAPTURE D IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 0 capture D.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL0_CAPTURE_D_IRQ_PRIORITY  0

// <o> CHANNEL0 CAPTURE E IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 0 capture E.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL0_CAPTURE_E_IRQ_PRIORITY  0

// <o> CHANNEL0 CAPTURE F IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 0 capture F.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL0_CAPTURE_F_IRQ_PRIORITY  0

// <o> CHANNEL0 OVERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 0 overflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL0_OVER_FLOW_IRQ_PRIORITY  0

// <o> CHANNEL0 UNDERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 0 underflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL0_UNDER_FLOW_IRQ_PRIORITY 0

// <o> CHANNEL0 DUTY CYCLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines duty cycle for UT channel 0.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL0_DUTY_CYCLE_ENABLE       0

// <o> CHANNEL0 DUTY CYCLE VALUE
//    <0=> COMP MATCH
//    <1=> COMP MATCH
//    <2=> 0 PERCENT
//    <3=> 100 PERCENT
// <i> Defines duty cycle value for UT channel 0.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL0_DUTY_CYCLE_VALUE        0
// </e> UTIMER CHANNEL 0


// <e> UTIMER CHANNEL 1
// <i> Configuration settings for UTIMER channel 1 in component ::Drivers:UTIMER
#define RTE_UTIMER_CHANNEL1   1

// <o> CHANNEL1 FIXED BUFFER
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Fixed Buffer features for UT channel 1.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL1_FIXED_BUFFER            0

// <o> CHANNEL1 DMA CONTROL
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines DMA feature control for UT channel 1.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL1_DMA_CONTROL            0

// <o> CHANNEL1 FAULT TYPE
//    <0=> LOW_UNTIL_CYCLE_END
//    <1=> LOW_UNTIL_COUNTER_STOP
// <i> Defines fault type for UT channel 1.
// <i> Default: LOW_UNTIL_COUNTER_STOP
#define RTE_UTIMER_CHANNEL1_FAULT_TYPE            0

// <o> CHANNEL1 TRIANGLE COUNTER BUFFER EVENT
//    <0=> BUF_AT_TROUGH
//    <1=> BUF_AT_TROUGH_N_CREST
// <i> Defines Buffer for triangle counter of UT channel 1.
// <i> Default: BUF_AT_TROUGH
#define RTE_UTIMER_CHANNEL1_BUF_TROUGH_N_CREST      0

// <o> CHANNEL1 DRIVER A
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver A state for UT channel 1.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL1_DRIVER_A                0

// <o> CHANNEL1 DRIVER B
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver B state for UT channel 1.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL1_DRIVER_B                0

// <o> CHANNEL1 DRIVER A START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A start state for UT channel 1.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL1_DRV_A_START_STATE       0

// <o> CHANNEL1 DRIVER A STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A stop state for UT channel 1.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL1_DRV_A_STOP_STATE        0

// <o> CHANNEL1 DRIVER A AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at compare match for UT channel 1.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL1_DRV_A_OP_AT_MATCH_COUNT  0

// <o> CHANNEL1 DRIVER A AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at cycle end for UT channel 1.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL1_DRV_A_OP_AT_CYCLE_END    0

// <o> CHANNEL1 DRIVER B START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B start state for UT channel 1.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL1_DRV_B_START_STATE       0

// <o> CHANNEL1 DRIVER B STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B stop state for UT channel 1.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL1_DRV_B_STOP_STATE        0

// <o> CHANNEL1 DRIVER B AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at compare match for UT channel 1.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL1_DRV_B_OP_AT_MATCH_COUNT  0

// <o> CHANNEL1 DRIVER B AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at cycle end for UT channel 1.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL1_DRV_B_OP_AT_CYCLE_END    0

// <o> CHANNEL1 BUFFER TRANSFER AT CREST
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at crest for UT channel 1.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL1_EVENT_AT_CREST          1

// <o> CHANNEL1 BUFFER TRANSFER AT TROUGH
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at trough for UT channel 1.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL1_EVENT_AT_TROUGH         0

// <o> CHANNEL1 BUFFERING TYPE
//    <0=> SINGLE BUFFER
//    <1=> DOUBLE BUFFER
// <i> Defines buffer type for UT channel 1.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL1_BUFFERING_TYPE          1

// <o> CHANNEL1 BUFFER OPERATION
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer operation for UT channel 1.
// <i> Default: ENABLE
#define RTE_UTIMER_CHANNEL1_BUFFER_OPERATION        1

// <o> CHANNEL1 CAPTURE BUFFER OPERATION FOR DRIVER A
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 1 capture mode.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL1_BUFFERING_TYPE_A        0

// <o> CHANNEL1 CAPTURE BUFFER OPERATION FOR DRIVER B
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 1 capture mode.
// <i> Default: NO BUFFER
#define RTE_UTIMER_CHANNEL1_BUFFERING_TYPE_B        0

// <o> CHANNEL1 CAPTURE A IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 1 capture A.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL1_CAPTURE_A_IRQ_PRIORITY  0

// <o> CHANNEL1 CAPTURE B IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 1 capture B.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL1_CAPTURE_B_IRQ_PRIORITY  0

// <o> CHANNEL1 CAPTURE C IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 1 capture C.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL1_CAPTURE_C_IRQ_PRIORITY  0

// <o> CHANNEL1 CAPTURE D IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 1 capture D.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL1_CAPTURE_D_IRQ_PRIORITY  0

// <o> CHANNEL1 CAPTURE E IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 1 capture E.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL1_CAPTURE_E_IRQ_PRIORITY  0

// <o> CHANNEL1 CAPTURE F IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 1 capture F.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL1_CAPTURE_F_IRQ_PRIORITY  0

// <o> CHANNEL1 OVERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 1 overflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL1_OVER_FLOW_IRQ_PRIORITY  0

// <o> CHANNEL1 UNDERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 1 underflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL1_UNDER_FLOW_IRQ_PRIORITY 0

// <o> CHANNEL1 DUTY CYCLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines duty cycle for UT channel 1.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL1_DUTY_CYCLE_ENABLE       0

// <o> CHANNEL1 DUTY CYCLE VALUE
//    <0=> COMP MATCH
//    <1=> COMP MATCH
//    <2=> 0 PERCENT
//    <3=> 100 PERCENT
// <i> Defines duty cycle value for UT channel 1.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL1_DUTY_CYCLE_VALUE        0
// </e> UTIMER CHANNEL 1


// <e> UTIMER CHANNEL 2
// <i> Configuration settings for UTIMER channel 2 in component ::Drivers:UTIMER
#define RTE_UTIMER_CHANNEL2   1

// <o> CHANNEL2 FIXED BUFFER
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Fixed Buffer features for UT channel 2.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL2_FIXED_BUFFER            0

// <o> CHANNEL2 DMA CONTROL
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines DMA feature control for UT channel 2.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL2_DMA_CONTROL            0

// <o> CHANNEL2 FAULT TYPE
//    <0=> LOW_UNTIL_CYCLE_END
//    <1=> LOW_UNTIL_COUNTER_STOP
// <i> Defines fault type for UT channel 2.
// <i> Default: LOW_UNTIL_COUNTER_STOP
#define RTE_UTIMER_CHANNEL2_FAULT_TYPE            0

// <o> CHANNEL2 TRIANGLE COUNTER BUFFER EVENT
//    <0=> BUF_AT_TROUGH
//    <1=> BUF_AT_TROUGH_N_CREST
// <i> Defines Buffer for triangle counter of UT channel 2.
// <i> Default: BUF_AT_TROUGH
#define RTE_UTIMER_CHANNEL2_BUF_TROUGH_N_CREST      0

// <o> CHANNEL2 DRIVER A
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver A state for UT channel 2.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL2_DRIVER_A                0

// <o> CHANNEL2 DRIVER B
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver B state for UT channel 2.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL2_DRIVER_B                0

// <o> CHANNEL2 DRIVER A START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A start state for UT channel 2.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL2_DRV_A_START_STATE       0

// <o> CHANNEL2 DRIVER A STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A stop state for UT channel 2.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL2_DRV_A_STOP_STATE        0

// <o> CHANNEL2 DRIVER A AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at compare match for UT channel 2.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL2_DRV_A_OP_AT_MATCH_COUNT  0

// <o> CHANNEL2 DRIVER A AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at cycle end for UT channel 2.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL2_DRV_A_OP_AT_CYCLE_END    0

// <o> CHANNEL2 DRIVER B START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B start state for UT channel 2.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL2_DRV_B_START_STATE       0

// <o> CHANNEL2 DRIVER B STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B stop state for UT channel 2.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL2_DRV_B_STOP_STATE        0

// <o> CHANNEL2 DRIVER B AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at compare match for UT channel 2.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL2_DRV_B_OP_AT_MATCH_COUNT  0

// <o> CHANNEL2 DRIVER B AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at cycle end for UT channel 2.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL2_DRV_B_OP_AT_CYCLE_END    0

// <o> CHANNEL2 BUFFER TRANSFER AT CREST
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at crest for UT channel 2.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL2_EVENT_AT_CREST          1

// <o> CHANNEL2 BUFFER TRANSFER AT TROUGH
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at trough for UT channel 2.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL2_EVENT_AT_TROUGH         0

// <o> CHANNEL2 BUFFERING TYPE
//    <0=> SINGLE BUFFER
//    <1=> DOUBLE BUFFER
// <i> Defines buffer type for UT channel 2.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL2_BUFFERING_TYPE          1

// <o> CHANNEL2 BUFFER OPERATION
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer operation for UT channel 2.
// <i> Default: ENABLE
#define RTE_UTIMER_CHANNEL2_BUFFER_OPERATION        1

// <o> CHANNEL2 CAPTURE BUFFER OPERATION FOR DRIVER A
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 2 capture mode.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL2_BUFFERING_TYPE_A        2

// <o> CHANNEL2 CAPTURE BUFFER OPERATION FOR DRIVER B
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 2 capture mode.
// <i> Default: NO BUFFER
#define RTE_UTIMER_CHANNEL2_BUFFERING_TYPE_B        0

// <o> CHANNEL2 CAPTURE A IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 2 capture A.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL2_CAPTURE_A_IRQ_PRIORITY  0

// <o> CHANNEL2 CAPTURE B IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 2 capture B.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL2_CAPTURE_B_IRQ_PRIORITY  0

// <o> CHANNEL2 CAPTURE C IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 2 capture C.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL2_CAPTURE_C_IRQ_PRIORITY  0

// <o> CHANNEL2 CAPTURE D IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 2 capture D.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL2_CAPTURE_D_IRQ_PRIORITY  0

// <o> CHANNEL2 CAPTURE E IRQ priority <0-255>
// <i> Defines Interrupt priority for channel2 capture E.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL2_CAPTURE_E_IRQ_PRIORITY  0

// <o> CHANNEL2 CAPTURE F IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 2 capture F.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL2_CAPTURE_F_IRQ_PRIORITY  0

// <o> CHANNEL2 OVERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 2 overflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL2_OVER_FLOW_IRQ_PRIORITY  0

// <o> CHANNEL2 UNDERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 2 underflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL2_UNDER_FLOW_IRQ_PRIORITY 0

// <o> CHANNEL2 DUTY CYCLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines duty cycle for UT channel 2.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL2_DUTY_CYCLE_ENABLE       0

// <o> CHANNEL2 DUTY CYCLE VALUE
//    <0=> COMP MATCH
//    <1=> COMP MATCH
//    <2=> 0 PERCENT
//    <3=> 100 PERCENT
// <i> Defines duty cycle value for UT channel 2.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL2_DUTY_CYCLE_VALUE        0
// </e> UTIMER CHANNEL 2


// <e> UTIMER CHANNEL 3
// <i> Configuration settings for UTIMER channel 3 in component ::Drivers:UTIMER
#define RTE_UTIMER_CHANNEL3   1

// <o> CHANNEL3 FIXED BUFFER
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Fixed Buffer features for UT channel 3.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL3_FIXED_BUFFER            0

// <o> CHANNEL3 DMA CONTROL
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines DMA feature control for UT channel 0.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL3_DMA_CONTROL            0

// <o> CHANNEL3 FAULT TYPE
//    <0=> LOW_UNTIL_CYCLE_END
//    <1=> LOW_UNTIL_COUNTER_STOP
// <i> Defines fault type for UT channel 3.
// <i> Default: LOW_UNTIL_COUNTER_STOP
#define RTE_UTIMER_CHANNEL3_FAULT_TYPE            0

// <o> CHANNEL3 TRIANGLE COUNTER BUFFER EVENT
//    <0=> BUF_AT_TROUGH
//    <1=> BUF_AT_TROUGH_N_CREST
// <i> Defines Buffer for triangle counter of UT channel 3.
// <i> Default: BUF_AT_TROUGH
#define RTE_UTIMER_CHANNEL3_BUF_TROUGH_N_CREST      0

// <o> CHANNEL3 DRIVER A
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver A state for UT channel 3.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL3_DRIVER_A                0

// <o> CHANNEL3 DRIVER B
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver B state for UT channel 3.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL3_DRIVER_B                0

// <o> CHANNEL3 DRIVER A START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A start state for UT channel 3.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL3_DRV_A_START_STATE       0

// <o> CHANNEL3 DRIVER A STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A stop state for UT channel 3.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL3_DRV_A_STOP_STATE        0

// <o> CHANNEL3 DRIVER A AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at compare match for UT channel 3.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL3_DRV_A_OP_AT_MATCH_COUNT  0

// <o> CHANNEL3 DRIVER A AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at cycle end for UT channel 3.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL3_DRV_A_OP_AT_CYCLE_END    0

// <o> CHANNEL3 DRIVER B START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B start state for UT channel 3.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL3_DRV_B_START_STATE       0

// <o> CHANNEL3 DRIVER B STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B stop state for UT channel 3.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL3_DRV_B_STOP_STATE        0

// <o> CHANNEL3 DRIVER B AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at compare match for UT channel 3.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL3_DRV_B_OP_AT_MATCH_COUNT  0

// <o> CHANNEL3 DRIVER B AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at cycle end for UT channel 3.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL3_DRV_B_OP_AT_CYCLE_END    0

// <o> CHANNEL3 BUFFER TRANSFER AT CREST
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at crest for UT channel 3.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL3_EVENT_AT_CREST          1

// <o> CHANNEL3 BUFFER TRANSFER AT TROUGH
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at trough for UT channel 3.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL3_EVENT_AT_TROUGH         0

// <o> CHANNEL3 BUFFERING TYPE
//    <0=> SINGLE BUFFER
//    <1=> DOUBLE BUFFER
// <i> Defines buffer type for UT channel 3.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL3_BUFFERING_TYPE          0

// <o> CHANNEL3 BUFFER OPERATION
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer operation for UT channel 3.
// <i> Default: ENABLE
#define RTE_UTIMER_CHANNEL3_BUFFER_OPERATION        0

// <o> CHANNEL3 CAPTURE BUFFER OPERATION FOR DRIVER A
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 3 capture mode.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL3_BUFFERING_TYPE_A        0

// <o> CHANNEL3 CAPTURE BUFFER OPERATION FOR DRIVER B
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 3 capture mode.
// <i> Default: NO BUFFER
#define RTE_UTIMER_CHANNEL3_BUFFERING_TYPE_B        0

// <o> CHANNEL3 CAPTURE A IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 3 capture A.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL3_CAPTURE_A_IRQ_PRIORITY  0

// <o> CHANNEL3 CAPTURE B IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 3 capture B.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL3_CAPTURE_B_IRQ_PRIORITY  0

// <o> CHANNEL3 CAPTURE C IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 3 capture C.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL3_CAPTURE_C_IRQ_PRIORITY  0

// <o> CHANNEL3 CAPTURE D IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 3 capture D.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL3_CAPTURE_D_IRQ_PRIORITY  0

// <o> CHANNEL3 CAPTURE E IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 3 capture E.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL3_CAPTURE_E_IRQ_PRIORITY  0

// <o> CHANNEL3 CAPTURE F IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 3 capture F.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL3_CAPTURE_F_IRQ_PRIORITY  0

// <o> CHANNEL3 OVERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 3 overflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL3_OVER_FLOW_IRQ_PRIORITY  0

// <o> CHANNEL3 UNDERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 3 underflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL3_UNDER_FLOW_IRQ_PRIORITY 0

// <o> CHANNEL3 DUTY CYCLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines duty cycle for UT channel 3.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL3_DUTY_CYCLE_ENABLE       0

// <o> CHANNEL3 DUTY CYCLE VALUE
//    <0=> COMP MATCH
//    <1=> COMP MATCH
//    <2=> 0 PERCENT
//    <3=> 100 PERCENT
// <i> Defines duty cycle value for UT channel 3.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL3_DUTY_CYCLE_VALUE        0
// </e> UTIMER CHANNEL 3


// <e> UTIMER CHANNEL 4
// <i> Configuration settings for UTIMER channel 4 in component ::Drivers:UTIMER
#define RTE_UTIMER_CHANNEL4   1

// <o> CHANNEL4 FIXED BUFFER
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Fixed Buffer features for UT channel 4.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL4_FIXED_BUFFER            0

// <o> CHANNEL4 DMA CONTROL
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines DMA feature control for UT channel 4.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL4_DMA_CONTROL            0

// <o> CHANNEL4 FAULT TYPE
//    <0=> LOW_UNTIL_CYCLE_END
//    <1=> LOW_UNTIL_COUNTER_STOP
// <i> Defines fault type for UT channel 4.
// <i> Default: LOW_UNTIL_COUNTER_STOP
#define RTE_UTIMER_CHANNEL4_FAULT_TYPE            0

// <o> CHANNEL4 TRIANGLE COUNTER BUFFER EVENT
//    <0=> BUF_AT_TROUGH
//    <1=> BUF_AT_TROUGH_N_CREST
// <i> Defines Buffer for triangle counter of UT channel 1.
// <i> Default: BUF_AT_TROUGH
#define RTE_UTIMER_CHANNEL4_BUF_TROUGH_N_CREST      0

// <o> CHANNEL4 DRIVER A
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver A state for UT channel 4.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL4_DRIVER_A                1

// <o> CHANNEL4 DRIVER B
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver B state for UT channel 4.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL4_DRIVER_B                0

// <o> CHANNEL4 DRIVER A START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A start state for UT channel 4.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL4_DRV_A_START_STATE       0

// <o> CHANNEL4 DRIVER A STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A stop state for UT channel 4.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL4_DRV_A_STOP_STATE        0

// <o> CHANNEL4 DRIVER A AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at compare match for UT channel 4.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL4_DRV_A_OP_AT_MATCH_COUNT  0

// <o> CHANNEL4 DRIVER A AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at cycle end for UT channel 4.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL4_DRV_A_OP_AT_CYCLE_END    0

// <o> CHANNEL4 DRIVER B START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B start state for UT channel 4.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL4_DRV_B_START_STATE       0

// <o> CHANNEL4 DRIVER B STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B stop state for UT channel 4.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL4_DRV_B_STOP_STATE        0

// <o> CHANNEL4 DRIVER B AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at compare match for UT channel 4.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL4_DRV_B_OP_AT_MATCH_COUNT  0

// <o> CHANNEL4 DRIVER B AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at cycle end for UT channel 4.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL4_DRV_B_OP_AT_CYCLE_END    0

// <o> CHANNEL4 BUFFER TRANSFER AT CREST
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at crest for UT channel 4.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL4_EVENT_AT_CREST          1

// <o> CHANNEL4 BUFFER TRANSFER AT TROUGH
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at trough for UT channel 4.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL4_EVENT_AT_TROUGH         1

// <o> CHANNEL4 BUFFERING TYPE
//    <0=> SINGLE BUFFER
//    <1=> DOUBLE BUFFER
// <i> Defines buffer type for UT channel 4.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL4_BUFFERING_TYPE          1

// <o> CHANNEL4 BUFFER OPERATION
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer operation for UT channel 4.
// <i> Default: ENABLE
#define RTE_UTIMER_CHANNEL4_BUFFER_OPERATION        1

// <o> CHANNEL4 CAPTURE BUFFER OPERATION FOR DRIVER A
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 4 capture mode.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL4_BUFFERING_TYPE_A        2

// <o> CHANNEL4 CAPTURE BUFFER OPERATION FOR DRIVER B
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 4 capture mode.
// <i> Default: NO BUFFER
#define RTE_UTIMER_CHANNEL4_BUFFERING_TYPE_B        2

// <o> CHANNEL4 CAPTURE A IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 4 capture A.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL4_CAPTURE_A_IRQ_PRIORITY  0

// <o> CHANNEL4 CAPTURE B IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 4 capture B.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL4_CAPTURE_B_IRQ_PRIORITY  0

// <o> CHANNEL4 CAPTURE C IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 4 capture C.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL4_CAPTURE_C_IRQ_PRIORITY  0

// <o> CHANNEL4 CAPTURE D IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 4 capture D.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL4_CAPTURE_D_IRQ_PRIORITY  0

// <o> CHANNEL4 CAPTURE E IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 4 capture E.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL4_CAPTURE_E_IRQ_PRIORITY  0

// <o> CHANNEL4 CAPTURE F IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 4 capture F.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL4_CAPTURE_F_IRQ_PRIORITY  0

// <o> CHANNEL4 OVERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 4 overflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL4_OVER_FLOW_IRQ_PRIORITY  0

// <o> CHANNEL4 UNDERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 4 underflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL4_UNDER_FLOW_IRQ_PRIORITY 0

// <o> CHANNEL4 DUTY CYCLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines duty cycle for UT channel 4.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL4_DUTY_CYCLE_ENABLE       0

// <o> CHANNEL4 DUTY CYCLE VALUE
//    <0=> COMP MATCH
//    <1=> COMP MATCH
//    <2=> 0 PERCENT
//    <3=> 100 PERCENT
// <i> Defines duty cycle value for UT channel 4.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL4_DUTY_CYCLE_VALUE        0
// </e> UTIMER CHANNEL 4


// <e> UTIMER CHANNEL 5
// <i> Configuration settings for UTIMER channel 5 in component ::Drivers:UTIMER
#define RTE_UTIMER_CHANNEL5   1

// <o> CHANNEL5 FIXED BUFFER
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Fixed Buffer features for UT channel 5.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL5_FIXED_BUFFER            0

// <o> CHANNEL5 DMA CONTROL
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines DMA feature control for UT channel 4.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL5_DMA_CONTROL            0

// <o> CHANNEL5 FAULT TYPE
//    <0=> LOW_UNTIL_CYCLE_END
//    <1=> LOW_UNTIL_COUNTER_STOP
// <i> Defines fault type for UT channel 5.
// <i> Default: LOW_UNTIL_COUNTER_STOP
#define RTE_UTIMER_CHANNEL5_FAULT_TYPE            0

// <o> CHANNEL5 TRIANGLE COUNTER BUFFER EVENT
//    <0=> BUF_AT_TROUGH
//    <1=> BUF_AT_TROUGH_N_CREST
// <i> Defines Buffer for triangle counter of UT channel 5.
// <i> Default: BUF_AT_TROUGH
#define RTE_UTIMER_CHANNEL5_BUF_TROUGH_N_CREST      0

// <o> CHANNEL5 DRIVER A
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver A state for UT channel 5.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL5_DRIVER_A                1

// <o> CHANNEL5 DRIVER B
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver B state for UT channel 5.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL5_DRIVER_B                0

// <o> CHANNEL5 DRIVER A START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A start state for UT channel 5.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL5_DRV_A_START_STATE       0

// <o> CHANNEL5 DRIVER A STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A stop state for UT channel 5.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL5_DRV_A_STOP_STATE        0

// <o> CHANNEL5 DRIVER A AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at compare match for UT channel 5.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL5_DRV_A_OP_AT_MATCH_COUNT  3

// <o> CHANNEL5 DRIVER A AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at cycle end for UT channel 5.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL5_DRV_A_OP_AT_CYCLE_END    3

// <o> CHANNEL5 DRIVER B START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B start state for UT channel 5.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL5_DRV_B_START_STATE       0

// <o> CHANNEL5 DRIVER B STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B stop state for UT channel 5.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL5_DRV_B_STOP_STATE        0

// <o> CHANNEL5 DRIVER B AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at compare match for UT channel 5.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL5_DRV_B_OP_AT_MATCH_COUNT  0

// <o> CHANNEL5 DRIVER B AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at cycle end for UT channel 5.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL5_DRV_B_OP_AT_CYCLE_END    0

// <o> CHANNEL5 BUFFER TRANSFER AT CREST
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at crest for UT channel 5.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL5_EVENT_AT_CREST          1

// <o> CHANNEL5 BUFFER TRANSFER AT TROUGH
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at trough for UT channel 5.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL5_EVENT_AT_TROUGH         0

// <o> CHANNEL5 BUFFERING TYPE
//    <0=> SINGLE BUFFER
//    <1=> DOUBLE BUFFER
// <i> Defines buffer type for UT channel 5.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL5_BUFFERING_TYPE          1

// <o> CHANNEL5 BUFFER OPERATION
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer operation for UT channel 5.
// <i> Default: ENABLE
#define RTE_UTIMER_CHANNEL5_BUFFER_OPERATION        1

// <o> CHANNEL5 CAPTURE BUFFER OPERATION FOR DRIVER A
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 5 capture mode.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL5_BUFFERING_TYPE_A        2

// <o> CHANNEL5 CAPTURE BUFFER OPERATION FOR DRIVER B
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 5 capture mode.
// <i> Default: NO BUFFER
#define RTE_UTIMER_CHANNEL5_BUFFERING_TYPE_B        0

// <o> CHANNEL5 CAPTURE A IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 5 capture A.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL5_CAPTURE_A_IRQ_PRIORITY  0

// <o> CHANNEL5 CAPTURE B IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 5 capture B.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL5_CAPTURE_B_IRQ_PRIORITY  0

// <o> CHANNEL5 CAPTURE C IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 5 capture C.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL5_CAPTURE_C_IRQ_PRIORITY  0

// <o> CHANNEL5 CAPTURE D IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 5 capture D.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL5_CAPTURE_D_IRQ_PRIORITY  0

// <o> CHANNEL5 CAPTURE E IRQ priority <0-255 >
// <i> Defines Interrupt priority for channel 5 capture E.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL5_CAPTURE_E_IRQ_PRIORITY  0

// <o> CHANNEL5 CAPTURE F IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 5 capture F.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL5_CAPTURE_F_IRQ_PRIORITY  0

// <o> CHANNEL5 OVERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 5 overflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL5_OVER_FLOW_IRQ_PRIORITY  0

// <o> CHANNEL5 UNDERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 5 underflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL5_UNDER_FLOW_IRQ_PRIORITY 0

// <o> CHANNEL5 DUTY CYCLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines duty cycle for UT channel 5.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL5_DUTY_CYCLE_ENABLE       0

// <o> CHANNEL5 DUTY CYCLE VALUE
//    <0=> COMP MATCH
//    <1=> COMP MATCH
//    <2=> 0 PERCENT
//    <3=> 100 PERCENT
// <i> Defines duty cycle value for UT channel 5.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL5_DUTY_CYCLE_VALUE        0
// </e> UTIMER CHANNEL 5


// <e> UTIMER CHANNEL 6
// <i> Configuration settings for UTIMER channel 6 in component ::Drivers:UTIMER
#define RTE_UTIMER_CHANNEL6   1

// <o> CHANNEL6 FIXED BUFFER
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Fixed Buffer features for UT channel 6.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL6_FIXED_BUFFER            0

// <o> CHANNEL6 DMA CONTROL
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines DMA feature control for UT channel 6.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL6_DMA_CONTROL            0

// <o> CHANNEL6 FAULT TYPE
//    <0=> LOW_UNTIL_CYCLE_END
//    <1=> LOW_UNTIL_COUNTER_STOP
// <i> Defines fault type for UT channel 6.
// <i> Default: LOW_UNTIL_COUNTER_STOP
#define RTE_UTIMER_CHANNEL6_FAULT_TYPE            0

// <o> CHANNEL6 TRIANGLE COUNTER BUFFER EVENT
//    <0=> BUF_AT_TROUGH
//    <1=> BUF_AT_TROUGH_N_CREST
// <i> Defines Buffer for triangle counter of UT channel 6.
// <i> Default: BUF_AT_TROUGH
#define RTE_UTIMER_CHANNEL6_BUF_TROUGH_N_CREST      0

// <o> CHANNEL6 DRIVER A
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver A state for UT channel 6.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL6_DRIVER_A                1

// <o> CHANNEL6 DRIVER B
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver B state for UT channel 6.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL6_DRIVER_B                0

// <o> CHANNEL6 DRIVER A START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A start state for UT channel 6.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL6_DRV_A_START_STATE       0

// <o> CHANNEL6 DRIVER A STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A stop state for UT channel 6.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL6_DRV_A_STOP_STATE        0

// <o> CHANNEL6 DRIVER A AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at compare match for UT channel 6.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL6_DRV_A_OP_AT_MATCH_COUNT  3

// <o> CHANNEL6 DRIVER A AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at cycle end for UT channel 6.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL6_DRV_A_OP_AT_CYCLE_END    3

// <o> CHANNEL6 DRIVER B START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B start state for UT channel 6.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL6_DRV_B_START_STATE       0

// <o> CHANNEL6 DRIVER B STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B stop state for UT channel 6.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL6_DRV_B_STOP_STATE        0

// <o> CHANNEL6 DRIVER B AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at compare match for UT channel 6.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL6_DRV_B_OP_AT_MATCH_COUNT  0

// <o> CHANNEL6 DRIVER B AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at cycle end for UT channel 6.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL6_DRV_B_OP_AT_CYCLE_END    0

// <o> CHANNEL6 BUFFER TRANSFER AT CREST
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at crest for UT channel 6.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL6_EVENT_AT_CREST          1

// <o> CHANNEL6 BUFFER TRANSFER AT TROUGH
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at trough for UT channel 6.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL6_EVENT_AT_TROUGH         0

// <o> CHANNEL6 BUFFERING TYPE
//    <0=> SINGLE BUFFER
//    <1=> DOUBLE BUFFER
// <i> Defines buffer type for UT channel 6.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL6_BUFFERING_TYPE          0

// <o> CHANNEL6 BUFFER OPERATION
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer operation for UT channel 6.
// <i> Default: ENABLE
#define RTE_UTIMER_CHANNEL6_BUFFER_OPERATION        0

// <o> CHANNEL6 CAPTURE BUFFER OPERATION FOR DRIVER A
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 6 capture mode.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL6_BUFFERING_TYPE_A        0

// <o> CHANNEL6 CAPTURE BUFFER OPERATION FOR DRIVER B
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 6 capture mode.
// <i> Default: NO BUFFER
#define RTE_UTIMER_CHANNEL6_BUFFERING_TYPE_B        0

// <o> CHANNEL6 CAPTURE A IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 6 capture A.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL6_CAPTURE_A_IRQ_PRIORITY  0

// <o> CHANNEL6 CAPTURE B IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 6 capture B.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL6_CAPTURE_B_IRQ_PRIORITY  0

// <o> CHANNEL6 CAPTURE C IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 6 capture C.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL6_CAPTURE_C_IRQ_PRIORITY  0

// <o> CHANNEL6 CAPTURE D IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 6 capture D.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL6_CAPTURE_D_IRQ_PRIORITY  0

// <o> CHANNEL6 CAPTURE E IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 6 capture E.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL6_CAPTURE_E_IRQ_PRIORITY  0

// <o> CHANNEL6 CAPTURE F IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 6 capture F.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL6_CAPTURE_F_IRQ_PRIORITY  0

// <o> CHANNEL6 OVERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 6 overflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL6_OVER_FLOW_IRQ_PRIORITY  0

// <o> CHANNEL6 UNDERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 6 underflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL6_UNDER_FLOW_IRQ_PRIORITY 0

// <o> CHANNEL6 DUTY CYCLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines duty cycle for UT channel 6.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL6_DUTY_CYCLE_ENABLE       0

// <o> CHANNEL6 DUTY CYCLE VALUE
//    <0=> COMP MATCH
//    <1=> COMP MATCH
//    <2=> 0 PERCENT
//    <3=> 100 PERCENT
// <i> Defines duty cycle value for UT channel 6.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL6_DUTY_CYCLE_VALUE        0
// </e> UTIMER CHANNEL 6


// <e> UTIMER CHANNEL 7
// <i> Configuration settings for UTIMER channel 7 in component ::Drivers:UTIMER
#define RTE_UTIMER_CHANNEL7   1

// <o> CHANNEL7 FIXED BUFFER
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Fixed Buffer features for UT channel 7.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL7_FIXED_BUFFER            0

// <o> CHANNEL7 DMA CONTROL
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines DMA feature control for UT channel 7.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL7_DMA_CONTROL            0

// <o> CHANNEL7 FAULT TYPE
//    <0=> LOW_UNTIL_CYCLE_END
//    <1=> LOW_UNTIL_COUNTER_STOP
// <i> Defines fault type for UT channel 7.
// <i> Default: LOW_UNTIL_COUNTER_STOP
#define RTE_UTIMER_CHANNEL7_FAULT_TYPE            0

// <o> CHANNEL7 TRIANGLE COUNTER BUFFER EVENT
//    <0=> BUF_AT_TROUGH
//    <1=> BUF_AT_TROUGH_N_CREST
// <i> Defines Buffer for triangle counter of UT channel 7.
// <i> Default: BUF_AT_TROUGH
#define RTE_UTIMER_CHANNEL7_BUF_TROUGH_N_CREST      0

// <o> CHANNEL7 DRIVER A
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver A state for UT channel 7.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL7_DRIVER_A                0

// <o> CHANNEL7 DRIVER B
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver B state for UT channel 7.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL7_DRIVER_B                0

// <o> CHANNEL7 DRIVER A START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A start state for UT channel 7.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL7_DRV_A_START_STATE       0

// <o> CHANNEL7 DRIVER A STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A stop state for UT channel 7.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL7_DRV_A_STOP_STATE        0

// <o> CHANNEL7 DRIVER A AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at compare match for UT channel 7.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL7_DRV_A_OP_AT_MATCH_COUNT  0

// <o> CHANNEL7 DRIVER A AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at cycle end for UT channel 7.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL7_DRV_A_OP_AT_CYCLE_END    0

// <o> CHANNEL7 DRIVER B START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B start state for UT channel 7.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL7_DRV_B_START_STATE       0

// <o> CHANNEL7 DRIVER B STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B stop state for UT channel 7.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL7_DRV_B_STOP_STATE        0

// <o> CHANNEL7 DRIVER B AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at compare match for UT channel 7.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL7_DRV_B_OP_AT_MATCH_COUNT  0

// <o> CHANNEL7 DRIVER B AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at cycle end for UT channel 7.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL7_DRV_B_OP_AT_CYCLE_END    0

// <o> CHANNEL7 BUFFER TRANSFER AT CREST
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at crest for UT channel 7.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL7_EVENT_AT_CREST          1

// <o> CHANNEL7 BUFFER TRANSFER AT TROUGH
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at trough for UT channel 7.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL7_EVENT_AT_TROUGH         0

// <o> CHANNEL7 BUFFERING TYPE
//    <0=> SINGLE BUFFER
//    <1=> DOUBLE BUFFER
// <i> Defines buffer type for UT channel 7.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL7_BUFFERING_TYPE          1

// <o> CHANNEL7 BUFFER OPERATION
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer operation for UT channel 7.
// <i> Default: ENABLE
#define RTE_UTIMER_CHANNEL7_BUFFER_OPERATION        1

// <o> CHANNEL7 CAPTURE BUFFER OPERATION FOR DRIVER A
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 7 capture mode.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL7_BUFFERING_TYPE_A        2

// <o> CHANNEL7 CAPTURE BUFFER OPERATION FOR DRIVER B
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 7 capture mode.
// <i> Default: NO BUFFER
#define RTE_UTIMER_CHANNEL7_BUFFERING_TYPE_B        0

// <o> CHANNEL7 CAPTURE A IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 7 capture A.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL7_CAPTURE_A_IRQ_PRIORITY  0

// <o> CHANNEL7 CAPTURE B IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 7 capture B.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL7_CAPTURE_B_IRQ_PRIORITY  0

// <o> CHANNEL7 CAPTURE C IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 7 capture C.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL7_CAPTURE_C_IRQ_PRIORITY  0

// <o> CHANNEL7 CAPTURE D IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 7 capture D.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL7_CAPTURE_D_IRQ_PRIORITY  0

// <o> CHANNEL7 CAPTURE E IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 7 capture E.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL7_CAPTURE_E_IRQ_PRIORITY  0

// <o> CHANNEL7 CAPTURE F IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 7 capture F.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL7_CAPTURE_F_IRQ_PRIORITY  0

// <o> CHANNEL7 OVERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 7 overflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL7_OVER_FLOW_IRQ_PRIORITY  0

// <o> CHANNEL7 UNDERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 7 underflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL7_UNDER_FLOW_IRQ_PRIORITY 0

// <o> CHANNEL7 DUTY CYCLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines duty cycle for UT channel 7.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL7_DUTY_CYCLE_ENABLE       0

// <o> CHANNEL7 DUTY CYCLE VALUE
//    <0=> COMP MATCH
//    <1=> COMP MATCH
//    <2=> 0 PERCENT
//    <3=> 100 PERCENT
// <i> Defines duty cycle value for UT channel 7.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL7_DUTY_CYCLE_VALUE        0
// </e> UTIMER CHANNEL 7


// <e> UTIMER CHANNEL 8
// <i> Configuration settings for UTIMER channel 8 in component ::Drivers:UTIMER
#define RTE_UTIMER_CHANNEL8   1

// <o> CHANNEL8 FIXED BUFFER
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Fixed Buffer features for UT channel 8.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL8_FIXED_BUFFER            0

// <o> CHANNEL8 DMA CONTROL
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines DMA feature control for UT channel 8.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL8_DMA_CONTROL            0

// <o> CHANNEL8 FAULT TYPE
//    <0=> LOW_UNTIL_CYCLE_END
//    <1=> LOW_UNTIL_COUNTER_STOP
// <i> Defines fault type for UT channel 8.
// <i> Default: LOW_UNTIL_COUNTER_STOP
#define RTE_UTIMER_CHANNEL8_FAULT_TYPE            0

// <o> CHANNEL8 TRIANGLE COUNTER BUFFER EVENT
//    <0=> BUF_AT_TROUGH
//    <1=> BUF_AT_TROUGH_N_CREST
// <i> Defines Buffer for triangle counter of UT channel 8.
// <i> Default: BUF_AT_TROUGH
#define RTE_UTIMER_CHANNEL8_BUF_TROUGH_N_CREST      0

// <o> CHANNEL8 DRIVER A
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver A state for UT channel 8.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL8_DRIVER_A                1

// <o> CHANNEL8 DRIVER B
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver B state for UT channel 8.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL8_DRIVER_B                0

// <o> CHANNEL8 DRIVER A START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A start state for UT channel 8.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL8_DRV_A_START_STATE       0

// <o> CHANNEL8 DRIVER A STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A stop state for UT channel 8.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL8_DRV_A_STOP_STATE        0

// <o> CHANNEL8 DRIVER A AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at compare match for UT channel 8.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL8_DRV_A_OP_AT_MATCH_COUNT  1

// <o> CHANNEL8 DRIVER A AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at cycle end for UT channel 8.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL8_DRV_A_OP_AT_CYCLE_END    2

// <o> CHANNEL8 DRIVER B START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B start state for UT channel 8.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL8_DRV_B_START_STATE       0

// <o> CHANNEL8 DRIVER B STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B stop state for UT channel 8.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL8_DRV_B_STOP_STATE        0

// <o> CHANNEL8 DRIVER B AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at compare match for UT channel 8.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL8_DRV_B_OP_AT_MATCH_COUNT  0

// <o> CHANNEL8 DRIVER B AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at cycle end for UT channel 8.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL8_DRV_B_OP_AT_CYCLE_END    0

// <o> CHANNEL8 BUFFER TRANSFER AT CREST
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at crest for UT channel 8.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL8_EVENT_AT_CREST          1

// <o> CHANNEL8 BUFFER TRANSFER AT TROUGH
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at trough for UT channel 8.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL8_EVENT_AT_TROUGH         0

// <o> CHANNEL8 BUFFERING TYPE
//    <0=> SINGLE BUFFER
//    <1=> DOUBLE BUFFER
// <i> Defines buffer type for UT channel 8.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL8_BUFFERING_TYPE          0

// <o> CHANNEL8 BUFFER OPERATION
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer operation for UT channel 8.
// <i> Default: ENABLE
#define RTE_UTIMER_CHANNEL8_BUFFER_OPERATION        0

// <o> CHANNEL8 CAPTURE BUFFER OPERATION FOR DRIVER A
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 8 capture mode.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL8_BUFFERING_TYPE_A        2

// <o> CHANNEL8 CAPTURE BUFFER OPERATION FOR DRIVER B
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 8 capture mode.
// <i> Default: NO BUFFER
#define RTE_UTIMER_CHANNEL8_BUFFERING_TYPE_B        0

// <o> CHANNEL8 CAPTURE A IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 8 capture A.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL8_CAPTURE_A_IRQ_PRIORITY  0

// <o> CHANNEL8 CAPTURE B IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 8 capture B.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL8_CAPTURE_B_IRQ_PRIORITY  0

// <o> CHANNEL8 CAPTURE C IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 8 capture C.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL8_CAPTURE_C_IRQ_PRIORITY  0

// <o> CHANNEL8 CAPTURE D IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 8 capture D.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL8_CAPTURE_D_IRQ_PRIORITY  0

// <o> CHANNEL8 CAPTURE E IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 8 capture E.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL8_CAPTURE_E_IRQ_PRIORITY  0

// <o> CHANNEL8 CAPTURE F IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 8 capture F.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL8_CAPTURE_F_IRQ_PRIORITY  0

// <o> CHANNEL8 OVERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 8 overflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL8_OVER_FLOW_IRQ_PRIORITY  0

// <o> CHANNEL8 UNDERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 8 underflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL8_UNDER_FLOW_IRQ_PRIORITY 0

// <o> CHANNEL8 DUTY CYCLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines duty cycle for UT channel 8.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL8_DUTY_CYCLE_ENABLE       0

// <o> CHANNEL8 DUTY CYCLE VALUE
//    <0=> COMP MATCH
//    <1=> COMP MATCH
//    <2=> 0 PERCENT
//    <3=> 100 PERCENT
// <i> Defines duty cycle value for UT channel 8.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL8_DUTY_CYCLE_VALUE        0
// </e> UTIMER CHANNEL 8


// <e> UTIMER CHANNEL 9
// <i> Configuration settings for UTIMER channel 9 in component ::Drivers:UTIMER
#define RTE_UTIMER_CHANNEL9   1

// <o> CHANNEL9 FIXED BUFFER
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Fixed Buffer features for UT channel 9.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL9_FIXED_BUFFER            0

// <o> CHANNEL9 DMA CONTROL
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines DMA feature control for UT channel 9.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL9_DMA_CONTROL            0

// <o> CHANNEL9 FAULT TYPE
//    <0=> LOW_UNTIL_CYCLE_END
//    <1=> LOW_UNTIL_COUNTER_STOP
// <i> Defines fault type for UT channel 9.
// <i> Default: LOW_UNTIL_COUNTER_STOP
#define RTE_UTIMER_CHANNEL9_FAULT_TYPE            0

// <o> CHANNEL9 TRIANGLE COUNTER BUFFER EVENT
//    <0=> BUF_AT_TROUGH
//    <1=> BUF_AT_TROUGH_N_CREST
// <i> Defines Buffer for triangle counter of UT channel 9.
// <i> Default: BUF_AT_TROUGH
#define RTE_UTIMER_CHANNEL9_BUF_TROUGH_N_CREST      0

// <o> CHANNEL9 DRIVER A
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver A state for UT channel 9.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL9_DRIVER_A                0

// <o> CHANNEL9 DRIVER B
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver B state for UT channel 9.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL9_DRIVER_B                1

// <o> CHANNEL9 DRIVER A START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A start state for UT channel 9.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL9_DRV_A_START_STATE       0

// <o> CHANNEL9 DRIVER A STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A stop state for UT channel 9.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL9_DRV_A_STOP_STATE        0

// <o> CHANNEL9 DRIVER A AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at compare match for UT channel 9.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL9_DRV_A_OP_AT_MATCH_COUNT  0

// <o> CHANNEL9 DRIVER A AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at cycle end for UT channel 9.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL9_DRV_A_OP_AT_CYCLE_END    0

// <o> CHANNEL9 DRIVER B START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B start state for UT channel 9.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL9_DRV_B_START_STATE       0

// <o> CHANNEL9 DRIVER B STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B stop state for UT channel 9.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL9_DRV_B_STOP_STATE        0

// <o> CHANNEL9 DRIVER B AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at compare match for UT channel 9.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL9_DRV_B_OP_AT_MATCH_COUNT  1

// <o> CHANNEL9 DRIVER B AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at cycle end for UT channel 9.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL9_DRV_B_OP_AT_CYCLE_END    2

// <o> CHANNEL9 BUFFER TRANSFER AT CREST
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at crest for UT channel 9.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL9_EVENT_AT_CREST          1

// <o> CHANNEL9 BUFFER TRANSFER AT TROUGH
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at trough for UT channel 9.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL9_EVENT_AT_TROUGH         0

// <o> CHANNEL9 BUFFERING TYPE
//    <0=> SINGLE BUFFER
//    <1=> DOUBLE BUFFER
// <i> Defines buffer type for UT channel 9.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL9_BUFFERING_TYPE          0

// <o> CHANNEL9 BUFFER OPERATION
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer operation for UT channel 9.
// <i> Default: ENABLE
#define RTE_UTIMER_CHANNEL9_BUFFER_OPERATION        0

// <o> CHANNEL9 CAPTURE BUFFER OPERATION FOR DRIVER A
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 9 capture mode.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL9_BUFFERING_TYPE_A        0

// <o> CHANNEL9 CAPTURE BUFFER OPERATION FOR DRIVER B
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 9 capture mode.
// <i> Default: NO BUFFER
#define RTE_UTIMER_CHANNEL9_BUFFERING_TYPE_B        0

// <o> CHANNEL9 CAPTURE A IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 9 capture A.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL9_CAPTURE_A_IRQ_PRIORITY  0

// <o> CHANNEL9 CAPTURE B IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 9 capture B.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL9_CAPTURE_B_IRQ_PRIORITY  0

// <o> CHANNEL9 CAPTURE C IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 9 capture C.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL9_CAPTURE_C_IRQ_PRIORITY  0

// <o> CHANNEL9 CAPTURE D IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 9 capture D.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL9_CAPTURE_D_IRQ_PRIORITY  0

// <o> CHANNEL9 CAPTURE E IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 9 capture E.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL9_CAPTURE_E_IRQ_PRIORITY  0

// <o> CHANNEL9 CAPTURE F IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 9 capture F.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL9_CAPTURE_F_IRQ_PRIORITY  0

// <o> CHANNEL9 OVERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 9 overflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL9_OVER_FLOW_IRQ_PRIORITY  0

// <o> CHANNEL9 UNDERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 9 underflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL9_UNDER_FLOW_IRQ_PRIORITY 0

// <o> CHANNEL9 DUTY CYCLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines duty cycle for UT channel 9.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL9_DUTY_CYCLE_ENABLE       0

// <o> CHANNEL9 DUTY CYCLE VALUE
//    <0=> COMP MATCH
//    <1=> COMP MATCH
//    <2=> 0 PERCENT
//    <3=> 100 PERCENT
// <i> Defines duty cycle value for UT channel 9.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL9_DUTY_CYCLE_VALUE        0
// </e> UTIMER CHANNEL 9


// <e> UTIMER CHANNEL 10
// <i> Configuration settings for UTIMER channel 10 in component ::Drivers:UTIMER
#define RTE_UTIMER_CHANNEL10   1

// <o> CHANNEL10 FIXED BUFFER
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Fixed Buffer features for UT channel 10.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL10_FIXED_BUFFER            0

// <o> CHANNEL10 DMA CONTROL
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines DMA feature control for UT channel 10.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL10_DMA_CONTROL            0

// <o> CHANNEL10 FAULT TYPE
//    <0=> LOW_UNTIL_CYCLE_END
//    <1=> LOW_UNTIL_COUNTER_STOP
// <i> Defines fault type for UT channel 10.
// <i> Default: LOW_UNTIL_COUNTER_STOP
#define RTE_UTIMER_CHANNEL10_FAULT_TYPE            0

// <o> CHANNEL10 TRIANGLE COUNTER BUFFER EVENT
//    <0=> BUF_AT_TROUGH
//    <1=> BUF_AT_TROUGH_N_CREST
// <i> Defines Buffer for triangle counter of UT channel 10.
// <i> Default: BUF_AT_TROUGH
#define RTE_UTIMER_CHANNEL10_BUF_TROUGH_N_CREST      0

// <o> CHANNEL10 DRIVER A
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver A state for UT channel 10.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL10_DRIVER_A                1

// <o> CHANNEL10 DRIVER B
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver B state for UT channel 10.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL10_DRIVER_B                0

// <o> CHANNEL10 DRIVER A START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A start state for UT channel 10.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL10_DRV_A_START_STATE       0

// <o> CHANNEL10 DRIVER A STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A stop state for UT channel 10.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL10_DRV_A_STOP_STATE        0

// <o> CHANNEL10 DRIVER A AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at compare match for UT channel 10.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL10_DRV_A_OP_AT_MATCH_COUNT  1

// <o> CHANNEL10 DRIVER A AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at cycle end for UT channel 10.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL10_DRV_A_OP_AT_CYCLE_END    2

// <o> CHANNEL10 DRIVER B START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B start state for UT channel 10.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL10_DRV_B_START_STATE       0

// <o> CHANNEL10 DRIVER B STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B stop state for UT channel 10.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL10_DRV_B_STOP_STATE        0

// <o> CHANNEL10 DRIVER B AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at compare match for UT channel 10.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL10_DRV_B_OP_AT_MATCH_COUNT  0

// <o> CHANNEL10 DRIVER B AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at cycle end for UT channel 10.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL10_DRV_B_OP_AT_CYCLE_END    0

// <o> CHANNEL10 BUFFER TRANSFER AT CREST
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at crest for UT channel 10.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL10_EVENT_AT_CREST          1

// <o> CHANNEL10 BUFFER TRANSFER AT TROUGH
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at trough for UT channel 10.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL10_EVENT_AT_TROUGH         0

// <o> CHANNEL10 BUFFERING TYPE
//    <0=> SINGLE BUFFER
//    <1=> DOUBLE BUFFER
// <i> Defines buffer type for UT channel 10.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL10_BUFFERING_TYPE          0

// <o> CHANNEL10 BUFFER OPERATION
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer operation for UT channel 10.
// <i> Default: ENABLE
#define RTE_UTIMER_CHANNEL10_BUFFER_OPERATION        0

// <o> CHANNEL10 CAPTURE BUFFER OPERATION FOR DRIVER A
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 10 capture mode.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL10_BUFFERING_TYPE_A        2

// <o> CHANNEL10 CAPTURE BUFFER OPERATION FOR DRIVER B
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 10 capture mode.
// <i> Default: NO BUFFER
#define RTE_UTIMER_CHANNEL10_BUFFERING_TYPE_B        0

// <o> CHANNEL10 CAPTURE A IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 10 capture A.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL10_CAPTURE_A_IRQ_PRIORITY  0

// <o> CHANNEL10 CAPTURE B IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 10 capture B.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL10_CAPTURE_B_IRQ_PRIORITY  0

// <o> CHANNEL10 CAPTURE C IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 10 capture C.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL10_CAPTURE_C_IRQ_PRIORITY  0

// <o> CHANNEL10 CAPTURE D IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 10 capture D.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL10_CAPTURE_D_IRQ_PRIORITY  0

// <o> CHANNEL10 CAPTURE E IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 10 capture E.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL10_CAPTURE_E_IRQ_PRIORITY  0

// <o> CHANNEL10 CAPTURE F IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 10 capture F.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL10_CAPTURE_F_IRQ_PRIORITY  0

// <o> CHANNEL10 OVERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 10 overflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL10_OVER_FLOW_IRQ_PRIORITY  0

// <o> CHANNEL10 UNDERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 10 underflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL10_UNDER_FLOW_IRQ_PRIORITY 0

// <o> CHANNEL10 DUTY CYCLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines duty cycle for UT channel 10.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL10_DUTY_CYCLE_ENABLE       0

// <o> CHANNEL10 DUTY CYCLE VALUE
//    <0=> COMP MATCH
//    <1=> COMP MATCH
//    <2=> 0 PERCENT
//    <3=> 100 PERCENT
// <i> Defines duty cycle value for UT channel 10.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL10_DUTY_CYCLE_VALUE        0
// </e> UTIMER CHANNEL 10


// <e> UTIMER CHANNEL 11
// <i> Configuration settings for UTIMER channel 11 in component ::Drivers:UTIMER
#define RTE_UTIMER_CHANNEL11   1

// <o> CHANNEL11 FIXED BUFFER
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Fixed Buffer features for UT channel 11.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL11_FIXED_BUFFER            0

// <o> CHANNEL11 DMA CONTROL
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines DMA feature control for UT channel 11.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL11_DMA_CONTROL            0

// <o> CHANNEL11 FAULT TYPE
//    <0=> LOW_UNTIL_CYCLE_END
//    <1=> LOW_UNTIL_COUNTER_STOP
// <i> Defines fault type for UT channel 11.
// <i> Default: LOW_UNTIL_COUNTER_STOP
#define RTE_UTIMER_CHANNEL11_FAULT_TYPE            0

// <o> CHANNEL11 TRIANGLE COUNTER BUFFER EVENT
//    <0=> BUF_AT_TROUGH
//    <1=> BUF_AT_TROUGH_N_CREST
// <i> Defines Buffer for triangle counter of UT channel 11.
// <i> Default: BUF_AT_TROUGH
#define RTE_UTIMER_CHANNEL11_BUF_TROUGH_N_CREST      0

// <o> CHANNEL11 DRIVER A
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver A state for UT channel 11.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL11_DRIVER_A                0

// <o> CHANNEL11 DRIVER B
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver B state for UT channel 11.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL11_DRIVER_B                0

// <o> CHANNEL11 DRIVER A START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A start state for UT channel 11.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL11_DRV_A_START_STATE       0

// <o> CHANNEL11 DRIVER A STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A stop state for UT channel 11.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL11_DRV_A_STOP_STATE        0

// <o> CHANNEL11 DRIVER A AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at compare match for UT channel 11.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL11_DRV_A_OP_AT_MATCH_COUNT  0

// <o> CHANNEL11 DRIVER A AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at cycle end for UT channel 11.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL11_DRV_A_OP_AT_CYCLE_END    0

// <o> CHANNEL11 DRIVER B START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B start state for UT channel 11.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL11_DRV_B_START_STATE       0

// <o> CHANNEL11 DRIVER B STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B stop state for UT channel 11.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL11_DRV_B_STOP_STATE        0

// <o> CHANNEL11 DRIVER B AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at compare match for UT channel 11.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL11_DRV_B_OP_AT_MATCH_COUNT  0

// <o> CHANNEL11 DRIVER B AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at cycle end for UT channel 11.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL11_DRV_B_OP_AT_CYCLE_END    0

// <o> CHANNEL11 BUFFER TRANSFER AT CREST
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at crest for UT channel 11.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL11_EVENT_AT_CREST          1

// <o> CHANNEL11 BUFFER TRANSFER AT TROUGH
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at trough for UT channel 11.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL11_EVENT_AT_TROUGH         0

// <o> CHANNEL11 BUFFERING TYPE
//    <0=> SINGLE BUFFER
//    <1=> DOUBLE BUFFER
// <i> Defines buffer type for UT channel 11.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL11_BUFFERING_TYPE          1

// <o> CHANNEL11 BUFFER OPERATION
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer operation for UT channel 11.
// <i> Default: ENABLE
#define RTE_UTIMER_CHANNEL11_BUFFER_OPERATION        1

// <o> CHANNEL11 CAPTURE BUFFER OPERATION FOR DRIVER A
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 11 capture mode.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL11_BUFFERING_TYPE_A        2

// <o> CHANNEL11 CAPTURE BUFFER OPERATION FOR DRIVER B
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 11 capture mode.
// <i> Default: NO BUFFER
#define RTE_UTIMER_CHANNEL11_BUFFERING_TYPE_B        0

// <o> CHANNEL11 CAPTURE A IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 11 capture A.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL11_CAPTURE_A_IRQ_PRIORITY  0

// <o> CHANNEL11 CAPTURE B IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 11 capture B.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL11_CAPTURE_B_IRQ_PRIORITY  0

// <o> CHANNEL11 CAPTURE C IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 11 capture C.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL11_CAPTURE_C_IRQ_PRIORITY  0

// <o> CHANNEL11 CAPTURE D IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 11 capture D.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL11_CAPTURE_D_IRQ_PRIORITY  0

// <o> CHANNEL11 CAPTURE E IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 11 capture E.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL11_CAPTURE_E_IRQ_PRIORITY  0

// <o> CHANNEL11 CAPTURE F IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 11 capture F.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL11_CAPTURE_F_IRQ_PRIORITY  0

// <o> CHANNEL11 OVERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 11 overflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL11_OVER_FLOW_IRQ_PRIORITY  0

// <o> CHANNEL11 UNDERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 11 underflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL11_UNDER_FLOW_IRQ_PRIORITY  0

// <o> CHANNEL11 DUTY CYCLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines duty cycle for UT channel 11.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL11_DUTY_CYCLE_ENABLE       0
// </e> UTIMER CHANNEL 11


// <e> UTIMER CHANNEL 12
// <i> Configuration settings for UTIMER channel 12 in component ::Drivers:UTIMER
#define RTE_UTIMER_CHANNEL12   1

// <o> CHANNEL11 DUTY CYCLE VALUE
//    <0=> COMP MATCH
//    <1=> COMP MATCH
//    <2=> 0 PERCENT
//    <3=> 100 PERCENT
// <i> Defines duty cycle value for UT channel 11.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL11_DUTY_CYCLE_VALUE        0

// <o> CHANNEL12 FIXED BUFFER
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Fixed Buffer features for UT channel 12.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL12_FIXED_BUFFER            0

// <o> CHANNEL12 DMA CONTROL
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines DMA feature control for UT channel 12.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL12_DMA_CONTROL            0

// <o> CHANNEL12 FAULT TYPE
//    <0=> LOW_UNTIL_CYCLE_END
//    <1=> LOW_UNTIL_COUNTER_STOP
// <i> Defines fault type for UT channel 12.
// <i> Default: LOW_UNTIL_COUNTER_STOP
#define RTE_UTIMER_CHANNEL12_FAULT_TYPE            0

// <o> CHANNEL12 TRIANGLE COUNTER BUFFER EVENT
//    <0=> BUF_AT_TROUGH
//    <1=> BUF_AT_TROUGH_N_CREST
// <i> Defines Buffer for triangle counter of UT channel 12.
// <i> Default: BUF_AT_TROUGH
#define RTE_UTIMER_CHANNEL12_BUF_TROUGH_N_CREST      0

// <o> CHANNEL12 DRIVER A
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver A state for UT channel 12.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL12_DRIVER_A                1

// <o> CHANNEL12 DRIVER B
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver B state for UT channel 12.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL12_DRIVER_B                0

// <o> CHANNEL12 DRIVER A START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A start state for UT channel 12.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL12_DRV_A_START_STATE       0

// <o> CHANNEL12 DRIVER A STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A stop state for UT channel 12.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL12_DRV_A_STOP_STATE        0

// <o> CHANNEL12 DRIVER A AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at compare match for UT channel 12.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL12_DRV_A_OP_AT_MATCH_COUNT  0

// <o> CHANNEL12 DRIVER A AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at cycle end for UT channel 12.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL12_DRV_A_OP_AT_CYCLE_END    0

// <o> CHANNEL12 DRIVER B START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B start state for UT channel 12.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL12_DRV_B_START_STATE       0

// <o> CHANNEL12 DRIVER B STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B stop state for UT channel 12.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL12_DRV_B_STOP_STATE        0

// <o> CHANNEL12 DRIVER B AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at compare match for UT channel 12.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL12_DRV_B_OP_AT_MATCH_COUNT  0

// <o> CHANNEL12 DRIVER B AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at cycle end for UT channel 12.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL12_DRV_B_OP_AT_CYCLE_END    0

// <o> CHANNEL12 BUFFER TRANSFER AT CREST
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at crest for UT channel 12.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL12_EVENT_AT_CREST          1

// <o> CHANNEL12 BUFFER TRANSFER AT TROUGH
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at trough for UT channel 12.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL12_EVENT_AT_TROUGH         0

// <o> CHANNEL12 BUFFERING TYPE
//    <0=> SINGLE BUFFER
//    <1=> DOUBLE BUFFER
// <i> Defines buffer type for UT channel 12.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL12_BUFFERING_TYPE          0

// <o> CHANNEL12 BUFFER OPERATION
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer operation for UT channel 12.
// <i> Default: ENABLE
#define RTE_UTIMER_CHANNEL12_BUFFER_OPERATION        0

// <o> CHANNEL12 CAPTURE BUFFER OPERATION FOR DRIVER A
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 12 capture mode.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL12_BUFFERING_TYPE_A        0

// <o> CHANNEL12 CAPTURE BUFFER OPERATION FOR DRIVER B
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 12 capture mode.
// <i> Default: NO BUFFER
#define RTE_UTIMER_CHANNEL12_BUFFERING_TYPE_B        0

// <o> CHANNEL12 CAPTURE A IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 12 capture A.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL12_CAPTURE_A_IRQ_PRIORITY  0

// <o> CHANNEL12 CAPTURE B IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 12 capture B.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL12_CAPTURE_B_IRQ_PRIORITY  0

// <o> CHANNEL12 CAPTURE C IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 12 capture C.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL12_CAPTURE_C_IRQ_PRIORITY  0

// <o> CHANNEL12 CAPTURE D IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 12 capture D.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL12_CAPTURE_D_IRQ_PRIORITY  0

// <o> CHANNEL12 CAPTURE E IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 12 capture E.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL12_CAPTURE_E_IRQ_PRIORITY  0

// <o> CHANNEL12 CAPTURE F IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 12 capture F.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL12_CAPTURE_F_IRQ_PRIORITY  0

// <o> CHANNEL12 OVERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 12 overflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL12_OVER_FLOW_IRQ_PRIORITY  0

// <o> CHANNEL12 UNDERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 12 underflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL12_UNDER_FLOW_IRQ_PRIORITY 0

// <o> CHANNEL12 DUTY CYCLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines duty cycle for UT channel 12.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL12_DUTY_CYCLE_ENABLE       0

// <o> CHANNEL12 DUTY CYCLE VALUE
//    <0=> COMP MATCH
//    <1=> COMP MATCH
//    <2=> 0 PERCENT
//    <3=> 100 PERCENT
// <i> Defines duty cycle value for UT channel 12.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL12_DUTY_CYCLE_VALUE        0
// </e> UTIMER CHANNEL 12


// <e> UTIMER CHANNEL 13
// <i> Configuration settings for UTIMER channel 13 in component ::Drivers:UTIMER
#define RTE_UTIMER_CHANNEL13   1

// <o> CHANNEL13 FIXED BUFFER
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Fixed Buffer features for UT channel 13.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL13_FIXED_BUFFER            0

// <o> CHANNEL13 DMA CONTROL
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines DMA feature control for UT channel 13.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL13_DMA_CONTROL            0

// <o> CHANNEL13 FAULT TYPE
//    <0=> LOW_UNTIL_CYCLE_END
//    <1=> LOW_UNTIL_COUNTER_STOP
// <i> Defines fault type for UT channel 13.
// <i> Default: LOW_UNTIL_COUNTER_STOP
#define RTE_UTIMER_CHANNEL13_FAULT_TYPE            0

// <o> CHANNEL13 TRIANGLE COUNTER BUFFER EVENT
//    <0=> BUF_AT_TROUGH
//    <1=> BUF_AT_TROUGH_N_CREST
// <i> Defines Buffer for triangle counter of UT channel 13.
// <i> Default: BUF_AT_TROUGH
#define RTE_UTIMER_CHANNEL13_BUF_TROUGH_N_CREST      0

// <o> CHANNEL13 DRIVER A
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver A state for UT channel 13.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL13_DRIVER_A                1

// <o> CHANNEL13 DRIVER B
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver B state for UT channel 13.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL13_DRIVER_B                0

// <o> CHANNEL13 DRIVER A START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A start state for UT channel 13.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL13_DRV_A_START_STATE       0

// <o> CHANNEL13 DRIVER A STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A stop state for UT channel 13.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL13_DRV_A_STOP_STATE        0

// <o> CHANNEL13 DRIVER A AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at compare match for UT channel 13.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL13_DRV_A_OP_AT_MATCH_COUNT  0

// <o> CHANNEL13 DRIVER A AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at cycle end for UT channel 13.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL13_DRV_A_OP_AT_CYCLE_END    0

// <o> CHANNEL13 DRIVER B START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B start state for UT channel 13.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL13_DRV_B_START_STATE       0

// <o> CHANNEL13 DRIVER B STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B stop state for UT channel 13.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL13_DRV_B_STOP_STATE        0

// <o> CHANNEL13 DRIVER B AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at compare match for UT channel 13.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL13_DRV_B_OP_AT_MATCH_COUNT  0

// <o> CHANNEL13 DRIVER B AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at cycle end for UT channel 13.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL13_DRV_B_OP_AT_CYCLE_END    0

// <o> CHANNEL13 BUFFER TRANSFER AT CREST
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at crest for UT channel 13.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL13_EVENT_AT_CREST          1

// <o> CHANNEL13 BUFFER TRANSFER AT TROUGH
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at trough for UT channel 13.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL13_EVENT_AT_TROUGH         0

// <o> CHANNEL13 BUFFERING TYPE
//    <0=> SINGLE BUFFER
//    <1=> DOUBLE BUFFER
// <i> Defines buffer type for UT channel 13.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL13_BUFFERING_TYPE          0

// <o> CHANNEL13 BUFFER OPERATION
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer operation for UT channel 13.
// <i> Default: ENABLE
#define RTE_UTIMER_CHANNEL13_BUFFER_OPERATION        0

// <o> CHANNEL13 CAPTURE BUFFER OPERATION FOR DRIVER A
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 13 capture mode.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL13_BUFFERING_TYPE_A        0

// <o> CHANNEL13 CAPTURE BUFFER OPERATION FOR DRIVER B
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 13 capture mode.
// <i> Default: NO BUFFER
#define RTE_UTIMER_CHANNEL13_BUFFERING_TYPE_B        0

// <o> CHANNEL13 CAPTURE A IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 13 capture A.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL13_CAPTURE_A_IRQ_PRIORITY  0

// <o> CHANNEL13 CAPTURE B IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 13 capture B.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL13_CAPTURE_B_IRQ_PRIORITY  0

// <o> CHANNEL13 CAPTURE C IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 13 capture C.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL13_CAPTURE_C_IRQ_PRIORITY  0

// <o> CHANNEL13 CAPTURE D IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 13 capture D.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL13_CAPTURE_D_IRQ_PRIORITY  0

// <o> CHANNEL13 CAPTURE E IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 13 capture E.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL13_CAPTURE_E_IRQ_PRIORITY  0

// <o> CHANNEL13 CAPTURE F IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 13 capture F.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL13_CAPTURE_F_IRQ_PRIORITY  0

// <o> CHANNEL13 OVERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 13 overflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL13_OVER_FLOW_IRQ_PRIORITY  0

// <o> CHANNEL13 UNDERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 13 underflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL13_UNDER_FLOW_IRQ_PRIORITY 0

// <o> CHANNEL13 DUTY CYCLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines duty cycle for UT channel 13.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL13_DUTY_CYCLE_ENABLE       0

// <o> CHANNEL13 DUTY CYCLE VALUE
//    <0=> COMP MATCH
//    <1=> COMP MATCH
//    <2=> 0 PERCENT
//    <3=> 100 PERCENT
// <i> Defines duty cycle value for UT channel 13.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL13_DUTY_CYCLE_VALUE        0
// </e> UTIMER CHANNEL 13


// <e> UTIMER CHANNEL 14
// <i> Configuration settings for UTIMER channel 14 in component ::Drivers:UTIMER
#define RTE_UTIMER_CHANNEL14   1

// <o> CHANNEL14 FIXED BUFFER
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Fixed Buffer features for UT channel 14.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL14_FIXED_BUFFER            0

// <o> CHANNEL14 DMA CONTROL
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines DMA feature control for UT channel 14.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL14_DMA_CONTROL            0

// <o> CHANNEL14 FAULT TYPE
//    <0=> LOW_UNTIL_CYCLE_END
//    <1=> LOW_UNTIL_COUNTER_STOP
// <i> Defines fault type for UT channel 14.
// <i> Default: LOW_UNTIL_COUNTER_STOP
#define RTE_UTIMER_CHANNEL14_FAULT_TYPE            0

// <o> CHANNEL14 TRIANGLE COUNTER BUFFER EVENT
//    <0=> BUF_AT_TROUGH
//    <1=> BUF_AT_TROUGH_N_CREST
// <i> Defines Buffer for triangle counter of UT channel 14.
// <i> Default: BUF_AT_TROUGH
#define RTE_UTIMER_CHANNEL14_BUF_TROUGH_N_CREST      0

// <o> CHANNEL14 DRIVER A
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver A state for UT channel 14.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL14_DRIVER_A                1

// <o> CHANNEL14 DRIVER B
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver B state for UT channel 14.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL14_DRIVER_B                0

// <o> CHANNEL14 DRIVER A START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A start state for UT channel 14.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL14_DRV_A_START_STATE       0

// <o> CHANNEL14 DRIVER A STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A stop state for UT channel 14.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL14_DRV_A_STOP_STATE        0

// <o> CHANNEL14 DRIVER A AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at compare match for UT channel 14.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL14_DRV_A_OP_AT_MATCH_COUNT  0

// <o> CHANNEL14 DRIVER A AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at cycle end for UT channel 14.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL14_DRV_A_OP_AT_CYCLE_END    0

// <o> CHANNEL14 DRIVER B START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B start state for UT channel 14.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL14_DRV_B_START_STATE       0

// <o> CHANNEL14 DRIVER B STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B stop state for UT channel 14.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL14_DRV_B_STOP_STATE        0

// <o> CHANNEL14 DRIVER B AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at compare match for UT channel 14.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL14_DRV_B_OP_AT_MATCH_COUNT  0

// <o> CHANNEL14 DRIVER B AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at cycle end for UT channel 14.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL14_DRV_B_OP_AT_CYCLE_END    0

// <o> CHANNEL14 BUFFER TRANSFER AT CREST
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at crest for UT channel 14.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL14_EVENT_AT_CREST          1

// <o> CHANNEL14 BUFFER TRANSFER AT TROUGH
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at trough for UT channel 14.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL14_EVENT_AT_TROUGH         0

// <o> CHANNEL14 BUFFERING TYPE
//    <0=> SINGLE BUFFER
//    <1=> DOUBLE BUFFER
// <i> Defines buffer type for UT channel 14.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL14_BUFFERING_TYPE          0

// <o> CHANNEL14 BUFFER OPERATION
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer operation for UT channel 14.
// <i> Default: ENABLE
#define RTE_UTIMER_CHANNEL14_BUFFER_OPERATION        0

// <o> CHANNEL14 CAPTURE BUFFER OPERATION FOR DRIVER A
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 14 capture mode.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL14_BUFFERING_TYPE_A        0

// <o> CHANNEL14 CAPTURE BUFFER OPERATION FOR DRIVER B
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 14 capture mode.
// <i> Default: NO BUFFER
#define RTE_UTIMER_CHANNEL14_BUFFERING_TYPE_B        0

// <o> CHANNEL14 CAPTURE A IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 14 capture A.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL14_CAPTURE_A_IRQ_PRIORITY  0

// <o> CHANNEL14 CAPTURE B IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 14 capture B.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL14_CAPTURE_B_IRQ_PRIORITY  0

// <o> CHANNEL14 CAPTURE C IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 14 capture C.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL14_CAPTURE_C_IRQ_PRIORITY  0

// <o> CHANNEL14 CAPTURE D IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 14 capture D.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL14_CAPTURE_D_IRQ_PRIORITY  0

// <o> CHANNEL14 CAPTURE E IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 14 capture E.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL14_CAPTURE_E_IRQ_PRIORITY  0

// <o> CHANNEL14 CAPTURE F IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 14 capture F.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL14_CAPTURE_F_IRQ_PRIORITY  0

// <o> CHANNEL14 OVERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 14 overflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL14_OVER_FLOW_IRQ_PRIORITY  0

// <o> CHANNEL14 UNDERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 14 underflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL14_UNDER_FLOW_IRQ_PRIORITY 0

// <o> CHANNEL14 DUTY CYCLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines duty cycle for UT channel 14.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL14_DUTY_CYCLE_ENABLE       0

// <o> CHANNEL14 DUTY CYCLE VALUE
//    <0=> COMP MATCH
//    <1=> COMP MATCH
//    <2=> 0 PERCENT
//    <3=> 100 PERCENT
// <i> Defines duty cycle value for UT channel 14.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL14_DUTY_CYCLE_VALUE        0
// </e> UTIMER CHANNEL 14


// <e> UTIMER CHANNEL 15
// <i> Configuration settings for UTIMER channel 15 in component ::Drivers:UTIMER
#define RTE_UTIMER_CHANNEL15   1

// <o> CHANNEL15 FIXED BUFFER
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Fixed Buffer features for UT channel 15.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL15_FIXED_BUFFER            0

// <o> CHANNEL15 DMA CONTROL
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines DMA feature control for UT channel 15.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL15_DMA_CONTROL            0

// <o> CHANNEL15 FAULT TYPE
//    <0=> LOW_UNTIL_CYCLE_END
//    <1=> LOW_UNTIL_COUNTER_STOP
// <i> Defines fault type for UT channel 15.
// <i> Default: LOW_UNTIL_CYCLE_END
#define RTE_UTIMER_CHANNEL15_FAULT_TYPE            0

// <o> CHANNEL15 TRIANGLE COUNTER BUFFER EVENT
//    <0=> BUF_AT_TROUGH
//    <1=> BUF_AT_TROUGH_N_CREST
// <i> Defines Buffer for triangle counter of UT channel 15.
// <i> Default: BUF_AT_TROUGH
#define RTE_UTIMER_CHANNEL15_BUF_TROUGH_N_CREST      0

// <o> CHANNEL15 DRIVER A
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver A state for UT channel 15.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL15_DRIVER_A                1

// <o> CHANNEL15 DRIVER B
//    <0=> DISABLED
//    <1=> ENABLED
// <i> Defines Driver B state for UT channel 15.
// <i> Default: DISABLED
#define RTE_UTIMER_CHANNEL15_DRIVER_B                0

// <o> CHANNEL15 DRIVER A START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A start state for UT channel 15.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL15_DRV_A_START_STATE       0

// <o> CHANNEL15 DRIVER A STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver A stop state for UT channel 15.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL15_DRV_A_STOP_STATE        0

// <o> CHANNEL15 DRIVER A AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at compare match for UT channel 15.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL15_DRV_A_OP_AT_MATCH_COUNT  0

// <o> CHANNEL15 DRIVER A AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver A state at cycle end for UT channel 15.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL15_DRV_A_OP_AT_CYCLE_END    0

// <o> CHANNEL15 DRIVER B START STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B start state for UT channel 15.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL15_DRV_B_START_STATE       0

// <o> CHANNEL15 DRIVER B STOP STATE
//    <0=> LOW
//    <1=> HIGH
// <i> Defines Driver B stop state for UT channel 15.
// <i> Default: LOW
#define RTE_UTIMER_CHANNEL15_DRV_B_STOP_STATE        0

// <o> CHANNEL15 DRIVER B AT COMP MATCH
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at compare match for UT channel 15.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL15_DRV_B_OP_AT_MATCH_COUNT  0

// <o> CHANNEL15 DRIVER B AT CYCLE END
//    <0=> NO CHANGE
//    <1=> LOW
//    <2=> HIGH
//    <3=> TOGGLE
// <i> Defines Driver B state at cycle end for UT channel 15.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL15_DRV_B_OP_AT_CYCLE_END    0

// <o> CHANNEL15 BUFFER TRANSFER AT CREST
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at crest for UT channel 15.
// <i> Default: NO CHANGE
#define RTE_UTIMER_CHANNEL15_EVENT_AT_CREST          1

// <o> CHANNEL15 BUFFER TRANSFER AT TROUGH
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer transfer at trough for UT channel 15.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL15_EVENT_AT_TROUGH         0

// <o> CHANNEL15 BUFFERING TYPE
//    <0=> SINGLE BUFFER
//    <1=> DOUBLE BUFFER
// <i> Defines buffer type for UT channel 15.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL15_BUFFERING_TYPE          0

// <o> CHANNEL15 BUFFER OPERATION
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines buffer operation for UT channel 15.
// <i> Default: ENABLE
#define RTE_UTIMER_CHANNEL15_BUFFER_OPERATION        0

// <o> CHANNEL15 CAPTURE BUFFER OPERATION FOR DRIVER A
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 15 capture mode.
// <i> Default: DOUBLE BUFFER
#define RTE_UTIMER_CHANNEL15_BUFFERING_TYPE_A        0

// <o> CHANNEL15 CAPTURE BUFFER OPERATION FOR DRIVER B
//    <0=> NO BUFFER
//    <1=> SINGLE BUFFER
//    <2=> DOUBLE BUFFER
// <i> Defines buffer operation for UT channel 15 capture mode.
// <i> Default: NO BUFFER
#define RTE_UTIMER_CHANNEL15_BUFFERING_TYPE_B        0

// <o> CHANNEL15 CAPTURE A IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 15 capture A.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL15_CAPTURE_A_IRQ_PRIORITY  0

// <o> CHANNEL15 CAPTURE B IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 15 capture B.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL15_CAPTURE_B_IRQ_PRIORITY  0

// <o> CHANNEL15 CAPTURE C IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 15 capture C.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL15_CAPTURE_C_IRQ_PRIORITY  0

// <o> CHANNEL15 CAPTURE D IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 15 capture D.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL15_CAPTURE_D_IRQ_PRIORITY  0

// <o> CHANNEL15 CAPTURE E IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 15 capture E.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL15_CAPTURE_E_IRQ_PRIORITY  0

// <o> CHANNEL15 CAPTURE F IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 15 capture F.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL15_CAPTURE_F_IRQ_PRIORITY  0

// <o> CHANNEL15 OVERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 15 overflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL15_OVER_FLOW_IRQ_PRIORITY  0

// <o> CHANNEL15 UNDERFLOW IRQ priority <0-255>
// <i> Defines Interrupt priority for channel 15 underflow interrupt.
// <i> Default: 0
#define RTE_UTIMER_CHANNEL15_UNDER_FLOW_IRQ_PRIORITY 0

// <o> CHANNEL15 DUTY CYCLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines duty cycle for UT channel 15.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL15_DUTY_CYCLE_ENABLE       0

// <o> CHANNEL15 DUTY CYCLE VALUE
//    <0=> COMP MATCH
//    <1=> COMP MATCH
//    <2=> 0 PERCENT
//    <3=> 100 PERCENT
// <i> Defines duty cycle value for UT channel 15.
// <i> Default: DISABLE
#define RTE_UTIMER_CHANNEL15_DUTY_CYCLE_VALUE        0
// </e>  UTIMER CHANNEL 15
#endif /*RTE_UTIMER*/
// </e> UTIMER (Universal timer) [Driver_UTIMER]
// </h> UTIMER (Universal Timer)


// <e> Analog configuration [vbat analog register2 and comparator register2]
#define RTE_ANALOG_CONFIG          1
#if RTE_ANALOG_CONFIG
#define RTE_VBAT_ANA_REG2_VAL      (0x388C4230)
#define RTE_COMP_REG2_VAL          (0x10200000 | 0x1C240100)
#endif
// </e> Analog configuration [vbat analog register2 and comparator register2]

// <h> DAC (Digital to analog converter )
// <e> DAC0 (Digital to analog converter ) [Driver_DAC0]
// <i> Configuration settings for Driver_DAC0 in component ::Drivers:DAC
#define RTE_DAC0                   1
#if RTE_DAC0
#define RTE_DAC0_INPUT_BYP_MUX_EN  0
#define RTE_DAC0_BYP_VAL           (0x1FF)
#define RTE_DAC0_CAP_CONT          (0X07)
#define RTE_DAC0_RES_CONT          (0X0C)
#define RTE_DAC0_TWOSCOMP_EN        0
#define RTE_DAC0_IBIAS             (0X0C)
#endif
// </e> DAC0 (Digital to analog converter) [Driver_DAC0]

// <e> DAC1 (Digital to analog converter ) [Driver_DAC1]
// <i> Configuration settings for Driver_DAC1 in component ::Drivers:DAC
#define RTE_DAC1                   1
#if RTE_DAC1
#define RTE_DAC1_INPUT_BYP_MUX_EN  0
#define RTE_DAC1_BYP_VAL           (0x1FF)
#define RTE_DAC1_CAP_CONT          (0X07)
#define RTE_DAC1_RES_CONT          (0X0C)
#define RTE_DAC1_TWOSCOMP_EN        0
#define RTE_DAC1_IBIAS             (0X0C)
#endif
// </e> DAC1 (Digital to Analog converter) [Driver_DAC1]
// </h>

// <h> ADC (Analog to Digital Converter)
// <e> ADC120 (Analog to Digital Converter 0) [Driver_ADC120]
// <i> Configuration settings for Driver_ADC120 in component ::Drivers:ADC12
#define RTE_ADC120     1
#if RTE_ADC120
#define RTE_ADC120_IRQ_PRIORITY        0

// <o> ADC120 DONE0 IRQ PRIORITY <0-225>
// <i> defines ADC120 done0 interrupt priority
// <i> default: 0
#define RTE_ADC120_DONE0_IRQ_PRIORITY       0

// <o> ADC120 DONE1 IRQ PRIORITY <0-225>
// <i> defines ADC120 interrupt priority
// <i> default: 0
#define RTE_ADC120_DONE1_IRQ_PRIORITY       0

// <o> ADC120 CMPA IRQ PRIORITY <0-225>
// <i> defines ADC120 interrupt priority
// <i> default: 0
#define RTE_ADC120_CMPA_IRQ_PRIORITY        0

// <o> ADC120 CMPB IRQ PRIORITY <0-225>
// <i> defines ADC120 interrupt priority
// <i> default: 0
#define RTE_ADC120_CMPB_IRQ_PRIORITY        0

// <o> Input channel number <0-8>
// <i> Defines select input channel number.
// <i> Default: 0
#define RTE_ADC120_INPUT_NUM               (0)

// <o> Clock Divider <2-16>
// <i> Defines Clock divider applied to main clock
// <i> Default: 2
#define RTE_ADC120_CLOCK_DIV               (2)

// <o> Sample Width <2-32>
// <i> Defines Width in divided clocks of the sample output signal
// <i> Default: 16
#define RTE_ADC120_SAMPLE_WIDTH            (16)

// <o> Number of average sample <0-256>
// <i> Defines Number of sample to be taken and it must be power of 2
// <i> Default: 256
#define RTE_ADC120_AVG_SAMPLE_NUM          (256)

// <o> Number of bits to shift <0-8>
// <i> Defines How much bit to shift before storing in sample register.
// <i> Default: 8
#define RTE_ADC120_SHIFT_N_BIT             (0)

// <o> Left or Right shift
//     <0=> LEFT SHIFT
//     <1=> RIGHT SHIFT
// <i> Defines shift averaged value before loading in sample register.
// <i> Default: RIGHT SHIFT
#define RTE_ADC120_SHIFT_LEFT_OR_RIGHT     (1)

// <e> RTE_ADC120_DIFFERENTIAL_EN
// <i> Enable differential Mode: 1 ==> in1/2, in3/4, in5/6 will be differential  other wise all single ended
#define RTE_ADC120_DIFFERENTIAL_EN          0

// <o> Enable PGA
//     <0=> DISABLE
//     <1=> ENABLE
// <i> Defines Enable ADC120 PGA enable
// <i> Default: DISABLE
#define RTE_ADC120_PGA_EN                  (0)

// <o> PGA gain value
//     <0=> 0 dB
//     <1=> 6 dB
//     <2=> 12 dB
//     <3=> 18 dB
//     <4=> 24 dB
//     <5=> 30 dB
//     <6=> 36 dB
//     <7=> 42 dB
// <i> Defines Select PGA gain value
// <i> Default: 0
#define RTE_ADC120_PGA_GAIN                (0)
// </e> RTE_ADC121_DIFFERENTIAL_EN

// <o> Comparator mode enable
//     <0=> DISABLE
//     <1=> ENABLE
// <i> Defines set 1 to ENABLE the comparator otherwise comparator "DISABLE"
// <i> Default: ENABLE
#define RTE_ADC120_COMPARATOR_EN            1

// <o> Comparator bias enable
//     <0=> 0.5MS/s
//     <1=> 1MS/s
//     <2=> 2.5MS/s
//     <3=> 5MS/s
// <i> Defines: "11":5MS/s; "10""2.5MS/s; "01":1MS/s;"00":0.5MS/s
// <i> Default: ENABLE
#define RTE_ADC120_COMPARATOR_BIAS          2

// <o> Resistive divider enable
//     <0=> DISABLE
//     <1=> ENABLE
// <i> Defines "0": the resistive divider is off, common mode is not well defined,    "1": the resistive divider is on, common mode is VDD/2
// <i> Default: ENABLE
#define RTE_ADC120_VCM_RDIV_EN              1

#endif
// </e> ADC120 (Analog to Digital Converter 0) [Driver_ADC120]

// <e> ADC121 (Analog to Digital Converter 1) [Driver_ADC121]
// <i> Configuration settings for Driver_ADC121 in component ::Drivers:ADC
#define RTE_ADC121   1
#if RTE_ADC121

// <o> ADC121 DONE0 IRQ PRIORITY <0-225>
// <i> defines ADC121 done0 interrupt priority
// <i> default: 0
#define RTE_ADC121_DONE0_IRQ_PRIORITY       0

// <o> ADC121 DONE1 IRQ PRIORITY <0-225>
// <i> defines ADC121 interrupt priority
// <i> default: 0
#define RTE_ADC121_DONE1_IRQ_PRIORITY       0

// <o> ADC121 CMPA IRQ PRIORITY <0-225>
// <i> defines ADC121 interrupt priority
// <i> default: 0
#define RTE_ADC121_CMPA_IRQ_PRIORITY        0

// <o> ADC121 CMPB IRQ PRIORITY <0-225>
// <i> defines ADC121 interrupt priority
// <i> default: 0
#define RTE_ADC121_CMPB_IRQ_PRIORITY        0

// <o> Input channel number <0-8>
// <i> Defines select input channel number.
// <i> Default: 0
#define RTE_ADC121_INPUT_NUM               (0)

// <o> Clock Divider <2-16>
// <i> Defines Clock divider applied to main clock
// <i> Default: 2
#define RTE_ADC121_CLOCK_DIV               (2)

// <o> Sample Width <2-32>
// <i> Defines Width in divided clocks of the sample output signal
// <i> Default: 16
#define RTE_ADC121_SAMPLE_WIDTH            (16)

// <o> Number of average sample <0-256>
// <i> Defines Number of sample to be taken and it must be power of 2
// <i> Default: 256
#define RTE_ADC121_AVG_SAMPLE_NUM          (256)

// <o> Number of bits to shift <0-8>
// <i> Defines How much bit to shift before storing in sample register.
// <i> Default: 8
#define RTE_ADC121_SHIFT_N_BIT             (8)

// <o> Left or Right shift
//     <0=> LEFT SHIFT
//     <1=> RIGHT SHIFT
// <i> Defines shift averaged value before loading in sample register.
// <i> Default: RIGHT SHIFT
#define RTE_ADC121_SHIFT_LEFT_OR_RIGHT     (1)

// <e> RTE_ADC121_DIFFERENTIAL_EN
// <i> Enable differential Mode: 1 ==> in1/2, in3/4, in5/6 will be differential  other wise all single ended
#define RTE_ADC121_DIFFERENTIAL_EN          0

// <o> Pga enable
//     <0=> DISABLE
//     <1=> ENABLE
// <i> Defines Enable ADC121 using PGA gain
// <i> Default: DISABLE
#define RTE_ADC121_PGA_EN                  (0)

// <o> PGA gain value
//     <0=> 0 dB
//     <1=> 6 dB
//     <2=> 12 dB
//     <3=> 18 dB
//     <4=> 24 dB
//     <5=> 30 dB
//     <6=> 36 dB
//     <7=> 42 dB
// <i> Defines Select PGA gain value
// <i> Default: 0
#define RTE_ADC121_PGA_GAIN                (0)
// </e> RTE_ADC121_DIFFERENTIAL_EN

// <o> Comparator mode enable
//     <0=> DISABLE
//     <1=> ENABLE
// <i> Defines set 1 to ENABLE the comparator otherwise comparator "DISABLE"
// <i> Default: ENABLE
#define RTE_ADC121_COMPARATOR_EN            1

// <o> Comparator bias enable
//     <0=> 0.5MS/s
//     <1=> 1MS/s
//     <2=> 2.5MS/s
//     <3=> 5MS/s
// <i> Defines: "11":5MS/s; "10""2.5MS/s; "01":1MS/s;"00":0.5MS/s
// <i> Default: ENABLE
#define RTE_ADC121_COMPARATOR_BIAS          2

// <o> Resistive divider enable
//     <0=> DISABLE
//     <1=> ENABLE
// <i> Defines "0": the resistive divider is off, common mode is not well defined,    "1": the resistive divider is on, common mode is VDD/2
// <i> Default: ENABLE
#define RTE_ADC121_VCM_RDIV_EN              1

#endif
// </e> ADC121 (Analog to Digital Converter 1) [Driver_ADC121]

// <e> ADC122 (Analog to Digital Converter 2) [Driver_ADC122]
// <i> Configuration settings for Driver_ADC122 in component ::Drivers:ADC
#define RTE_ADC122   1
#if RTE_ADC122

// <o> ADC122 DONE0 IRQ PRIORITY <0-225>
// <i> defines ADC122 done0 interrupt priority
// <i> default: 0
#define RTE_ADC122_DONE0_IRQ_PRIORITY       0

// <o> ADC122 DONE1 IRQ PRIORITY <0-225>
// <i> defines ADC122 interrupt priority
// <i> default: 0
#define RTE_ADC122_DONE1_IRQ_PRIORITY       0

// <o> ADC122 CMPA IRQ PRIORITY <0-225>
// <i> defines ADC122 interrupt priority
// <i> default: 0
#define RTE_ADC122_CMPA_IRQ_PRIORITY        0

// <o> ADC122 CMPB IRQ PRIORITY <0-225>
// <i> defines ADC122 interrupt priority
// <i> default: 0
#define RTE_ADC122_CMPB_IRQ_PRIORITY        0

// <o> Input channel number <0-8>
// <i> Defines select input channel number.
// <i> Default: 0
#define RTE_ADC122_INPUT_NUM               (0)

// <o> Clock Divider <2-16>
// <i> Defines Clock divider applied to main clock
// <i> Default: 2
#define RTE_ADC122_CLOCK_DIV               (2)

// <o> Sample Width <2-32>
// <i> defines Width in divided clocks of the sample output signal
// <i> default: 16
#define RTE_ADC122_SAMPLE_WIDTH            (16)

// <o> Number of average sample <0-256>
// <i> Defines Number of sample to be taken and it must be power of 2
// <i> Default: 256
#define RTE_ADC122_AVG_SAMPLE_NUM          (256)

// <o> Number of bits to shift <0-8>
// <i> Defines How much bit to shift before storing in sample register.
// <i> Default: 8
#define RTE_ADC122_SHIFT_N_BIT             (8)

// <o> Left or Right shift
//     <0=> LEFT SHIFT
//     <1=> RIGHT SHIFT
// <i> Defines shift averaged value before loading in sample register.
// <i> Default: RIGHT SHIFT
#define RTE_ADC122_SHIFT_LEFT_OR_RIGHT     (1)

// <e> RTE_ADC122_DIFFERENTIAL_EN
// <i> Enable differential Mode: 1 ==> in1/2, in3/4, in5/6 will be differential  other wise all single ended
#define RTE_ADC122_DIFFERENTIAL_EN          0

// <o> Enable PGA ADC122
//     <0=> DISABLE
//     <1=> ENABLE
// <i> Defines Enable ADC122 using PGA gain
// <i> Default: DISABLE
#define RTE_ADC122_PGA_EN                  (0)

// <o> PGA gain value
//     <0=> 0 dB
//     <1=> 6 dB
//     <2=> 12 dB
//     <3=> 18 dB
//     <4=> 24 dB
//     <5=> 30 dB
//     <6=> 36 dB
//     <7=> 42 dB
// <i> Defines Select PGA gain value
// <i> Default: 0
#define RTE_ADC122_PGA_GAIN                (0)
// </e> RTE_ADC122_DIFFERENTIAL_EN

// <o> Comparator mode enable
//     <0=> DISABLE
//     <1=> ENABLE
// <i> Defines set 1 to ENABLE the comparator otherwise comparator "DISABLE"
// <i> Default: ENABLE
#define RTE_ADC122_COMPARATOR_EN            1

// <o> Comparator bias enable
//     <0=> 0.5MS/s
//     <1=> 1MS/s
//     <2=> 2.5MS/s
//     <3=> 5MS/s
// <i> Defines: "11":5MS/s; "10""2.5MS/s; "01":1MS/s;"00":0.5MS/s
// <i> Default: ENABLE
#define RTE_ADC122_COMPARATOR_BIAS          2

// <o> Resistive divider enable
//     <0=> DISABLE
//     <1=> ENABLE
// <i> Defines "0": the resistive divider is off, common mode is not well defined,    "1": the resistive divider is on, common mode is VDD/2
// <i> Default: ENABLE
#define RTE_ADC122_VCM_RDIV_EN              1

#endif
// </e> ADC122 (Analog to Digital Converter 2) [Driver_ADC122]

// <e> ADC24 (Analog to Digital Converter 0) [Driver_ADC24]
// <i> Configuration settings for Driver_ADC24 in component ::Drivers:ADC
#define RTE_ADC24   1
#if RTE_ADC24

// <o> ADC24 DONE0 IRQ PRIORITY <0-225>
// <i> defines ADC24 done0 interrupt priority
// <i> default: 0
#define RTE_ADC24_DONE0_IRQ_PRIORITY        0

// <o> ADC24 DONE1 IRQ PRIORITY <0-225>
// <i> defines ADC24 interrupt priority
// <i> default: 0
#define RTE_ADC24_DONE1_IRQ_PRIORITY        0

// <o> ADC24 CMPA IRQ PRIORITY <0-225>
// <i> defines ADC24 interrupt priority
// <i> default: 0
#define RTE_ADC24_CMPA_IRQ_PRIORITY         0

// <o> ADC24 CMPB IRQ PRIORITY <0-225>
// <i> defines ADC24 interrupt priority
// <i> default: 0
#define RTE_ADC24_CMPB_IRQ_PRIORITY         0

// <o> Input channel number <0-4>
// <i> Defines select input channel number.
// <i> Default: 0
#define RTE_ADC24_INPUT_NUM                (0)

// <o> Clock Divider <2-16>
// <i> Defines Clock divider applied to main clock
// <i> Default: 2
#define RTE_ADC24_CLOCK_DIV                (2)

// <o> Number of average sample <0-256>
// <i> Defines Number of sample to be taken and it must be power of 2
// <i> Default: 256
#define RTE_ADC24_AVG_SAMPLE_NUM           (256)

// <o> Number of bits to shift <0-8>
// <i> Defines How much bit to shift before storing in sample register.
// <i> Default: 8
#define RTE_ADC24_SHIFT_N_BIT              (8)

// <o> Left or Right shift
//     <0=> LEFT SHIFT
//     <1=> RIGHT SHIFT
// <i> Defines shift averaged value before loading in sample register.
// <i> Default: RIGHT SHIFT
#define RTE_ADC24_SHIFT_LEFT_OR_RIGHT      (1)

// <o> Ouput rate select
//     <0=> 1 KS/s
//     <1=> 2 KS/s
//     <2=> 4 KS/s
//     <3=> 8 KS/s
//     <4=> 16 KS/s
// <i> Defines Select adc24 output rate
// <i> Default: 0
#define RTE_ADC24_OUTPUT_RATE              (0)

// <o> PGA enable
//     <0=> DISABLE
//     <1=> ENABLE
// <i> Defines Enable PGA to set PGA gain
// <i> Default: DISABLE
#define RTE_ADC24_PGA_EN                   (0)

// <o> PGA gain value
//     <0=> 0 dB
//     <1=> 6 dB
//     <2=> 12 dB
//     <3=> 18 dB
//     <4=> 24 dB
//     <5=> 30 dB
//     <6=> 36 dB
//     <7=> 42 dB
// <i> Defines Select PGA gain value
// <i> Default: 0
#define RTE_ADC24_PGA_GAIN                 (0)

// <o> Bias Control
//     <0=> 5 uA
//     <1=> 6.25 uA
//     <2=> 12 dB
//     <3=> 7.5 uA
//     <4=> 8.75 uA
// <i> Defines Bias control
// <i> Default: 3
#define RTE_ADC24_BIAS                     (3)

#endif
// </e> ADC24 (Analog to Digital Converter 0) [Driver_ADC24]
// </h> ADC (Analog to Digital Converter)

// <h> CRC (Cyclic Redundancy Check)
// <e> CRC0 (Cyclic Redundancy Check) [Driver_CRC0]
// <i> Configuration settings for Driver_CRC0 in component ::Drivers:CRC
#define RTE_CRC0      1
#if RTE_CRC0

// <o> CRC0 DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for CRC0
// <i> Default: 0
#define RTE_CRC0_DMA_ENABLE    0
#if RTE_CRC0_DMA_ENABLE

// <o RTE_CRC0_SELECT_DMA> CRC0 DMA Selection
//    <0=> DMA0
//    <1=> DMA1
//    <2=> DMA2
// <i> Defines Select DMA for CRC0. By default DMA0 will be considered
// <i> Default: 0
#define RTE_CRC0_SELECT_DMA    0
#endif

// <o> CRC0 DMA IRQ priority <0-255>
// <i> Defines CRC0 DMA Interrupt priority
// <i> Default: 0
#define RTE_CRC0_DMA_IRQ_PRI   0
#endif
// </e> CRC0 (Cyclic Redundancy Check) [Driver_CRC0]

// <e> CRC1 (Cyclic Redundancy Check) [Driver_CRC1]
// <i> Configuration settings for Driver_CRC1 in component ::Drivers:CRC
#define RTE_CRC1      1
#if RTE_CRC1

// <o> CRC1 DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for CRC1
// <i> Default: 0
#define RTE_CRC1_DMA_ENABLE    0
#if RTE_CRC1_DMA_ENABLE

// <o RTE_CRC1_SELECT_DMA> CRC1 DMA Selection
//    <0=> DMA0
//    <1=> DMA1
//    <2=> DMA2
// <i> Defines Select DMA for CRC1. By default DMA0 will be considered
// <i> Default: 0
#define RTE_CRC1_SELECT_DMA    0
#endif

// <o> CRC1 DMA IRQ priority <0-255>
// <i> Defines CRC1 DMA Interrupt priority
// <i> Default: 0
#define RTE_CRC1_DMA_IRQ_PRI   0
#endif
// </e> CRC1 (Cyclic Redundancy Check) [Driver_CRC1]
// </h>

// <h> HWSEM (Hardware Semaphore)
// <e> HWSEM0 (Hardware Semaphore) [Driver_HWSEM0]
// <i> Configuration settings for Driver_HWSEM0 in component ::Drivers:HWSEM
#define RTE_HWSEM0                                     1
#if RTE_HWSEM0

// <o> HWSEM0 IRQ priority <0-255>
// <i> Defines HWSEM0 Interrupt priority
// <i> Default: 0
#define RTE_HWSEM0_IRQPRIORITY                         0
#endif

// </e> HWSEM0 (Hardware Semaphore) [Driver_HWSEM0]


// <e> HWSEM1 (Hardware Semaphore) [Driver_HWSEM1]
// <i> Configuration settings for Driver_HWSEM1 in component ::Drivers:HWSEM
#define RTE_HWSEM1                                     0
#if RTE_HWSEM1

// <o> HWSEM1 IRQ priority <0-255>
// <i> Defines HWSEM1 Interrupt priority
// <i> Default: 0
#define RTE_HWSEM1_IRQPRIORITY                         0
#endif

// </e> HWSEM1 (Hardware Semaphore) [Driver_HWSEM1]


// <e> HWSEM2 (Hardware Semaphore) [Driver_HWSEM2]
// <i> Configuration settings for Driver_HWSEM2 in component ::Drivers:HWSEM
#define RTE_HWSEM2                                     0
#if RTE_HWSEM2

// <o> HWSEM2 IRQ priority <0-255>
// <i> Defines HWSEM2 Interrupt priority
// <i> Default: 0
#define RTE_HWSEM2_IRQPRIORITY                         0
#endif

// </e> HWSEM2 (Hardware Semaphore) [Driver_HWSEM2]


// <e> HWSEM3 (Hardware Semaphore) [Driver_HWSEM3]
// <i> Configuration settings for Driver_HWSEM3 in component ::Drivers:HWSEM
#define RTE_HWSEM3                                     0
#if RTE_HWSEM3

// <o> HWSEM3 IRQ priority <0-255>
// <i> Defines HWSEM3 Interrupt priority
// <i> Default: 0
#define RTE_HWSEM3_IRQPRIORITY                         0
#endif

// </e> HWSEM3 (Hardware Semaphore) [Driver_HWSEM3]


// <e> HWSEM4 (Hardware Semaphore) [Driver_HWSEM4]
// <i> Configuration settings for Driver_HWSEM4 in component ::Drivers:HWSEM
#define RTE_HWSEM4                                     0
#if RTE_HWSEM4

// <o> HWSEM4 IRQ priority <0-255>
// <i> Defines HWSEM4 Interrupt priority
// <i> Default: 0
#define RTE_HWSEM4_IRQPRIORITY                         0
#endif

// </e> HWSEM4 (Hardware Semaphore) [Driver_HWSEM4]


// <e> HWSEM5 (Hardware Semaphore) [Driver_HWSEM5]
// <i> Configuration settings for Driver_HWSEM5 in component ::Drivers:HWSEM
#define RTE_HWSEM5                                     0
#if RTE_HWSEM5

// <o> HWSEM5 IRQ priority <0-255>
// <i> Defines HWSEM5 Interrupt priority
// <i> Default: 0
#define RTE_HWSEM5_IRQPRIORITY                         0
#endif

// </e> HWSEM5 (Hardware Semaphore) [Driver_HWSEM5]


// <e> HWSEM6 (Hardware Semaphore) [Driver_HWSEM6]
// <i> Configuration settings for Driver_HWSEM6 in component ::Drivers:HWSEM
#define RTE_HWSEM6                                     0
#if RTE_HWSEM6

// <o> HWSEM6 IRQ priority <0-255>
// <i> Defines HWSEM6 Interrupt priority
// <i> Default: 0
#define RTE_HWSEM6_IRQPRIORITY                         0
#endif

// </e> HWSEM6 (Hardware Semaphore) [Driver_HWSEM6]


// <e> HWSEM7 (Hardware Semaphore) [Driver_HWSEM7]
// <i> Configuration settings for Driver_HWSEM7 in component ::Drivers:HWSEM
#define RTE_HWSEM7                                     0
#if RTE_HWSEM7

// <o> HWSEM7 IRQ priority <0-255>
// <i> Defines HWSEM7 Interrupt priority
// <i> Default: 0
#define RTE_HWSEM7_IRQPRIORITY                         0
#endif

// </e> HWSEM7 (Hardware Semaphore) [Driver_HWSEM7]


// <e> HWSEM8 (Hardware Semaphore) [Driver_HWSEM8]
// <i> Configuration settings for Driver_HWSEM8 in component ::Drivers:HWSEM
#define RTE_HWSEM8                                     0
#if RTE_HWSEM8

// <o> HWSEM8 IRQ priority <0-255>
// <i> Defines HWSEM8 Interrupt priority
// <i> Default: 0
#define RTE_HWSEM8_IRQPRIORITY                         0
#endif

// </e> HWSEM8 (Hardware Semaphore) [Driver_HWSEM8]


// <e> HWSEM9 (Hardware Semaphore) [Driver_HWSEM9]
// <i> Configuration settings for Driver_HWSEM9 in component ::Drivers:HWSEM
#define RTE_HWSEM9                                     0
#if RTE_HWSEM9

// <o> HWSEM9 IRQ priority <0-255>
// <i> Defines HWSEM9 Interrupt priority
// <i> Default: 0
#define RTE_HWSEM9_IRQPRIORITY                         0
#endif

// </e> HWSEM9 (Hardware Semaphore) [Driver_HWSEM9]


// <e> HWSEM10 (Hardware Semaphore) [Driver_HWSEM10]
// <i> Configuration settings for Driver_HWSEM10 in component ::Drivers:HWSEM
#define RTE_HWSEM10                                     0
#if RTE_HWSEM10

// <o> HWSEM10 IRQ priority <0-255>
// <i> Defines HWSEM10 Interrupt priority
// <i> Default: 0
#define RTE_HWSEM10_IRQPRIORITY                         0
#endif

// </e> HWSEM10 (Hardware Semaphore) [Driver_HWSEM10]


// <e> HWSEM11 (Hardware Semaphore) [Driver_HWSEM11]
// <i> Configuration settings for Driver_HWSEM11 in component ::Drivers:HWSEM
#define RTE_HWSEM11                                     0
#if RTE_HWSEM11

// <o> HWSEM11 IRQ priority <0-255>
// <i> Defines HWSEM11 Interrupt priority
// <i> Default: 0
#define RTE_HWSEM11_IRQPRIORITY                         0
#endif

// </e> HWSEM11 (Hardware Semaphore) [Driver_HWSEM11]


// <e> HWSEM12 (Hardware Semaphore) [Driver_HWSEM12]
// <i> Configuration settings for Driver_HWSEM12 in component ::Drivers:HWSEM
#define RTE_HWSEM12                                     0
#if RTE_HWSEM12

// <o> HWSEM12 IRQ priority <0-255>
// <i> Defines HWSEM12 Interrupt priority
// <i> Default: 0
#define RTE_HWSEM12_IRQPRIORITY                         0
#endif

// </e> HWSEM12 (Hardware Semaphore) [Driver_HWSEM12]


// <e> HWSEM13 (Hardware Semaphore) [Driver_HWSEM13]
// <i> Configuration settings for Driver_HWSEM13 in component ::Drivers:HWSEM
#define RTE_HWSEM13                                     0
#if RTE_HWSEM13

// <o> HWSEM13 IRQ priority <0-255>
// <i> Defines HWSEM13 Interrupt priority
// <i> Default: 0
#define RTE_HWSEM13_IRQPRIORITY                         0
#endif

// </e> HWSEM13 (Hardware Semaphore) [Driver_HWSEM13]


// <e> HWSEM14 (Hardware Semaphore) [Driver_HWSEM14]
// <i> Configuration settings for Driver_HWSEM14 in component ::Drivers:HWSEM
#define RTE_HWSEM14                                     0
#if RTE_HWSEM14

// <o> HWSEM14 IRQ priority <0-255>
// <i> Defines HWSEM14 Interrupt priority
// <i> Default: 0
#define RTE_HWSEM14_IRQPRIORITY                         0
#endif

// </e> HWSEM14 (Hardware Semaphore) [Driver_HWSEM14]


// <e> HWSEM15 (Hardware Semaphore) [Driver_HWSEM15]
// <i> Configuration settings for Driver_HWSEM15 in component ::Drivers:HWSEM
#define RTE_HWSEM15                                     0
#if RTE_HWSEM15

// <o> HWSEM15 IRQ priority <0-255>
// <i> Defines HWSEM15 Interrupt priority
// <i> Default: 0
#define RTE_HWSEM15_IRQPRIORITY                         0
#endif

// </e> HWSEM15 (Hardware Semaphore) [Driver_HWSEM15]
// </h>

// <h> High speed Analog Comparator
// <e> CMP0 (Analog Comparator ) [Driver_CMP0]
// <i> Configuration settings for Driver_CMP0 in component ::Drivers:Comparator
#define RTE_HSCMP0    1
#if RTE_HSCMP0

// <o> CMP0 IRQ priority <0-255>
// <i> Defines Interrupt priority for CMP0.
// <i> Default: 0
#define RTE_CMP0_IRQ_PRIORITY        0

// <o> Select positive terminal
//     <0=> PIN_P0_0
//     <1=> PIN_P0_6
//     <2=> PIN_P1_4
//     <3=> PIN_P0_4
// <i> Defines:select input to the positive terminal of comparator;
// <i> 00 = P0_0; 01 = P0_6; 10 = P1_4; 11 = P0_4
// Default : 0
#define RTE_CMP0_SEL_POSITIVE        0

// <o> Select negative terminal
//     <0=> PIN_P2_0
//     <1=> PIN_P2_1
//     <2=> PIN_INT_VREF
//     <3=> PIN_DAC6
// <i> Defines: select input to the negative terminal of comparator;
// <i> 00 = P2_0; 01 = P2_1; 10 = Internal Vref; 11 = DAC6
// Default : 3
#define RTE_CMP0_SEL_NEGATIVE        3

// <o> Select Hysteresis level
//     <0=> 0mV
//     <1=> 6mV
//     <2=> 12mV
//     <3=> 18mV
//     <4=> 24mV
//     <5=> 30mV
//     <6=> 36mV
//     <7=> 42mV
// <i> Defines: Select comparator hysteresis level, step size is 6mv;
// <i> 000 = 0mv; 001 = 6mv; 010 = 12mv; 011 = 18mv; 100 = 24mv; 101 = 30mv;
// <i> 110 = 36mv; 111 = 42mV
// <i> Default: 7
#define RTE_CMP0_SEL_HYSTERISIS      7

#endif
// </e> CMP0 (Analog Comparator) [Driver_CMP0]

// <e> CMP1 (Analog Comparator ) [Driver_CMP1]
// <i> Configuration settings for Driver_CMP1 in component ::Drivers:Comparator
#define RTE_HSCMP1    1
#if RTE_HSCMP1

// <o> CMP1 IRQ priority <0-255>
// <i> Defines Interrupt priority for CMP1.
// <i> Default: 0
#define RTE_CMP1_IRQ_PRIORITY        0

// <o> Select positive terminal
//     <0=> PIN_P0_1
//     <1=> PIN_P0_7
//     <2=> PIN_P1_5
//     <3=> PIN_P0_5
// <i> Defines:select input to the positive terminal of comparator;
// <i> 00 = P0_1; 01 = P0_7; 10 = P1_5; 11 = P0_5
// Default : 0
#define RTE_CMP1_SEL_POSITIVE        0

// <o> Select negative terminal
//     <0=> PIN_P2_0
//     <1=> PIN_P2_1
//     <2=> PIN_INT_VREF
//     <3=> PIN_DAC6
// <i> Defines: select input to the negative terminal of comparator;
// <i> 00 = P2_0; 01 = P2_1; 10 = Internal Vref; 11 = DAC6
// Default : 3
#define RTE_CMP1_SEL_NEGATIVE        3

// <o> Select Hysteresis level
//     <0=> 0mV
//     <1=> 6mV
//     <2=> 12mV
//     <3=> 18mV
//     <4=> 24mV
//     <5=> 30mV
//     <6=> 36mV
//     <7=> 42mV
// <i> Defines: Select comparator hysteresis level, step size is 6mv;
// <i> 000 = 0mv; 001 = 6mv; 010 = 12mv; 011 = 18mv; 100 = 24mv; 101 = 30mv;
// <i> 110 = 36mv; 111 = 42mV
// <i> Default: 7
#define RTE_CMP1_SEL_HYSTERISIS      7

#endif
// </e> CMP1 (Analog Comparator) [Driver_CMP1]

// <e> CMP2 (Analog Comparator ) [Driver_CMP2]
// <i> Configuration settings for Driver_CMP2 in component ::Drivers:Comparator
#define RTE_HSCMP2    1
#if RTE_HSCMP2

// <o> CMP2 IRQ priority <0-255>
// <i> Defines Interrupt priority for CMP2.
// <i> Default: 0
#define RTE_CMP2_IRQ_PRIORITY        0

// <o> Select positive terminal
//     <0=> PIN_P0_2
//     <1=> PIN_P1_0
//     <2=> PIN_P1_6
//     <3=> PIN_P1_2
// <i> Defines:select input to the positive terminal of comparator;
// <i> 00 = P0_2; 01 = P1_0; 10 = P1_6; 11 = P1_2
// Default : 0
#define RTE_CMP2_SEL_POSITIVE        0

// <o> Select negative terminal
//     <0=> PIN_P2_0
//     <1=> PIN_P2_1
//     <2=> PIN_INT_VREF
//     <3=> PIN_DAC6
// <i> Defines: select input to the negative terminal of comparator;
// <i> 00 = P2_0; 01 = P2_1; 10 = Internal Vref; 11 = DAC6
// Default : 3
#define RTE_CMP2_SEL_NEGATIVE        3

// <o> Select Hysteresis level
//     <0=> 0mV
//     <1=> 6mV
//     <2=> 12mV
//     <3=> 18mV
//     <4=> 24mV
//     <5=> 30mV
//     <6=> 36mV
//     <7=> 42mV
// <i> Defines: Select comparator hysteresis level, step size is 6mv;
// <i> 000 = 0mv; 001 = 6mv; 010 = 12mv; 011 = 18mv; 100 = 24mv; 101 = 30mv;
// <i> 110 = 36mv; 111 = 42mV
// <i> Default: 7
#define RTE_CMP2_SEL_HYSTERISIS      7

#endif
// </e> CMP2 (Analog Comparator) [Driver_CMP2]

// <e> CMP3 (Analog Comparator ) [Driver_CMP3]
// <i> Configuration settings for Driver_CMP3 in component ::Drivers:Comparator
#define RTE_HSCMP3    1
#if RTE_HSCMP3

// <o> CMP3 IRQ priority <0-255>
// <i> Defines Interrupt priority for CMP3.
// <i> Default: 0
#define RTE_CMP3_IRQ_PRIORITY        0

// <o> Select positive terminal
//     <0=> PIN_P0_3
//     <1=> PIN_P1_1
//     <2=> PIN_P1_7
//     <3=> PIN_P1_3
// <i> Defines:select input to the positive terminal of comparator;
// <i> 00 = P0_3; 01 = P1_1; 10 = P1_7; 11 = P1_3
// Default : 0
#define RTE_CMP3_SEL_POSITIVE        0

// <o> Select negative terminal
//     <0=> PIN_P2_0
//     <1=> PIN_P2_1
//     <2=> PIN_INT_VREF
//     <3=> PIN_DAC6
// <i> Defines: select input to the negative terminal of comparator;
// <i> 00 = P2_0; 01 = P2_1; 10 = Internal Vref; 11 = DAC6
// Default : 3
#define RTE_CMP3_SEL_NEGATIVE        3

// <o> Select Hysteresis level
//     <0=> 0mV
//     <1=> 6mV
//     <2=> 12mV
//     <3=> 18mV
//     <4=> 24mV
//     <5=> 30mV
//     <6=> 36mV
//     <7=> 42mV
// <i> Defines: Select comparator hysteresis level, step size is 6mv;
// <i> 000 = 0mv; 001 = 6mv; 010 = 12mv; 011 = 18mv; 100 = 24mv; 101 = 30mv;
// <i> 110 = 36mv; 111 = 42mV
// <i> Default: 7
#define RTE_CMP3_SEL_HYSTERISIS      7

#endif
// </e> CMP3 (Analog Comparator) [Driver_CMP3]

#if defined(M55_HE)
// <e> LPCMP (Low Power Analog Comparator) [Driver_LPCMP]
// <i> Configuration settings for Driver_LPCMP in component ::Drivers:LPCMP
#define RTE_LPCMP      1

#if RTE_LPCMP

// <o> LPCMP IRQ priority <0-255>
// <i> Defines Interrupt priority for LPCMP.
// <i> Default: 0
#define RTE_LPCMP_IRQ_PRIORITY        0

// <o> Select positive terminal
//     <0=> PIN_P2_4
//     <1=> PIN_P2_5
//     <2=> PIN_P2_6
//     <3=> PIN_P2_7
// <i> Defines:select input to the positive terminal of LPCMP;
// <i> 00 = P2_4; 01 = P2_5; 10 = P2_6; 11 = P2_7
// Default : 0
#define RTE_LPCMP_SEL_POSITIVE        0

// <o> Select negative terminal
//     <0=> AON_VREF
//     <1=> PIN_P2_0
//     <2=> PIN_P2_1
//     <3=> PIN_P2_2
// <i> Defines: select input to the negative terminal of LPCMP;
// <i> 00 = AON Vref(0.8v); 01 = P2_0; 10 = P2_1 Vref; 11 = P2_2
// Default : 0
#define RTE_LPCMP_SEL_NEGATIVE        0

// <o> Select Hysteresis level
//     <0=> 0mV
//     <1=> 6mV
//     <2=> 12mV
//     <3=> 18mV
//     <4=> 24mV
//     <5=> 30mV
//     <6=> 36mV
//     <7=> 42mV
// <i> Defines: Select LPCMP hysteresis level, step size is 6mv;
// <i> 000 = 0mv; 001 = 6mv; 010 = 12mv; 011 = 18mv; 100 = 24mv; 101 = 30mv;
// <i> 110 = 36mv; 111 = 42mV
// <i> Default: 7
#define RTE_LPCMP_SEL_HYSTERISIS      7

#endif
// </e> LPCMP (Low Power Analog Comparator) [Driver_LPCMP]
#endif //defined(M55_HE)
// </h> CMP (Analog Comparator)

// <h> I2C (Inter Integrated Circuit)
// <e> I2C (Inter Integrated Circuit) [Driver_I2C0]
// <i> Configuration settings for Driver_I2C0 in component ::Drivers:I2C
#define RTE_I2C0                     1

#if RTE_I2C0

// <o> I2C0 IRQ priority <0-255>
// <i> Defines Interrupt priority for I2C0.
// <i> Default: 0
#define RTE_I2C0_IRQ_PRIORITY        0
// <o> I2C0 TX FIFO threshold <0-31>
// <i> Defines TX FIFO threshold for I2C0.
// <i> Default: 16
#define RTE_I2C0_TX_FIFO_THRESHOLD   16

// <o> I2C0 RX FIFO threshold <0-31>
// <i> Defines RX FIFO threshold for I2C0.
// <i> Default: 16
#define RTE_I2C0_RX_FIFO_THRESHOLD   16
#endif
// </e> I2C (Inter Integrated Circuit) [Driver_I2C0]

// <e> I2C (Inter Integrated Circuit) [Driver_I2C1]
// <i> Configuration settings for Driver_I2C1 in component ::Drivers:I2C

#define RTE_I2C1                     1
#if RTE_I2C1

// <o> I2C1 IRQ priority <0-255>
// <i> Defines Interrupt priority for I2C1.
// <i> Default: 0
#define RTE_I2C1_IRQ_PRIORITY        0
// <o> I2C1 TX FIFO threshold <0-31>
// <i> Defines TX FIFO threshold for I2C1.
// <i> Default: 16
#define RTE_I2C1_TX_FIFO_THRESHOLD   16

// <o> I2C1 RX FIFO threshold <0-31>
// <i> Defines RX FIFO threshold for I2C1.
// <i> Default: 16
#define RTE_I2C1_RX_FIFO_THRESHOLD   16
#endif
// </e> I2C (Inter Integrated Circuit) [Driver_I2C1]

// <e> I2C (Inter Integrated Circuit) [Driver_I2C2]
// <i> Configuration settings for Driver_I2C2 in component ::Drivers:I2C

#define RTE_I2C2                     1
#if RTE_I2C2

// <o> I2C2 IRQ priority <0-255>
// <i> Defines Interrupt priority for I2C2.
// <i> Default: 0
#define RTE_I2C2_IRQ_PRIORITY        0
// <o> I2C2 TX FIFO threshold <0-31>
// <i> Defines TX FIFO threshold for I2C2.
// <i> Default: 16
#define RTE_I2C2_TX_FIFO_THRESHOLD   16

// <o> I2C2 RX FIFO threshold <0-31>
// <i> Defines RX FIFO threshold for I2C2.
// <i> Default: 16
#define RTE_I2C2_RX_FIFO_THRESHOLD   16
#endif
// </e> I2C (Inter Integrated Circuit) [Driver_I2C2]

// <e> I2C (Inter Integrated Circuit) [Driver_I2C3]
// <i> Configuration settings for Driver_I2C3 in component ::Drivers:I2C

#define RTE_I2C3                     1

#if RTE_I2C3

// <o> I2C3 IRQ priority <0-255>
// <i> Defines Interrupt priority for I2C3.
// <i> Default: 0
#define RTE_I2C3_IRQ_PRIORITY        0
// <o> I2C3 TX FIFO threshold <0-31>
// <i> Defines TX FIFO threshold for I2C3.
// <i> Default: 16
#define RTE_I2C3_TX_FIFO_THRESHOLD   16

// <o> I2C3 RX FIFO threshold <0-31>
// <i> Defines RX FIFO threshold for I2C3.
// <i> Default: 16
#define RTE_I2C3_RX_FIFO_THRESHOLD   16
#endif
// </e> I2C (Inter Integrated Circuit) [Driver_I2C3]

// <e> LPI2C (Low Power Inter-Integrated Circuit) [Driver_LPI2C]
// <i> Configuration settings for Driver_LPI2C in component ::Drivers:LPI2C
#if defined(M55_HE)
#define RTE_LPI2C   1
#if RTE_LPI2C

// <o> LPI2C IRQ PRIORITY <0-225>
// <i> defines LPI2C interrupt priority
// <i> default: 0
#define RTE_LPI2C_IRQ_PRIORITY       0
#endif
// </e> LPI2C (Low Power Inter-Integrated Circuit) [Driver_LPI2C]
#endif //defined(M55_HE)
// </h>

// <e> DMA0 (Direct Memory Access Controller) [Driver_DMA0]
// <i> Configuration settings for Driver_DMA0 in component ::Drivers:DMA
#define RTE_DMA0      1
#if RTE_DMA0

// <o> DMA APB Interface to be used
//   <0=> Secure APB interface
//   <1=> Non-Secure
// <i> Default: 0
#define RTE_DMA0_APB_INTERFACE  0
// <o> DMA0 Abort interrupt priority <0-255>
// <i> Defines DMA0 Abort interrupt priority.
// <i> Default: 0
#define RTE_DMA0_ABORT_IRQ_PRI 0
// <o> DMA0 Boot Event-IRQ Non-secure state <0-0xFFFFFFFF>
// <i> Defines DMA0 Boot Event-IRQ Non-secure state
// <i> Default: 0
#define RTE_DMA0_BOOT_IRQ_NS_STATE 0
// <o> DMA0 Boot Peripheral Non-secure state <0-0xFFFFFFFF>
// <i> Defines DMA0 Boot Peripheral Non-secure state
// <i> Default: 0
#define RTE_DMA0_BOOT_PERIPH_NS_STATE 0
#endif
// </e> DMA0 (Direct Memory Access Controller) [Driver_DMA0]

// <e> DMA1 (Direct Memory Access Controller) [Driver_DMA1]
#if defined(M55_HP)

// <i> Configuration settings for Driver_DMA1 in component ::Drivers:DMA
#define RTE_DMA1      1
#if RTE_DMA1

// <o> DMA APB Interface to be used
//   <0=> Secure APB interface
//   <1=> Non-Secure
// <i> Default: 0
#define RTE_DMA1_APB_INTERFACE  0
// <o> DMA1 Abort interrupt priority <0-255>
// <i> Defines DMA1 Abort interrupt priority.
// <i> Default: 0
#define RTE_DMA1_ABORT_IRQ_PRI 0
// <o> DMA1 Boot Event-IRQ Non-secure state <0-0xFFFFFFFF>
// <i> Defines DMA1 Boot Event-IRQ Non-secure state
// <i> Default: 0
#define RTE_DMA1_BOOT_IRQ_NS_STATE 0
// <o> DMA1 Boot Peripheral Non-secure state <0-0xFFFFFFFF>
// <i> Defines DMA1 Boot Peripheral Non-secure state
// <i> Default: 0
#define RTE_DMA1_BOOT_PERIPH_NS_STATE 0
#endif

#endif
// </e> DMA1 (Direct Memory Access Controller) [Driver_DMA1]

// <e> DMA2 (Direct Memory Access Controller) [Driver_DMA2]
// <i> Configuration settings for Driver_DMA2 in component ::Drivers:DMA
#if defined(M55_HE)

#define RTE_DMA2      1
#if RTE_DMA2

// <o> DMA APB Interface to be used
//   <0=> Secure APB interface
//   <1=> Non-Secure
// <i> Default: 0
#define RTE_DMA2_APB_INTERFACE  0
// <o> DMA2 Abort interrupt priority <0-255>
// <i> Defines DMA2 Abort interrupt priority.
// <i> Default: 0
#define RTE_DMA2_ABORT_IRQ_PRI 0
// <o> DMA2 Boot Event-IRQ Non-secure state <0-0xFFFFFFFF>
// <i> Defines DMA2 Boot Event-IRQ Non-secure state
// <i> Default: 0
#define RTE_DMA2_BOOT_IRQ_NS_STATE 0
// <o> DMA2 Boot Peripheral Non-secure state <0-0xFFFFFFFF>
// <i> Defines DMA2 Boot Peripheral Non-secure state
// <i> Default: 0
#define RTE_DMA2_BOOT_PERIPH_NS_STATE 0
#endif

#endif
// </e> DMA2 (Direct Memory Access Controller) [Driver_DMA2]

// <h> GPIO (general purpose input and output)
// <e> GPIO0
// <i> Configuration settings for Driver_GPIO0 in component ::Drivers:GPIO
#define RTE_GPIO0   1

#if RTE_GPIO0

// <o> GPIO0 Debounce Clock Divisor <0-1023>
// <i> Defines debounce clock divisor for GPIO0.
// <i> Default: 0
#define RTE_GPIO0_DB_CLK_DIV            1

// <o> GPIO0_PIN0 IRQ Priority
// <i> Defines Interrupt priority for GPIO0_PIN0.
// <i> Default: 0
#define RTE_GPIO0_PIN0_IRQ_PRIORITY     0

// <o> GPIO0_PIN1 IRQ Priority
// <i> Defines Interrupt priority for GPIO0_PIN1.
// <i> Default: 0
#define RTE_GPIO0_PIN1_IRQ_PRIORITY     0

// <o> GPIO0_PIN2 IRQ Priority
// <i> Defines Interrupt priority for GPIO0_PIN2.
// <i> Default: 0
#define RTE_GPIO0_PIN2_IRQ_PRIORITY     0

// <o> GPIO0_PIN3 IRQ Priority
// <i> Defines Interrupt priority for GPIO0_PIN3.
// <i> Default: 0
#define RTE_GPIO0_PIN3_IRQ_PRIORITY     0

// <o> GPIO0_PIN4 IRQ Priority
// <i> Defines Interrupt priority for GPIO0_PIN4.
// <i> Default: 0
#define RTE_GPIO0_PIN4_IRQ_PRIORITY     0

// <o> GPIO0_PIN5 IRQ Priority
// <i> Defines Interrupt priority for GPIO0_PIN5.
// <i> Default: 0
#define RTE_GPIO0_PIN5_IRQ_PRIORITY     0

// <o> GPIO0_PIN6 IRQ Priority
// <i> Defines Interrupt priority for GPIO0_PIN6.
// <i> Default: 0
#define RTE_GPIO0_PIN6_IRQ_PRIORITY     0

// <o> GPIO0_PIN7 IRQ Priority
// <i> Defines Interrupt priority for GPIO0_PIN7.
// <i> Default: 0
#define RTE_GPIO0_PIN7_IRQ_PRIORITY     0
#endif
// </e> GPIO0

// <e> GPIO1
// <i> Configuration settings for Driver_GPIO1 in component ::Drivers:GPIO
#define RTE_GPIO1   1

#if RTE_GPIO1

// <o> GPIO1 Debounce Clock Divisor <0-1023>
// <i> Defines debounce clock divisor for GPIO1.
// <i> Default: 0
#define RTE_GPIO1_DB_CLK_DIV            1

// <o> GPIO1_PIN0 IRQ Priority
// <i> Defines Interrupt priority for GPIO1_PIN0.
// <i> Default: 0
#define RTE_GPIO1_PIN0_IRQ_PRIORITY     0

// <o> GPIO1_PIN1 IRQ Priority
// <i> Defines Interrupt priority for GPIO1_PIN1.
// <i> Default: 0
#define RTE_GPIO1_PIN1_IRQ_PRIORITY     0

// <o> GPIO1_PIN2 IRQ Priority
// <i> Defines Interrupt priority for GPIO1_PIN2.
// <i> Default: 0
#define RTE_GPIO1_PIN2_IRQ_PRIORITY     0

// <o> GPIO1_PIN3 IRQ Priority
// <i> Defines Interrupt priority for GPIO1_PIN3.
// <i> Default: 0
#define RTE_GPIO1_PIN3_IRQ_PRIORITY     0

// <o> GPIO1_PIN4 IRQ Priority
// <i> Defines Interrupt priority for GPIO1_PIN4.
// <i> Default: 0
#define RTE_GPIO1_PIN4_IRQ_PRIORITY     0

// <o> GPIO1_PIN5 IRQ Priority
// <i> Defines Interrupt priority for GPIO1_PIN5.
// <i> Default: 0
#define RTE_GPIO1_PIN5_IRQ_PRIORITY     0

// <o> GPIO1_PIN6 IRQ Priority
// <i> Defines Interrupt priority for GPIO1_PIN6.
// <i> Default: 0
#define RTE_GPIO1_PIN6_IRQ_PRIORITY     0

// <o> GPIO1_PIN7 IRQ Priority
// <i> Defines Interrupt priority for GPIO1_PIN7.
// <i> Default: 0
#define RTE_GPIO1_PIN7_IRQ_PRIORITY     0
#endif
// </e> GPIO1

// <e> GPIO2
// <i> Configuration settings for Driver_GPIO2 in component ::Drivers:GPIO
#define RTE_GPIO2   1

#if RTE_GPIO2

// <o> GPIO2 Debounce Clock Divisor <0-1023>
// <i> Defines debounce clock divisor for GPIO2.
// <i> Default: 0
#define RTE_GPIO2_DB_CLK_DIV            1

// <o> GPIO2_PIN0 IRQ Priority
// <i> Defines Interrupt priority for GPIO2_PIN0.
// <i> Default: 0
#define RTE_GPIO2_PIN0_IRQ_PRIORITY     0

// <o> GPIO2_PIN1 IRQ Priority
// <i> Defines Interrupt priority for GPIO2_PIN1.
// <i> Default: 0
#define RTE_GPIO2_PIN1_IRQ_PRIORITY     0

// <o> GPIO2_PIN2 IRQ Priority
// <i> Defines Interrupt priority for GPIO2_PIN2.
// <i> Default: 0
#define RTE_GPIO2_PIN2_IRQ_PRIORITY     0

// <o> GPIO2_PIN3 IRQ Priority
// <i> Defines Interrupt priority for GPIO1_PIN3.
// <i> Default: 0
#define RTE_GPIO2_PIN3_IRQ_PRIORITY     0

// <o> GPIO2_PIN4 IRQ Priority
// <i> Defines Interrupt priority for GPIO2_PIN4.
// <i> Default: 0
#define RTE_GPIO2_PIN4_IRQ_PRIORITY     0

// <o> GPIO2_PIN5 IRQ Priority
// <i> Defines Interrupt priority for GPIO2_PIN5.
// <i> Default: 0
#define RTE_GPIO2_PIN5_IRQ_PRIORITY     0

// <o> GPIO2_PIN6 IRQ Priority
// <i> Defines Interrupt priority for GPIO2_PIN6.
// <i> Default: 0
#define RTE_GPIO2_PIN6_IRQ_PRIORITY     0

// <o> GPIO2_PIN7 IRQ Priority
// <i> Defines Interrupt priority for GPIO2_PIN7.
// <i> Default: 0
#define RTE_GPIO2_PIN7_IRQ_PRIORITY     0
#endif
// </e> GPIO2

// <e> GPIO3
// <i> Configuration settings for Driver_GPIO3 in component ::Drivers:GPIO
#define RTE_GPIO3   1

#if RTE_GPIO3

// <o> GPIO3 Debounce Clock Divisor <0-1023>
// <i> Defines debounce clock divisor for GPIO3.
// <i> Default: 0
#define RTE_GPIO3_DB_CLK_DIV            1

// <o> GPIO3_PIN0 IRQ Priority
// <i> Defines Interrupt priority for GPIO3_PIN0.
// <i> Default: 0
#define RTE_GPIO3_PIN0_IRQ_PRIORITY     0

// <o> GPIO3_PIN0 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO3_PIN0_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO3_PIN1 IRQ Priority
// <i> Defines Interrupt priority for GPIO3_PIN1.
// <i> Default: 0
#define RTE_GPIO3_PIN1_IRQ_PRIORITY     0

// <o> GPIO3_PIN1 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO3_PIN1_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO3_PIN2 IRQ Priority
// <i> Defines Interrupt priority for GPIO3_PIN2.
// <i> Default: 0
#define RTE_GPIO3_PIN2_IRQ_PRIORITY     0

// <o> GPIO3_PIN2 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO3_PIN2_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO3_PIN3 IRQ Priority
// <i> Defines Interrupt priority for GPIO3_PIN3.
// <i> Default: 0
#define RTE_GPIO3_PIN3_IRQ_PRIORITY     0

// <o> GPIO3_PIN3 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO3_PIN3_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO3_PIN4 IRQ Priority
// <i> Defines Interrupt priority for GPIO3_PIN4.
// <i> Default: 0
#define RTE_GPIO3_PIN4_IRQ_PRIORITY     0

// <o> GPIO3_PIN4 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO3_PIN4_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO3_PIN5 IRQ Priority
// <i> Defines Interrupt priority for GPIO3_PIN5.
// <i> Default: 0
#define RTE_GPIO3_PIN5_IRQ_PRIORITY     0

// <o> GPIO3_PIN5 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO3_PIN5_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO3_PIN6 IRQ Priority
// <i> Defines Interrupt priority for GPIO3_PIN6.
// <i> Default: 0
#define RTE_GPIO3_PIN6_IRQ_PRIORITY     0

// <o> GPIO3_PIN6 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO3_PIN6_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO3_PIN7 IRQ Priority
// <i> Defines Interrupt priority for GPIO3_PIN7.
// <i> Default: 0
#define RTE_GPIO3_PIN7_IRQ_PRIORITY     0

// <o> GPIO3_PIN7 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO3_PIN7_DMA_GLITCH_FILTER_ENABLE  0
#endif
// </e> GPIO3

// <e> GPIO4
// <i> Configuration settings for Driver_GPIO4 in component ::Drivers:GPIO
#define RTE_GPIO4   1

#if RTE_GPIO4

// <o> GPIO4 Debounce Clock Divisor <0-1023>
// <i> Defines debounce clock divisor for GPIO4.
// <i> Default: 0
#define RTE_GPIO4_DB_CLK_DIV            1

// <o> GPIO4_PIN0 IRQ Priority
// <i> Defines Interrupt priority for GPIO4_PIN0.
// <i> Default: 0
#define RTE_GPIO4_PIN0_IRQ_PRIORITY     0

// <o> GPIO4_PIN0 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO4_PIN0_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO4_PIN1 IRQ Priority
// <i> Defines Interrupt priority for GPIO4_PIN1.
// <i> Default: 0
#define RTE_GPIO4_PIN1_IRQ_PRIORITY     0

// <o> GPIO4_PIN1 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO4_PIN1_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO4_PIN2 IRQ Priority
// <i> Defines Interrupt priority for GPIO4_PIN2.
// <i> Default: 0
#define RTE_GPIO4_PIN2_IRQ_PRIORITY     0

// <o> GPIO4_PIN2 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO4_PIN2_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO4_PIN3 IRQ Priority
// <i> Defines Interrupt priority for GPIO4_PIN3.
// <i> Default: 0
#define RTE_GPIO4_PIN3_IRQ_PRIORITY     0

// <o> GPIO4_PIN3 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO4_PIN3_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO4_PIN4 IRQ Priority
// <i> Defines Interrupt priority for GPIO4_PIN4.
// <i> Default: 0
#define RTE_GPIO4_PIN4_IRQ_PRIORITY     0

// <o> GPIO4_PIN4 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO4_PIN4_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO4_PIN5 IRQ Priority
// <i> Defines Interrupt priority for GPIO4_PIN5.
// <i> Default: 0
#define RTE_GPIO4_PIN5_IRQ_PRIORITY     0

// <o> GPIO4_PIN5 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO4_PIN5_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO4_PIN6 IRQ Priority
// <i> Defines Interrupt priority for GPIO4_PIN6.
// <i> Default: 0
#define RTE_GPIO4_PIN6_IRQ_PRIORITY     0

// <o> GPIO4_PIN6 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO4_PIN6_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO4_PIN7 IRQ Priority
// <i> Defines Interrupt priority for GPIO4_PIN7.
// <i> Default: 0
#define RTE_GPIO4_PIN7_IRQ_PRIORITY     0

// <o> GPIO4_PIN7 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO4_PIN7_DMA_GLITCH_FILTER_ENABLE  0

#endif
// </e> GPIO4

// <e> GPIO5
// <i> Configuration settings for Driver_GPIO5 in component ::Drivers:GPIO
#define RTE_GPIO5   1

#if RTE_GPIO5

// <o> GPIO5 Debounce Clock Divisor <0-1023>
// <i> Defines debounce clock divisor for GPIO5.
// <i> Default: 0
#define RTE_GPIO5_DB_CLK_DIV            1

// <o> GPIO5_PIN0 IRQ Priority
// <i> Defines Interrupt priority for GPIO5_PIN0.
// <i> Default: 0
#define RTE_GPIO5_PIN0_IRQ_PRIORITY     0

// <o> GPIO5_PIN1 IRQ Priority
// <i> Defines Interrupt priority for GPIO5_PIN1.
// <i> Default: 0
#define RTE_GPIO5_PIN1_IRQ_PRIORITY     0

// <o> GPIO5_PIN2 IRQ Priority
// <i> Defines Interrupt priority for GPIO5_PIN2.
// <i> Default: 0
#define RTE_GPIO5_PIN2_IRQ_PRIORITY     0

// <o> GPIO5_PIN3 IRQ Priority
// <i> Defines Interrupt priority for GPIO5_PIN3.
// <i> Default: 0
#define RTE_GPIO5_PIN3_IRQ_PRIORITY     0

// <o> GPIO5_PIN4 IRQ Priority
// <i> Defines Interrupt priority for GPIO5_PIN4.
// <i> Default: 0
#define RTE_GPIO5_PIN4_IRQ_PRIORITY     0

// <o> GPIO5_PIN5 IRQ Priority
// <i> Defines Interrupt priority for GPIO5_PIN5.
// <i> Default: 0
#define RTE_GPIO5_PIN5_IRQ_PRIORITY     0

// <o> GPIO5_PIN6 IRQ Priority
// <i> Defines Interrupt priority for GPIO5_PIN6.
// <i> Default: 0
#define RTE_GPIO5_PIN6_IRQ_PRIORITY     0

// <o> GPIO5_PIN7 IRQ Priority
// <i> Defines Interrupt priority for GPIO5_PIN7.
// <i> Default: 0
#define RTE_GPIO5_PIN7_IRQ_PRIORITY     0
#endif /* RTE_GPIO5 */
// </e> GPIO5

// <e> GPIO6
// <i> Configuration settings for Driver_GPIO6 in component ::Drivers:GPIO
#define RTE_GPIO6   1

#if RTE_GPIO6

// <o> GPIO6 Debounce Clock Divisor <0-1023>
// <i> Defines debounce clock divisor for GPIO6.
// <i> Default: 0
#define RTE_GPIO6_DB_CLK_DIV            1

// <o> GPIO6_PIN0 IRQ Priority
// <i> Defines Interrupt priority for GPIO6_PIN0.
// <i> Default: 0
#define RTE_GPIO6_PIN0_IRQ_PRIORITY     0

// <o> GPIO6_PIN1 IRQ Priority
// <i> Defines Interrupt priority for GPIO6_PIN1.
// <i> Default: 0
#define RTE_GPIO6_PIN1_IRQ_PRIORITY     0

// <o> GPIO6_PIN2 IRQ Priority
// <i> Defines Interrupt priority for GPIO6_PIN2.
// <i> Default: 0
#define RTE_GPIO6_PIN2_IRQ_PRIORITY     0

// <o> GPIO6_PIN3 IRQ Priority
// <i> Defines Interrupt priority for GPIO6_PIN3.
// <i> Default: 0
#define RTE_GPIO6_PIN3_IRQ_PRIORITY     0

// <o> GPIO6_PIN4 IRQ Priority
// <i> Defines Interrupt priority for GPIO6_PIN4.
// <i> Default: 0
#define RTE_GPIO6_PIN4_IRQ_PRIORITY     0

// <o> GPIO6_PIN5 IRQ Priority
// <i> Defines Interrupt priority for GPIO6_PIN5.
// <i> Default: 0
#define RTE_GPIO6_PIN5_IRQ_PRIORITY     0

// <o> GPIO6_PIN6 IRQ Priority
// <i> Defines Interrupt priority for GPIO6_PIN6.
// <i> Default: 0
#define RTE_GPIO6_PIN6_IRQ_PRIORITY     0

// <o> GPIO6_PIN7 IRQ Priority
// <i> Defines Interrupt priority for GPIO6_PIN7.
// <i> Default: 0
#define RTE_GPIO6_PIN7_IRQ_PRIORITY     0
#endif /* RTE_GPIO6 */
// </e> GPIO6

// <e> GPIO7
// <i> Configuration settings for Driver_GPIO7 in component ::Drivers:GPIO
#define RTE_GPIO7   1

#if RTE_GPIO7

// <o> GPIO7 Debounce Clock Divisor <0-1023>
// <i> Defines debounce clock divisor for GPIO7.
// <i> Default: 0
#define RTE_GPIO7_DB_CLK_DIV            1

// <o> GPIO7_PIN0 IRQ Priority
// <i> Defines Interrupt priority for GPIO7_PIN0.
// <i> Default: 0
#define RTE_GPIO7_PIN0_IRQ_PRIORITY     0

// <o> GPIO7_PIN0 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO7_PIN0_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO7_PIN1 IRQ Priority
// <i> Defines Interrupt priority for GPIO7_PIN1.
// <i> Default: 0
#define RTE_GPIO7_PIN1_IRQ_PRIORITY     0

// <o> GPIO7_PIN1 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO7_PIN1_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO7_PIN2 IRQ Priority
// <i> Defines Interrupt priority for GPIO7_PIN2.
// <i> Default: 0
#define RTE_GPIO7_PIN2_IRQ_PRIORITY     0

// <o> GPIO7_PIN2 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO7_PIN2_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO7_PIN3 IRQ Priority
// <i> Defines Interrupt priority for GPIO7_PIN3.
// <i> Default: 0
#define RTE_GPIO7_PIN3_IRQ_PRIORITY     0

// <o> GPIO7_PIN3 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO7_PIN3_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO7_PIN4 IRQ Priority
// <i> Defines Interrupt priority for GPIO7_PIN4.
// <i> Default: 0
#define RTE_GPIO7_PIN4_IRQ_PRIORITY     0

// <o> GPIO7_PIN4 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO7_PIN4_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO7_PIN5 IRQ Priority
// <i> Defines Interrupt priority for GPIO7_PIN5.
// <i> Default: 0
#define RTE_GPIO7_PIN5_IRQ_PRIORITY     0

// <o> GPIO7_PIN5 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO7_PIN5_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO7_PIN6 IRQ Priority
// <i> Defines Interrupt priority for GPIO7_PIN6.
// <i> Default: 0
#define RTE_GPIO7_PIN6_IRQ_PRIORITY     0

// <o> GPIO7_PIN6 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO7_PIN6_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO7_PIN7 IRQ Priority
// <i> Defines Interrupt priority for GPIO7_PIN7.
// <i> Default: 0
#define RTE_GPIO7_PIN7_IRQ_PRIORITY     0

// <o> GPIO7_PIN7 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO7_PIN7_DMA_GLITCH_FILTER_ENABLE  0
#endif /* RTE_GPIO7 */
// </e> GPIO7

// <e> GPIO8
// <i> Configuration settings for Driver_GPIO8 in component ::Drivers:GPIO
#define RTE_GPIO8   1

#if RTE_GPIO8

// <o> GPIO8 Debounce Clock Divisor <0-1023>
// <i> Defines debounce clock divisor for GPIO8.
// <i> Default: 0
#define RTE_GPIO8_DB_CLK_DIV            1

// <o> GPIO8_PIN0 IRQ Priority
// <i> Defines Interrupt priority for GPIO8_PIN0.
// <i> Default: 0
#define RTE_GPIO8_PIN0_IRQ_PRIORITY     0

// <o> GPIO8_PIN0 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO8_PIN0_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO8_PIN1 IRQ Priority
// <i> Defines Interrupt priority for GPIO8_PIN1.
// <i> Default: 0
#define RTE_GPIO8_PIN1_IRQ_PRIORITY     0

// <o> GPIO8_PIN1 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO8_PIN1_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO8_PIN2 IRQ Priority
// <i> Defines Interrupt priority for GPIO8_PIN2.
// <i> Default: 0
#define RTE_GPIO8_PIN2_IRQ_PRIORITY     0

// <o> GPIO8_PIN2 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO8_PIN2_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO8_PIN3 IRQ Priority
// <i> Defines Interrupt priority for GPIO8_PIN3.
// <i> Default: 0
#define RTE_GPIO8_PIN3_IRQ_PRIORITY     0

// <o> GPIO8_PIN3 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO8_PIN3_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO8_PIN4 IRQ Priority
// <i> Defines Interrupt priority for GPIO8_PIN4.
// <i> Default: 0
#define RTE_GPIO8_PIN4_IRQ_PRIORITY     0

// <o> GPIO8_PIN4 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO8_PIN4_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO8_PIN5 IRQ Priority
// <i> Defines Interrupt priority for GPIO8_PIN5.
// <i> Default: 0
#define RTE_GPIO8_PIN5_IRQ_PRIORITY     0

// <o> GPIO8_PIN5 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO8_PIN5_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO8_PIN6 IRQ Priority
// <i> Defines Interrupt priority for GPIO8_PIN6.
// <i> Default: 0
#define RTE_GPIO8_PIN6_IRQ_PRIORITY     0

// <o> GPIO8_PIN6 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO8_PIN6_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO8_PIN7 IRQ Priority
// <i> Defines Interrupt priority for GPIO8_PIN7.
// <i> Default: 0
#define RTE_GPIO8_PIN7_IRQ_PRIORITY     0

// <o> GPIO8_PIN7 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO8_PIN7_DMA_GLITCH_FILTER_ENABLE  0
#endif /* RTE_GPIO8 */
// </e> GPIO8

// <e> GPIO9
// <i> Configuration settings for Driver_GPIO9 in component ::Drivers:GPIO
#define RTE_GPIO9   1

#if RTE_GPIO9

// <o> GPIO9 Debounce Clock Divisor <0-1023>
// <i> Defines debounce clock divisor for GPIO9.
// <i> Default: 0
#define RTE_GPIO9_DB_CLK_DIV            1

// <o> GPIO9_PIN0 IRQ Priority
// <i> Defines Interrupt priority for GPIO9_PIN0.
// <i> Default: 0
#define RTE_GPIO9_PIN0_IRQ_PRIORITY     0

// <o> GPIO9_PIN0 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO9_PIN0_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO9_PIN1 IRQ Priority
// <i> Defines Interrupt priority for GPIO9_PIN1.
// <i> Default: 0
#define RTE_GPIO9_PIN1_IRQ_PRIORITY     0

// <o> GPIO9_PIN1 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO9_PIN1_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO9_PIN2 IRQ Priority
// <i> Defines Interrupt priority for GPIO9_PIN2.
// <i> Default: 0
#define RTE_GPIO9_PIN2_IRQ_PRIORITY     0

// <o> GPIO9_PIN2 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO9_PIN2_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO9_PIN3 IRQ Priority
// <i> Defines Interrupt priority for GPIO9_PIN3.
// <i> Default: 0
#define RTE_GPIO9_PIN3_IRQ_PRIORITY     0

// <o> GPIO9_PIN3 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO9_PIN3_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO9_PIN4 IRQ Priority
// <i> Defines Interrupt priority for GPIO9_PIN4.
// <i> Default: 0
#define RTE_GPIO9_PIN4_IRQ_PRIORITY     0

// <o> GPIO9_PIN4 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO9_PIN4_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO9_PIN5 IRQ Priority
// <i> Defines Interrupt priority for GPIO9_PIN5.
// <i> Default: 0
#define RTE_GPIO9_PIN5_IRQ_PRIORITY     0

// <o> GPIO9_PIN5 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO9_PIN5_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO9_PIN6 IRQ Priority
// <i> Defines Interrupt priority for GPIO9_PIN6.
// <i> Default: 0
#define RTE_GPIO9_PIN6_IRQ_PRIORITY     0

// <o> GPIO9_PIN6 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO9_PIN6_DMA_GLITCH_FILTER_ENABLE  0

// <o> GPIO9_PIN7 IRQ Priority
// <i> Defines Interrupt priority for GPIO9_PIN7.
// <i> Default: 0
#define RTE_GPIO9_PIN7_IRQ_PRIORITY     0

// <o> GPIO9_PIN7 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_GPIO9_PIN7_DMA_GLITCH_FILTER_ENABLE  0
#endif /* RTE_GPIO9 */
// </e> GPIO9

// <e> GPIO10
// <i> Configuration settings for Driver_GPIO10 in component ::Drivers:GPIO
#define RTE_GPIO10   1

#if RTE_GPIO10

// <o> GPIO10 Debounce Clock Divisor <0-1023>
// <i> Defines debounce clock divisor for GPIO10.
// <i> Default: 0
#define RTE_GPIO10_DB_CLK_DIV            1

// <o> GPIO10_PIN0 IRQ Priority
// <i> Defines Interrupt priority for GPIO10_PIN0.
// <i> Default: 0
#define RTE_GPIO10_PIN0_IRQ_PRIORITY     0

// <o> GPIO10_PIN1 IRQ Priority
// <i> Defines Interrupt priority for GPIO10_PIN1.
// <i> Default: 0
#define RTE_GPIO10_PIN1_IRQ_PRIORITY     0

// <o> GPIO10_PIN2 IRQ Priority
// <i> Defines Interrupt priority for GPIO10_PIN2.
// <i> Default: 0
#define RTE_GPIO10_PIN2_IRQ_PRIORITY     0

// <o> GPIO10_PIN3 IRQ Priority
// <i> Defines Interrupt priority for GPIO10_PIN3.
// <i> Default: 0
#define RTE_GPIO10_PIN3_IRQ_PRIORITY     0

// <o> GPIO10_PIN4 IRQ Priority
// <i> Defines Interrupt priority for GPIO10_PIN4.
// <i> Default: 0
#define RTE_GPIO10_PIN4_IRQ_PRIORITY     0

// <o> GPIO10_PIN5 IRQ Priority
// <i> Defines Interrupt priority for GPIO10_PIN5.
// <i> Default: 0
#define RTE_GPIO10_PIN5_IRQ_PRIORITY     0

// <o> GPIO10_PIN6 IRQ Priority
// <i> Defines Interrupt priority for GPIO10_PIN6.
// <i> Default: 0
#define RTE_GPIO10_PIN6_IRQ_PRIORITY     0

// <o> GPIO10_PIN7 IRQ Priority
// <i> Defines Interrupt priority for GPIO10_PIN7.
// <i> Default: 0
#define RTE_GPIO10_PIN7_IRQ_PRIORITY     0
#endif /* RTE_GPIO10 */
// </e> GPIO10

// <e> GPIO11
// <i> Configuration settings for Driver_GPIO11 in component ::Drivers:GPIO
#define RTE_GPIO11   1

#if RTE_GPIO11

// <o> GPIO11 Debounce Clock Divisor <0-1023>
// <i> Defines debounce clock divisor for GPIO11.
// <i> Default: 0
#define RTE_GPIO11_DB_CLK_DIV            1

// <o> GPIO11_PIN0 IRQ Priority
// <i> Defines Interrupt priority for GPIO11_PIN0.
// <i> Default: 0
#define RTE_GPIO11_PIN0_IRQ_PRIORITY     0

// <o> GPIO11_PIN1 IRQ Priority
// <i> Defines Interrupt priority for GPIO11_PIN1.
// <i> Default: 0
#define RTE_GPIO11_PIN1_IRQ_PRIORITY     0

// <o> GPIO11_PIN2 IRQ Priority
// <i> Defines Interrupt priority for GPIO11_PIN2.
// <i> Default: 0
#define RTE_GPIO11_PIN2_IRQ_PRIORITY     0

// <o> GPIO11_PIN3 IRQ Priority
// <i> Defines Interrupt priority for GPIO11_PIN3.
// <i> Default: 0
#define RTE_GPIO11_PIN3_IRQ_PRIORITY     0

// <o> GPIO11_PIN4 IRQ Priority
// <i> Defines Interrupt priority for GPIO11_PIN4.
// <i> Default: 0
#define RTE_GPIO11_PIN4_IRQ_PRIORITY     0

// <o> GPIO11_PIN5 IRQ Priority
// <i> Defines Interrupt priority for GPIO11_PIN5.
// <i> Default: 0
#define RTE_GPIO11_PIN5_IRQ_PRIORITY     0

// <o> GPIO11_PIN6 IRQ Priority
// <i> Defines Interrupt priority for GPIO11_PIN6.
// <i> Default: 0
#define RTE_GPIO11_PIN6_IRQ_PRIORITY     0

// <o> GPIO11_PIN7 IRQ Priority
// <i> Defines Interrupt priority for GPIO11_PIN7.
// <i> Default: 0
#define RTE_GPIO11_PIN7_IRQ_PRIORITY     0
#endif /* RTE_GPIO11 */
// </e> GPIO11

// <e> GPIO12
// <i> Configuration settings for Driver_GPIO12 in component ::Drivers:GPIO
#define RTE_GPIO12   1

#if RTE_GPIO12

// <o> GPIO12 Debounce Clock Divisor <0-1023>
// <i> Defines debounce clock divisor for GPIO12.
// <i> Default: 0
#define RTE_GPIO12_DB_CLK_DIV            1

// <o> GPIO12_PIN0 IRQ Priority
// <i> Defines Interrupt priority for GPIO12_PIN0.
// <i> Default: 0
#define RTE_GPIO12_PIN0_IRQ_PRIORITY     0

// <o> GPIO12_PIN1 IRQ Priority
// <i> Defines Interrupt priority for GPIO12_PIN1.
// <i> Default: 0
#define RTE_GPIO12_PIN1_IRQ_PRIORITY     0

// <o> GPIO12_PIN2 IRQ Priority
// <i> Defines Interrupt priority for GPIO12_PIN2.
// <i> Default: 0
#define RTE_GPIO12_PIN2_IRQ_PRIORITY     0

// <o> GPIO12_PIN3 IRQ Priority
// <i> Defines Interrupt priority for GPIO12_PIN3.
// <i> Default: 0
#define RTE_GPIO12_PIN3_IRQ_PRIORITY     0

// <o> GPIO12_PIN4 IRQ Priority
// <i> Defines Interrupt priority for GPIO12_PIN4.
// <i> Default: 0
#define RTE_GPIO12_PIN4_IRQ_PRIORITY     0

// <o> GPIO12_PIN5 IRQ Priority
// <i> Defines Interrupt priority for GPIO12_PIN5.
// <i> Default: 0
#define RTE_GPIO12_PIN5_IRQ_PRIORITY     0

// <o> GPIO12_PIN6 IRQ Priority
// <i> Defines Interrupt priority for GPIO12_PIN6.
// <i> Default: 0
#define RTE_GPIO12_PIN6_IRQ_PRIORITY     0

// <o> GPIO12_PIN7 IRQ Priority
// <i> Defines Interrupt priority for GPIO12_PIN7.
// <i> Default: 0
#define RTE_GPIO12_PIN7_IRQ_PRIORITY     0
#endif /* RTE_GPIO12 */
// </e> GPIO12

// <e> GPIO13
// <i> Configuration settings for Driver_GPIO13 in component ::Drivers:GPIO
#define RTE_GPIO13   1

#if RTE_GPIO13

// <o> GPIO13 Debounce Clock Divisor <0-1023>
// <i> Defines debounce clock divisor for GPIO13.
// <i> Default: 0
#define RTE_GPIO13_DB_CLK_DIV            1

// <o> GPIO13_PIN0 IRQ Priority
// <i> Defines Interrupt priority for GPIO13_PIN0.
// <i> Default: 0
#define RTE_GPIO13_PIN0_IRQ_PRIORITY     0

// <o> GPIO13_PIN1 IRQ Priority
// <i> Defines Interrupt priority for GPIO13_PIN1.
// <i> Default: 0
#define RTE_GPIO13_PIN1_IRQ_PRIORITY     0

// <o> GPIO13_PIN2 IRQ Priority
// <i> Defines Interrupt priority for GPIO13_PIN2.
// <i> Default: 0
#define RTE_GPIO13_PIN2_IRQ_PRIORITY     0

// <o> GPIO13_PIN3 IRQ Priority
// <i> Defines Interrupt priority for GPIO13_PIN3.
// <i> Default: 0
#define RTE_GPIO13_PIN3_IRQ_PRIORITY     0

// <o> GPIO13_PIN4 IRQ Priority
// <i> Defines Interrupt priority for GPIO13_PIN4.
// <i> Default: 0
#define RTE_GPIO13_PIN4_IRQ_PRIORITY     0

// <o> GPIO13_PIN5 IRQ Priority
// <i> Defines Interrupt priority for GPIO13_PIN5.
// <i> Default: 0
#define RTE_GPIO13_PIN5_IRQ_PRIORITY     0

// <o> GPIO13_PIN6 IRQ Priority
// <i> Defines Interrupt priority for GPIO13_PIN6.
// <i> Default: 0
#define RTE_GPIO13_PIN6_IRQ_PRIORITY     0

// <o> GPIO13_PIN7 IRQ Priority
// <i> Defines Interrupt priority for GPIO13_PIN7.
// <i> Default: 0
#define RTE_GPIO13_PIN7_IRQ_PRIORITY     0
#endif /* RTE_GPIO13 */
// </e> GPIO13

// <e> GPIO14
// <i> Configuration settings for Driver_GPIO14 in component ::Drivers:GPIO
#define RTE_GPIO14   1

#if RTE_GPIO14

// <o> GPIO14 Debounce Clock Divisor <0-1023>
// <i> Defines debounce clock divisor for GPIO14.
// <i> Default: 0
#define RTE_GPIO14_DB_CLK_DIV            1

// <o> GPIO14_PIN0 IRQ Priority
// <i> Defines Interrupt priority for GPIO14_PIN0.
// <i> Default: 0
#define RTE_GPIO14_PIN0_IRQ_PRIORITY     0

// <o> GPIO14_PIN1 IRQ Priority
// <i> Defines Interrupt priority for GPIO14_PIN1.
// <i> Default: 0
#define RTE_GPIO14_PIN1_IRQ_PRIORITY     0

// <o> GPIO14_PIN2 IRQ Priority
// <i> Defines Interrupt priority for GPIO14_PIN2.
// <i> Default: 0
#define RTE_GPIO14_PIN2_IRQ_PRIORITY     0

// <o> GPIO14_PIN3 IRQ Priority
// <i> Defines Interrupt priority for GPIO14_PIN3.
// <i> Default: 0
#define RTE_GPIO14_PIN3_IRQ_PRIORITY     0

// <o> GPIO14_PIN4 IRQ Priority
// <i> Defines Interrupt priority for GPIO14_PIN4.
// <i> Default: 0
#define RTE_GPIO14_PIN4_IRQ_PRIORITY     0

// <o> GPIO14_PIN5 IRQ Priority
// <i> Defines Interrupt priority for GPIO14_PIN5.
// <i> Default: 0
#define RTE_GPIO14_PIN5_IRQ_PRIORITY     0

// <o> GPIO14_PIN6 IRQ Priority
// <i> Defines Interrupt priority for GPIO14_PIN6.
// <i> Default: 0
#define RTE_GPIO14_PIN6_IRQ_PRIORITY     0

// <o> GPIO14_PIN7 IRQ Priority
// <i> Defines Interrupt priority for GPIO14_PIN7.
// <i> Default: 0
#define RTE_GPIO14_PIN7_IRQ_PRIORITY     0
#endif /* RTE_GPIO14 */
// </e> GPIO14

// <e> LPGPIO
// <i> Configuration settings for Driver_LPGPIO in component ::Drivers:GPIO
#define RTE_LPGPIO   1

#if RTE_LPGPIO

// <o> LPGPIO_PIN0 IRQ Priority
// <i> Defines Interrupt priority for LPGPIO_PIN0.
// <i> Default: 0
#define RTE_LPGPIO_PIN0_IRQ_PRIORITY     0

// <o> LPGPIO_PIN0 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_LPGPIO_PIN0_DMA_GLITCH_FILTER_ENABLE  0

// <o> LPGPIO_PIN1 IRQ Priority
// <i> Defines Interrupt priority for LPGPIO_PIN1.
// <i> Default: 0
#define RTE_LPGPIO_PIN1_IRQ_PRIORITY     0

// <o> LPGPIO_PIN1 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_LPGPIO_PIN1_DMA_GLITCH_FILTER_ENABLE  0

// <o> LPGPIO_PIN2 IRQ Priority
// <i> Defines Interrupt priority for LPGPIO_PIN2.
// <i> Default: 0
#define RTE_LPGPIO_PIN2_IRQ_PRIORITY     0

// <o> LPGPIO_PIN2 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_LPGPIO_PIN2_DMA_GLITCH_FILTER_ENABLE  0

// <o> LPGPIO_PIN3 IRQ Priority
// <i> Defines Interrupt priority for LPGPIO_PIN3.
// <i> Default: 0
#define RTE_LPGPIO_PIN3_IRQ_PRIORITY     0

// <o> LPGPIO_PIN3 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_LPGPIO_PIN3_DMA_GLITCH_FILTER_ENABLE  0

// <o> LPGPIO_PIN4 IRQ Priority
// <i> Defines Interrupt priority for LPGPIO_PIN4.
// <i> Default: 0
#define RTE_LPGPIO_PIN4_IRQ_PRIORITY     0

// <o> LPGPIO_PIN4 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_LPGPIO_PIN4_DMA_GLITCH_FILTER_ENABLE  0

// <o> LPGPIO_PIN5 IRQ Priority
// <i> Defines Interrupt priority for LPGPIO_PIN5.
// <i> Default: 0
#define RTE_LPGPIO_PIN5_IRQ_PRIORITY     0

// <o> LPGPIO_PIN5 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_LPGPIO_PIN5_DMA_GLITCH_FILTER_ENABLE  0

// <o> LPGPIO_PIN6 IRQ Priority
// <i> Defines Interrupt priority for LPGPIO_PIN6.
// <i> Default: 0
#define RTE_LPGPIO_PIN6_IRQ_PRIORITY     0

// <o> LPGPIO_PIN6 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_LPGPIO_PIN6_DMA_GLITCH_FILTER_ENABLE  0

// <o> LPGPIO_PIN7 IRQ Priority
// <i> Defines Interrupt priority for LPGPIO_PIN7.
// <i> Default: 0
#define RTE_LPGPIO_PIN7_IRQ_PRIORITY     0

// <o> LPGPIO_PIN7 DMA Glitch Filter enable
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines Enable Glitch Filter
// <i> Default: DISABLE
#define RTE_LPGPIO_PIN7_DMA_GLITCH_FILTER_ENABLE  0

#endif /* RTE_LPGPIO */
// </e> LPGPIO

// </h> GPIO (general purpose input and output)

// <h> PDM(Pulse Density Modulation)
// <e> PDM (Pulse density modulation) [Driver_PDM]
// <i> Configuration settings for Driver_PDM in component ::Drivers:PDM
#define RTE_PDM      1
#if RTE_PDM

// <o> PDM DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for PDM
// <i> Default: ENABLE
#define RTE_PDM_DMA_ENABLE        0

// <o> PDM DMA IRQ priority <0-255>
// <i> Defines PDM DMA Interrupt priority
// <i> Default: 0
#define RTE_PDM_DMA_IRQ_PRIORITY  0

// <o> PDM IRQ priority <0-255>
// <i> Defines Interrupt priority for PDM.
// <i> Default: 0
#define RTE_PDM_IRQ_PRIORITY   0

// <o> PDM Fifo watermark <0-7>
// <i> Defines number of PCM samples in the internal FIFO
// <i> Default: 5
#define RTE_PDM_FIFO_WATERMARK   5

#endif
// </e> PDM (Pulse density modulation) [Driver_PDM]

// </h> PDM (Pulse density modulation)

#if defined(M55_HE)
// <h> LPPDM(Low Power Pulse Density Modulation)
// <e> LPPDM (Low Power Pulse density modulation) [Driver_LPPDM]
// <i> Configuration settings for Driver_LPPDM in component ::Drivers:LPPDM
#define RTE_LPPDM      1

#if RTE_LPPDM

// <o> LPPDM IRQ priority <0-255>
// <i> Defines Interrupt priority for LPPDM.
// <i> Default: 0
#define RTE_LPPDM_IRQ_PRIORITY   0

// <o> LPPDM Fifo watermark <0-7>
// <i> Defines number of PCM samples in the internal FIFO
// <i> Default: 5
#define RTE_LPPDM_FIFO_WATERMARK   5

// <o> LPPDM DMA ENABLE
//    <0=> DISABLE
//    <1=> ENABLE
// <i> Defines DMA feature for LPPDM
// <i> Default: ENABLE
#define RTE_LPPDM_DMA_ENABLE    0
#if RTE_LPPDM_DMA_ENABLE

// <o> LPPDM DMA Selection
//    <0=> DMA2
//    <1=> DMA0
// <i> Defines Select DMA0 for LPPDM. By default DMA2 will be considered
// <i> Default: DMA2
#define RTE_LPPDM_SELECT_DMA0        0

// <o> PDM DMA IRQ priority <0-255>
// <i> Defines PDM DMA Interrupt priority
// <i> Default: 0
#define RTE_LPPDM_DMA_IRQ_PRIORITY  0
#endif
#endif
// </e> LPPDM (Low Power Pulse density modulation) [Driver_LPPDM]
#endif //defined(M55_HE)
// </h> LPPDM (Low Power Pulse density modulation)

// <h> CANFD (Controller Area Network - Fast Mode)
// <e> CANFD (Controller Area Network - Fast Mode Interface) [Driver_CANFD]
// <i> Configuration settings for Driver_CANFD in component ::Drivers:CANFD
#define RTE_CANFD 1

#ifdef RTE_CANFD

// <o> CANFD IRQ priority <0-255>
// <i> Defines Interrupt priority for CANFD.
// <i> Default: 0
#define RTE_CANFD_IRQ_PRIORITY              0

// <o> CANFD Clock Source
//    <0=> 38.4 MHz Clock
//    <1=> 160 MHz Clock
// <i> Defines Clock Source for CANFD.
// <i> Default: 160 MHz
#define RTE_CANFD_CLK_SOURCE                1

// <o> CANFD Clock Speed (Hz) <160000-80000000>
// <i> Defines Clock Speed for CANFD.
// <i> Maximum Clock speed is 80MHz
// <i> Recommended speeds with 160MHz clock source: 20MHz, 40MHz, 80MHz
// <i> Default: 20MHz
#define RTE_CANFD_CLK_SPEED                 20000000

#endif
// </e> CANFD (Controller Area Network - Fast Mode Interface) [Driver_CANFD]
// </h> CANFD (Controller Area Network - Fast Mode)

// <h> SDC (Secure Digital Controller)
// <e> SDC (Secure Digital Controller 0) [Driver_SDC0]
// <i> Configuration settings for Driver_SDC0 in component ::Drivers:SDC
#define RTE_SDC   1

#if RTE_SDC

//    <o> SDC BUS WIDTH SELECT
//    <0=> BUS_WIDTH_1BIT
//    <1=> BUS_WIDTH_4BIT
// <i> Defines SDC0 size of bus width
// <i> Default: BUS_WIDTH_4BIT
#define RTE_SDC_BUS_WIDTH 0

//    <o> SDC DMA SELECT
//    <0=> SDMA
//    <1=> ADMA2
// <i> Defines SDC0 DMA select
// <i> Default: ADMA2
#define RTE_SDC_DMA_SELECT 1

#endif
// </e> SDC0 (Secure Digital Controller 0) [Driver_SDC0]
// </h> SDC (Secure Digital Controller)

#endif  /* __RTE_DEVICE_H */

