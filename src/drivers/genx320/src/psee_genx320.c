/**
  ******************************************************************************
  * @file    genx320.c
  * @author  PSEE Applications Team
  * @brief   GenX320 driver.
  *          This file provides functions to manage the following
  *          functionalities of the GenX320 sensor:
  *           + Sensor Register Manipulation
  *           + Sensor Bring Up
  *           + Operating Mode Control
  *           + Firmware Flashing
  *           + Mailbox Communication
  *           + ROI Access
  *           + RISC-V Debugger
  *           + Read Statistics Register
  *           + Activity Map Configuration
  *           + AFK Configuration
  *
  ******************************************************************************
  * @attention
  * Copyright (c) Prophesee S.A.
  *
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
  * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
  * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "psee_genx320.h"
#include "sensor.h"
#include "py/mphal.h"

/** @defgroup GenX320 - Sensor Register Manipulation Functions
 *  @brief    Sensor Register Manipulation Functions
 *
@verbatim
 ===============================================================================
                      ##### Sensor Register Manipulation Functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) To perform single register access
      (+) To perform sequential register access

@endverbatim
  * @{
  */

/**
  * @brief  Platform dependent function to perform a single-read operation from the sensor's register.
  * @param  register_address Sensor's register from which the data needs to be read
  * @param	buf Pointer to the variable where the data needs to be stored
  */
void psee_sensor_read(uint16_t register_address, uint32_t *buf) {
	uint8_t addr[] = {(register_address >> 8), register_address};
	omv_i2c_write_bytes(&sensor.i2c_bus, sensor.slv_addr, addr, 2, OMV_I2C_XFER_NO_STOP);
	omv_i2c_read_bytes(&sensor.i2c_bus, sensor.slv_addr, (uint8_t *) buf, 4, OMV_I2C_XFER_NO_FLAGS);
	*buf = __REV(*buf);
};

/**
  * @brief  Platform dependent function to perform a single-write operation to the sensor's register.
  * @param	register_address Sensor's register to which the data needs to be written
  * @param  register_data Data to be written
  */
void psee_sensor_write(uint16_t register_address, uint32_t register_data) {
	uint8_t buf[] = {(register_address >> 8), register_address,
					 (register_data >> 24), (register_data >> 16), (register_data >> 8), register_data};
	omv_i2c_write_bytes(&sensor.i2c_bus, sensor.slv_addr, buf, 6, OMV_I2C_XFER_NO_FLAGS);
};

/**
  * @brief  Platform dependent function to perform a sequential-read operation from the sensor's register.
  * @param  reg Sensor's starting register address from which the data needs to be read
  * @param	data Pointer to the array where the data needs to be stored
  * @param  n_word Total number of 32-bits that needs to be read
  */
void psee_sensor_sequential_read(uint16_t reg, uint32_t *data, uint16_t n_word) {
	uint8_t buf[] = {(reg >> 8), reg};
	omv_i2c_write_bytes(&sensor.i2c_bus, sensor.slv_addr, buf, sizeof(buf), OMV_I2C_XFER_NO_STOP);
	omv_i2c_read_bytes(&sensor.i2c_bus, sensor.slv_addr, (uint8_t *) data, n_word * sizeof(uint32_t),
					   OMV_I2C_XFER_NO_FLAGS);
	for (int32_t i = 0; i < n_word; i++) {
		data[i] = __builtin_bswap32(data[i]);
	}
}

/**
  * @brief  Platform dependent function to perform a sequential-write operation to the sensor's register.
  * @param  register_data Address of the data array that needs to be written
  * @param  n_bytes Total number of bytes that needs to be written
  */
void psee_sensor_sequential_write(uint8_t *register_data, uint16_t n_bytes) {
	omv_i2c_write_bytes(&sensor.i2c_bus, sensor.slv_addr, register_data, n_bytes, OMV_I2C_XFER_NO_FLAGS);
}

/**
  * @}
  */

/** @defgroup GenX320 - Sensor Bring Up Functions
 *  @brief    Sensor Bring Up Functions
 *
@verbatim
 ===============================================================================
                      ##### Sensor Bring Up Functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Initialize the platform
      (+) Implement platform specific delay function
      (+) Initialize the Sensor
      (+) Execute Operation sequence
	  (+) Execute ISSD sequence
	  (+) Init sensor in EVT mode
	  (+) Init sensor in Histo mode
      (+) Set & Change Bias
      (+) Set CPI
      (+) Set Flip

@endverbatim
  * @{
  */

/**
  * @brief  Platform dependent initialization routine, this can include Sensor reset, Sensor power up, etc.
  */
void psee_sensor_platform_init() {
};

/**
  * @brief  Platform dependent delay function.
  * @param  duration Delay duration in milliseconds
  */
void psee_sleep_ms_imp(uint32_t duration) {
	mp_hal_delay_ms(duration);
};

/**
  * @brief  Platform dependent delay function.
  * @param  duration Delay duration in microseconds
  */
void psee_sleep_us_imp(uint32_t duration) {
	mp_hal_delay_us(duration);
};

/**
  * @brief Function to execute the operation sequence.
  * @param seq Operation sequence that needs to be executed
  */
static void execute_reg_op_sequence(const struct sequence seq) {
	for(unsigned int i = 0; i < seq.len; i++) {
		if(seq.ops[i].op == WRITE) {
			psee_sensor_write(seq.ops[i].args.write.addr, seq.ops[i].args.write.data);
		}
		else if (seq.ops[i].op == READ) {
			uint32_t data = 0;
			psee_sensor_read(seq.ops[i].args.write.addr, &data);
			(void)data;
			// if (data == seq.ops[i].args.read.data){}
		}
		else if (seq.ops[i].op == DELAY) {
			uint32_t delay_ms = seq.ops[i].args.delay.us/1000<1?1:seq.ops[i].args.delay.us/1000;

			psee_sleep_ms_imp(delay_ms);

		}
		psee_sleep_ms_imp(1);
	}
}

/**
  * @brief Function to execute the Init sequence of the ISSD.
  */
void psee_sensor_init(const struct issd *issd) {
	if (issd)
		execute_reg_op_sequence(issd->init);
}

/**
  * @brief Function to execute the Destroy sequence of the ISSD.
  */
void psee_sensor_destroy(const struct issd *issd) {
	if (issd)
		execute_reg_op_sequence(issd->destroy);
}

/**
  * @brief Function to execute the Start sequence of the ISSD.
  */
void psee_sensor_start(const struct issd *issd) {
	if (issd)
		execute_reg_op_sequence(issd->start);
}

/**
  * @brief Function to execute the Stop sequence of the ISSD.
  */
void psee_sensor_stop(const struct issd *issd) {
	if (issd)
		execute_reg_op_sequence(issd->stop);
}

/**
  * @brief Function to detect the boot mode of the sensor.
  */
Boot_Mode psee_detect_boot_mode() {

	Boot_Mode boot_mode = 0;

	/* Read the boot message */
	uint32_t boot_value = psee_get_mbx_misc();

	if(boot_value == 0) {
		boot_mode = IMEM_BOOT;
	}
	else if((boot_value == 0xCAFEBABE) || (boot_value == 0xBAADF00D)) {
		boot_mode = ROM_BOOT;
	}

	return boot_mode;
}

