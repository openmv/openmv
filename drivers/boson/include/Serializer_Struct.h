//  /////////////////////////////////////////////////////
//  // DO NOT EDIT.  This is a machine generated file. //
//  /////////////////////////////////////////////////////

/******************************************************************************/
/*                                                                            */
/*  Copyright (C) 2018, FLIR Systems                                          */
/*  All rights reserved.                                                      */
/*                                                                            */
/*  This document is controlled to FLIR Technology Level 2. The information   */
/*  contained in this document pertains to a dual use product controlled for  */
/*  export by the Export Administration Regulations (EAR). Diversion contrary */
/*  to US law is prohibited. US Department of Commerce authorization is not   */
/*  required prior to export or transfer to foreign persons or parties unless */
/*  otherwise prohibited.                                                     */
/*                                                                            */
/******************************************************************************/


#ifndef SERIALIZER_STRUCT_H
#define SERIALIZER_STRUCT_H

#include <stdint.h>
#include "EnumTypes.h"
#include "Serializer_BuiltIn.h"

struct t_FLR_ROI_T {
    uint16_t rowStart;
    uint16_t rowStop;
    uint16_t colStart;
    uint16_t colStop;
};
typedef struct t_FLR_ROI_T FLR_ROI_T;

void byteToFLR_ROI_T(const uint8_t *inBuff, FLR_ROI_T *outVal);
void FLR_ROI_TToByte(const FLR_ROI_T *inVal, const uint8_t *outBuff);

struct t_FLR_BOSON_PARTNUMBER_T {
    uint8_t value[20];
};
typedef struct t_FLR_BOSON_PARTNUMBER_T FLR_BOSON_PARTNUMBER_T;

void byteToFLR_BOSON_PARTNUMBER_T(const uint8_t *inBuff, FLR_BOSON_PARTNUMBER_T *outVal);
void FLR_BOSON_PARTNUMBER_TToByte(const FLR_BOSON_PARTNUMBER_T *inVal, const uint8_t *outBuff);

struct t_FLR_BOSON_SENSOR_PARTNUMBER_T {
    uint8_t value[32];
};
typedef struct t_FLR_BOSON_SENSOR_PARTNUMBER_T FLR_BOSON_SENSOR_PARTNUMBER_T;

void byteToFLR_BOSON_SENSOR_PARTNUMBER_T(const uint8_t *inBuff, FLR_BOSON_SENSOR_PARTNUMBER_T *outVal);
void FLR_BOSON_SENSOR_PARTNUMBER_TToByte(const FLR_BOSON_SENSOR_PARTNUMBER_T *inVal, const uint8_t *outBuff);

struct t_FLR_BOSON_GAIN_SWITCH_PARAMS_T {
    uint32_t pHighToLowPercent;
    uint32_t cHighToLowPercent;
    uint32_t pLowToHighPercent;
    uint32_t hysteresisPercent;
};
typedef struct t_FLR_BOSON_GAIN_SWITCH_PARAMS_T FLR_BOSON_GAIN_SWITCH_PARAMS_T;

void byteToFLR_BOSON_GAIN_SWITCH_PARAMS_T(const uint8_t *inBuff, FLR_BOSON_GAIN_SWITCH_PARAMS_T *outVal);
void FLR_BOSON_GAIN_SWITCH_PARAMS_TToByte(const FLR_BOSON_GAIN_SWITCH_PARAMS_T *inVal, const uint8_t *outBuff);

struct t_FLR_BOSON_GAIN_SWITCH_RADIOMETRIC_PARAMS_T {
    uint32_t pHighToLowPercent;
    float TempHighToLowDegK;
    uint32_t pLowToHighPercent;
    float TempLowToHighDegK;
};
typedef struct t_FLR_BOSON_GAIN_SWITCH_RADIOMETRIC_PARAMS_T FLR_BOSON_GAIN_SWITCH_RADIOMETRIC_PARAMS_T;

void byteToFLR_BOSON_GAIN_SWITCH_RADIOMETRIC_PARAMS_T(const uint8_t *inBuff, FLR_BOSON_GAIN_SWITCH_RADIOMETRIC_PARAMS_T *outVal);
void FLR_BOSON_GAIN_SWITCH_RADIOMETRIC_PARAMS_TToByte(const FLR_BOSON_GAIN_SWITCH_RADIOMETRIC_PARAMS_T *inVal, const uint8_t *outBuff);

