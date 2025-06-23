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
#include <math.h>
#include <stdint.h>
#include <stdio.h>

/* User Includes*/
#include "genx320_all_pub_registers.h"
#include "psee_issd.h"
#include "omv_csi.h"

/* Sensor I2C Address */
#define I2C_ADDRESS 	  0x3C

/* Sensor Chip ID */
#define SAPHIR_CHIP_ID  0x0014
#define SAPHIR_ES_ID    0x30501C01
#define SAPHIR_MP_ID    0xB0602003

/* Memory Bank Defines */
#define IMEM_START          0x00200000UL
#define DMEM_START          0x00300000UL
#define MEM_BANK_SELECT     0x0000F800UL
#define MEM_BANK_MEM0       0x0000F900UL
#define GENX_MEM_BANK_IMEM  0b10
#define GENX_MEM_BANK_DMEM  0b11

/* RISC-V Defines */
#define RISC_BOOT_DONE 	0x0ABCDEF

/**
  * @brief  GenX320 Histomode pin structure definition
  */
typedef enum {
    HM_EVT                  = 0U,
	HM_HISTO    			= 1U,
} GenX320_Histomode;

/**
  * @brief  GenX320 Mipimode pin structure definition
  */
typedef enum {
    MM_CPI                  = 0U,
	MM_MIPI    				= 1U,
} GenX320_Mipimode;

/**
  * @brief  GenX320 Rommode pin structure definition
  */
typedef enum {
    RM_IMEM                 = 0U,
	RM_ROM    				= 1U,
} GenX320_Rommode;

/**
 * @brief  GenX320 Modules structures definition
 */
typedef enum {
    EHC         = 0U,
    AFK         = 1U,
    STC         = 2U,
    ERC         = 3U,
    ERC_DFIFO   = 4U,
    ERC_ILG     = 5U,
    ERC_TDROP   = 6U,
    MIPI        = 7U,
    CPI         = 8U,
    CPU         = 9U,
} GenX320_ModulesTypeDef;

/**
 * @brief  RISCV FW status structures definition
 */
typedef enum {
    fw_OK                   = 0U,
    fw_Error_InvalidSize    = 1U,
    fw_Error_InvalidPointer = 2U,
    fw_Flash_Error          = 3U,
    fw_Start_Error          = 4U,
    fw_Flashed_Already      = 5U
} FW_StatusTypeDef;

/**
 * @brief  Shadow register status structures definition
 */
typedef enum {
    shadow_valid        = 0U,
    shadow_not_valid    = 01
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
} BIAS_Params_t;

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
} AM_Borders_t;

/**
 * @brief Structure to hold the risc-v firmware
 */
typedef struct {
    uint32_t address;
    uint32_t value;
} Firmware;

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

/**
 * @brief Structure to hold AFK Invalidation
 */
typedef struct {
    uint32_t dt_fifo_wait_time;
    uint32_t dt_fifo_timeout;
} AFK_InvalidationTypeDef;

/**
 * @brief AFK Init structures definition
 */
typedef enum {
	AFK_INIT_NOT_DONE	= 0x00U,
    AFK_INIT_DONE 		= 0x01U,
} AFK_InitTypeDef;

/**
 * @brief AFK Status structures definition
 */
typedef enum {
    AFK_OK        	    = 0x00U,
    AFK_BUSY            = 0x01U,
	AFK_ERROR		    = 0x02U,
	AFK_STATE_ERROR		= 0x03U,
    AFK_STATS_ERROR		= 0x04U,
	AFK_FREQ_ERROR		= 0x05U,
	AFK_CLK_ERROR		= 0x06U,
} AFK_StatusTypeDef;

/**
 * @brief AFK State structures definition
 */
typedef enum {
    AFK_STATE_RESET     = 0x00U,
    AFK_STATE_READY	    = 0x01U,
    AFK_STATE_BUSY      = 0x02U,
} AFK_StateTypeDef;

/**
 * @brief AFK Statistics structures definition
 */
typedef enum {
	AFK_STATS_RESET     = 0x00U,
    AFK_STATS_READY	    = 0x01U,
} AFK_StatisticsTypeDef;

/**
 * @brief AFK Handle Structure definition
 */
typedef struct
{
	omv_csi_t               *csi;
	AFK_InitTypeDef   		Init;		/*!< AFK Init */
	AFK_StateTypeDef    	State;		/*!< AFK State */
	AFK_StatisticsTypeDef 	Stats;		/*!< AFK Statistics */
	uint16_t 				f_min;		/*!< AFK Minimum frequency */
	uint16_t 				f_max;		/*!< AFK Maximum frequency */
    uint32_t                stats_acc;  /*!< AFK Statistics Accumulation time */
} AFK_HandleTypeDef;