/**
  * @brief Function to enable and configure the sensor in Event streaming mode.
  */
BIAS_Params_t genx320_default_biases;
Boot_Mode genx320_boot_mode;
const struct issd *psee_open_evt() {

	/* Setup I2C */
	psee_sensor_platform_init();
	psee_sleep_ms_imp(100);

   /* Check chip ID */
   uint32_t chip_id;
   psee_sensor_read(SAPHIR_CHIP_ID, &chip_id);
   printf("Sensor ID : %lX \n\r",chip_id);
   if ((chip_id != SAPHIR_MP_ID) && (chip_id != SAPHIR_ES_ID)) {
       while(1);
   }

   /* Detect the Boot mode */
   genx320_boot_mode = psee_detect_boot_mode();

   if(chip_id == SAPHIR_ES_ID) {
	   genx320_default_biases = genx320es_default_biases;
   } else {
	   genx320_default_biases = genx320mp_default_biases;
   }

   /* Force CPI with chicken bits */
   psee_sensor_write(TOP_CHICKEN, 0x03E8000A); //EVT

   /* Start the Init sequence */
   psee_sensor_init(&dcmi_evt);

   /* Configure CPI for event streaming */
   psee_sensor_set_CPI_EVT20();

   return &dcmi_evt;

}

/**
  * @brief Default bias parameters for ES sensor
  */
const BIAS_Params_t genx320es_default_biases = {
	.pr       		= 61,
	.fo         	= 19,
	.fes        	= 63,
	.hpf        	= 0,
	.diff_on    	= 40,
	.diff       	= 51,
	.diff_off 		= 28,
	.diff       	= 51,
	.inv 			= 57,
	.refr        	= 82,
	.invp           = 66,
	.req_pu 		= 116,
	.sm_pdy 		= 164
};

/**
  * @brief Default bias parameters for MP sensor
  */
const BIAS_Params_t genx320mp_default_biases = {
	.pr       		= 61,
	.fo         	= 34,
	.fes            = 63,
	.hpf        	= 0,
	.diff_on    	= 30,
	.diff       	= 51,
	.diff_off 		= 33,
	.inv 			= 57,
	.refr			= 10,
	.invp           = 56,
	.req_pu 		= 116,
	.sm_pdy 		= 164
};

/**
  * @brief Function to set the standard biases
  */
void psee_sensor_set_biases(const BIAS_Params_t *bias) {

	/* PR */
	psee_sensor_write(BIAS_BIAS_PR_HV0, 		(0x03010000 + bias->pr) | BIAS_PR_HV0_SINGLE_Msk);
	/* FO */
  	psee_sensor_write(BIAS_BIAS_FO_HV0, 		(0x03010000 + bias->fo) | BIAS_FO_HV0_SINGLE_Msk);
  	/* FES */
  	psee_sensor_write(BIAS_BIAS_FES_HV0, 		(0x01010000 + bias->fes) | BIAS_FES_HV0_SINGLE_Msk);
  	/* HPF */
  	psee_sensor_write(BIAS_BIAS_HPF_LV0, 		(0x03010000 + bias->hpf) | BIAS_HPF_LV0_SINGLE_Msk);
  	/* DIFF ON */
  	psee_sensor_write(BIAS_BIAS_DIFF_ON_LV1, 	(0x01010000 + bias->diff_on) | BIAS_DIFF_ON_LV1_SINGLE_Msk);
  	/* DIFF */
  	psee_sensor_write(BIAS_BIAS_DIFF_LV1, 		(0x01010000 + bias->diff) | BIAS_DIFF_LV1_SINGLE_Msk);
  	/* DIFF OFF */
  	psee_sensor_write(BIAS_BIAS_DIFF_OFF_LV1,	(0x01010000 + bias->diff_off) | BIAS_DIFF_OFF_LV1_SINGLE_Msk);
  	/* INV */
  	psee_sensor_write(BIAS_BIAS_INV_LV0, 		(0x01010000 + bias->inv) | BIAS_INV_LV0_SINGLE_Msk);
  	/* REFR */
  	psee_sensor_write(BIAS_BIAS_REFR_LV0, 		(0x03010000 + bias->refr) | BIAS_REFR_LV0_SINGLE_Msk);
  	/* INVP */
	psee_sensor_write(BIAS_BIAS_INVP_LV0, 		(0x03010000 + bias->invp) | BIAS_INVP_LV0_SINGLE_Msk);
	/* REQ_PU */
	psee_sensor_write(BIAS_BIAS_REQ_PU_LV0, 	(0x03010000 + bias->req_pu) | BIAS_REQ_PU_LV0_SINGLE_Msk);
	/* SM_PDy */
	psee_sensor_write(BIAS_BIAS_SM_PDY_LV0, 	(0x01010000 + bias->sm_pdy) | BIAS_SM_PDY_LV0_SINGLE_Msk);

}

/**
  * @brief Function to change the Bias setting
  */
void psee_sensor_set_bias(BIAS_Name_t bias, uint32_t val) {
	switch(bias) {
	case PR:
		psee_sensor_write(BIAS_BIAS_PR_HV0, 		(0x03010000 + val) | BIAS_PR_HV0_SINGLE_Msk);
		break;
	case FO:
		psee_sensor_write(BIAS_BIAS_FO_HV0, 		(0x03010000 + val) | BIAS_FO_HV0_SINGLE_Msk);
		break;
	case FES:
		psee_sensor_write(BIAS_BIAS_FES_HV0, 		(0x01010000 + val) | BIAS_FES_HV0_SINGLE_Msk);
		break;
	case HPF:
		psee_sensor_write(BIAS_BIAS_HPF_LV0, 		(0x03010000 + val) | BIAS_HPF_LV0_SINGLE_Msk);
		break;
	case DIFF_ON:
		psee_sensor_write(BIAS_BIAS_DIFF_ON_LV1, 	(0x01010000 + val) | BIAS_DIFF_ON_LV1_SINGLE_Msk);
		break;
	case DIFF:
		psee_sensor_write(BIAS_BIAS_DIFF_LV1, 		(0x01010000 + val) | BIAS_DIFF_LV1_SINGLE_Msk);
		break;
	case DIFF_OFF:
		psee_sensor_write(BIAS_BIAS_DIFF_OFF_LV1,	(0x01010000 + val) | BIAS_DIFF_OFF_LV1_SINGLE_Msk);
		break;
	case INV:
		psee_sensor_write(BIAS_BIAS_INV_LV0, 		(0x01010000 + val) | BIAS_INV_LV0_SINGLE_Msk);
		break;
	case REFR:
		psee_sensor_write(BIAS_BIAS_REFR_LV0, 		(0x03010000 + val) | BIAS_REFR_LV0_SINGLE_Msk);
		break;
	case INVP:
		psee_sensor_write(BIAS_BIAS_INVP_LV0, 		(0x03010000 + val) | BIAS_INVP_LV0_SINGLE_Msk);
		break;
	case REQ_PU:
		psee_sensor_write(BIAS_BIAS_REQ_PU_LV0, 	(0x03010000 + val) | BIAS_REQ_PU_LV0_SINGLE_Msk);
		break;
	case SM_PDY:
		psee_sensor_write(BIAS_BIAS_SM_PDY_LV0, 	(0x01010000 + val) | BIAS_SM_PDY_LV0_SINGLE_Msk);
		break;
	default:
		break;
	}
}