struct t_FLR_BOSON_SATURATION_LUT_T {
    uint16_t value[17];
};
typedef struct t_FLR_BOSON_SATURATION_LUT_T FLR_BOSON_SATURATION_LUT_T;

void byteToFLR_BOSON_SATURATION_LUT_T(const uint8_t *inBuff, FLR_BOSON_SATURATION_LUT_T *outVal);
void FLR_BOSON_SATURATION_LUT_TToByte(const FLR_BOSON_SATURATION_LUT_T *inVal, const uint8_t *outBuff);

struct t_FLR_BOSON_SATURATION_HEADER_LUT_T {
    FLR_BOSON_SATURATION_LUT_T lut;
    uint16_t tableIndex;
};
typedef struct t_FLR_BOSON_SATURATION_HEADER_LUT_T FLR_BOSON_SATURATION_HEADER_LUT_T;

void byteToFLR_BOSON_SATURATION_HEADER_LUT_T(const uint8_t *inBuff, FLR_BOSON_SATURATION_HEADER_LUT_T *outVal);
void FLR_BOSON_SATURATION_HEADER_LUT_TToByte(const FLR_BOSON_SATURATION_HEADER_LUT_T *inVal, const uint8_t *outBuff);

struct t_FLR_CAPTURE_SETTINGS_T {
    FLR_CAPTURE_SRC_E dataSrc;
    uint32_t numFrames;
    uint16_t bufferIndex;
};
typedef struct t_FLR_CAPTURE_SETTINGS_T FLR_CAPTURE_SETTINGS_T;

void byteToFLR_CAPTURE_SETTINGS_T(const uint8_t *inBuff, FLR_CAPTURE_SETTINGS_T *outVal);
void FLR_CAPTURE_SETTINGS_TToByte(const FLR_CAPTURE_SETTINGS_T *inVal, const uint8_t *outBuff);

struct t_FLR_CAPTURE_FILE_SETTINGS_T {
    FLR_CAPTURE_FILE_TYPE_E captureFileType;
    uint8_t filePath[128];
};
typedef struct t_FLR_CAPTURE_FILE_SETTINGS_T FLR_CAPTURE_FILE_SETTINGS_T;

void byteToFLR_CAPTURE_FILE_SETTINGS_T(const uint8_t *inBuff, FLR_CAPTURE_FILE_SETTINGS_T *outVal);
void FLR_CAPTURE_FILE_SETTINGS_TToByte(const FLR_CAPTURE_FILE_SETTINGS_T *inVal, const uint8_t *outBuff);

struct t_FLR_CAPTURE_STATUS_T {
    FLR_CAPTURE_STATE_E state;
    uint32_t result;
    uint32_t capturedFrames;
    uint32_t missedFrames;
    uint32_t savedFrames;
    uint32_t unsyncFrames;
};
typedef struct t_FLR_CAPTURE_STATUS_T FLR_CAPTURE_STATUS_T;

void byteToFLR_CAPTURE_STATUS_T(const uint8_t *inBuff, FLR_CAPTURE_STATUS_T *outVal);
void FLR_CAPTURE_STATUS_TToByte(const FLR_CAPTURE_STATUS_T *inVal, const uint8_t *outBuff);

struct t_FLR_DVO_YCBCR_SETTINGS_T {
    FLR_DVO_OUTPUT_YCBCR_FORMAT_E ycbcrFormat;
    FLR_DVO_OUTPUT_CBCR_ORDER_E cbcrOrder;
    FLR_DVO_OUTPUT_Y_ORDER_E yOrder;
};
typedef struct t_FLR_DVO_YCBCR_SETTINGS_T FLR_DVO_YCBCR_SETTINGS_T;

void byteToFLR_DVO_YCBCR_SETTINGS_T(const uint8_t *inBuff, FLR_DVO_YCBCR_SETTINGS_T *outVal);
void FLR_DVO_YCBCR_SETTINGS_TToByte(const FLR_DVO_YCBCR_SETTINGS_T *inVal, const uint8_t *outBuff);

