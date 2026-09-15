/*******************************************************************************
  This module contains WINC1500 ASIC specific internal APIs.

  File Name:
    nmasic.h

  Summary:
    This module contains WINC1500 ASIC specific internal APIs.

  Description:
    This module contains WINC1500 ASIC specific internal APIs.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2022, Microchip Technology Inc., and its subsidiaries. All rights reserved.

The software and documentation is provided by microchip and its contributors
"as is" and any express, implied or statutory warranties, including, but not
limited to, the implied warranties of merchantability, fitness for a particular
purpose and non-infringement of third party intellectual property rights are
disclaimed to the fullest extent permitted by law. In no event shall microchip
or its contributors be liable for any direct, indirect, incidental, special,
exemplary, or consequential damages (including, but not limited to, procurement
of substitute goods or services; loss of use, data, or profits; or business
interruption) however caused and on any theory of liability, whether in contract,
strict liability, or tort (including negligence or otherwise) arising in any way
out of the use of the software and documentation, even if advised of the
possibility of such damage.

Except as expressly permitted hereunder and subject to the applicable license terms
for any third-party software incorporated in the software and any applicable open
source software license terms, no license or other rights, whether express or
implied, are granted under any patent or other intellectual property rights of
Microchip or any third party.
*/
// DOM-IGNORE-END

#ifndef _NMASIC_H_
#define _NMASIC_H_

#include "nm_common.h"

#define NMI_PERIPH_REG_BASE     0x1000
#define NMI_CHIPID              (NMI_PERIPH_REG_BASE)
#define rNMI_GP_REG_0           (0x149c)
#define rNMI_GP_REG_1           (0x14A0)
#define rNMI_GP_REG_2           (0xc0008)
#define rNMI_GLB_RESET          (0x1400)
#define rNMI_BOOT_RESET_MUX     (0x1118)
#define NMI_STATE_REG           (0x108c)
#define BOOTROM_REG             (0xc000c)
#define NMI_REV_REG             (0x207ac)   /*Also, Used to load ATE firmware from SPI Flash and to ensure that it is running too*/
#define NMI_REV_REG_ATE         (0x1048)    /*Revision info register in case of ATE FW*/
#define M2M_WAIT_FOR_HOST_REG   (0x207bc)
#define M2M_FINISH_INIT_STATE   0x02532636UL
#define M2M_FINISH_BOOT_ROM      0x10add09eUL
#define M2M_START_FIRMWARE       0xef522f61UL
#define M2M_START_PS_FIRMWARE    0x94992610UL

#define M2M_ATE_FW_START_VALUE  (0x3C1CD57D)    /*Also, Change this value in boot_firmware if it will be changed here*/
#define M2M_ATE_FW_IS_UP_VALUE  (0xD75DC1C3)    /*Also, Change this value in ATE (Burst) firmware if it will be changed here*/

#define REV_2B0        (0x2B0)
#define REV_B0         (0x2B0)
#define REV_3A0        (0x3A0)
#define GET_CHIPID()    nmi_get_chipid()
#define ISNMC1000(id)   ((((id) & 0xfffff000) == 0x100000) ? 1 : 0)
#define ISNMC1500(id)   ((((id) & 0xfffff000) == 0x150000) ? 1 : 0)
#define ISNMC3000(id)   ((((id) & 0xfff00000) == 0x300000) ? 1 : 0)
#define REV(id)         (((id) & 0x00000fff ))
#define EFUSED_MAC(value) (value & 0xffff0000)

#define rHAVE_SDIO_IRQ_GPIO_BIT     (NBIT0)
#define rHAVE_USE_PMU_BIT           (NBIT1)
#define rHAVE_SLEEP_CLK_SRC_RTC_BIT (NBIT2)
#define rHAVE_SLEEP_CLK_SRC_XO_BIT  (NBIT3)
#define rHAVE_EXT_PA_INV_TX_RX      (NBIT4)
#define rHAVE_LEGACY_RF_SETTINGS    (NBIT5)
#define rHAVE_LOGS_DISABLED_BIT     (NBIT6)
#define rHAVE_ETHERNET_MODE_BIT     (NBIT7)
#define rHAVE_RESERVED1_BIT         (NBIT8)
#define rHAVE_RESERVED2_BIT         (NBIT9)
#define rHAVE_XO_XTALGM2_DIS_BIT    (NBIT10)

typedef struct{
    uint32_t u32Mac_efuse_mib;
    uint32_t u32Firmware_Ota_rev;
}tstrGpRegs;

#ifdef __cplusplus
     extern "C" {
#endif

/*
*   @fn     cpu_halt
*   @brief
*/
int8_t cpu_halt(void);
/*
*   @fn     chip_sleep
*   @brief
*/
int8_t chip_sleep(void);
/*
*   @fn     chip_wake
*   @brief
*/
int8_t chip_wake(void);
/*
*   @fn     chip_idle
*   @brief
*/
void chip_idle(void);
/*
*   @fn     enable_interrupts
*   @brief
*/
int8_t enable_interrupts(void);
/*
*   @fn     cpu_start
*   @brief
*/
int8_t cpu_start(void);
/*
*   @fn     nmi_get_chipid
*   @brief
*/
uint32_t nmi_get_chipid(void);
/*
*   @fn     nmi_get_rfrevid
*   @brief
*/
uint32_t nmi_get_rfrevid(void);
/*
*   @fn     restore_pmu_settings_after_global_reset
*   @brief
*/
void restore_pmu_settings_after_global_reset(void);
/*
*   @fn     nmi_update_pll
*   @brief
*/
void nmi_update_pll(void);
/*
*   @fn     nmi_set_sys_clk_src_to_xo
*   @brief
*/
void nmi_set_sys_clk_src_to_xo(void);
/*
*   @fn     chip_reset
*   @brief
*/
int8_t chip_reset(void);
/*
*   @fn     wait_for_bootrom
*   @brief
*/
int8_t wait_for_bootrom(uint8_t);
/*
*   @fn     wait_for_firmware_start
*   @brief
*/
int8_t wait_for_firmware_start(uint8_t);
/*
*   @fn     chip_deinit
*   @brief
*/
int8_t chip_deinit(void);
/*
*   @fn     set_gpio_dir
*   @brief
*/
int8_t set_gpio_dir(uint8_t gpio, uint8_t dir);
/*
*   @fn     set_gpio_val
*   @brief
*/
int8_t set_gpio_val(uint8_t gpio, uint8_t val);
/*
*   @fn     get_gpio_val
*   @brief
*/
int8_t get_gpio_val(uint8_t gpio, uint8_t* val);
/*
*   @fn     pullup_ctrl
*   @brief
*/
int8_t pullup_ctrl(uint32_t pinmask, uint8_t enable);
/*
*   @fn     nmi_get_otp_mac_address
*   @brief
*/
int8_t nmi_get_otp_mac_address(uint8_t *pu8MacAddr, uint8_t * pu8IsValid);
/*
*   @fn     nmi_get_mac_address
*   @brief
*/
int8_t nmi_get_mac_address(uint8_t *pu8MacAddr);
/*
*   @fn     chip_apply_conf
*   @brief
*/
int8_t chip_apply_conf(uint32_t u32conf);

#ifdef __cplusplus
     }
#endif

#endif  /*_NMASIC_H_*/