/**
  * @brief Function to configure the CPI and configure the sensor data format in EVT2.0
  */
void psee_sensor_set_CPI_EVT20() {

	/* Set EVT20 mode */
  	psee_sensor_write(EDF_CONTROL, 				0x00UL);											/* Event Format : Legacy EVT2.0 */

	/* Configure Packet and Frame sizes */
  	psee_sensor_write(CPI_FRAME_SIZE_CONTROL, 	0x4UL);												/* Frame Size in Packets : 4 Packets */

	/* Polarities fr VSYNC & HSYNC */
  	psee_sensor_write(CPI_OUTPUT_IF_CONTROL, 	CPI_OUTPUT_IF_CONTROL_HSYNC_POL |					/* HSYNC polarity control : Synchronize at active high */
  												CPI_OUTPUT_IF_CONTROL_VSYNC_POL						/* VSYNC polarity control : Synchronize at active high */
												);

	/* Enable dropping */
  	psee_sensor_write(RO_READOUT_CTRL, 			RO_READOUT_CTRL_DIGITAL_PIPE_EN |					/* Enable Whole Readout block */
  												RO_READOUT_CTRL_AVOID_BPRESS_TD |					/* Avoid back pressure propagation when readout is busy */
												RO_READOUT_CTRL_DROP_EN |							/* Drop all events at the output of the RO except first events and timer high */
												RO_READOUT_CTRL_DROP_ON_FULL_EN						/* Drop a packet only if cannot be accepted by the following stage */
												);

  	/* Configure CPI Pipeline */
	psee_sensor_write(CPI_PIPELINE_CONTROL, 	CPI_PIPELINE_CONTROL_ENABLE |						/* Enable Output Interface */
												CPI_PIPELINE_CONTROL_OUTPUT_IF_MODE |				/* Output Interface Mode : DCMI */
												CPI_PIPELINE_CONTROL_PACKET_FIXED_SIZE_ENABLE |		/* Fixed Packet Size */
												CPI_PIPELINE_CONTROL_FRAME_FIXED_SIZE_ENABLE |		/* Fixed Frame Size */
												CPI_PIPELINE_CONTROL_CLK_OUT_EN						/* Enable Clock Out */
												);
}

/**
  * @brief Function to flip
  * @param flip_x To indicate that x axis needs to be flipped
  * @param flip_y To indicate that y axis needs to be flipped
  */
void psee_sensor_set_flip(uint32_t flip_x, uint32_t flip_y) {
	uint32_t reg_val;

	/* Read the old value*/
	psee_sensor_read(RO_READOUT_CTRL, &reg_val );
	reg_val &= 0x003FF6FF;

	/* Check if it is a X axis flip*/
	if(flip_x)	reg_val |= (flip_x << 6);
	else 		reg_val &= ~(flip_x << 6);

	/* Check if it is a Y axis flip*/
	if(flip_y) 	reg_val |= (flip_y << 7);
	else 		reg_val &= ~(flip_y << 7);

	/* Update the new value*/
	psee_sensor_write(RO_READOUT_CTRL, reg_val);
}

/**
  * @brief Function to execute mipi_phy power down sequence.
  */
static void mipi_dphy_power_down() {

	/* MIPI Power and Clock control */
	psee_sensor_write(MIPI_CSI_POWER,		(0UL << MIPI_CSI_POWER_CL_ENABLE_Pos) |					/* Clock lane power up input signal */
											(0UL << MIPI_CSI_POWER_DL_ENABLE_Pos)					/* Data lane power up input signal */
											);

	psee_sensor_write(MIPI_CSI_CLK_CTRL,	(0UL << MIPI_CSI_CLK_CTRL_STBUS_EN_Pos)  |				/* STbus clock */
											(0UL << MIPI_CSI_CLK_CTRL_CL_CLKESC_EN_Pos)  |			/* Clock Lane escape clock */
											(0UL << MIPI_CSI_CLK_CTRL_DL_CLKESC_EN_Pos)				/* Data Lane escape clock */
											);

	psee_sensor_write(MIPI_CSI_POWER,		(0UL << MIPI_CSI_POWER_CL_RSTN_Pos) |					/* Asynchronous Reset of Clock Lane */
											(0UL << MIPI_CSI_POWER_DL_RSTN_Pos) 					/* Asynchronous Reset of Data Lane */
											);

	psee_sensor_write(MIPI_CSI_CLK_CTRL,	(0UL << MIPI_CSI_CLK_CTRL_TXCLKESC_EN_Pos) );			/* Escape clock */

	/* Disable Calibration */
	psee_sensor_write(LDO_PMU,				(0UL << LDO_PMU_V2I_CAL_EN_Pos) );						/* Calibration to adjust resistor value */

	psee_sensor_write(ADC_CONTROL,			(0UL << ADC_CONTROL_CLK_EN_Pos) );						/* ADC Clock */

	psee_sensor_write(MIPI_CSI_POWER,		(0UL << MIPI_CSI_POWER_CUR_EN_Pos) );					/* MIPI Bias */

	/* Disable pmu_igcm, v2i, mipi_v2i */
	psee_sensor_write(LDO_PMU,				(0UL << LDO_PMU_ICGM_EN_Pos) |							/* CGM Current */
											(0UL << LDO_PMU_V2I_EN_Pos) |							/* V2i */
											(0UL << LDO_PMU_MIPI_V2I_EN_Pos)						/* Reference bias current for MIPI V2i */
											);

	/* Disable BG */
	psee_sensor_write(LDO_BG,				(0UL << LDO_BG_EN_Pos) |								/* Bandgap */
											(0UL << LDO_BG_BUF_EN_Pos)								/* Bandgap Buffer */
											);


}

/**
  * @brief Function to read all the sensor clock tree registers and compute the value of each clocks.
  * @args in_freq Sensor input clock in MHz
  */
