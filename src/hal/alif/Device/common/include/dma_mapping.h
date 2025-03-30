/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef DMA_MAPPING_H
#define DMA_MAPPING_H

#include "RTE_Device.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/*                         DMA0 peripheral map                              */
/******************************************************************************/

/************************* DMA0 Group 0 Mapping *******************************/

#if RTE_CMP0_SELECT_DMA0
#define CMP0_DMA                       0
#define CMP0_DMA_PERIPH_REQ            0
#define CMP0_DMA_GROUP                 0
#define CMP0_DMA_HANDSHAKE_ENABLE      0
#endif

#if RTE_CMP1_SELECT_DMA0
#define CMP1_DMA                       0
#define CMP1_DMA_PERIPH_REQ            1
#define CMP1_DMA_GROUP                 0
#define CMP1_DMA_HANDSHAKE_ENABLE      0
#endif

#if RTE_CMP2_SELECT_DMA0
#define CMP2_DMA                       0
#define CMP2_DMA_PERIPH_REQ            2
#define CMP2_DMA_GROUP                 0
#define CMP2_DMA_HANDSHAKE_ENABLE      0
#endif

#if RTE_CMP3_SELECT_DMA0
#define CMP3_DMA                       0
#define CMP3_DMA_PERIPH_REQ            3
#define CMP3_DMA_GROUP                 0
#define CMP3_DMA_HANDSHAKE_ENABLE      0
#endif

#if RTE_ADC120_SELECT_DMA0
#define ADC120_DMA                     0
#define ADC120_DMA_DONE1_PERIPH_REQ    4
#define ADC120_DMA_GROUP               0
#define ADC120_DMA_HANDSHAKE_ENABLE    0
#endif

#if RTE_ADC121_SELECT_DMA0
#define ADC121_DMA                     0
#define ADC121_DMA_DONE1_PERIPH_REQ    5
#define ADC121_DMA_GROUP               0
#define ADC121_DMA_HANDSHAKE_ENABLE    0
#endif

#if RTE_ADC122_SELECT_DMA0
#define ADC122_DMA                     0
#define ADC122_DMA_DONE1_PERIPH_REQ    6
#define ADC122_DMA_GROUP               0
#define ADC122_DMA_HANDSHAKE_ENABLE    0
#endif

#if RTE_BOD_SELECT_DMA0
#define BOD_DMA                        0
#define BOD_DMA_PERIPH_REQ             7
#define BOD_DMA_GROUP                  0
#define BOD_DMA_HANDSHAKE_ENABLE       0
#endif

#define UART0_DMA                      0
#define UART0_DMA_RX_PERIPH_REQ        8
#define UART0_DMA_TX_PERIPH_REQ        16
#define UART0_DMA_GROUP                0
#define UART0_DMA_HANDSHAKE_ENABLE     1

#define UART1_DMA                      0
#define UART1_DMA_RX_PERIPH_REQ        9
#define UART1_DMA_TX_PERIPH_REQ        17
#define UART1_DMA_GROUP                0
#define UART1_DMA_HANDSHAKE_ENABLE     1

#define UART2_DMA                      0
#define UART2_DMA_RX_PERIPH_REQ        10
#define UART2_DMA_TX_PERIPH_REQ        18
#define UART2_DMA_GROUP                0
#define UART2_DMA_HANDSHAKE_ENABLE     1

#define UART3_DMA                      0
#define UART3_DMA_RX_PERIPH_REQ        11
#define UART3_DMA_TX_PERIPH_REQ        19
#define UART3_DMA_GROUP                0
#define UART3_DMA_HANDSHAKE_ENABLE     1

#if RTE_UART4_SELECT_DMA0
#define UART4_DMA                      0
#define UART4_DMA_RX_PERIPH_REQ        12
#define UART4_DMA_TX_PERIPH_REQ        20
#define UART4_DMA_GROUP                0
#define UART4_DMA_HANDSHAKE_ENABLE     1
#endif

