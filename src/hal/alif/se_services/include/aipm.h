/* SPDX-License-Identifier: GPL-2.0 */
/**
 * @file aipm.h
 * @brief Autonomous Intelligent Power Management API header
 * @defgroup host_services host_services
 * @par
 *
 * Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
#ifndef __AIPM_H__
#define __AIPM_H__

#include <stdint.h>

/* Power Domains Enumeration */
typedef enum  {
	PD0 = 0,
	PD1,
	PD2,
	PD3,
	PD4,
	PD5,
	PD6,
	PD7,
	PD8,
	PD9
} power_domain_t;

/* Power Domains Bit mask */
#define PD0_MASK (1 << PD0)    // bit0
#define PD1_MASK (1 << PD1)    // bit1
#define PD2_MASK (1 << PD2)    // bit2
#define PD3_MASK (1 << PD3)    // bit3
#define PD4_MASK (1 << PD4)    // bit4
#define PD5_MASK (1 << PD5)    // bit5
#define PD6_MASK (1 << PD6)    // bit6
#define PD7_MASK (1 << PD7)    // bit7
#define PD8_MASK (1 << PD8)    // bit8
#define PD9_MASK (1 << PD9)    // bit9

/* Power Domain Aliases */
#define PD_VBAT_AON      PD0    // bit0
#define PD_SRAM_CTRL_AON PD1    // bit1
#define PD_SSE700_AON    PD2    // bit2
#define PD_RTSS_HE       PD3    // bit3
#define PD_SRAMS         PD4    // bit4
#define PD_SESS          PD5    // bit5
#define PD_SYST          PD6    // bit6
#define PD_RTSS_HP       PD7    // bit7
#define PD_DBSS          PD8    // bit8
#define PD2_APPS         PD9    // bit9

/* Power Domain Aliases Bit mask */
#define PD_VBAT_AON_MASK      PD0_MASK    // bit0
#define PD_SRAM_CTRL_AON_MASK PD1_MASK    // bit1
#define PD_SSE700_AON_MASK    PD2_MASK    // bit2
#define PD_RTSS_HE_MASK       PD3_MASK    // bit3
#define PD_SRAMS_MASK         PD4_MASK    // bit4
#define PD_SESS_MASK          PD5_MASK    // bit5
#define PD_SYST_MASK          PD6_MASK    // bit6
#define PD_RTSS_HP_MASK       PD7_MASK    // bit7
#define PD_DBSS_MASK          PD8_MASK    // bit8
#define PD2_APPS_MASK         PD9_MASK    // bit9

/* LF Clock Sources */
typedef enum {
	CLK_SRC_LFRC = 0,
	CLK_SRC_LFXO,
} lfclock_t;

/* HF Clock Sources */
typedef enum {
	CLK_SRC_HFRC = 0,
	CLK_SRC_HFXO,
	CLK_SRC_PLL
} hfclock_t;

/* Clocks frequencies */
typedef enum {
	CLOCK_FREQUENCY_800MHZ,        /* Application CPU values */
	CLOCK_FREQUENCY_400MHZ,
	CLOCK_FREQUENCY_300MHZ,
	CLOCK_FREQUENCY_200MHZ,
	CLOCK_FREQUENCY_160MHZ,
	CLOCK_FREQUENCY_120MHZ,
	CLOCK_FREQUENCY_80MHZ,
	CLOCK_FREQUENCY_60MHZ,
	CLOCK_FREQUENCY_100MHZ,       /* Peripheral Clock values */
	CLOCK_FREQUENCY_50MHZ,
	CLOCK_FREQUENCY_20MHZ,
	CLOCK_FREQUENCY_10MHZ,
	CLOCK_FREQUENCY_76_8_RC_MHZ,  /* RC and XO clocks */
	CLOCK_FREQUENCY_38_4_RC_MHZ,
	CLOCK_FREQUENCY_76_8_XO_MHZ,
	CLOCK_FREQUENCY_38_4_XO_MHZ,
	CLOCK_FREQUENCY_DISABLED
} clock_frequency_t;