static clk_freq_dict get_sensor_clk_freq(uint32_t in_freq) {

	/* Create a Instance of Clock frequency structure */
	clk_freq_dict clk_freq;

	/* Reset the values */
	clk_freq.in_freq = in_freq;
	clk_freq.sys_freq = 0;
	clk_freq.evt_icn_freq = 0;
	clk_freq.cpu_ss_freq = 0;
	clk_freq.pll_in_freq = 0;
	clk_freq.pll_vco_freq = 0;
	clk_freq.phy_freq = 0;
	clk_freq.mipi_freq = 0;
	clk_freq.esc_freq = 0;

	/* Get sys clock freq */
	uint32_t sys_clk_ctrl = 0, sys_clk_switch = 0, sys_clk_auto_mode = 0;
	psee_sensor_read(SYS_CLK_CTRL, &sys_clk_ctrl);
	sys_clk_switch = (sys_clk_ctrl & SYS_CLK_CTRL_SWITCH_Msk) >> SYS_CLK_CTRL_SWITCH_Pos;

	if(sys_clk_switch == 0) {
		clk_freq.sys_freq = clk_freq.in_freq;
	}
	else {
		/* Get all PLL freq */
		uint32_t ref_clk_ctrl = 0, ref_clk_switch = 0, ref_clk_div = 0;
		psee_sensor_read(REF_CLK_CTRL, &ref_clk_ctrl);
		ref_clk_switch = (ref_clk_ctrl & REF_CLK_CTRL_SWITCH_Msk) >> REF_CLK_CTRL_SWITCH_Pos;

		if(ref_clk_switch == 0) {
			clk_freq.pll_in_freq = clk_freq.in_freq;
		}
		else {
			ref_clk_div = (ref_clk_ctrl & REF_CLK_CTRL_DIV_Msk) >> REF_CLK_CTRL_DIV_Pos;
			clk_freq.pll_in_freq = clk_freq.in_freq / ref_clk_div;
		}

		uint32_t pll_ctrl = 0, pl_ndiv = 0, pl_odf = 0;
		psee_sensor_read(PLL_CTRL, &pll_ctrl);
		pl_ndiv = (pll_ctrl & PLL_CTRL_PL_NDIV_Msk) >> PLL_CTRL_PL_NDIV_Pos;
		pl_odf = (pll_ctrl & PLL_CTRL_PL_ODF_Msk) >> PLL_CTRL_PL_ODF_Pos;
		clk_freq.pll_vco_freq = clk_freq.pll_in_freq * pl_ndiv * 2;
		clk_freq.phy_freq = clk_freq.pll_vco_freq / 2;
		clk_freq.mipi_freq = clk_freq.pll_vco_freq / pl_odf;

		uint32_t phy_clk_div2 = 0, phy_clk_off_count = 0 , phy_freq_divider = 0;
		phy_clk_div2 = (sys_clk_ctrl & SYS_CLK_CTRL_PHY_DIV2_Msk) >> SYS_CLK_CTRL_PHY_DIV2_Pos;
		phy_clk_off_count = (sys_clk_ctrl & SYS_CLK_CTRL_PHY_OFF_COUNT_Msk) >> SYS_CLK_CTRL_PHY_OFF_COUNT_Pos;

		if(phy_clk_div2 == 0) {
			phy_freq_divider = phy_clk_off_count + 3;
		}
		else {
			phy_freq_divider = ((phy_clk_off_count + 3) << 1);
		}
		clk_freq.sys_freq = clk_freq.phy_freq / phy_freq_divider;
	}

	/* Get evt_icn and cpu clock freq */
	sys_clk_auto_mode = (sys_clk_ctrl & SYS_CLK_CTRL_AUTO_MODE_Msk) >> SYS_CLK_CTRL_AUTO_MODE_Pos;

	uint32_t evt_icn_clk_ctrl = 0, evt_icn_clk_switch = 0, esp_clk_en = 0;
	psee_sensor_read(EVT_ICN_CLK_CTRL, &evt_icn_clk_ctrl);
	evt_icn_clk_switch = (evt_icn_clk_ctrl & EVT_ICN_CLK_CTRL_SWITCH_Msk) >> EVT_ICN_CLK_CTRL_SWITCH_Pos;

	uint32_t cpu_ss_clk_ctrl = 0, cpu_ss_clk_switch = 0;
	psee_sensor_read(CPU_SS_CLK_CTRL, &cpu_ss_clk_ctrl);
	cpu_ss_clk_switch = (cpu_ss_clk_ctrl & CPU_SS_CLK_CTRL_SWITCH_Msk) >> CPU_SS_CLK_CTRL_SWITCH_Pos;

	if (((sys_clk_auto_mode == 0) && (evt_icn_clk_switch == 1)) || ((sys_clk_auto_mode == 1) && (sys_clk_switch == 1))) {
		uint32_t evt_icn_clk_div = (evt_icn_clk_ctrl & CPU_SS_CLK_CTRL_DIV_Msk) >> CPU_SS_CLK_CTRL_DIV_Pos;
		clk_freq.evt_icn_freq = clk_freq.sys_freq / evt_icn_clk_div;
	}
	else {
		clk_freq.evt_icn_freq = clk_freq.sys_freq;
	}

    if (((sys_clk_auto_mode == 0) && (cpu_ss_clk_switch == 1)) || ((sys_clk_auto_mode == 1) && (sys_clk_switch == 1))) {
    	uint32_t cpu_ss_clk_div = (cpu_ss_clk_ctrl & CPU_SS_CLK_CTRL_DIV_Msk) >> CPU_SS_CLK_CTRL_DIV_Pos;
    	clk_freq.cpu_ss_freq = clk_freq.sys_freq / cpu_ss_clk_div;
    }
    else {
    	clk_freq.cpu_ss_freq = clk_freq.sys_freq;
    }

    /* Get mipi escape clock freq */
    esp_clk_en = (evt_icn_clk_ctrl & EVT_ICN_CLK_CTRL_EN_Msk) >> EVT_ICN_CLK_CTRL_EN_Pos;
    if (esp_clk_en == 1) {
    	uint32_t mipi_csi_clk_ctrl = 0, txclkesc_en = 0;
    	psee_sensor_read(MIPI_CSI_CLK_CTRL, &mipi_csi_clk_ctrl);
    	txclkesc_en = (mipi_csi_clk_ctrl & MIPI_CSI_CLK_CTRL_TXCLKESC_EN_Msk) >> MIPI_CSI_CLK_CTRL_TXCLKESC_EN_Pos;

    	if(txclkesc_en == 1) {
    		uint32_t txclkesc_divider = (mipi_csi_clk_ctrl & MIPI_CSI_CLK_CTRL_TXCLKESC_DIVIDER_Msk) >> MIPI_CSI_CLK_CTRL_TXCLKESC_DIVIDER_Pos;
    		clk_freq.esc_freq = clk_freq.sys_freq / txclkesc_divider;
    	}
    }

	return clk_freq;

}

/**
  * @brief Function to execute pll power down sequence
  */