#if RTE_UART5_SELECT_DMA0
#define UART5_DMA                      0
#define UART5_DMA_RX_PERIPH_REQ        13
#define UART5_DMA_TX_PERIPH_REQ        21
#define UART5_DMA_GROUP                0
#define UART5_DMA_HANDSHAKE_ENABLE     1
#endif

#if RTE_UART6_SELECT_DMA0
#define UART6_DMA                      0
#define UART6_DMA_RX_PERIPH_REQ        14
#define UART6_DMA_TX_PERIPH_REQ        22
#define UART6_DMA_GROUP                0
#define UART6_DMA_HANDSHAKE_ENABLE     1
#endif

#if RTE_UART7_SELECT_DMA0
#define UART7_DMA                      0
#define UART7_DMA_RX_PERIPH_REQ        15
#define UART7_DMA_TX_PERIPH_REQ        23
#define UART7_DMA_GROUP                0
#define UART7_DMA_HANDSHAKE_ENABLE     1
#endif

#define I2C0_DMA                       0
#define I2C0_DMA_RX_PERIPH_REQ         24
#define I2C0_DMA_TX_PERIPH_REQ         28
#define I2C0_DMA_GROUP                 0
#define I2C0_DMA_HANDSHAKE_ENABLE      1

#define I2C1_DMA                       0
#define I2C1_DMA_RX_PERIPH_REQ         25
#define I2C1_DMA_TX_PERIPH_REQ         29
#define I2C1_DMA_GROUP                 0
#define I2C1_DMA_HANDSHAKE_ENABLE      1

#define I2C2_DMA                       0
#define I2C2_DMA_RX_PERIPH_REQ         26
#define I2C2_DMA_TX_PERIPH_REQ         30
#define I2C2_DMA_GROUP                 0
#define I2C2_DMA_HANDSHAKE_ENABLE      1


#define I2C3_DMA                       0
#define I2C3_DMA_RX_PERIPH_REQ         27
#define I2C3_DMA_TX_PERIPH_REQ         31
#define I2C3_DMA_GROUP                 0
#define I2C3_DMA_HANDSHAKE_ENABLE      1

/************************* DMA0 Group 1 Mapping *******************************/
#if RTE_LPTIMER0_SELECT_DMA0
#define LPTIMER0_DMA                   0
#define LPTIMER0_DMA_PERIPH_REQ        0
#define LPTIMER0_DMA_GROUP             1
#define LPTIMER0_DMA_HANDSHAKE_ENABLE  0
#endif

#if RTE_LPTIMER1_SELECT_DMA0
#define LPTIMER1_DMA                   0
#define LPTIMER1_DMA_PERIPH_REQ        1
#define LPTIMER1_DMA_GROUP             1
#define LPTIMER1_DMA_HANDSHAKE_ENABLE  0
#endif

#if RTE_LPTIMER2_SELECT_DMA0
#define LPTIMER2_DMA                   0
#define LPTIMER2_DMA_PERIPH_REQ        2
#define LPTIMER2_DMA_GROUP             1
#define LPTIMER2_DMA_HANDSHAKE_ENABLE  0
#endif

#if RTE_LPTIMER3_SELECT_DMA0
#define LPTIMER3_DMA                   0
#define LPTIMER3_DMA_PERIPH_REQ        3
#define LPTIMER3_DMA_GROUP             1
#define LPTIMER3_DMA_HANDSHAKE_ENABLE  0
#endif

#if RTE_QEC0_SELECT_DMA0
#define QEC0_DMA                       0
#define QEC0_DMA_PERIPH_REQ            4
#define QEC0_DMA_GROUP                 1
#define QEC0_DMA_HANDSHAKE_ENABLE      0
#endif

#if RTE_QEC1_SELECT_DMA0
#define QEC1_DMA                       0
#define QEC1_DMA_PERIPH_REQ            5
#define QEC1_DMA_GROUP                 1
#define QEC1_DMA_HANDSHAKE_ENABLE      0
#endif

#if RTE_QEC2_SELECT_DMA0
#define QEC2_DMA                       0
#define QEC2_DMA_PERIPH_REQ            6
#define QEC2_DMA_GROUP                 1
#define QEC2_DMA_HANDSHAKE_ENABLE      0
#endif