/**
 * @brief EHC Init structures definition
 */
typedef enum {
	EHC_INIT_NOT_DONE   = 0x00U,
    EHC_INIT_DONE 	    = 0x01U,
} EHC_InitTypeDef;

/**
 * @brief  EHC Status structures definition
 */
typedef enum {
    EHC_OK        	    = 0x00U,
    EHC_BUSY            = 0x01U,
	EHC_ERROR		    = 0x02U,
    EHC_STATE_ERROR     = 0x03U,
    EHC_PARAM_ERROR	    = 0x04U,
} EHC_StatusTypeDef;

/**
 * @brief EHC State structures definition
 */
typedef enum {
    EHC_STATE_RESET     = 0x00U,
    EHC_STATE_READY	    = 0x01U,
    EHC_STATE_BUSY      = 0x02U,
} EHC_StateTypeDef;

/**
 * @brief EHC Algo structures definition
 */
typedef enum {
    EHC_ALGO_DIFF3D     = 0x00U,
    EHC_ALGO_HISTO3D    = 0x01U,
    EHC_ALGO_RESET      = 0x02U,
} EHC_AlgoTypeDef;

/**
 * @brief EHC Trigger structures definition
 */
typedef enum {
    EHC_TRIGGER_EVENT_RATE          = 0x00U,
    EHC_TRIGGER_INTEGRATION_PERIOD  = 0x01U,
    EHC_TRIGGER_RESET               = 0x02U,
} EHC_TriggerTypeDef;

/**
 * @brief EHC Padding structures definition
 */
typedef enum {
    EHC_WITHOUT_PADDING = 0x00U,
    EHC_WITH_PADDING    = 0x01U,
    EHC_PADDING_RESET   = 0x02U,
} EHC_PaddingTypeDef;

/**
 * @brief  EHC Override structures definition
 */
typedef enum {
    EHC_NO_OVERRIDE     = 0x00U,
    EHC_OVERRIDE        = 0x01U,
    EHC_OVERRIDE_RESET  = 0x02U,
} EHC_OverrideTypeDef;

/**
 * @brief EHC Handle Structure definition
 */
typedef struct
{
    omv_csi_t             *csi;
    EHC_InitTypeDef       Init;                 /*!< EHC Init */
    EHC_StateTypeDef  	  State;                /*!< EHC State */
    EHC_AlgoTypeDef       Algo;                 /*!< EHC Algo */
    EHC_TriggerTypeDef    Trigger;              /*!< EHC Trigger */
    EHC_PaddingTypeDef    Padding;              /*!< EHC Padding */
    EHC_OverrideTypeDef   Override;             /*!< EHC Override */
    uint32_t              accumulation_period;  /*!< EHC Accumulation Period */
    uint8_t               p_bits_size;          /*!< EHC Positive bits size */
    uint8_t               n_bits_size;          /*!< EHC Negative bits size */
} EHC_HandleTypeDef;

/**
 * @brief  STC Params structures definition
 */
typedef struct {
    uint16_t 	Presc;
    uint16_t 	Mult;
    uint16_t 	Timeout;
} STC_ParamsTypeDef;

/**
 * @brief STC Init structures definition
 */
typedef enum {
	STC_INIT_NOT_DONE	= 0x00U,
    STC_INIT_DONE 		= 0x01U,
} STC_InitTypeDef;

/**
 * @brief STC Status structures definition
 */
typedef enum {
    STC_OK 				= 0x00U,
    STC_BUSY 			= 0x01U,
    STC_ERROR 			= 0x02U,
    STC_STATE_ERROR 	= 0x03U,
    STC_THRESHOLD_ERROR = 0x04U,
    STC_CLK_ERROR 		= 0x05U,
} STC_StatusTypeDef;

/**
 * @brief STC State structures definition
 */
typedef enum {
    STC_STATE_RESET 	= 0x00U,
    STC_STATE_READY 	= 0x01U,
    STC_STATE_BUSY 		= 0x02U,
} STC_StateTypeDef;

/**
 * @brief STC Mode structures definition
 */
typedef enum {
    STC_MODE_RESET 		= 0x00U,
    STC_MODE_STC 		= 0x01U,
    STC_MODE_TRAIL 		= 0x02U,
    STC_MODE_STC_TRAIL 	= 0x03U,
} STC_ModeTypeDef;

/**
 * @brief STC Handle Structure definition
 */