/* Scaled HFRC/HFXO clock frequencies*/
typedef enum {
	SCALED_FREQ_RC_ACTIVE_76_8_MHZ = 0,    /* HFRC frequencies in ACTIVE mode */
	SCALED_FREQ_RC_ACTIVE_38_4_MHZ,
	SCALED_FREQ_RC_ACTIVE_19_2_MHZ,
	SCALED_FREQ_RC_ACTIVE_9_6_MHZ,
	SCALED_FREQ_RC_ACTIVE_4_8_MHZ,
	SCALED_FREQ_RC_ACTIVE_2_4_MHZ,
	SCALED_FREQ_RC_ACTIVE_1_2_MHZ,
	SCALED_FREQ_RC_ACTIVE_0_6_MHZ,

	SCALED_FREQ_RC_STDBY_76_8_MHZ = 8,     /* HFRC frequencies in STANDBY mode */
	SCALED_FREQ_RC_STDBY_38_4_MHZ,
	SCALED_FREQ_RC_STDBY_19_2_MHZ,
	SCALED_FREQ_RC_STDBY_4_8_MHZ,
	SCALED_FREQ_RC_STDBY_1_2_MHZ,
	SCALED_FREQ_RC_STDBY_0_6_MHZ,
	SCALED_FREQ_RC_STDBY_0_3_MHZ,
	SCALED_FREQ_RC_STDBY_0_075_MHZ,

	SCALED_FREQ_XO_LOW_DIV_38_4_MHZ = 16,  /* HFXO frequencies using the LOW divider */
	SCALED_FREQ_XO_LOW_DIV_19_2_MHZ,
	SCALED_FREQ_XO_LOW_DIV_9_6_MHZ,
	SCALED_FREQ_XO_LOW_DIV_4_8_MHZ,
	SCALED_FREQ_XO_LOW_DIV_2_4_MHZ,
	SCALED_FREQ_XO_LOW_DIV_1_2_MHZ,
	SCALED_FREQ_XO_LOW_DIV_0_6_MHZ,
	SCALED_FREQ_XO_LOW_DIV_0_3_MHZ,

	SCALED_FREQ_XO_HIGH_DIV_38_4_MHZ = 24,  /* HFXO frequencies using the HIGH divider */
	SCALED_FREQ_XO_HIGH_DIV_19_2_MHZ,
	SCALED_FREQ_XO_HIGH_DIV_9_6_MHZ,
	SCALED_FREQ_XO_HIGH_DIV_2_4_MHZ,
	SCALED_FREQ_XO_HIGH_DIV_0_6_MHZ,
	SCALED_FREQ_XO_HIGH_DIV_0_3_MHZ,
	SCALED_FREQ_XO_HIGH_DIV_0_15_MHZ,
	SCALED_FREQ_XO_HIGH_DIV_0_0375_MHZ,
	SCALED_FREQ_NONE
} scaled_clk_freq_t;

#ifdef BALLETTO_DEVICE
/* Memory Blocks */
typedef enum {
  MB_SRAM2,
  MB_SRAM3,
  MB_SRAM4_1, // M55-HE ITCM RET1 itcm 64kb;
  MB_SRAM4_2, // M55-HE ITCM RET2 itcm 64kb;
  MB_SRAM4_3, // M55-HE ITCM RET3 itcm 128kb;
  MB_SRAM4_4, // M55-HE ITCM RET4 itcm 256kb;
  MB_SRAM5_1, // M55-HE DTCM RET1 dtcm 64kb
  MB_SRAM5_2, // M55-HE DTCM RET2 dtcm 64kb
  MB_SRAM5_3, // M55-HE DTCM RET3 dtcm 128kb
  MB_SRAM5_4, // M55-HE DTCM RET4 dtcm 256kb
  MB_SRAM5_5, // M55-HE DTCM RET5 dtcm 1024kb
  MB_MRAM,
  MB_OSPI0,
  MB_OSPI1,
  MB_SERAM_1,
  MB_SERAM_2,
  MB_SERAM_3,
  MB_SERAM_4,
  MB_FWRAM,
  MB_BACKUP4K
} memory_block_t;