static void pll_power_down_and_switch(uint32_t in_freq) {

	/* Read sensor clock tree */
	clk_freq_dict clk_freq;
	clk_freq = get_sensor_clk_freq(in_freq);

	/* Put back the cpu_ss and evt_icn clocks to sys_freq/2 before switch the sys clock to ref clock */
	if(clk_freq.sys_freq != clk_freq.cpu_ss_freq) {
		psee_sensor_write(CPU_SS_CLK_CTRL, (2UL << CPU_SS_CLK_CTRL_DIV_Pos) );
	}
	if(clk_freq.sys_freq != clk_freq.evt_icn_freq) {
		psee_sensor_write(EVT_ICN_CLK_CTRL, (2UL << EVT_ICN_CLK_CTRL_DIV_Pos));
	}

	/* Switch sys clock and pll power down */
	if(clk_freq.pll_vco_freq != 0) {
		psee_sensor_write(SYS_CLK_CTRL, 			(1UL << SYS_CLK_CTRL_EN_Pos)  |
													(0UL << SYS_CLK_CTRL_SWITCH_Pos) );

		psee_sensor_write(SYS_CLK_CTRL, 			(0UL << SYS_CLK_CTRL_EN_Pos)  |
													(0UL << SYS_CLK_CTRL_SWITCH_Pos) );

		psee_sensor_write(PLL_CTRL, 				(0UL << PLL_CTRL_PL_ENABLE_Pos));
	}

	if(clk_freq.sys_freq != clk_freq.cpu_ss_freq) {
		psee_sensor_write(CPU_SS_CLK_CTRL, 			(0UL << CPU_SS_CLK_CTRL_SWITCH_Pos));
	}

	if(clk_freq.sys_freq != clk_freq.evt_icn_freq) {
		psee_sensor_write(EVT_ICN_CLK_CTRL, 		(0UL << EVT_ICN_CLK_CTRL_SWITCH_Pos));
	}


}

/**
  * @brief Function to execute the destroy sequence of the ISSD.
  */
void psee_sensor_powerdown() {

	/* MIPI power down */
	mipi_dphy_power_down();

	/* PLL power down and switch */
	pll_power_down_and_switch(10);

}

/**
  * @brief Function to reconfigure the sensor.
  */
void psee_sensor_reconfig() {

	/* Execute the DESTROY sequence of the ISSD */
	psee_sensor_powerdown();

	/* Execute the INIT sequence of the ISSD */
	psee_sensor_init(current_issd);

	/* Configure CPI for event streaming */
	psee_sensor_set_CPI_EVT20();

	/* Set Standard biases */
	psee_sensor_set_biases(&genx320_default_biases);

	/* Start the sensor */
	psee_sensor_start(current_issd);

	/* Set Flip */
	psee_sensor_set_flip(0,0);

}

/**
  * @}
  */

/** @defgroup GenX320 - Operating Mode Control Functions
 *  @brief    Operating Mode Control Functions
 *
@verbatim
 ===============================================================================
                      ##### Operating Mode Control Functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) To configure the sensor for different power modes

@endverbatim
  * @{
  */

/**
  * @brief  Function to configure the sensor for streaming mode.
  */
void psee_PM3C_config() {

	/*
	 * RO 		- ON
	 * CPU 		- ON
	 * Clock 	- ON
	 * ESP 		- ON
	 * CPI 		- ON
	 */

	/* CPI Default Pipeline Setting */
	psee_sensor_write(CPI_PIPELINE_CONTROL, CPI_PIPELINE_CONTROL_ENABLE |							/* Enable Output Interface */
											CPI_PIPELINE_CONTROL_OUTPUT_IF_MODE |					/* Output Interface Mode : DCMI */
											CPI_PIPELINE_CONTROL_PACKET_FIXED_SIZE_ENABLE |			/* Fixed Packet Size */
											CPI_PIPELINE_CONTROL_FRAME_FIXED_SIZE_ENABLE |			/* Fixed Frame Size */
											CPI_PIPELINE_CONTROL_CLK_OUT_EN							/* Enable Clock Out */
											);

}

/**
  * @brief  Function to configure the sensor for low power monitor mode.
  */
void psee_PM2_config() {

	/*
	 * RO 		- ON
	 * CPU 		- ON
	 * Clock 	- ON
	 * ESP 		- OFF
	 * CPI 		- OFF
	 */

	/* CPI Hot Disable */
	psee_sensor_write(CPI_PIPELINE_CONTROL, (0x0UL << CPI_PIPELINE_CONTROL_ENABLE_Pos) |			/* Disable Output Interface */
											CPI_PIPELINE_CONTROL_DROP_NBACKPRESSURE |				/* Drop the back-pressure when the block is disabled */
											CPI_PIPELINE_CONTROL_HOT_DISABLE_ENABLE					/* Hot Disable Mode */
											);

}

/**
  * @}
  */

/** @defgroup GenX320 - Firmware Flashing Functions
 *  @brief    Firmware Flashing Functions
 *
@verbatim
 ===============================================================================
                      ##### Firmware Flashing Functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Flash the firmware
      (+) Start the firmware
      (+) Reset the firmware

@endverbatim
  * @{
  */

/**
  * @brief  Function to read and flash the firmware from a string.
  * @param  firmwareArray Pointer to the firmware array
  * @param  n_bytes Size of the firmware
  */
FW_StatusTypeDef psee_write_firmware(const Firmware *firmwareArray, uint32_t n_bytes) {

    /* Check for invalid pointer */
    if (firmwareArray == NULL) {
        return fw_Error_InvalidPointer;
    }

	/* Check for invalid size */
    if (n_bytes == 0) {
        return fw_Error_InvalidSize;
    }

	uint32_t mem_bank_size[80] = {0};
	uint32_t mem_bank_index = 0;
	uint32_t count = 0;
	uint32_t write_count = 0;

	uint32_t reg;
	uint8_t buff[258] = {0};

	/* Calculate the bank index of the first bank */
	uint32_t bank = (firmwareArray[0].address - IMEM_START) / 0x100;

	/* Calculate number of Memory banks required and its corresponding size */
	for(uint32_t i = 0; i < n_bytes ; i ++ ){

		if(firmwareArray[i].address == (IMEM_START+0x100) + (mem_bank_index * 0x100) ){
			mem_bank_size[mem_bank_index] = count;
			count = 0;
			mem_bank_index ++;

		}else if(firmwareArray[i].address == firmwareArray[n_bytes-1].address)
		{
			mem_bank_size[mem_bank_index] = count + 1;
			count = 0;
			mem_bank_index ++;
		}
		count ++;
	}

	/* Write value on the corresponding memory bank */
	for(uint32_t b_index = 0; b_index < mem_bank_index; b_index++)
	{

		/* Select the memory bank for IMEM */
		psee_sensor_write(MEM_BANK_SELECT, ((0x2 << 28) + (bank+b_index)));

		reg = MEM_BANK_MEM0;

		/* Send the Register Address */
		buff[0] = (uint8_t)((reg & 0x0000ff00) >> 8);
		buff[1] = (uint8_t)(reg & 0x000000ff);

		for(uint32_t d_index = 0; d_index < mem_bank_size[b_index] ; d_index ++)
		{
			/* Write value */
			buff[2 + (d_index*4)] = (uint8_t)((firmwareArray[write_count].value & 0xff000000) >> 24);
			buff[3 + (d_index*4)] = (uint8_t)((firmwareArray[write_count].value & 0x00ff0000) >> 16);
			buff[4 + (d_index*4)] = (uint8_t)((firmwareArray[write_count].value & 0x0000ff00) >> 8);
			buff[5 + (d_index*4)] = (uint8_t)(firmwareArray[write_count].value & 0x000000ff);
			write_count++;
		}

		/* Flash the firmware on the IMEM */
		psee_sensor_sequential_write(buff,((mem_bank_size[b_index]*4)+2));

	}

	/* Reset Memory Bank */
	psee_sensor_write(MEM_BANK_SELECT, 0x00000000);

	return fw_OK;
}