struct t_FLR_DVO_RGB_SETTINGS_T {
    FLR_DVO_OUTPUT_RGB_FORMAT_E rgbFormat;
    FLR_DVO_OUTPUT_RGB_ORDER_E rgbOrder;
};
typedef struct t_FLR_DVO_RGB_SETTINGS_T FLR_DVO_RGB_SETTINGS_T;

void byteToFLR_DVO_RGB_SETTINGS_T(const uint8_t *inBuff, FLR_DVO_RGB_SETTINGS_T *outVal);
void FLR_DVO_RGB_SETTINGS_TToByte(const FLR_DVO_RGB_SETTINGS_T *inVal, const uint8_t *outBuff);

struct t_FLR_DVO_LCD_CONFIG_T {
    uint32_t width;
    uint32_t hPulseWidth;
    uint32_t hBackP;
    uint32_t hFrontP;
    uint32_t height;
    uint32_t vPulseWidth;
    uint32_t vBackP;
    uint32_t vFrontP;
    uint32_t outputFormat;
    uint32_t control;
    uint32_t rotation;
    uint32_t pixelClockkHz;
};
typedef struct t_FLR_DVO_LCD_CONFIG_T FLR_DVO_LCD_CONFIG_T;

void byteToFLR_DVO_LCD_CONFIG_T(const uint8_t *inBuff, FLR_DVO_LCD_CONFIG_T *outVal);
void FLR_DVO_LCD_CONFIG_TToByte(const FLR_DVO_LCD_CONFIG_T *inVal, const uint8_t *outBuff);

struct t_FLR_GAO_RNS_COL_CORRECT_T {
    int16_t value[20];
};
typedef struct t_FLR_GAO_RNS_COL_CORRECT_T FLR_GAO_RNS_COL_CORRECT_T;

void byteToFLR_GAO_RNS_COL_CORRECT_T(const uint8_t *inBuff, FLR_GAO_RNS_COL_CORRECT_T *outVal);
void FLR_GAO_RNS_COL_CORRECT_TToByte(const FLR_GAO_RNS_COL_CORRECT_T *inVal, const uint8_t *outBuff);

struct t_FLR_ISOTHERM_COLOR_T {
    uint16_t r;
    uint16_t g;
    uint16_t b;
};
typedef struct t_FLR_ISOTHERM_COLOR_T FLR_ISOTHERM_COLOR_T;

void byteToFLR_ISOTHERM_COLOR_T(const uint8_t *inBuff, FLR_ISOTHERM_COLOR_T *outVal);
void FLR_ISOTHERM_COLOR_TToByte(const FLR_ISOTHERM_COLOR_T *inVal, const uint8_t *outBuff);

struct t_FLR_ISOTHERM_COLORS_T {
    FLR_ISOTHERM_COLOR_T range1;
    FLR_ISOTHERM_COLOR_T range2;
    FLR_ISOTHERM_COLOR_T range3;
    uint16_t num;
};
typedef struct t_FLR_ISOTHERM_COLORS_T FLR_ISOTHERM_COLORS_T;

void byteToFLR_ISOTHERM_COLORS_T(const uint8_t *inBuff, FLR_ISOTHERM_COLORS_T *outVal);
void FLR_ISOTHERM_COLORS_TToByte(const FLR_ISOTHERM_COLORS_T *inVal, const uint8_t *outBuff);

struct t_FLR_ISOTHERM_SETTINGS_T {
    int32_t thIsoT1;
    int32_t thIsoT2;
    int32_t thIsoT3;
    int32_t thIsoT4;
    int32_t thIsoT5;
    FLR_ISOTHERM_COLORS_T color0;
    FLR_ISOTHERM_COLORS_T color1;
    FLR_ISOTHERM_COLORS_T color2;
    FLR_ISOTHERM_COLORS_T color3;
    FLR_ISOTHERM_COLORS_T color4;
    FLR_ISOTHERM_COLORS_T color5;
    FLR_ISOTHERM_REGION_E region0;
    FLR_ISOTHERM_REGION_E region1;
    FLR_ISOTHERM_REGION_E region2;
    FLR_ISOTHERM_REGION_E region3;
    FLR_ISOTHERM_REGION_E region4;
    FLR_ISOTHERM_REGION_E region5;
};
typedef struct t_FLR_ISOTHERM_SETTINGS_T FLR_ISOTHERM_SETTINGS_T;