/* Memory block bit mask */
#define SRAM2_MASK      (1 << MB_SRAM2)          // bit0
#define SRAM3_MASK      (1 << MB_SRAM3)          // bit1
#define SRAM4_1_MASK    (1 << MB_SRAM4_1)        // bit2
#define SRAM4_2_MASK    (1 << MB_SRAM4_2)        // bit3
#define SRAM4_3_MASK    (1 << MB_SRAM4_3)        // bit4
#define SRAM4_4_MASK    (1 << MB_SRAM4_4)        // bit5
#define SRAM5_1_MASK    (1 << MB_SRAM5_1)        // bit6
#define SRAM5_2_MASK    (1 << MB_SRAM5_2)        // bit7
#define SRAM5_3_MASK    (1 << MB_SRAM5_3)        // bit8
#define SRAM5_4_MASK    (1 << MB_SRAM5_4)        // bit9
#define SRAM5_5_MASK    (1 << MB_SRAM5_5)        // bit10
#define MRAM_MASK       (1 << MB_MRAM)           // bit11
#define OSPI0_MASK      (1 << MB_OSPI0)          // bit12
#define OSPI1_MASK      (1 << MB_OSPI1)          // bit13
#define SERAM_1_MASK    (1 << MB_SERAM_1)        // bit14
#define SERAM_2_MASK    (1 << MB_SERAM_2)        // bit15
#define SERAM_3_MASK    (1 << MB_SERAM_3)        // bit16
#define SERAM_4_MASK    (1 << MB_SERAM_4)        // bit17
#define FWRAM_MASK      (1 << MB_FWRAM)          // bit18
#define BACKUP4K_MASK   (1 << MB_BACKUP4K)       // bit19

#else
/* Memory Blocks */
typedef enum {
	MB_SRAM0 = 0,
	MB_SRAM1,
	MB_SRAM2,
	MB_SRAM3,
	MB_SRAM4_1, // M55-HE ITCM RET1 itcm 128kb;
	MB_SRAM4_2, // M55-HE ITCM RET2 itcm 128kb;
	MB_SRAM5_1, // M55-HE DTCM RET1 dtcm 128kb
	MB_SRAM5_2, // M55-HE DTCM RET2 dtcm 128kb
	MB_SRAM6A,  // Modem ITCM
	MB_SRAM6B,  // Modem DTCM
	MB_SRAM7_1, // XTENSA ITCM1
	MB_SRAM7_2, // XTENSA ITCM2
	MB_SRAM7_3, // XTENSA ITCM3
	MB_SRAM8,
	MB_SRAM9,
	MB_MRAM,
	MB_OSPI0,
	MB_OSPI1,
	MB_SERAM,
	MB_FWRAM,
	MB_BACKUP4K
} memory_block_t;

/* Memory block bit mask */
#define SRAM0_MASK      (1 << MB_SRAM0)          // bit0
#define SRAM1_MASK      (1 << MB_SRAM1)          // bit1
#define SRAM2_MASK      (1 << MB_SRAM2)          // bit2
#define SRAM3_MASK      (1 << MB_SRAM3)          // bit3
#define SRAM4_1_MASK    (1 << MB_SRAM4_1)        // bit4
#define SRAM4_2_MASK    (1 << MB_SRAM4_2)        // bit5
#define SRAM5_1_MASK    (1 << MB_SRAM5_1)        // bit6
#define SRAM5_2_MASK    (1 << MB_SRAM5_2)        // bit7
#define SRAM6A_MASK     (1 << MB_SRAM6A)         // bit8
#define SRAM6B_MASK     (1 << MB_SRAM6B)         // bit9
#define SRAM7_1_MASK    (1 << MB_SRAM7_1)        // bit10
#define SRAM7_2_MASK    (1 << MB_SRAM7_2)        // bit11
#define SRAM7_3_MASK    (1 << MB_SRAM7_3)        // bit12
#define SRAM8_MASK      (1 << MB_SRAM8)          // bit13
#define SRAM9_MASK      (1 << MB_SRAM9)          // bit14
#define MRAM_MASK       (1 << MB_MRAM)           // bit15
#define OSPI0_MASK      (1 << MB_OSPI0)          // bit16
#define OSPI1_MASK      (1 << MB_OSPI1)          // bit17
#define SERAM_MASK      (1 << MB_SERAM)          // bit18
#define FWRAM_MASK      (1 << MB_FWRAM)          // bit19
#define BACKUP4K_MASK   (1 << MB_BACKUP4K)       // bit20
#endif // #ifdef BALLETTO_DEVICE