#if RTE_QEC3_SELECT_DMA0
#define QEC3_DMA                       0
#define QEC3_DMA_PERIPH_REQ            7
#define QEC3_DMA_GROUP                 1
#define QEC3_DMA_HANDSHAKE_ENABLE      0
#endif

#if RTE_LPUART_SELECT_DMA0
#define LPUART_DMA                     0
#define LPUART_DMA_RX_PERIPH_REQ       8
#define LPUART_DMA_TX_PERIPH_REQ       9
#define LPUART_DMA_GROUP               1
#define LPUART_DMA_HANDSHAKE_ENABLE    1
#endif

#if RTE_ADC24_SELECT_DMA0
#define ADC24_DMA                      0
#define ADC24_DMA_DONE0_PERIPH_REQ     10
#define ADC24_DMA_DONE1_PERIPH_REQ     11
#define ADC24_DMA_GROUP                1
#define ADC24_DMA_HANDSHAKE_ENABLE     0
#endif

#if (RTE_LPSPI_SELECT_DMA0 && RTE_LPSPI_SELECT_DMA0_GROUP == 1)
#define LPSPI_DMA                      0
#define LPSPI_DMA_RX_PERIPH_REQ        12
#define LPSPI_DMA_TX_PERIPH_REQ        13
#define LPSPI_DMA_GROUP                1
#define LPSPI_DMA_HANDSHAKE_ENABLE     1
#endif

#if (RTE_LPI2S_SELECT_DMA0)
#define LPI2S_DMA                      0
#define LPI2S_DMA_RX_PERIPH_REQ        14
#define LPI2S_DMA_TX_PERIPH_REQ        15
#define LPI2S_DMA_GROUP                1
#define LPI2S_DMA_HANDSHAKE_ENABLE     1
#endif

#define OSPI0_DMA                      0
#define OSPI0_DMA_RX_PERIPH_REQ        16
#define OSPI0_DMA_TX_PERIPH_REQ        18
#define OSPI0_DMA_GROUP                1
#define OSPI0_DMA_HANDSHAKE_ENABLE     1

#define OSPI1_DMA                      0
#define OSPI1_DMA_RX_PERIPH_REQ        17
#define OSPI1_DMA_TX_PERIPH_REQ        19
#define OSPI1_DMA_GROUP                1
#define OSPI1_DMA_HANDSHAKE_ENABLE     1

#if RTE_I3C_SELECT_DMA0
#define I3C_DMA                        0
#define I3C_DMA_RX_PERIPH_REQ          20
#define I3C_DMA_TX_PERIPH_REQ          21
#define I3C_DMA_GROUP                  1
#define I3C_DMA_HANDSHAKE_ENABLE       1
#endif

#if RTE_CANFD_SELECT_DMA0
#define CANFD_DMA                      0
#define CANFD_DMA_RX_PERIPH_REQ        22
#define CANFD_DMA_TX_PERIPH_REQ        23
#define CANFD_DMA_GROUP                1
#define CANFD_DMA_HANDSHAKE_ENABLE     0
#endif

#define I2S0_DMA                       0
#define I2S0_DMA_RX_PERIPH_REQ         24
#define I2S0_DMA_TX_PERIPH_REQ         28
#define I2S0_DMA_GROUP                 1
#define I2S0_DMA_HANDSHAKE_ENABLE      1

#define I2S1_DMA                       0
#define I2S1_DMA_RX_PERIPH_REQ         25
#define I2S1_DMA_TX_PERIPH_REQ         29
#define I2S1_DMA_GROUP                 1
#define I2S1_DMA_HANDSHAKE_ENABLE      1

#define I2S2_DMA                       0
#define I2S2_DMA_RX_PERIPH_REQ         26
#define I2S2_DMA_TX_PERIPH_REQ         30
#define I2S2_DMA_GROUP                 1
#define I2S2_DMA_HANDSHAKE_ENABLE      1

#define I2S3_DMA                       0
#define I2S3_DMA_RX_PERIPH_REQ         27
#define I2S3_DMA_TX_PERIPH_REQ         31
#define I2S3_DMA_GROUP                 1
#define I2S3_DMA_HANDSHAKE_ENABLE      1