/**
  * @brief  Function to start the firmware.
  * @retval FW status
  */
static FW_StatusTypeDef psee_start_fw_imem() {

	/* CPU is active, no reset is applied */
	psee_sensor_write(MBX_CPU_SOFT_RESET, (0UL << MBX_CPU_SOFT_RESET_Pos) );

	/* Release the IMEM code execution */
	psee_sensor_write(MBX_CPU_START_EN, MBX_CPU_START_EN_Msk);

	return fw_OK;
}

/**
  * @brief  Function to reset the firmware.
  * @retval FW status
  */
static FW_StatusTypeDef psee_reset_fw_imem() {

	/* Keep CPU core under reset */
	psee_sensor_write(MBX_CPU_START_EN, (0UL << MBX_CPU_START_EN_Pos));

	/* Force a reset on CPU system*/
	psee_sensor_write(MBX_CPU_SOFT_RESET, MBX_CPU_SOFT_RESET_Msk);

	return fw_OK;
}

/**
  * @brief  Function to reset the firmware for ROM_BOOT.
  * @retval FW status
  */
static FW_StatusTypeDef psee_reset_fw_rom() {

	/* Force a reset on CPU system*/
	psee_sensor_write(MBX_CPU_SOFT_RESET, MBX_CPU_SOFT_RESET_Msk);
	psee_sleep_ms_imp(1);

	return fw_OK;
}

/**
  * @brief  Function to start the firmware for ROM_BOOT.
  * @retval FW status
  */
static FW_StatusTypeDef psee_start_fw_rom() {

	/* Remove the Reset */
	psee_sensor_write(MBX_CPU_SOFT_RESET, (0UL << MBX_CPU_SOFT_RESET_Pos));

	/* Check if the ROM_BOOT is successful */
	uint32_t mbx_misc_val = psee_get_mbx_misc();
	while(mbx_misc_val != 0xCAFEBABE) {
		mbx_misc_val = psee_get_mbx_misc();
	}

	/* Reconfigure the sensor for CPI */
	psee_sensor_reconfig();

	/* Release the IMEM code execution */
	psee_sensor_write(MBX_CMD_PTR, (0x7 << 28) | 0x200200);

	/* Check if the application has started processing */
	uint32_t mbx_val = psee_get_mbx_misc();
	while(mbx_val == 0xCAFEBABE) {
		mbx_val = psee_get_mbx_misc();
	}

	return fw_OK;
}

/**
  * @brief  Function to start the RISC-V firmware.
  * @retval FW status
  */
FW_StatusTypeDef psee_start_firmware(Boot_Mode boot_mode) {

	/* Check the boot mode and flash the FW accordingly */
	if(boot_mode == IMEM_BOOT) {
		psee_start_fw_imem();
	}
	else if(boot_mode == ROM_BOOT) {
		psee_start_fw_rom();
	}
	return fw_OK;
}

/**
  * @brief  Function to reset the RISC-V firmware.
  * @retval FW status
  */
FW_StatusTypeDef psee_reset_firmware(Boot_Mode boot_mode) {

	/* Check the boot mode and reset the FW accordingly */
	if(boot_mode == IMEM_BOOT) {
		psee_reset_fw_imem();
	}
	else if(boot_mode == ROM_BOOT) {
		psee_reset_fw_rom();
	}
	return fw_OK;
}

/**
  * @}
  */

/** @defgroup GenX320 - Mailbox Communication Functions
 *  @brief    Mailbox Communication Functions
 *
@verbatim
 ===============================================================================
                      ##### Mailbox Communication Functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Write on the Mailbox MISC/CMD_PTR/STATUS_PTR registers
      (+) Read from the Mailbox MISC/CMD_PTR/STATUS_PTR registers

@endverbatim
  * @{
  */

/**
  * @brief  Function to read the 32-bit value from the Mailbox cmd_ptr register.
  * @retval  Value of Mailbox cmd_ptr register
  */
uint32_t psee_get_mbx_cmd_ptr() {
	uint32_t mbx_cmd_ptr = 0;
	psee_sensor_read(MBX_CMD_PTR, &mbx_cmd_ptr);
	return mbx_cmd_ptr;
}

/**
  * @brief  Function to read the 32-bit value from the Mailbox misc register.
  * @retval  Value of Mailbox misc register
  */
uint32_t psee_get_mbx_misc() {
	uint32_t mbx_misc = 0;
	psee_sensor_read(MBX_MISC, &mbx_misc);
	return mbx_misc;
}

/**
  * @brief  Function to read the 32-bit value from the Mailbox status_ptr register.
  * @retval  Value of Mailbox status_ptr register
  */
uint32_t psee_get_mbx_status_ptr() {
	uint32_t mbx_status_ptr = 0;
	psee_sensor_read(MBX_STATUS_PTR, &mbx_status_ptr);
	return mbx_status_ptr;
}

/**
  * @brief  Function to write a 32-bit value on the Mailbox cmd_ptr register.
  * @param  val Data to be written on the Mailbox cmd_ptr register
  */
void psee_set_mbx_cmd_ptr(uint32_t val) {
	psee_sensor_write(MBX_CMD_PTR, val);
}

/**
  * @brief  Function to write a 32-bit value on the Mailbox misc register.
  * @param  val Data to be written on the Mailbox misc register
  */
void psee_set_mbx_misc(uint32_t val) {
	psee_sensor_write(MBX_MISC, val);
}

/**
  * @brief  Function to write a 32-bit value on the Mailbox status_ptr register.
  * @param  val Data to be written on the Mailbox status_ptr register
  */
void psee_set_mbx_status_ptr(uint32_t val) {
	psee_sensor_write(MBX_STATUS_PTR, val);
}

/**
  * @}
  */

/** @defgroup GenX320 - ROI Access Functions
 *  @brief    ROI Access Functions
 *
@verbatim
 ===============================================================================
                      ##### ROI Access Functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Read/Write ROI Registers

@endverbatim
  * @{
  */

/**
  * @brief  Function to write a 32-bit value on the TD_ROI_X register
  * @param	offset Register Offset
  * @param  val Data to be written on the TD_ROI_X register
  */
void psee_write_ROI_X(uint32_t offset, uint32_t val) {
	psee_sensor_write(ROI_TD_ROI_X00 + offset, val);
}

/**
  * @brief  Function to write a 32-bit value on the TD_ROI_Y register
  * @param	offset Register Offset
  * @param  val Data to be written on the TD_ROI_Y register
  */
void psee_write_ROI_Y(uint32_t offset, uint32_t val) {
	psee_sensor_write(ROI_TD_ROI_Y00 + offset, val);
}

/**
  * @brief  Function to write a 32-bit value on the TD_ROI_CTR register
  * @param  val Data to be written on the TD_ROI_CTR register
  */
void psee_write_ROI_CTRL(uint32_t val) {
	psee_sensor_write(ROI_CTRL, val);
}

/**
  * @}
  */

/** @defgroup GenX320 - RISC-V Debugger Function
 *  @brief    RISC-V Debugger Function
 *
@verbatim
 ===============================================================================
                      ##### RISC-V Debugger Functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Read value from the RISC-V MCU with a handshake mechanism
      (+) Read from DMEM

@endverbatim
  * @{
  */