/* PD0 Wakeup events */
#define WE_SERTC    (1 << 4)     // bit4
#define WE_LPRTC    (1 << 5)     // bit5
#define WE_LPCMP    (1 << 6)     // bit6
#define WE_BOD      (1 << 7)     // bit7
#define WE_LPTIMER0 (1 << 8)     // bit8
#define WE_LPTIMER1 (1 << 9)     // bit9
#define WE_LPTIMER2 (1 << 10)    // bit10
#define WE_LPTIMER3 (1 << 11)    // bit11
#define WE_LPGPIO0  (1 << 16)    // bit16
#define WE_LPGPIO1  (1 << 17)    // bit17
#define WE_LPGPIO2  (1 << 18)    // bit18
#define WE_LPGPIO3  (1 << 19)    // bit19
#define WE_LPGPIO4  (1 << 20)    // bit20
#define WE_LPGPIO5  (1 << 21)    // bit21
#define WE_LPGPIO6  (1 << 22)    // bit22
#define WE_LPGPIO7  (1 << 23)    // bit23

#define WE_LPTIMER  0XF00     // bit11:8
#define WE_LPGPIO   0XFF0000  // bit23:16

// EWIC
#ifdef BALLETTO_DEVICE
#define EWIC_RTC_SE                  (1)         // bit0
#define EWIC_ES0_WAKEUP              (1 << 1)    // bit1
#define EWIC_ES0_OSC_EN              (1 << 2)    // bit2
#define EWIC_ES0_RADIO_EN            (1 << 3)    // bit3
#define EWIC_RTC_A                   (1 << 4)    // bit4
#define EWIC_VBAT_TIMER              (0x001E0)   // bit8:5
#define EWIC_VBAT_GPIO               (0x1FE00)   // bit16:9
#define EWIC_VBAT_LP_CMP_IRQ         (1 << 17)   // bit17
#define EWIC_ES1_LP_I2C_IRQ          (1 << 18)   // bit18
#define EWIC_ES1_LP_UART_IRQ         (1 << 19)   // bit19
#define EWIC_BROWN_OUT               (1 << 20)   // bit20
#define EWIC_RTC_B                   (1 << 21)   // bit21

#else

#define EWIC_RTC_SE                  0x1         // bit0
#define EWIC_RTC_A                   0x40        // bit6
#define EWIC_VBAT_TIMER              0x780       // bit10:7
#define EWIC_VBAT_GPIO               0x7F800     // bit18:11
#define EWIC_VBAT_LP_CMP_IRQ         0x00080000UL// bit19
#define EWIC_ES1_LP_I2C_IRQ          0x00100000UL// bit20
#define EWIC_ES1_LP_UART_IRQ         0x00200000UL// bit21
#define EWIC_BROWN_OUT               0x00400000UL// bit22
#endif // #ifdef BALLETTO_DEVICE

typedef enum {
  DCDC_VOUT_0800 = 1,
  DCDC_VOUT_0825 = 2,
  DCDC_VOUT_0850 = 3
} dcdc_voltage_t;

typedef enum {
	DCDC_MODE_OFF = 0,
	DCDC_MODE_PFM_AUTO,
	DCDC_MODE_PFM_FORCED,
	DCDC_MODE_PWM
} dcdc_mode_t;

typedef enum {
	IOFLEX_LEVEL_3V3,
	IOFLEX_LEVEL_1V8
} ioflex_mode_t;

typedef enum {
	NPU_HP,
	NPU_HE,
	ISIM,
	OSPI_1,
	CANFD,
	SDC,
	USB,
	ETH,
	GPU,
	CDC200,
	CAMERA,
	MIPI_DSI,
	MIPI_CSI,
	LP_PERIPH
} ip_clock_gating_t;