typedef struct
{
    omv_csi_t           *csi;
    STC_InitTypeDef     Init;       /*!< STC block Init */
    STC_StateTypeDef    State;      /*!< STC block State */
    STC_ModeTypeDef     Mode;       /*!< STC Mode */
    STC_ParamsTypeDef   Params;     /*!< STC Params */
} STC_HandleTypeDef;

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
extern GenX320_Rommode genx320_boot_mode;

/* Sensor Register Manipulation Functions */
extern void psee_sensor_read(omv_csi_t *csi, uint16_t register_address, uint32_t *buf);
extern void psee_sensor_write(omv_csi_t *csi, uint16_t register_address, uint32_t register_data);

/* Sensor Bring Up Functions */
extern void psee_sensor_init(omv_csi_t *csi, const struct issd *);
extern void psee_sensor_start(omv_csi_t *csi, const struct issd *);
extern void psee_sensor_stop(omv_csi_t *csi, const struct issd *);
extern const struct issd *psee_open_evt(omv_csi_t *csi);
extern void psee_sensor_set_bias(omv_csi_t *csi, BIAS_Name_t bias, uint32_t val);
extern void psee_sensor_set_biases(omv_csi_t *csi, const BIAS_Params_t *bias);
extern void psee_sensor_set_CPI_EVT20(omv_csi_t *csi);
extern void psee_sensor_set_CPI_HISTO(omv_csi_t *csi);
extern void psee_sensor_set_flip(omv_csi_t *csi, uint32_t flip_x, uint32_t flip_y);
extern void psee_sensor_destroy(omv_csi_t *csi);

/* Operating Mode Control Functions */
extern void psee_PM3C_config(omv_csi_t *csi);
extern void psee_PM3C_Histo_config(omv_csi_t *csi);
extern void psee_PM2_config(omv_csi_t *csi);
extern void psee_PM2_Histo_config(omv_csi_t *csi);

/* Firmware flashing functions */
extern FW_StatusTypeDef psee_write_firmware_single(omv_csi_t *csi, const Firmware *firmwareArray, uint32_t n_bytes);
extern FW_StatusTypeDef psee_test_firmware_single(omv_csi_t *csi, const Firmware *firmwareArray, uint32_t n_bytes);
extern uint32_t psee_read_firmware_register(omv_csi_t *csi, uint32_t reg_address);
extern FW_StatusTypeDef psee_write_firmware_burst(omv_csi_t *csi, const Firmware *firmwareArray, uint32_t n_bytes);
extern FW_StatusTypeDef psee_start_firmware(omv_csi_t *csi, GenX320_Rommode boot_mode, uint32_t jump_add);
extern FW_StatusTypeDef psee_reset_firmware(omv_csi_t *csi, GenX320_Rommode boot_mode);

/* Mailbox Communication Functions */
extern uint32_t psee_get_mbx_cmd_ptr(omv_csi_t *csi);
extern uint32_t psee_get_mbx_misc(omv_csi_t *csi);
extern uint32_t psee_get_mbx_status_ptr(omv_csi_t *csi);
extern void psee_set_mbx_cmd_ptr(omv_csi_t *csi, uint32_t val);
extern void psee_set_mbx_misc(omv_csi_t *csi, uint32_t val);
extern void psee_set_mbx_status_ptr(omv_csi_t *csi, uint32_t val);

/* ROI Access Functions */
extern void psee_write_ROI_X(omv_csi_t *csi, uint32_t offset, uint32_t val);
extern void psee_write_ROI_Y(omv_csi_t *csi, uint32_t offset, uint32_t val);
extern void psee_write_ROI_CTRL(omv_csi_t *csi, uint32_t val);

/* RISC-V Debugger Functions */
extern uint32_t psee_mbx_read_uint32(omv_csi_t *csi);
extern void psee_decode_debugger(omv_csi_t *csi);
extern uint32_t psee_sensor_read_dmem(omv_csi_t *csi, uint32_t address);
extern uint32_t psee_sensor_read_imem(omv_csi_t *csi, uint32_t address);

/* Statistics Register Read Functions */
extern void psee_read_ro_lp_evt_cnt(omv_csi_t *csi, uint32_t *p_data);
extern uint32_t psee_read_ro_evt_cd_cnt(omv_csi_t *csi);

/* Activity Map Configuration Functions */
extern void psee_configure_activity_map(omv_csi_t *csi);
extern void psee_set_default_XY_borders(omv_csi_t *csi, AM_Borders_t *border);