/**
  * @brief  Helper function of the debugger, which reads one 32-bit value from
  * the RISC-V and sends an acknowledgment to receive the next value.
  * @retval  Value of Mailbox status_ptr register
  */
uint32_t psee_mbx_read_uint32(){

    /* Wait until a new value is available */
    while(psee_get_mbx_misc()==0);

    /* Get the value from the RISC V */
    uint32_t val = psee_get_mbx_status_ptr();

    /* Indicate that the value has been consumed */
    psee_set_mbx_misc(0);

    return val;
}

/**
  * @brief  Decoder function to decode the message sent from the RISC-V.
  */
void psee_decode_debugger(){

	printf("Starting Debugger...\r\n");

	while(1)
	{
        uint32_t v = psee_mbx_read_uint32();
        for (int pos =0; pos < sizeof(uint32_t); ++pos) {
            char c = v & 0xff;
            if (c == 0){
            	printf("\r");
                break;
            }
            putchar(c);
            v >>= 8;
        }
        fflush(stdout);
	}
}


/**
  * @brief  Function to read the 32-bit value from a DMEM register.
  * @param  address Address of the DMEM register
  * @retval  Value of the DMEM register
  */
uint32_t psee_sensor_read_dmem(uint32_t address) {

	static uint32_t offsets[80] = {0};

	/* Sensor read variable */
	uint32_t dmem_data;

	/* Calculate the bank index */
	uint32_t bank = (address - DMEM_START) / 0x100;

	/* Calculate the offset */
	offsets[bank] = (address - DMEM_START) - (bank * 0x100);

	if (bank < 80)
	{
		/* Select the memory bank for DMEM */
		psee_sensor_write(MEM_BANK_SELECT, ((0x3 << 28) + bank));

		/* Read the value from corresponding bank */
		psee_sensor_read((MEM_BANK_MEM0 + offsets[bank]), &dmem_data);

	}

	/* Reset the Mem_bank */
	psee_sensor_write(MEM_BANK_SELECT, 0x00000000);

	return dmem_data;

}


/**
  * @brief  Function to read the 32-bit value from a IMEM register.
  * @param  address Address of the IMEM register
  * @retval  Value of the IMEM register
  */
uint32_t psee_sensor_read_imem(uint32_t address) {

	static uint32_t offsets[80] = {0};

	/* Sensor read variable */
	uint32_t imem_data;

	/* Calculate the bank index */
	uint32_t bank = (address - IMEM_START) / 0x100;

	/* Calculate the offset */
	offsets[bank] = (address - IMEM_START) - (bank * 0x100);

	if (bank < 80)
	{
		/* Select the memory bank for DMEM */
		psee_sensor_write(MEM_BANK_SELECT, ((0x2 << 28) + bank));

		/* Read the value from corresponding bank */
		psee_sensor_read((MEM_BANK_MEM0 + offsets[bank]), &imem_data );

	}

	/* Reset the Mem_bank */
	psee_sensor_write(MEM_BANK_SELECT, 0x00000000);

	return imem_data;

}

/**
  * @}
  */

/** @defgroup GenX320 - Statistics Register Read Functions
 *  @brief    Statistics Register Read Functions
 *
@verbatim
 ===============================================================================
                      ##### Statistics Register Read Functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Read from low power event counters
      (+) Read total pixel's CD events
      (+) Read total AFK Flickering events
      (+) Read total AFK Valid events

@endverbatim
  * @{
  */

/**
  * @brief  Reads the counter value from the 16 low-power event counters (RO_LP_CNT00 - RO_LP_CNT15).
  * @param  p_data Pointer to data buffer
  */
void psee_read_ro_lp_evt_cnt(uint32_t *p_data) {

	uint16_t total_counters = 16;
	uint32_t start_address = RO_LP_CNT00;
	uint32_t ro_shadow_status = 0;

	/* Check if shadow status is valid */
	psee_sensor_read(RO_SHADOW_STATUS, &ro_shadow_status);
	while((ro_shadow_status & 1) != 1) {
		psee_sensor_read(RO_SHADOW_STATUS, &ro_shadow_status );
		psee_sleep_ms_imp(1);
	}

	/* If shadow status is valid read the counter values*/
	psee_sensor_sequential_read(start_address, p_data, total_counters);

}

/**
  * @brief  Reads the value from the RO_EVT_CD_CNT register
  * @retval  Value of the RO_EVT_CD_CNT register
  */
uint32_t psee_read_ro_evt_cd_cnt() {
	uint32_t evt_cd_cnt;

	psee_sensor_read(RO_EVT_CD_CNT, &evt_cd_cnt);
	return evt_cd_cnt;
}

/**
  * @brief  Reads the value from the AFK_FLICKER_EVT_CNT register
  * @retval  Value of the AFK_FLICKER_EVT_CNT register
  */
uint32_t psee_read_afk_flicker_evt_cnt() {
	uint32_t flicker_evt_cnt;

	psee_sensor_read(AFK_FLICKER_EVT_CNT, &flicker_evt_cnt);
	return flicker_evt_cnt;
}

/**
  * @brief  Reads the value from the AFK_TOTAL_EVT_CNT register
  * @retval  Value of the AFK_TOTAL_EVT_CNT register
  */
uint32_t psee_read_afk_total_evt_cnt() {
	uint32_t total_evt_cnt;

	psee_sensor_read(AFK_TOTAL_EVT_CNT, &total_evt_cnt);
	return total_evt_cnt;
}

/**
  * @}
  */

/** @defgroup GenX320 - Activity Map Configuration Functions
 *  @brief    Activity Map Configuration Functions
 *
@verbatim
 ===============================================================================
                      ##### Activity Map Configuration Functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) To enable the low power event counters and configure it
      (+) To configure the X and Y borders

@endverbatim
  * @{
  */

/**
  * @brief  Function to enable and configure the Activity map
  */
void psee_configure_activity_map() {

	/* Enable Low power control*/
	psee_sensor_write(RO_RO_LP_CTRL, 	RO_LP_CTRL_CNT_EN |												/* Enable the 16 event counters */
										RO_LP_CTRL_KEEP_TH												/* Keep timer high and trash all the rest when in lp_out_disable mode */
										);

	/* Enable Shadow control*/
	psee_sensor_write(RO_SHADOW_CTRL,	RO_SHADOW_CTRL_TIMER_EN |										/* Enable microsecond-based timer */
										RO_SHADOW_CTRL_RESET_ON_COPY									/* Reset the counters after a copy&reset IRQ */
										);

	/* Set Shadow Timer Threshold - Accumulation Time */
	psee_sensor_write(RO_SHADOW_TIMER_THRESHOLD,25000);
}

/**
  * @brief Default borders for the activity map
  */
AM_Borders_t genx320mp_default_am_borders = {

	.x0       		= 0,
	.x1	       		= 64,
	.x2            	= 160,
	.x3            	= 256,
	.x4            	= 320,
	.y0       		= 0,
	.y1	       		= 64,
	.y2            	= 160,
	.y3            	= 256,
	.y4            	= 320,

};