#define NPU_HP_MASK       (1 << NPU_HP)   // bit0
#define NPU_HE_MASK       (1 << NPU_HE)   // bit1
#define OSPI_1_MASK       (1 << OSPI_1)   // bit3
#define CANFD_MASK        (1 << CANFD)    // bit4
#define SDC_MASK          (1 << SDC)      // bit5
#define USB_MASK          (1 << USB)      // bit6
#define ETH_MASK          (1 << ETH)      // bit7
#define GPU_MASK          (1 << GPU)      // bit8
#define CDC200_MASK       (1 << CDC200)   // bit9
#define CAMERA_MASK       (1 << CAMERA)   // bit10
#define MIPI_DSI_MASK     (1 << MIPI_DSI) // bit11
#define MIPI_CSI_MASK     (1 << MIPI_CSI) // bit12
#define LP_PERIPH_MASK    (1 << LP_PERIPH)// bit13

typedef enum {
	LDO_PHY,
	USB_PHY,
	MIPI_TX_DPHY,
	MIPI_RX_DPHY,
	MIPI_PLL_DPHY
} phy_gating_t;

#define LDO_PHY_MASK          (1 << LDO_PHY)       // bit0
#define USB_PHY_MASK          (1 << USB_PHY)       // bit1
#define MIPI_TX_DPHY_MASK     (1 << MIPI_TX_DPHY)  // bit2
#define MIPI_RX_DPHY_MASK     (1 << MIPI_RX_DPHY)  // bit3
#define MIPI_PLL_DPHY_MASK    (1 << MIPI_PLL_DPHY) // bit4

/* Power Management Data Structures */
typedef struct {
	uint32_t power_domains;         /* Power domains to stay on */
	uint32_t dcdc_voltage;          /* DCDC output voltage 750-850mv */
	dcdc_mode_t dcdc_mode;          /* DCDC mode - PWM or PFM based on the workload */
	lfclock_t aon_clk_src;          /* LFRC/LFXO */
	hfclock_t run_clk_src;          /* HFRC/HFXO/PLL */
	clock_frequency_t cpu_clk_freq; /* APSS/RTSS-HP/RTSS-HE specific setting */
	scaled_clk_freq_t scaled_clk_freq; /* Scaled HFRC/HFXO frequency */
	uint32_t memory_blocks;         /* Memories blocks to be retained/powered */
	uint32_t ip_clock_gating;       /* IP Clock Gating */
	uint32_t phy_pwr_gating;        /* PHY Power Gating */
	ioflex_mode_t vdd_ioflex_3V3;   /* Enable 3.3V GPIOs */
} run_profile_t;

typedef struct {
	uint32_t power_domains;         /* Power domains to stay on */
  uint32_t dcdc_voltage;          /* DCDC output voltage 750-850mv */
	dcdc_mode_t dcdc_mode;          /* DCDC mode - PWM or PFM based on the workload */
	lfclock_t aon_clk_src;          /* LFRC/LFXO */
	hfclock_t stby_clk_src;         /* HFRC/HFXO/PLL */
	scaled_clk_freq_t stby_clk_freq; /* Selected automatically in SoC Standby mode */
	uint32_t memory_blocks;          /* Memories blocks to be retained/powered */
	uint32_t ip_clock_gating;        /* IP Clock Gating */
	uint32_t phy_pwr_gating;         /* PHY Power Gating */
	ioflex_mode_t vdd_ioflex_3V3;    /* Flex GPIOs voltage */
	uint32_t wakeup_events;
	uint32_t ewic_cfg;
	uint32_t vtor_address;
	uint32_t vtor_address_ns;
} off_profile_t;

uint32_t SERVICES_get_run_cfg(uint32_t handle, run_profile_t *pp, uint32_t *error_code);
uint32_t SERVICES_set_run_cfg(uint32_t handle, run_profile_t *pp, uint32_t *error_code);
uint32_t SERVICES_get_off_cfg(uint32_t handle, off_profile_t *wp, uint32_t *error_code);
uint32_t SERVICES_set_off_cfg(uint32_t handle, off_profile_t *wp, uint32_t *error_code);

#endif