void byteToFLR_ISOTHERM_SETTINGS_T(const uint8_t *inBuff, FLR_ISOTHERM_SETTINGS_T *outVal);
void FLR_ISOTHERM_SETTINGS_TToByte(const FLR_ISOTHERM_SETTINGS_T *inVal, const uint8_t *outBuff);

struct t_FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_LUT_T {
    uint16_t value[17];
};
typedef struct t_FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_LUT_T FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_LUT_T;

void byteToFLR_RADIOMETRY_SIGNAL_COMP_FACTOR_LUT_T(const uint8_t *inBuff, FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_LUT_T *outVal);
void FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_LUT_TToByte(const FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_LUT_T *inVal, const uint8_t *outBuff);

struct t_FLR_RADIOMETRY_NOISE_COMP_FACTOR_LUT_T {
    uint16_t value[17];
};
typedef struct t_FLR_RADIOMETRY_NOISE_COMP_FACTOR_LUT_T FLR_RADIOMETRY_NOISE_COMP_FACTOR_LUT_T;

void byteToFLR_RADIOMETRY_NOISE_COMP_FACTOR_LUT_T(const uint8_t *inBuff, FLR_RADIOMETRY_NOISE_COMP_FACTOR_LUT_T *outVal);
void FLR_RADIOMETRY_NOISE_COMP_FACTOR_LUT_TToByte(const FLR_RADIOMETRY_NOISE_COMP_FACTOR_LUT_T *inVal, const uint8_t *outBuff);

struct t_FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_HEADER_LUT_T {
    FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_LUT_T lut;
    uint16_t tableIndex;
};
typedef struct t_FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_HEADER_LUT_T FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_HEADER_LUT_T;

void byteToFLR_RADIOMETRY_SIGNAL_COMP_FACTOR_HEADER_LUT_T(const uint8_t *inBuff, FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_HEADER_LUT_T *outVal);
void FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_HEADER_LUT_TToByte(const FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_HEADER_LUT_T *inVal, const uint8_t *outBuff);

struct t_FLR_RADIOMETRY_NOISE_COMP_FACTOR_HEADER_LUT_T {
    FLR_RADIOMETRY_NOISE_COMP_FACTOR_LUT_T lut;
    uint16_t tableIndex;
};
typedef struct t_FLR_RADIOMETRY_NOISE_COMP_FACTOR_HEADER_LUT_T FLR_RADIOMETRY_NOISE_COMP_FACTOR_HEADER_LUT_T;

void byteToFLR_RADIOMETRY_NOISE_COMP_FACTOR_HEADER_LUT_T(const uint8_t *inBuff, FLR_RADIOMETRY_NOISE_COMP_FACTOR_HEADER_LUT_T *outVal);
void FLR_RADIOMETRY_NOISE_COMP_FACTOR_HEADER_LUT_TToByte(const FLR_RADIOMETRY_NOISE_COMP_FACTOR_HEADER_LUT_T *inVal, const uint8_t *outBuff);

struct t_FLR_RADIOMETRY_RBFO_PARAMS_T {
    float RBFO_R;
    float RBFO_B;
    float RBFO_F;
    float RBFO_O;
};
typedef struct t_FLR_RADIOMETRY_RBFO_PARAMS_T FLR_RADIOMETRY_RBFO_PARAMS_T;

void byteToFLR_RADIOMETRY_RBFO_PARAMS_T(const uint8_t *inBuff, FLR_RADIOMETRY_RBFO_PARAMS_T *outVal);
void FLR_RADIOMETRY_RBFO_PARAMS_TToByte(const FLR_RADIOMETRY_RBFO_PARAMS_T *inVal, const uint8_t *outBuff);

struct t_FLR_RADIOMETRY_TAUX_PARAMS_T {
    float A3;
    float A2;
    float A1;
    float A0;
};
typedef struct t_FLR_RADIOMETRY_TAUX_PARAMS_T FLR_RADIOMETRY_TAUX_PARAMS_T;

