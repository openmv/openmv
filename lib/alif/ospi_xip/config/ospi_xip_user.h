/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     ospi_xip_user.h
 * @author   Khushboo Singh
 * @email    khushboo.singh@alifsemi.com
 * @version  V1.0.0
 * @date     05-Dec-2022
 * @brief    User configuration parameters for flash XIP application.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef OSPI_XIP_USER_H_
#define OSPI_XIP_USER_H_

#ifdef  __cplusplus
extern "C"
{
#endif
//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <h> OSPI XIP Configuration
// =========================================

#define OSPI0                                    0U
#define OSPI1                                    1U


/**
  \def OSPI_XIP_INSTANCE
  \brief OSPI instance configuration. Can be set to either \ref OSPI0 or \ref OSPI1.
*/

//   <o OSPI_XIP_INSTANCE> OSPI instance selection
//      <OSPI0=>  0
//      <OSPI1=>  1
//   <i> OSPI instance selection

#define OSPI_XIP_INSTANCE                        OSPI1

/**
  \def OSPI_XIP_IMAGE_ADDRESS
  \brief Address of the OSPI flash image (in XIP address space)
*/

//   <o> OSPI XIP Image address
//   <i> Defines the base (XIP) address of the OSPI flash image.
//   <i> Default: 0xC0000000

#define OSPI_XIP_IMAGE_ADDRESS                   0xC0000000

/**
  \def OSPI_CLOCK_MHZ
  \brief Required ospi clock output (sclk_out).
*/

//   <o> OSPI Clock (in MHz)
//   <i> Defines the frequency of sclk_out of SPI controller.
//   <i> Default:100

#define OSPI_CLOCK_MHZ                           100
#define OSPI_CLOCK                               (OSPI_CLOCK_MHZ * 1000000)

/**
  \def OSPI_XIP_ENABLE_AES_DECRYPTION
  \brief OSPI AES Decryption support. Can be set to either 0(disable) or 1(enable).
*/

//   <o OSPI_XIP_ENABLE_AES_DECRYPTION> OSPI AES Decryption support
//      <0=>  Disable AES Decryption
//      <1=>  Enable AES Decryption
//   <i> OSPI AES Decryption support

#define OSPI_XIP_ENABLE_AES_DECRYPTION           0

/**
  \def OSPI_XIP_SKIP_INITIALIZATION
  \brief Skip OSPI/AES initialization. Can be set to either 0(disable) or 1(enable).
*/

//   <o OSPI_XIP_SKIP_INITIALIZATION> Skip OSPI/AES initialization
//      <0=>  Do not skip OSPI/AES initialization
//      <1=>  Skip OSPI/AES initialization
//   <i> Skip OSPI/AES initialization

#define OSPI_XIP_SKIP_INITIALIZATION             0

//   <o> Number of flash wait cycles
//   <i> Defines the number of wait(dummy) cycles needed for flash read operations.
//   <i> Default:16

#define OSPI_XIP_FLASH_WAIT_CYCLES               16

//   <o> Receive sample delay
//   <i> Defines the number of internal clock cycles that are delayed before rx sampling.
//   <i> Default:4
#define OSPI_XIP_RX_SAMPLE_DELAY                 4

//   <o> DDR transmit drive edge
//   <i> Defines the driving edge in DDR mode.
//   <i> Default:1
#define OSPI_XIP_DDR_DRIVE_EDGE                  1

//   <o> RXDS delay
//   <i> Defines the delay added to the OSPI Read Data Strobe signal.
//   <i> Default:11
#define OSPI_XIP_RXDS_DELAY                      11

// </h>
//------------- <<< end of configuration section >>> ---------------------------

/* Pin/Pad mux and configuration for OSPI0 pads */
#define OSPI0_D0_PORT                            PORT_6
#define OSPI0_D0_PIN                             PIN_0
#define OSPI0_D0_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_1

#define OSPI0_D1_PORT                            PORT_6
#define OSPI0_D1_PIN                             PIN_1
#define OSPI0_D1_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_1

#define OSPI0_D2_PORT                            PORT_6
#define OSPI0_D2_PIN                             PIN_2
#define OSPI0_D2_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_1

#define OSPI0_D3_PORT                            PORT_6
#define OSPI0_D3_PIN                             PIN_3
#define OSPI0_D3_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_1

#define OSPI0_D4_PORT                            PORT_6
#define OSPI0_D4_PIN                             PIN_4
#define OSPI0_D4_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_1

#define OSPI0_D5_PORT                            PORT_6
#define OSPI0_D5_PIN                             PIN_5
#define OSPI0_D5_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_1

#define OSPI0_D6_PORT                            PORT_6
#define OSPI0_D6_PIN                             PIN_6
#define OSPI0_D6_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_1

#define OSPI0_D7_PORT                            PORT_6
#define OSPI0_D7_PIN                             PIN_7
#define OSPI0_D7_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_1

#define OSPI0_SCLK_PORT                          PORT_3
#define OSPI0_SCLK_PIN                           PIN_0
#define OSPI0_SCLK_PIN_FUNCTION                  PINMUX_ALTERNATE_FUNCTION_1

#define OSPI0_RXDS_PORT                          PORT_3
#define OSPI0_RXDS_PIN                           PIN_4
#define OSPI0_RXDS_PIN_FUNCTION                  PINMUX_ALTERNATE_FUNCTION_1

#define OSPI0_CS_PORT                            PORT_3
#define OSPI0_CS_PIN                             PIN_2
#define OSPI0_CS_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_1

#define OSPI0_SCLKN_PORT                         PORT_3
#define OSPI0_SCLKN_PIN                          PIN_1
#define OSPI0_SCLKN_PIN_FUNCTION                 PINMUX_ALTERNATE_FUNCTION_1

/* Pin/Pad mux and configuration for OSPI1 pads */
#define OSPI1_D0_PORT                            PORT_9
#define OSPI1_D0_PIN                             PIN_5
#define OSPI1_D0_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_1

#define OSPI1_D1_PORT                            PORT_9
#define OSPI1_D1_PIN                             PIN_6
#define OSPI1_D1_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_1

#define OSPI1_D2_PORT                            PORT_9
#define OSPI1_D2_PIN                             PIN_7
#define OSPI1_D2_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_1

#define OSPI1_D3_PORT                            PORT_10
#define OSPI1_D3_PIN                             PIN_0
#define OSPI1_D3_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_1

#define OSPI1_D4_PORT                            PORT_10
#define OSPI1_D4_PIN                             PIN_1
#define OSPI1_D4_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_1

#define OSPI1_D5_PORT                            PORT_10
#define OSPI1_D5_PIN                             PIN_2
#define OSPI1_D5_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_1

#define OSPI1_D6_PORT                            PORT_10
#define OSPI1_D6_PIN                             PIN_3
#define OSPI1_D6_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_1

#define OSPI1_D7_PORT                            PORT_10
#define OSPI1_D7_PIN                             PIN_4
#define OSPI1_D7_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_1

#define OSPI1_RXDS_PORT                          PORT_10
#define OSPI1_RXDS_PIN                           PIN_7
#define OSPI1_RXDS_PIN_FUNCTION                  PINMUX_ALTERNATE_FUNCTION_1

#define OSPI1_SCLK_PORT                          PORT_5
#define OSPI1_SCLK_PIN                           PIN_5
#define OSPI1_SCLK_PIN_FUNCTION                  PINMUX_ALTERNATE_FUNCTION_1

#define OSPI1_CS_PORT                            PORT_5
#define OSPI1_CS_PIN                             PIN_7
#define OSPI1_CS_PIN_FUNCTION                    PINMUX_ALTERNATE_FUNCTION_1

#define OSPI1_SCLKN_PORT                         PORT_8
#define OSPI1_SCLKN_PIN                          PIN_0
#define OSPI1_SCLKN_PIN_FUNCTION                 PINMUX_ALTERNATE_FUNCTION_1

#ifdef  __cplusplus
}
#endif

#endif /* OSPI_XIP_USER_H_ */


