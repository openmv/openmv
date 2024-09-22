/**********************************************************************************************************************
 * Copyright (c) Prophesee S.A.                                                                                       *
 *                                                                                                                    *
 * Licensed under the Apache License, Version 2.0 (the "License");                                                    *
 * you may not use this file except in compliance with the License.                                                   *
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0                                 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed   *
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.                      *
 * See the License for the specific language governing permissions and limitations under the License.                 *
 **********************************************************************************************************************/

#ifndef __GENX320_H
#define __GENX320_H

/* Standard Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

/* User Includes*/
#include "genx320_all_pub_registers.h"
#include "psee_issd.h"

/* Sensor I2C Address */
#define I2C_ADDRESS 0x3C

/* Sensor Chip ID */
#define SAPHIR_CHIP_ID 0x0014
#define SAPHIR_ES_ID 0x30501C01
#define SAPHIR_MP_ID 0xB0602003

/* Memory Bank Defines */
#define IMEM_START	(0x00200000UL)
#define DMEM_START	(0x00300000UL)
#define MEM_BANK_SELECT	(0x0000F800UL)
#define MEM_BANK_MEM0	(0x0000F900UL)

/* RISC-V Defines */
#define RISC_BOOT_DONE 0x0ABCDEF

/**
  * @brief  RISCV FW status structures definition
  */
typedef enum
{
  fw_OK       				= 0U,
  fw_Error_InvalidSize 		= 1U,
  fw_Error_InvalidPointer 	= 2U,
  fw_Flash_Error			= 3U,
  fw_Start_Error			= 4U,
  fw_Flashed_Already		= 5U
} FW_StatusTypeDef;

/**
  * @brief  Shadow register status structures definition
  */
typedef enum
{
	shadow_valid       		= 0U,
	shadow_not_valid		= 01
} Shadow_StatusTypeDef;

/**
  * @brief Structure to hold the parameters for the bias
  */
typedef struct {
    uint8_t pr;
    uint8_t fo;
    uint8_t fes;
    uint8_t hpf;
    uint8_t diff_on;
    uint8_t diff;
    uint8_t diff_off;
    uint8_t inv;
    uint8_t refr;
    uint8_t invp;
    uint8_t req_pu;
    uint8_t sm_pdy;
}BIAS_Params_t;

/**
  * @brief Structure to hold the border value for the Activity Map
  */
typedef struct {
    uint16_t x0;
    uint16_t x1;
    uint16_t x2;
    uint16_t x3;
    uint16_t x4;
    uint16_t y0;
    uint16_t y1;
    uint16_t y2;
    uint16_t y3;
    uint16_t y4;
}AM_Borders_t;

/**
 * @brief Structure to hold the risc-v firmware
 */
typedef struct {
	uint32_t address;
	uint32_t value;
} Firmware;

/**
 * @brief Structure to hold AFK Invalidation
 */
typedef struct {
	uint32_t df_wait_time;
	uint32_t df_timeout;
} Invalidation;

/**
  * @brief Structure to hold the sensor clock frequencies
  */
typedef struct {
	uint32_t in_freq;
	uint32_t sys_freq;
	uint32_t evt_icn_freq;
	uint32_t cpu_ss_freq;
	uint32_t pll_in_freq;
	uint32_t pll_vco_freq;
	uint32_t phy_freq;
	uint32_t mipi_freq;
	uint32_t esc_freq;
} clk_freq_dict;

/**
  * @brief  Sensor Boot mode structure definition
  */
typedef enum
{
	ROM_BOOT       			= 0U,
	IMEM_BOOT				= 1U
} Boot_Mode;

/**
 * @brief Enum to hold the parameters for the bias
 */
typedef enum {
	PR,
	FO,
	FES,
	HPF,
	DIFF_ON,
	DIFF,
	DIFF_OFF,
	INV,
	REFR,
	INVP,
	REQ_PU,
	SM_PDY,
} BIAS_Name_t;

/* Constants/Variables to hold default bias */
extern const BIAS_Params_t genx320es_default_biases;
extern const BIAS_Params_t genx320mp_default_biases;
extern BIAS_Params_t genx320_default_biases;

/* Variable to hold default borders for Activity map */
extern AM_Borders_t genx320mp_default_am_borders;

/* ISSD sequences. */
extern const struct issd dcmi_evt;
extern const struct issd dcmi_histo;
extern const struct issd *current_issd;

/* Variable to hold the current boot mode */
extern Boot_Mode genx320_boot_mode;

/* Sensor Register Manipulation Functions */
extern void psee_sensor_read(uint16_t register_address, uint32_t *buf);
extern void psee_sensor_write(uint16_t register_address, uint32_t register_data);

/* Sensor Bring Up Functions */
extern void psee_sensor_init(const struct issd *);
extern void psee_sensor_destroy(const struct issd *);
extern void psee_sensor_start(const struct issd *);
extern void psee_sensor_stop(const struct issd *);
extern Boot_Mode psee_detect_boot_mode();
extern const struct issd *psee_open_evt();
extern void psee_sensor_set_bias(BIAS_Name_t bias, uint32_t val);
extern void psee_sensor_set_biases(const BIAS_Params_t *bias);
extern void psee_sensor_set_CPI_EVT20();
extern void psee_sensor_set_CPI_HISTO();
extern void psee_sensor_set_flip(uint32_t flip_x, uint32_t flip_y);
extern void psee_sensor_powerdown();

/* Operating Mode Control Functions */
extern void psee_PM3C_config();
extern void psee_PM2_config();

/* Firmware flashing functions */
extern FW_StatusTypeDef psee_write_firmware(const Firmware *firmwareArray, uint32_t n_bytes);
extern FW_StatusTypeDef psee_start_firmware(Boot_Mode boot_mode);
extern FW_StatusTypeDef psee_reset_firmware(Boot_Mode boot_mode);

/* Mailbox Communication Functions */
extern uint32_t psee_get_mbx_cmd_ptr();
extern uint32_t psee_get_mbx_misc();
extern uint32_t psee_get_mbx_status_ptr();
extern void psee_set_mbx_cmd_ptr(uint32_t val);
extern void psee_set_mbx_misc(uint32_t val);
extern void psee_set_mbx_status_ptr(uint32_t val);

/* ROI Access Functions */
extern void psee_write_ROI_X(uint32_t offset, uint32_t val);
extern void psee_write_ROI_Y(uint32_t offset, uint32_t val);
extern void psee_write_ROI_CTRL(uint32_t val);

/* RISC-V Debugger Functions */
extern uint32_t psee_mbx_read_uint32();
extern void psee_decode_debugger();
extern uint32_t psee_sensor_read_dmem(uint32_t address);
extern uint32_t psee_sensor_read_imem(uint32_t address);

/* Statistics Register Read Functions */
extern void psee_read_ro_lp_evt_cnt(uint32_t *p_data);
extern uint32_t psee_read_ro_evt_cd_cnt();
extern uint32_t psee_read_afk_flicker_evt_cnt();
extern uint32_t psee_read_afk_total_evt_cnt();

/* Activity Map Configuration Functions */
extern void psee_configure_activity_map();
extern void psee_set_default_XY_borders(AM_Borders_t *border);

/* AFK Configuration Functions */
extern void psee_enable_afk(uint16_t min_freq, uint16_t max_freq);
extern void psee_disable_afk();

#endif