void byteToFLR_RADIOMETRY_TAUX_PARAMS_T(const uint8_t *inBuff, FLR_RADIOMETRY_TAUX_PARAMS_T *outVal);
void FLR_RADIOMETRY_TAUX_PARAMS_TToByte(const FLR_RADIOMETRY_TAUX_PARAMS_T *inVal, const uint8_t *outBuff);

struct t_FLR_ROIC_FPATEMP_TABLE_T {
    int16_t value[32];
};
typedef struct t_FLR_ROIC_FPATEMP_TABLE_T FLR_ROIC_FPATEMP_TABLE_T;

void byteToFLR_ROIC_FPATEMP_TABLE_T(const uint8_t *inBuff, FLR_ROIC_FPATEMP_TABLE_T *outVal);
void FLR_ROIC_FPATEMP_TABLE_TToByte(const FLR_ROIC_FPATEMP_TABLE_T *inVal, const uint8_t *outBuff);

struct t_FLR_SCALER_ZOOM_PARAMS_T {
    uint32_t zoom;
    uint32_t xCenter;
    uint32_t yCenter;
};
typedef struct t_FLR_SCALER_ZOOM_PARAMS_T FLR_SCALER_ZOOM_PARAMS_T;

void byteToFLR_SCALER_ZOOM_PARAMS_T(const uint8_t *inBuff, FLR_SCALER_ZOOM_PARAMS_T *outVal);
void FLR_SCALER_ZOOM_PARAMS_TToByte(const FLR_SCALER_ZOOM_PARAMS_T *inVal, const uint8_t *outBuff);

struct t_FLR_SPNR_PSD_KERNEL_T {
    float fvalue[64];
};
typedef struct t_FLR_SPNR_PSD_KERNEL_T FLR_SPNR_PSD_KERNEL_T;

void byteToFLR_SPNR_PSD_KERNEL_T(const uint8_t *inBuff, FLR_SPNR_PSD_KERNEL_T *outVal);
void FLR_SPNR_PSD_KERNEL_TToByte(const FLR_SPNR_PSD_KERNEL_T *inVal, const uint8_t *outBuff);

struct t_FLR_SPOTMETER_SPOT_PARAM_T {
    uint16_t row;
    uint16_t column;
    uint16_t value;
};
typedef struct t_FLR_SPOTMETER_SPOT_PARAM_T FLR_SPOTMETER_SPOT_PARAM_T;

void byteToFLR_SPOTMETER_SPOT_PARAM_T(const uint8_t *inBuff, FLR_SPOTMETER_SPOT_PARAM_T *outVal);
void FLR_SPOTMETER_SPOT_PARAM_TToByte(const FLR_SPOTMETER_SPOT_PARAM_T *inVal, const uint8_t *outBuff);

struct t_FLR_SPOTMETER_STAT_PARAM_TEMP_T {
    uint16_t row;
    uint16_t column;
    float value;
};
typedef struct t_FLR_SPOTMETER_STAT_PARAM_TEMP_T FLR_SPOTMETER_STAT_PARAM_TEMP_T;

void byteToFLR_SPOTMETER_STAT_PARAM_TEMP_T(const uint8_t *inBuff, FLR_SPOTMETER_STAT_PARAM_TEMP_T *outVal);
void FLR_SPOTMETER_STAT_PARAM_TEMP_TToByte(const FLR_SPOTMETER_STAT_PARAM_TEMP_T *inVal, const uint8_t *outBuff);

struct t_FLR_SYSINFO_MONITOR_BUILD_VARIANT_T {
    uint8_t value[50];
};
typedef struct t_FLR_SYSINFO_MONITOR_BUILD_VARIANT_T FLR_SYSINFO_MONITOR_BUILD_VARIANT_T;

void byteToFLR_SYSINFO_MONITOR_BUILD_VARIANT_T(const uint8_t *inBuff, FLR_SYSINFO_MONITOR_BUILD_VARIANT_T *outVal);
void FLR_SYSINFO_MONITOR_BUILD_VARIANT_TToByte(const FLR_SYSINFO_MONITOR_BUILD_VARIANT_T *inVal, const uint8_t *outBuff);