/************************* DMA0 Group 2 Mapping *******************************/
#if RTE_UT0_SELECT_DMA0
#define UT0_DMA                        0
#define UT0_DMA_T0_PERIPH_REQ          0
#define UT0_DMA_T1_PERIPH_REQ          1
#define UT0_DMA_GROUP                  2
#define UT0_DMA_HANDSHAKE_ENABLE       0
#endif

#if RTE_UT1_SELECT_DMA0
#define UT1_DMA                        0
#define UT1_DMA_T0_PERIPH_REQ          2
#define UT1_DMA_T1_PERIPH_REQ          3
#define UT1_DMA_GROUP                  2
#define UT1_DMA_HANDSHAKE_ENABLE       0
#endif

#if RTE_UT2_SELECT_DMA0
#define UT2_DMA                        0
#define UT2_DMA_T0_PERIPH_REQ          4
#define UT2_DMA_T1_PERIPH_REQ          5
#define UT2_DMA_GROUP                  2
#define UT2_DMA_HANDSHAKE_ENABLE       0
#endif

#if RTE_UT3_SELECT_DMA0
#define UT3_DMA                        0
#define UT3_DMA_T0_PERIPH_REQ          6
#define UT3_DMA_T1_PERIPH_REQ          7
#define UT3_DMA_GROUP                  2
#define UT3_DMA_HANDSHAKE_ENABLE       0
#endif

#define UT4_DMA                        0
#define UT4_DMA_T0_PERIPH_REQ          8
#define UT4_DMA_T1_PERIPH_REQ          9
#define UT4_DMA_GROUP                  2
#define UT4_DMA_HANDSHAKE_ENABLE       0

#define UT5_DMA                        0
#define UT5_DMA_T0_PERIPH_REQ          10
#define UT5_DMA_T1_PERIPH_REQ          11
#define UT5_DMA_GROUP                  2
#define UT5_DMA_HANDSHAKE_ENABLE       0

#define UT6_DMA                        0
#define UT6_DMA_T0_PERIPH_REQ          12
#define UT6_DMA_T1_PERIPH_REQ          13
#define UT6_DMA_GROUP                  2
#define UT6_DMA_HANDSHAKE_ENABLE       0

#define UT7_DMA                        0
#define UT7_DMA_T0_PERIPH_REQ          14
#define UT7_DMA_T1_PERIPH_REQ          15
#define UT7_DMA_GROUP                  2
#define UT7_DMA_HANDSHAKE_ENABLE       0

#define SPI0_DMA                       0
#define SPI0_DMA_RX_PERIPH_REQ         16
#define SPI0_DMA_TX_PERIPH_REQ         20
#define SPI0_DMA_GROUP                 2
#define SPI0_DMA_HANDSHAKE_ENABLE      1

#define SPI1_DMA                       0
#define SPI1_DMA_RX_PERIPH_REQ         17
#define SPI1_DMA_TX_PERIPH_REQ         21
#define SPI1_DMA_GROUP                 2
#define SPI1_DMA_HANDSHAKE_ENABLE      1

#define SPI2_DMA                       0
#define SPI2_DMA_RX_PERIPH_REQ         18
#define SPI2_DMA_TX_PERIPH_REQ         22
#define SPI2_DMA_GROUP                 2
#define SPI2_DMA_HANDSHAKE_ENABLE      1

#define SPI3_DMA                       0
#define SPI3_DMA_RX_PERIPH_REQ         19
#define SPI3_DMA_TX_PERIPH_REQ         23
#define SPI3_DMA_GROUP                 2
#define SPI3_DMA_HANDSHAKE_ENABLE      1

#if (RTE_LPSPI_SELECT_DMA0 && RTE_LPSPI_SELECT_DMA0_GROUP == 2)
#define LPSPI_DMA                      0
#define LPSPI_DMA_RX_PERIPH_REQ        24
#define LPSPI_DMA_TX_PERIPH_REQ        25
#define LPSPI_DMA_GROUP                2
#define LPSPI_DMA_HANDSHAKE_ENABLE     1
#endif