/* AFK Configuration Functions */
extern AFK_StatusTypeDef psee_afk_init(omv_csi_t *csi, AFK_HandleTypeDef *afk);
extern AFK_StatusTypeDef psee_afk_activate(AFK_HandleTypeDef *afk,
                                           uint16_t f_min,
                                           uint16_t f_max,
                                           uint16_t evt_clk_freq);
extern AFK_StatusTypeDef psee_afk_deactivate(AFK_HandleTypeDef *afk);
extern AFK_StatusTypeDef psee_afk_start_statistics(AFK_HandleTypeDef *afk,
                                                   uint32_t acc_time);
extern AFK_StatusTypeDef psee_afk_stop_statistics(AFK_HandleTypeDef *afk);
extern uint32_t psee_afk_read_total_evt_cnt(AFK_HandleTypeDef *afk);
extern uint32_t psee_afk_read_flicker_evt_cnt(AFK_HandleTypeDef *afk);
extern uint16_t psee_afk_get_f_min(AFK_HandleTypeDef *afk);
extern uint16_t psee_afk_get_f_max(AFK_HandleTypeDef *afk);
extern AFK_StateTypeDef psee_afk_get_state(AFK_HandleTypeDef *afk);
extern AFK_StatisticsTypeDef psee_afk_get_stats(AFK_HandleTypeDef *afk);
extern uint32_t psee_afk_get_stats_acc_time(AFK_HandleTypeDef *afk);

/* EHC Configuration Functions */
extern const struct issd *psee_open_ehc(omv_csi_t *csi);
extern EHC_StatusTypeDef psee_ehc_init(omv_csi_t *csi, EHC_HandleTypeDef *ehc);
extern EHC_StatusTypeDef psee_ehc_activate(EHC_HandleTypeDef *ehc,
                                           EHC_AlgoTypeDef sel_algo,
                                           uint8_t p_bits_size,
                                           uint8_t n_bits_size,
                                           uint32_t integration_period,
                                           EHC_PaddingTypeDef bits_padding);
extern EHC_StatusTypeDef psee_ehc_deactivate(EHC_HandleTypeDef *ehc);
extern EHC_AlgoTypeDef psee_ehc_get_algo(EHC_HandleTypeDef *ehc);
extern uint8_t psee_ehc_get_p_bits_size(EHC_HandleTypeDef *ehc);
extern uint8_t psee_ehc_get_n_bits_size(EHC_HandleTypeDef *ehc);
extern uint32_t psee_ehc_get_accumulation_period(EHC_HandleTypeDef *ehc);
extern EHC_PaddingTypeDef psee_ehc_get_bits_padding(EHC_HandleTypeDef *ehc);
extern EHC_StatusTypeDef psee_ehc_activate_override(EHC_HandleTypeDef *ehc,
                                                    EHC_AlgoTypeDef sel_algo,
                                                    uint8_t p_bits_size,
                                                    uint8_t n_bits_size,
                                                    uint32_t integration_period,
                                                    EHC_PaddingTypeDef bits_padding,
                                                    EHC_OverrideTypeDef override);
extern EHC_StatusTypeDef psee_ehc_start_histo_acc(EHC_HandleTypeDef *ehc);
extern EHC_StatusTypeDef psee_ehc_drain_histo(EHC_HandleTypeDef *ehc);
extern void psee_close_ehc(omv_csi_t *csi, const struct issd *issd);

/* STC Configuration Functions */
extern STC_StatusTypeDef psee_stc_init(omv_csi_t *csi, STC_HandleTypeDef *stc);
extern STC_StatusTypeDef psee_stc_only_activate(STC_HandleTypeDef *stc,
                                                uint32_t stc_threshold,
                                                uint8_t evt_clk_freq);
extern STC_StatusTypeDef psee_trail_only_activate(STC_HandleTypeDef *stc,
                                                  uint32_t trail_threshold,
                                                  uint8_t evt_clk_freq);
extern STC_StatusTypeDef psee_stc_trail_activate(STC_HandleTypeDef *stc,
                                                 uint32_t stc_threshold,
                                                 uint32_t trail_threshold,
                                                 uint8_t evt_clk_freq);
extern STC_StatusTypeDef psee_stc_trail_deactivate(STC_HandleTypeDef *stc);
extern STC_StateTypeDef psee_stc_get_state(STC_HandleTypeDef *stc);
extern STC_ModeTypeDef psee_stc_get_mode(STC_HandleTypeDef *stc);
extern uint16_t psee_stc_get_params_mult(STC_HandleTypeDef *stc);
extern uint16_t psee_stc_get_params_presc(STC_HandleTypeDef *stc);
extern uint16_t psee_stc_get_params_timeout(STC_HandleTypeDef *stc);

#endif