struct t_FLR_SYSINFO_PROBE_TIP_TYPE {
    FLR_SYSINFO_PROBE_TIP_MODEL_E model;
    uint8_t hwRevision;
};
typedef struct t_FLR_SYSINFO_PROBE_TIP_TYPE FLR_SYSINFO_PROBE_TIP_TYPE;

void byteToFLR_SYSINFO_PROBE_TIP_TYPE(const uint8_t *inBuff, FLR_SYSINFO_PROBE_TIP_TYPE *outVal);
void FLR_SYSINFO_PROBE_TIP_TYPEToByte(const FLR_SYSINFO_PROBE_TIP_TYPE *inVal, const uint8_t *outBuff);

struct t_FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T {
    uint8_t id;
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
    uint32_t color;
    int16_t size;
};
typedef struct t_FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T;

void byteToFLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T(const uint8_t *inBuff, FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T *outVal);
void FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_TToByte(const FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T *inVal, const uint8_t *outBuff);

struct t_FLR_SYSTEMSYMBOLS_SPOTCONFIG_T {
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T symbol;
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T area;
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T min;
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T max;
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T mean;
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T meanBar;
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T greenBarOutline;
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T greenBar;
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T greenBarText1;
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T greenBarText2;
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T greenBarText3;
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T greenBarText4;
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T greenBarText5;
};
typedef struct t_FLR_SYSTEMSYMBOLS_SPOTCONFIG_T FLR_SYSTEMSYMBOLS_SPOTCONFIG_T;

void byteToFLR_SYSTEMSYMBOLS_SPOTCONFIG_T(const uint8_t *inBuff, FLR_SYSTEMSYMBOLS_SPOTCONFIG_T *outVal);
void FLR_SYSTEMSYMBOLS_SPOTCONFIG_TToByte(const FLR_SYSTEMSYMBOLS_SPOTCONFIG_T *inVal, const uint8_t *outBuff);

struct t_FLR_SYSTEMSYMBOLS_SPOTCONFIG_ID_T {
    uint8_t symbol;
    uint8_t area;
    uint8_t min;
    uint8_t max;
    uint8_t mean;
    uint8_t meanBar;
    uint8_t greenBarOutline;
    uint8_t greenBar;
    uint8_t greenBarText1;
    uint8_t greenBarText2;
    uint8_t greenBarText3;
    uint8_t greenBarText4;
    uint8_t greenBarText5;
};
typedef struct t_FLR_SYSTEMSYMBOLS_SPOTCONFIG_ID_T FLR_SYSTEMSYMBOLS_SPOTCONFIG_ID_T;

void byteToFLR_SYSTEMSYMBOLS_SPOTCONFIG_ID_T(const uint8_t *inBuff, FLR_SYSTEMSYMBOLS_SPOTCONFIG_ID_T *outVal);
void FLR_SYSTEMSYMBOLS_SPOTCONFIG_ID_TToByte(const FLR_SYSTEMSYMBOLS_SPOTCONFIG_ID_T *inVal, const uint8_t *outBuff);

struct t_FLR_SYSTEMSYMBOLS_ISOCONFIG_T {
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T colorBar;
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T colorBarOutline;
};
typedef struct t_FLR_SYSTEMSYMBOLS_ISOCONFIG_T FLR_SYSTEMSYMBOLS_ISOCONFIG_T;

void byteToFLR_SYSTEMSYMBOLS_ISOCONFIG_T(const uint8_t *inBuff, FLR_SYSTEMSYMBOLS_ISOCONFIG_T *outVal);
void FLR_SYSTEMSYMBOLS_ISOCONFIG_TToByte(const FLR_SYSTEMSYMBOLS_ISOCONFIG_T *inVal, const uint8_t *outBuff);

struct t_FLR_SYSTEMSYMBOLS_ISOCONFIG_ID_T {
    uint8_t colorBar;
    uint8_t colorBarOutline;
};
typedef struct t_FLR_SYSTEMSYMBOLS_ISOCONFIG_ID_T FLR_SYSTEMSYMBOLS_ISOCONFIG_ID_T;