#define CAM_DMA                        0
#define CAM_DMA_VSYNC_PERIPH_REQ       26
#define CAM_DMA_HSYNC_PERIPH_REQ       27
#define CAM_DMA_GROUP                  2
#define CAM_DMA_HANDSHAKE_ENABLE       0

#define CDC_DMA                        0
#define CDC_DMA_VSYNC_PERIPH_REQ       28
#define CDC_DMA_HSYNC_PERIPH_REQ       29
#define CDC_DMA_GROUP                  2
#define CDC_DMA_HANDSHAKE_ENABLE       0

#if RTE_LPPDM_SELECT_DMA0
#define LPPDM_DMA                      0
#define LPPDM_DMA_PERIPH_REQ           30
#define LPPDM_DMA_GROUP                2
#define LPPDM_DMA_HANDSHAKE_ENABLE     1
#endif

#define PDM_DMA                        0
#define PDM_DMA_PERIPH_REQ             31
#define PDM_DMA_GROUP                  2
#define PDM_DMA_HANDSHAKE_ENABLE       1

/************************* DMA0 Group 3 Mapping *******************************/
#define GPIO3_DMA                      0
#define GPIO3_DMA_PIN0_PERIPH_REQ      0
#define GPIO3_DMA_PIN1_PERIPH_REQ      1
#define GPIO3_DMA_PIN2_PERIPH_REQ      2
#define GPIO3_DMA_PIN3_PERIPH_REQ      3
#define GPIO3_DMA_PIN4_PERIPH_REQ      4
#define GPIO3_DMA_PIN5_PERIPH_REQ      5
#define GPIO3_DMA_PIN6_PERIPH_REQ      6
#define GPIO3_DMA_PIN7_PERIPH_REQ      7
#define GPIO3_DMA_GROUP                3
#define GPIO3_DMA_HANDSHAKE_ENABLE     0

#define GPIO4_DMA                      0
#define GPIO4_DMA_PIN0_PERIPH_REQ      8
#define GPIO4_DMA_PIN1_PERIPH_REQ      9
#define GPIO4_DMA_PIN2_PERIPH_REQ      10
#define GPIO4_DMA_PIN3_PERIPH_REQ      11
#define GPIO4_DMA_PIN4_PERIPH_REQ      12
#define GPIO4_DMA_PIN5_PERIPH_REQ      13
#define GPIO4_DMA_PIN6_PERIPH_REQ      14
#define GPIO4_DMA_PIN7_PERIPH_REQ      15
#define GPIO4_DMA_GROUP                3
#define GPIO4_DMA_HANDSHAKE_ENABLE     0

#define GPIO7_DMA                      0
#define GPIO7_DMA_PIN0_PERIPH_REQ      16
#define GPIO7_DMA_PIN1_PERIPH_REQ      17
#define GPIO7_DMA_PIN2_PERIPH_REQ      18
#define GPIO7_DMA_PIN3_PERIPH_REQ      19
#define GPIO7_DMA_PIN4_PERIPH_REQ      20
#define GPIO7_DMA_PIN5_PERIPH_REQ      21
#define GPIO7_DMA_PIN6_PERIPH_REQ      22
#define GPIO7_DMA_PIN7_PERIPH_REQ      23
#define GPIO7_DMA_GROUP                3
#define GPIO7_DMA_HANDSHAKE_ENABLE     0

#define GPIO8_DMA                      0
#define GPIO8_DMA_PIN0_PERIPH_REQ      24
#define GPIO8_DMA_PIN1_PERIPH_REQ      25
#define GPIO8_DMA_PIN2_PERIPH_REQ      26
#define GPIO8_DMA_PIN3_PERIPH_REQ      27
#define GPIO8_DMA_PIN4_PERIPH_REQ      28
#define GPIO8_DMA_PIN5_PERIPH_REQ      29
#define GPIO8_DMA_PIN6_PERIPH_REQ      30
#define GPIO8_DMA_PIN7_PERIPH_REQ      31
#define GPIO8_DMA_GROUP                3
#define GPIO8_DMA_HANDSHAKE_ENABLE     0