/**
  * @brief  Function to set the default X and Y borders for the Activity map
  */
void psee_set_default_XY_borders(AM_Borders_t *border) {

	/* Set the default X borders */
	psee_sensor_write(RO_LP_X0, border->x0);
	psee_sensor_write(RO_LP_X1, border->x1);
	psee_sensor_write(RO_LP_X2, border->x2);
	psee_sensor_write(RO_LP_X3, border->x3);
	psee_sensor_write(RO_LP_X4, border->x4);

	/* Set the default Y borders */
	psee_sensor_write(RO_LP_Y0, border->y0);
	psee_sensor_write(RO_LP_Y1, border->y1);
	psee_sensor_write(RO_LP_Y2, border->y2);
	psee_sensor_write(RO_LP_Y3, border->y3);
	psee_sensor_write(RO_LP_Y4, border->y4);

}


/**
  * @}
  */

/** @defgroup GenX320 - AFK Configuration Functions
 *  @brief    AFK Configuration Functions
 *
@verbatim
 ===============================================================================
                      ##### AFK Configuration Functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Enable and Disable the AFK

@endverbatim
  * @{
  */

/**
  * @brief  Function to convert frequency to period
  * @param  freq Frequency to be converted
  * @retval period Period that corresponds to the frequency
  */
static uint32_t psee_freq_to_period(const uint32_t freq) {

	/* Convert frequency to period */
	uint32_t period = 1000000 / freq;

    /* Convert it to Units of 128us */
    return period >> 7;
}

/**
  * @brief  Function to compute the invalidation period
  * @param  max_cutoff_period Maximum cut off period
  * @retval clk_freq Frequency of the MCU in MHz
  */
static Invalidation compute_invalidation(const uint32_t max_cutoff_period, const uint32_t clk_freq) {

	Invalidation result;

	/* computation of invalidation speed parameter based on processing frequency and max_cut_freq
           max_cutoff_period : in unit of 128us
           evt_clk_freq; processing frequency, in unit of MHz */

    /* Processing clock period in nanoseconds */
    uint32_t clk_period_ns = 1000 / clk_freq;
    uint32_t in_parallel   = 5;

    float invalidation_period =
        (((2 << 15) - (128 * (3 + max_cutoff_period))) * in_parallel * 1000) / (160 * 5 * clk_period_ns);

    result.df_wait_time = 4;

    result.df_timeout = floor(invalidation_period) - result.df_wait_time;

    if (result.df_timeout > 4095) {
    	result.df_timeout = 4095;
    }

    return result;
}

/**
  * @brief  Function to enable and configure the Anti-Flickering Filter(AFK)
  */
void psee_enable_afk(uint16_t min_freq, uint16_t max_freq) {

	Invalidation afk_invalidation;

	/* Disable/bypass the block in order to configure AFK parameters */
	psee_sensor_write(AFK_PIPELINE_CONTROL, AFK_PIPELINE_CONTROL_BYPASS);							/* Bypass the block */

	/* SRAM Powerup */
	uint32_t sram_initn = 0;
	psee_sensor_read(SRAM_INITN, &sram_initn);
	psee_sensor_write(SRAM_INITN, 			sram_initn |											/* Previous State of SRAM_INITN */
											(1UL << SRAM_INITN_AFK_Pos)								/* Initialize the memory state machine : AFK's SRAM control init_n */
											);

	psee_sensor_write(SRAM_PD0, 			(0UL << SRAM_PD0_AFK_ALR_PD_Pos) |						/* SRAM's Power Down Register : 0 - AFK's SRAM cuts are supplied */
											(0UL << SRAM_PD0_AFK_STR0_PD_Pos) |						/* SRAM's Power Down Register : 0 - AFK's SRAM cuts are supplied */
											(0UL << SRAM_PD0_AFK_STR1_PD_Pos)						/* SRAM's Power Down Register : 0 - AFK's SRAM cuts are supplied */
											);

	/* Set the AFK Frequency Bandwidth */
	uint32_t min_period = psee_freq_to_period(max_freq);
	uint32_t max_period = psee_freq_to_period(min_freq);
	psee_sensor_write(AFK_FILTER_PERIOD, 	(min_period << AFK_FILTER_PERIOD_MIN_CUTOFF_Pos) |		/* AntiFlickering Filter : Minimum cutoff period of undesired flickering light */
											(max_period << AFK_FILTER_PERIOD_MAX_CUTOFF_Pos) |		/* AntiFlickering Filter : Maximum  cutoff period of undesired flickering light */
											(8UL << AFK_FILTER_PERIOD_INVERTED_DUTY_CYCLE_Pos)		/* Flickering duty cycle ratio */
											);

	/* Configure the AFK Invalidation */
	afk_invalidation = compute_invalidation(max_period,10);

	psee_sensor_write(AFK_INVALIDATION, 	(afk_invalidation.df_wait_time << AFK_INVALIDATION_DT_FIFO_WAIT_TIME_Pos) |		/* Deadtime fifo wait time, in unit of clock cycle */
											(afk_invalidation.df_timeout << AFK_INVALIDATION_DT_FIFO_TIMEOUT_Pos) |			/* Deadtime fifo time out, in unit of clock cycles */
											(5UL << AFK_INVALIDATION_IN_PARALLEL_Pos)										/* Number of ram invalidated together this is then changed into one hot, in unit of SRAM cut on parallel */
											);

	/* Configure the AFK parameters */
	psee_sensor_write(AFK_AFK_PARAM, 		(6UL << AFK_PARAM_COUNTER_HIGH_Pos) |					/* Hysteresis threshold above which we consider the block flickering */
											(4UL << AFK_PARAM_COUNTER_LOW_Pos) |					/* Hysteresis threshold below which we consider the block is not flickering anymore */
											(0UL << AFK_PARAM_INVERT_Pos)							/* Invert the output of anti-flicker filter : 0 No-flickering event pass */
											);

	/* Configure the Atomic Register */
	psee_sensor_write(AFK_SHADOW_CTRL, 		AFK_SHADOW_CTRL_TIMER_EN |								/* Enable Microsecond-based Timer */
											AFK_SHADOW_CTRL_RESET_ON_COPY							/* Reset the Counters after a copy&reset IRQ */
											);

	/* Set Shadow Timer Threshold - Accumulation Time */
	psee_sensor_write(AFK_SHADOW_TIMER_THRESHOLD, 20000UL);											/* Accumulation time in Microseconds */

	/* Enable the AFK_PIPELINE_CONTROL */
	psee_sensor_write(AFK_PIPELINE_CONTROL, AFK_PIPELINE_CONTROL_ENABLE);							/* Enable the block */

}

/**
  * @brief  Function to disable the Anti-Flickering Filter(AFK)
  */
void psee_disable_afk() {

	/* Disable AFK Output */
	psee_sensor_write(AFK_PIPELINE_CONTROL,	AFK_PIPELINE_CONTROL_ENABLE |							/* Enable the block */
											AFK_PIPELINE_CONTROL_BYPASS								/* Bypass the block */
											);

}

/**
  * @}
  */