void byteToFLR_SYSTEMSYMBOLS_ISOCONFIG_ID_T(const uint8_t *inBuff, FLR_SYSTEMSYMBOLS_ISOCONFIG_ID_T *outVal);
void FLR_SYSTEMSYMBOLS_ISOCONFIG_ID_TToByte(const FLR_SYSTEMSYMBOLS_ISOCONFIG_ID_T *inVal, const uint8_t *outBuff);

struct t_FLR_SYSTEMSYMBOLS_BARCONFIG_T {
    int16_t val0;
    int16_t val1;
    int16_t val2;
    int16_t val3;
    int16_t val4;
};
typedef struct t_FLR_SYSTEMSYMBOLS_BARCONFIG_T FLR_SYSTEMSYMBOLS_BARCONFIG_T;

void byteToFLR_SYSTEMSYMBOLS_BARCONFIG_T(const uint8_t *inBuff, FLR_SYSTEMSYMBOLS_BARCONFIG_T *outVal);
void FLR_SYSTEMSYMBOLS_BARCONFIG_TToByte(const FLR_SYSTEMSYMBOLS_BARCONFIG_T *inVal, const uint8_t *outBuff);

struct t_FLR_TESTRAMP_SETTINGS_T {
    uint16_t start;
    uint16_t end;
    uint16_t increment;
};
typedef struct t_FLR_TESTRAMP_SETTINGS_T FLR_TESTRAMP_SETTINGS_T;

void byteToFLR_TESTRAMP_SETTINGS_T(const uint8_t *inBuff, FLR_TESTRAMP_SETTINGS_T *outVal);
void FLR_TESTRAMP_SETTINGS_TToByte(const FLR_TESTRAMP_SETTINGS_T *inVal, const uint8_t *outBuff);

struct t_FLR_TESTRAMP_ANIMATION_SETTINGS_T {
    int16_t moveLines;
    uint16_t moveFrames;
};
typedef struct t_FLR_TESTRAMP_ANIMATION_SETTINGS_T FLR_TESTRAMP_ANIMATION_SETTINGS_T;

void byteToFLR_TESTRAMP_ANIMATION_SETTINGS_T(const uint8_t *inBuff, FLR_TESTRAMP_ANIMATION_SETTINGS_T *outVal);
void FLR_TESTRAMP_ANIMATION_SETTINGS_TToByte(const FLR_TESTRAMP_ANIMATION_SETTINGS_T *inVal, const uint8_t *outBuff);

struct t_FLR_TF_WLUT_T {
    uint8_t value[32];
};
typedef struct t_FLR_TF_WLUT_T FLR_TF_WLUT_T;

void byteToFLR_TF_WLUT_T(const uint8_t *inBuff, FLR_TF_WLUT_T *outVal);
void FLR_TF_WLUT_TToByte(const FLR_TF_WLUT_T *inVal, const uint8_t *outBuff);

struct t_FLR_TF_NF_LUT_T {
    uint16_t value[17];
};
typedef struct t_FLR_TF_NF_LUT_T FLR_TF_NF_LUT_T;

void byteToFLR_TF_NF_LUT_T(const uint8_t *inBuff, FLR_TF_NF_LUT_T *outVal);
void FLR_TF_NF_LUT_TToByte(const FLR_TF_NF_LUT_T *inVal, const uint8_t *outBuff);

struct t_FLR_TF_TEMP_SIGNAL_COMP_FACTOR_LUT_T {
    uint16_t value[17];
};
typedef struct t_FLR_TF_TEMP_SIGNAL_COMP_FACTOR_LUT_T FLR_TF_TEMP_SIGNAL_COMP_FACTOR_LUT_T;

void byteToFLR_TF_TEMP_SIGNAL_COMP_FACTOR_LUT_T(const uint8_t *inBuff, FLR_TF_TEMP_SIGNAL_COMP_FACTOR_LUT_T *outVal);
void FLR_TF_TEMP_SIGNAL_COMP_FACTOR_LUT_TToByte(const FLR_TF_TEMP_SIGNAL_COMP_FACTOR_LUT_T *inVal, const uint8_t *outBuff);

#endif //SERIALIZER_STRUCT_H