#if defined (M55_HP)
/******************************************************************************/
/*                         DMA1 peripheral map                              */
/******************************************************************************/
#if (!RTE_CMP0_SELECT_DMA0)
#define CMP0_DMA                       1
#define CMP0_DMA_PERIPH_REQ            0
#define CMP0_DMA_GROUP                 0
#define CMP0_DMA_HANDSHAKE_ENABLE      0
#endif

#if (!RTE_CMP1_SELECT_DMA0)
#define CMP1_DMA                       1
#define CMP1_DMA_PERIPH_REQ            1
#define CMP1_DMA_GROUP                 0
#define CMP1_DMA_HANDSHAKE_ENABLE      0
#endif

#if (!RTE_CMP2_SELECT_DMA0)
#define CMP2_DMA                       1
#define CMP2_DMA_PERIPH_REQ            2
#define CMP2_DMA_GROUP                 0
#define CMP2_DMA_HANDSHAKE_ENABLE      0
#endif

#if (!RTE_CMP3_SELECT_DMA0)
#define CMP3_DMA                       1
#define CMP3_DMA_PERIPH_REQ            3
#define CMP3_DMA_GROUP                 0
#define CMP3_DMA_HANDSHAKE_ENABLE      0
#endif

#if (!RTE_QEC0_SELECT_DMA0)
#define QEC0_DMA                       1
#define QEC0_DMA_PERIPH_REQ            4
#define QEC0_DMA_GROUP                 0
#define QEC0_DMA_HANDSHAKE_ENABLE      0
#endif

#if (!RTE_QEC1_SELECT_DMA0)
#define QEC1_DMA                       1
#define QEC1_DMA_PERIPH_REQ            5
#define QEC1_DMA_GROUP                 0
#define QEC1_DMA_HANDSHAKE_ENABLE      0
#endif

#if (!RTE_QEC2_SELECT_DMA0)
#define QEC2_DMA                       1
#define QEC2_DMA_PERIPH_REQ            6
#define QEC2_DMA_GROUP                 0
#define QEC2_DMA_HANDSHAKE_ENABLE      0
#endif

#if (!RTE_QEC3_SELECT_DMA0)
#define QEC3_DMA                       1
#define QEC3_DMA_PERIPH_REQ            7
#define QEC3_DMA_GROUP                 0
#define QEC3_DMA_HANDSHAKE_ENABLE      0
#endif

#if (!RTE_UT0_SELECT_DMA0)
#define UT0_DMA                        1
#define UT0_DMA_T0_PERIPH_REQ          8
#define UT0_DMA_T1_PERIPH_REQ          9
#define UT0_DMA_GROUP                  0
#define UT0_DMA_HANDSHAKE_ENABLE       0
#endif

#if (!RTE_UT1_SELECT_DMA0)
#define UT1_DMA                        1
#define UT1_DMA_T0_PERIPH_REQ          10
#define UT1_DMA_T1_PERIPH_REQ          11
#define UT1_DMA_GROUP                  0
#define UT1_DMA_HANDSHAKE_ENABLE       0
#endif

#if (!RTE_UT2_SELECT_DMA0)
#define UT2_DMA                        1
#define UT2_DMA_T0_PERIPH_REQ          12
#define UT2_DMA_T1_PERIPH_REQ          13
#define UT2_DMA_GROUP                  0
#define UT2_DMA_HANDSHAKE_ENABLE       0
#endif

#if (!RTE_UT3_SELECT_DMA0)
#define UT3_DMA                        1
#define UT3_DMA_T0_PERIPH_REQ          14
#define UT3_DMA_T1_PERIPH_REQ          15
#define UT3_DMA_GROUP                  0
#define UT3_DMA_HANDSHAKE_ENABLE       0
#endif

#if (!RTE_UART4_SELECT_DMA0)
#define UART4_DMA                      1
#define UART4_DMA_RX_PERIPH_REQ        16
#define UART4_DMA_TX_PERIPH_REQ        20
#define UART4_DMA_GROUP                0
#define UART4_DMA_HANDSHAKE_ENABLE     1
#endif

#if (!RTE_UART5_SELECT_DMA0)
#define UART5_DMA                      1
#define UART5_DMA_RX_PERIPH_REQ        17
#define UART5_DMA_TX_PERIPH_REQ        21
#define UART5_DMA_GROUP                0
#define UART5_DMA_HANDSHAKE_ENABLE     1
#endif

#if (!RTE_UART6_SELECT_DMA0)
#define UART6_DMA                      1
#define UART6_DMA_RX_PERIPH_REQ        18
#define UART6_DMA_TX_PERIPH_REQ        22
#define UART6_DMA_GROUP                0
#define UART6_DMA_HANDSHAKE_ENABLE     1
#endif

#if (!RTE_UART7_SELECT_DMA0)
#define UART7_DMA                      1
#define UART7_DMA_RX_PERIPH_REQ        19
#define UART7_DMA_TX_PERIPH_REQ        23
#define UART7_DMA_GROUP                0
#define UART7_DMA_HANDSHAKE_ENABLE     1
#endif

#define GPIO9_DMA                      1
#define GPIO9_DMA_PIN0_PERIPH_REQ      24
#define GPIO9_DMA_PIN1_PERIPH_REQ      25
#define GPIO9_DMA_PIN2_PERIPH_REQ      26
#define GPIO9_DMA_PIN3_PERIPH_REQ      27
#define GPIO9_DMA_PIN4_PERIPH_REQ      28
#define GPIO9_DMA_PIN5_PERIPH_REQ      29
#define GPIO9_DMA_PIN6_PERIPH_REQ      30
#define GPIO9_DMA_PIN7_PERIPH_REQ      31
#define GPIO9_DMA_GROUP                0
#define GPIO9_DMA_HANDSHAKE_ENABLE     0

#elif defined (M55_HE)
/******************************************************************************/
/*                         DMA2 peripheral map                              */
/******************************************************************************/
#if (!RTE_LPTIMER0_SELECT_DMA0)
#define LPTIMER0_DMA                   2
#define LPTIMER0_DMA_PERIPH_REQ        0
#define LPTIMER0_DMA_GROUP             0
#define LPTIMER0_DMA_HANDSHAKE_ENABLE  0
#endif

#if (!RTE_LPTIMER1_SELECT_DMA0)
#define LPTIMER1_DMA                   2
#define LPTIMER1_DMA_PERIPH_REQ        1
#define LPTIMER1_DMA_GROUP             0
#define LPTIMER1_DMA_HANDSHAKE_ENABLE  0
#endif

#if (!RTE_LPTIMER2_SELECT_DMA0)
#define LPTIMER2_DMA                   2
#define LPTIMER2_DMA_PERIPH_REQ        2
#define LPTIMER2_DMA_GROUP             0
#define LPTIMER2_DMA_HANDSHAKE_ENABLE  0
#endif

#if (!RTE_LPTIMER3_SELECT_DMA0)
#define LPTIMER3_DMA                   2
#define LPTIMER3_DMA_PERIPH_REQ        3
#define LPTIMER3_DMA_GROUP             0
#define LPTIMER3_DMA_HANDSHAKE_ENABLE  0
#endif

#if (!RTE_ADC120_SELECT_DMA0)
#define ADC120_DMA                     2
#define ADC120_DMA_DONE1_PERIPH_REQ    4
#define ADC120_DMA_GROUP               0
#define ADC120_DMA_HANDSHAKE_ENABLE    0
#endif

#if (!RTE_ADC121_SELECT_DMA0)
#define ADC121_DMA                     2
#define ADC121_DMA_DONE1_PERIPH_REQ    5
#define ADC121_DMA_GROUP               0
#define ADC121_DMA_HANDSHAKE_ENABLE    0
#endif

#if (!RTE_ADC122_SELECT_DMA0)
#define ADC122_DMA                     2
#define ADC122_DMA_DONE1_PERIPH_REQ    6
#define ADC122_DMA_GROUP               0
#define ADC122_DMA_HANDSHAKE_ENABLE    0
#endif

#if (!RTE_BOD_SELECT_DMA0)
#define BOD_DMA                        2
#define BOD_DMA_PERIPH_REQ             7
#define BOD_DMA_GROUP                  0
#define BOD_DMA_HANDSHAKE_ENABLE       0
#endif

#if (!RTE_LPUART_SELECT_DMA0)
#define LPUART_DMA                     2
#define LPUART_DMA_RX_PERIPH_REQ       8
#define LPUART_DMA_TX_PERIPH_REQ       9
#define LPUART_DMA_GROUP               0
#define LPUART_DMA_HANDSHAKE_ENABLE    1
#endif

#if (!RTE_ADC24_SELECT_DMA0)
#define ADC24_DMA                      2
#define ADC24_DMA_DONE0_PERIPH_REQ     10
#define ADC24_DMA_DONE1_PERIPH_REQ     11
#define ADC24_DMA_GROUP                0
#define ADC24_DMA_HANDSHAKE_ENABLE     0
#endif

#if (!RTE_LPSPI_SELECT_DMA0)
#define LPSPI_DMA                      2
#define LPSPI_DMA_RX_PERIPH_REQ        12
#define LPSPI_DMA_TX_PERIPH_REQ        13
#define LPSPI_DMA_GROUP                0
#define LPSPI_DMA_HANDSHAKE_ENABLE     1
#endif

#if (!RTE_LPI2S_SELECT_DMA0)
#define LPI2S_DMA                      2
#define LPI2S_DMA_RX_PERIPH_REQ        14
#define LPI2S_DMA_TX_PERIPH_REQ        15
#define LPI2S_DMA_GROUP                0
#define LPI2S_DMA_HANDSHAKE_ENABLE     1
#endif

#define LPCAM_DMA                      2
#define LPCAM_DMA_VSYNC_PERIPH_REQ     16
#define LPCAM_DMA_HSYNC_PERIPH_REQ     17
#define LPCAM_DMA_GROUP                0
#define LPCAM_DMA_HANDSHAKE_ENABLE     0

#define LPCMP_DMA                      2
#define LPCMP_DMA_PERIPH_REQ           18
#define LPCMP_DMA_GROUP                0
#define LPCMP_DMA_HANDSHAKE_ENABLE     0

#if (!RTE_LPPDM_SELECT_DMA0)
#define LPPDM_DMA                      2
#define LPPDM_DMA_PERIPH_REQ           19
#define LPPDM_DMA_GROUP                0
#define LPPDM_DMA_HANDSHAKE_ENABLE     1
#endif

#if (!RTE_I3C_SELECT_DMA0)
#define I3C_DMA                        2
#define I3C_DMA_RX_PERIPH_REQ          20
#define I3C_DMA_TX_PERIPH_REQ          21
#define I3C_DMA_GROUP                  0
#define I3C_DMA_HANDSHAKE_ENABLE       1
#endif

#if (!RTE_CANFD_SELECT_DMA0)
#define CANFD_DMA                      2
#define CANFD_DMA_RX_PERIPH_REQ        22
#define CANFD_DMA_TX_PERIPH_REQ        23
#define CANFD_DMA_GROUP                0
#define CANFD_DMA_HANDSHAKE_ENABLE     0
#endif

#define LPGPIO_DMA                     2
#define LPGPIO_DMA_PIN0_PERIPH_REQ     24
#define LPGPIO_DMA_PIN1_PERIPH_REQ     25
#define LPGPIO_DMA_PIN2_PERIPH_REQ     26
#define LPGPIO_DMA_PIN3_PERIPH_REQ     27
#define LPGPIO_DMA_PIN4_PERIPH_REQ     28
#define LPGPIO_DMA_PIN5_PERIPH_REQ     29
#define LPGPIO_DMA_PIN6_PERIPH_REQ     30
#define LPGPIO_DMA_PIN7_PERIPH_REQ     31
#define LPGPIO_DMA_GROUP               0
#define LPGPIO_DMA_HANDSHAKE_ENABLE    0

#endif /* M55_HE */

#ifdef __cplusplus
}
#endif

#endif /* DMA_MAPPING_H */
