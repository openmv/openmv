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

#ifndef CLIENT_PACKAGER_H
#define CLIENT_PACKAGER_H

#include <stdint.h>
#include "EnumTypes.h"
#include "ReturnCodes.h"
#include "Serializer_Struct.h"
#include "FunctionCodes.h"
#include "Client_Dispatcher.h"

// Begin Module: TLinear
FLR_RESULT CLIENT_pkgTlinearSetControl(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgTlinearGetControl(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgTlinearGetLUT(const FLR_BOSON_TABLETYPE_E mode, const uint16_t offset, float a[], float b[]);
FLR_RESULT CLIENT_pkgTlinearRefreshLUT(const FLR_BOSON_TABLETYPE_E mode);
// End Module: TLinear

// Begin Module: agc
FLR_RESULT CLIENT_pkgAgcSetPercentPerBin(const float data);
FLR_RESULT CLIENT_pkgAgcGetPercentPerBin(float *data);
FLR_RESULT CLIENT_pkgAgcSetLinearPercent(const float data);
FLR_RESULT CLIENT_pkgAgcGetLinearPercent(float *data);
FLR_RESULT CLIENT_pkgAgcSetOutlierCut(const float data);
FLR_RESULT CLIENT_pkgAgcGetOutlierCut(float *data);
FLR_RESULT CLIENT_pkgAgcGetDrOut(float *data);
FLR_RESULT CLIENT_pkgAgcSetMaxGain(const float data);
FLR_RESULT CLIENT_pkgAgcGetMaxGain(float *data);
FLR_RESULT CLIENT_pkgAgcSetdf(const float data);
FLR_RESULT CLIENT_pkgAgcGetdf(float *data);
FLR_RESULT CLIENT_pkgAgcSetGamma(const float data);
FLR_RESULT CLIENT_pkgAgcGetGamma(float *data);
FLR_RESULT CLIENT_pkgAgcGetFirstBin(uint32_t *data);
FLR_RESULT CLIENT_pkgAgcGetLastBin(uint32_t *data);
FLR_RESULT CLIENT_pkgAgcSetDetailHeadroom(const float data);
FLR_RESULT CLIENT_pkgAgcGetDetailHeadroom(float *data);
FLR_RESULT CLIENT_pkgAgcSetd2br(const float data);
FLR_RESULT CLIENT_pkgAgcGetd2br(float *data);
FLR_RESULT CLIENT_pkgAgcSetSigmaR(const float data);
FLR_RESULT CLIENT_pkgAgcGetSigmaR(float *data);
FLR_RESULT CLIENT_pkgAgcSetUseEntropy(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgAgcGetUseEntropy(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgAgcSetROI(const FLR_ROI_T roi);
FLR_RESULT CLIENT_pkgAgcGetROI(FLR_ROI_T *roi);
FLR_RESULT CLIENT_pkgAgcGetMaxGainApplied(float *data);
FLR_RESULT CLIENT_pkgAgcGetSigmaRApplied(float *data);
FLR_RESULT CLIENT_pkgAgcSetOutlierCutBalance(const float data);
FLR_RESULT CLIENT_pkgAgcGetOutlierCutBalance(float *data);
FLR_RESULT CLIENT_pkgAgcGetOutlierCutApplied(float *percentHigh, float *percentLow);
FLR_RESULT CLIENT_pkgAgcSetDetailHeadroomBalance(const float data);
FLR_RESULT CLIENT_pkgAgcGetDetailHeadroomBalance(float *data);
FLR_RESULT CLIENT_pkgAgcGetDetailHeadroomApplied(float *countsHigh, float *countsLow);
FLR_RESULT CLIENT_pkgAgcGetTfThresholds(uint16_t *tf_thresholdMin, uint16_t *tf_thresholdMax);
FLR_RESULT CLIENT_pkgAgcSetTfThresholds(const uint16_t tf_thresholdMin, const uint16_t tf_thresholdMax);
FLR_RESULT CLIENT_pkgAgcGetMode(FLR_AGC_MODE_E *mode);
FLR_RESULT CLIENT_pkgAgcSetMode(const FLR_AGC_MODE_E mode);
FLR_RESULT CLIENT_pkgAgcSetHighTempAlarmValues(const uint32_t lowGain, const uint32_t highGain, const uint32_t pixPopulation);
FLR_RESULT CLIENT_pkgAgcGetContrast(int32_t *contrast);
FLR_RESULT CLIENT_pkgAgcSetContrast(const int32_t contrast);
FLR_RESULT CLIENT_pkgAgcGetBrightnessBias(int32_t *brightnessBias);
FLR_RESULT CLIENT_pkgAgcSetBrightnessBias(const int32_t brightnessBias);
FLR_RESULT CLIENT_pkgAgcGetBrightness(int32_t *brightness);
FLR_RESULT CLIENT_pkgAgcSetBrightness(const int32_t brightness);
FLR_RESULT CLIENT_pkgAgcSetMaxGainForLowGain(const float data);
FLR_RESULT CLIENT_pkgAgcGetMaxGainForLowGain(float *data);
FLR_RESULT CLIENT_pkgAgcSetRadius(const uint32_t data);
FLR_RESULT CLIENT_pkgAgcGetRadius(uint32_t *data);
FLR_RESULT CLIENT_pkgAgcSetGmax(const float data);
FLR_RESULT CLIENT_pkgAgcGetGmax(float *data);
FLR_RESULT CLIENT_pkgAgcSetGmin(const float data);
FLR_RESULT CLIENT_pkgAgcGetGmin(float *data);
// End Module: agc

// Begin Module: boson
FLR_RESULT CLIENT_pkgBosonGetCameraSN(uint32_t *data);
FLR_RESULT CLIENT_pkgBosonGetCameraPN(FLR_BOSON_PARTNUMBER_T *data);
FLR_RESULT CLIENT_pkgBosonGetSensorSN(uint32_t *data);
FLR_RESULT CLIENT_pkgBosonRunFFC();
FLR_RESULT CLIENT_pkgBosonSetFFCTempThreshold(const uint16_t data);
FLR_RESULT CLIENT_pkgBosonGetFFCTempThreshold(uint16_t *data);
FLR_RESULT CLIENT_pkgBosonSetFFCFrameThreshold(const uint32_t data);
FLR_RESULT CLIENT_pkgBosonGetFFCFrameThreshold(uint32_t *data);
FLR_RESULT CLIENT_pkgBosonGetFFCInProgress(int16_t *data);
FLR_RESULT CLIENT_pkgBosonReboot();
FLR_RESULT CLIENT_pkgBosonSetFFCMode(const FLR_BOSON_FFCMODE_E ffcMode);
FLR_RESULT CLIENT_pkgBosonGetFFCMode(FLR_BOSON_FFCMODE_E *ffcMode);
FLR_RESULT CLIENT_pkgBosonSetGainMode(const FLR_BOSON_GAINMODE_E gainMode);
FLR_RESULT CLIENT_pkgBosonGetGainMode(FLR_BOSON_GAINMODE_E *gainMode);
FLR_RESULT CLIENT_pkgBosonWriteDynamicHeaderToFlash();
FLR_RESULT CLIENT_pkgBosonReadDynamicHeaderFromFlash();
FLR_RESULT CLIENT_pkgBosonRestoreFactoryDefaultsFromFlash();
FLR_RESULT CLIENT_pkgBosonRestoreFactoryBadPixelsFromFlash();
FLR_RESULT CLIENT_pkgBosonWriteBadPixelsToFlash();
FLR_RESULT CLIENT_pkgBosonGetSoftwareRev(uint32_t *major, uint32_t *minor, uint32_t *patch);
FLR_RESULT CLIENT_pkgBosonSetBadPixelLocation(const uint32_t row, const uint32_t col);
FLR_RESULT CLIENT_pkgBosonlookupFPATempDegCx10(int16_t *data);
FLR_RESULT CLIENT_pkgBosonlookupFPATempDegKx10(uint16_t *data);
FLR_RESULT CLIENT_pkgBosonWriteLensNvFfcToFlash();
FLR_RESULT CLIENT_pkgBosonWriteLensGainToFlash();
FLR_RESULT CLIENT_pkgBosonSetLensNumber(const uint32_t lensNumber);
FLR_RESULT CLIENT_pkgBosonGetLensNumber(uint32_t *lensNumber);
FLR_RESULT CLIENT_pkgBosonSetTableNumber(const uint32_t tableNumber);
FLR_RESULT CLIENT_pkgBosonGetTableNumber(uint32_t *tableNumber);
FLR_RESULT CLIENT_pkgBosonGetSensorPN(FLR_BOSON_SENSOR_PARTNUMBER_T *sensorPN);
FLR_RESULT CLIENT_pkgBosonSetGainSwitchParams(const FLR_BOSON_GAIN_SWITCH_PARAMS_T parm_struct);
FLR_RESULT CLIENT_pkgBosonGetGainSwitchParams(FLR_BOSON_GAIN_SWITCH_PARAMS_T *parm_struct);
FLR_RESULT CLIENT_pkgBosonGetSwitchToHighGainFlag(uint8_t *switchToHighGainFlag);
FLR_RESULT CLIENT_pkgBosonGetSwitchToLowGainFlag(uint8_t *switchToLowGainFlag);
FLR_RESULT CLIENT_pkgBosonGetCLowToHighPercent(uint32_t *cLowToHighPercent);
FLR_RESULT CLIENT_pkgBosonGetMaxNUCTables(uint32_t *maxNUCTables);
FLR_RESULT CLIENT_pkgBosonGetMaxLensTables(uint32_t *maxLensTables);
FLR_RESULT CLIENT_pkgBosonGetFfcWaitCloseFrames(uint16_t *data);
FLR_RESULT CLIENT_pkgBosonSetFfcWaitCloseFrames(const uint16_t data);
FLR_RESULT CLIENT_pkgBosonCheckForTableSwitch();
FLR_RESULT CLIENT_pkgBosonGetDesiredTableNumber(uint32_t *desiredTableNumber);
FLR_RESULT CLIENT_pkgBosonGetFfcStatus(FLR_BOSON_FFCSTATUS_E *ffcStatus);
FLR_RESULT CLIENT_pkgBosonGetFfcDesired(uint32_t *ffcDesired);
FLR_RESULT CLIENT_pkgBosonGetSwRevInHeader(uint32_t *major, uint32_t *minor, uint32_t *patch);
FLR_RESULT CLIENT_pkgBosonGetLastFFCFrameCount(uint32_t *frameCount);
FLR_RESULT CLIENT_pkgBosonGetLastFFCTempDegKx10(uint16_t *temp);
FLR_RESULT CLIENT_pkgBosonGetTableSwitchDesired(uint16_t *tableSwitchDesired);
FLR_RESULT CLIENT_pkgBosonGetOverTempThreshold(float *temperatureInC);
FLR_RESULT CLIENT_pkgBosonGetLowPowerMode(uint16_t *lowPowerMode);
FLR_RESULT CLIENT_pkgBosonGetOverTempEventOccurred(uint16_t *overTempEventOccurred);
FLR_RESULT CLIENT_pkgBosonSetPermitThermalShutdownOverride(const FLR_ENABLE_E permitThermalShutdownOverride);
FLR_RESULT CLIENT_pkgBosonGetPermitThermalShutdownOverride(FLR_ENABLE_E *permitThermalShutdownOverride);
FLR_RESULT CLIENT_pkgBosonGetMyriadTemp(float *myriadTemp);
FLR_RESULT CLIENT_pkgBosonGetNvFFCNucTableNumberLens0(int32_t *nvFFCNucTableNumberLens0);
FLR_RESULT CLIENT_pkgBosonGetNvFFCNucTableNumberLens1(int32_t *nvFFCNucTableNumberLens1);
FLR_RESULT CLIENT_pkgBosonGetNvFFCFPATempDegKx10Lens0(uint16_t *nvFFCFPATempDegKx10Lens0);
FLR_RESULT CLIENT_pkgBosonGetNvFFCFPATempDegKx10Lens1(uint16_t *nvFFCFPATempDegKx10Lens1);
FLR_RESULT CLIENT_pkgBosonSetFFCWarnTimeInSecx10(const uint16_t ffcWarnTime);
FLR_RESULT CLIENT_pkgBosonGetFFCWarnTimeInSecx10(uint16_t *ffcWarnTime);
FLR_RESULT CLIENT_pkgBosonGetOverTempEventCounter(uint32_t *overTempEventCounter);
FLR_RESULT CLIENT_pkgBosonSetOverTempTimerInSec(const uint16_t overTempTimerInSec);
FLR_RESULT CLIENT_pkgBosonGetOverTempTimerInSec(uint16_t *overTempTimerInSec);
FLR_RESULT CLIENT_pkgBosonUnloadCurrentLensCorrections();
FLR_RESULT CLIENT_pkgBosonSetTimeForQuickFFCsInSecs(const uint32_t timeForQuickFFCsInSecs);
FLR_RESULT CLIENT_pkgBosonGetTimeForQuickFFCsInSecs(uint32_t *timeForQuickFFCsInSecs);
FLR_RESULT CLIENT_pkgBosonReloadCurrentLensCorrections();
FLR_RESULT CLIENT_pkgBosonGetBootTimestamps(float *FirstLight, float *StartInit, float *BosonExecDone, float *Timestamp4);
FLR_RESULT CLIENT_pkgBosonSetExtSyncMode(const FLR_BOSON_EXT_SYNC_MODE_E mode);
FLR_RESULT CLIENT_pkgBosonGetExtSyncMode(FLR_BOSON_EXT_SYNC_MODE_E *mode);
FLR_RESULT CLIENT_pkgBosonGetLastCommand(uint32_t *sequenceNum, uint32_t *cmdID);
FLR_RESULT CLIENT_pkgBosonGetSensorHostCalVersion(uint32_t *version);
FLR_RESULT CLIENT_pkgBosonSetDesiredStartupTableNumber(const int32_t table);
FLR_RESULT CLIENT_pkgBosonGetDesiredStartupTableNumber(int32_t *table);
FLR_RESULT CLIENT_pkgBosonSetNvFFCMeanValueLens0(const float meanValue);
FLR_RESULT CLIENT_pkgBosonGetNvFFCMeanValueLens0(float *meanValue);
FLR_RESULT CLIENT_pkgBosonSetNvFFCMeanValueLens1(const float meanValue);
FLR_RESULT CLIENT_pkgBosonGetNvFFCMeanValueLens1(float *meanValue);
FLR_RESULT CLIENT_pkgBosonSetInvertImage(const FLR_ENABLE_E invertImage);
FLR_RESULT CLIENT_pkgBosonGetInvertImage(FLR_ENABLE_E *invertImage);
FLR_RESULT CLIENT_pkgBosonSetRevertImage(const FLR_ENABLE_E revertImage);
FLR_RESULT CLIENT_pkgBosonGetRevertImage(FLR_ENABLE_E *revertImage);
FLR_RESULT CLIENT_pkgBosonGetTimeStamp(const FLR_BOSON_TIMESTAMPTYPE_E timeStampType, float *timeStamp);
FLR_RESULT CLIENT_pkgBosonGetISPFrameCount(uint32_t *ispFrameCount);
FLR_RESULT CLIENT_pkgBosonWriteUserBadPixelsToAllTables();
FLR_RESULT CLIENT_pkgBosonWriteFactoryBadPixelsToAllTables();
FLR_RESULT CLIENT_pkgBosonGetTempDiodeStatus(FLR_BOSON_TEMP_DIODE_STATUS_E *status);
FLR_RESULT CLIENT_pkgBosonClearFactoryBadPixelsInDDR();
FLR_RESULT CLIENT_pkgBosonGetFfcWaitOpenFrames(uint16_t *data);
FLR_RESULT CLIENT_pkgBosonSetFfcWaitOpenFrames(const uint16_t data);
FLR_RESULT CLIENT_pkgBosonGetFfcWaitOpenFlagSettleFrames(uint16_t *data);
FLR_RESULT CLIENT_pkgBosonSetFfcWaitOpenFlagSettleFrames(const uint16_t data);
FLR_RESULT CLIENT_pkgBosonGetTauExtFfcCompatibilityMode(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgBosonSetTauExtFfcCompatibilityMode(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgBosonGetInitialTableSelectionTempOffset(int16_t *data);
FLR_RESULT CLIENT_pkgBosonSetInitialTableSelectionTempOffset(const int16_t data);
FLR_RESULT CLIENT_pkgBosonGetImageValid(int16_t *data);
FLR_RESULT CLIENT_pkgBosonGetCurrentTableType(FLR_BOSON_TABLETYPE_E *data);
FLR_RESULT CLIENT_pkgBosonGetGainSwitchFrameThreshold(uint32_t *data);
FLR_RESULT CLIENT_pkgBosonSetGainSwitchFrameThreshold(const uint32_t data);
FLR_RESULT CLIENT_pkgBosonGetGainSwitchHysteresisTime(float *data);
FLR_RESULT CLIENT_pkgBosonSetGainSwitchHysteresisTime(const float data);
FLR_RESULT CLIENT_pkgBosonGetGainSwitchDesired(uint32_t *data);
FLR_RESULT CLIENT_pkgBosonGetGainSwitchRadiometricParams(FLR_BOSON_GAIN_SWITCH_RADIOMETRIC_PARAMS_T *parm_struct);
FLR_RESULT CLIENT_pkgBosonSetGainSwitchRadiometricParams(const FLR_BOSON_GAIN_SWITCH_RADIOMETRIC_PARAMS_T parm_struct);
FLR_RESULT CLIENT_pkgBosonSetSaturationOverrideMode(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgBosonGetSaturationOverrideMode(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgBosonSetSaturationOverrideValue(const uint16_t data);
FLR_RESULT CLIENT_pkgBosonGetSaturationOverrideValue(uint16_t *data);
FLR_RESULT CLIENT_pkgBosonSetffcHighLowGainThresholdMode(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgBosonGetffcHighLowGainThresholdMode(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgBosonSetFFCTempThresholdLowGain(const uint16_t data);
FLR_RESULT CLIENT_pkgBosonGetFFCTempThresholdLowGain(uint16_t *data);
FLR_RESULT CLIENT_pkgBosonSetFFCFrameThresholdLowGain(const uint32_t data);
FLR_RESULT CLIENT_pkgBosonGetFFCFrameThresholdLowGain(uint32_t *data);
FLR_RESULT CLIENT_pkgBosonGetBoardID(uint32_t *data);
FLR_RESULT CLIENT_pkgBosonSetAutoGainSwitchConditions(const FLR_BOSON_AUTOGAIN_SWITCH_CONDITION_E data);
FLR_RESULT CLIENT_pkgBosonGetAutoGainSwitchConditions(FLR_BOSON_AUTOGAIN_SWITCH_CONDITION_E *data);
FLR_RESULT CLIENT_pkgBosonSetGainSwitchParamsCATS(const FLR_BOSON_GAIN_SWITCH_PARAMS_T parm_struct);
FLR_RESULT CLIENT_pkgBosonGetGainSwitchParamsCATS(FLR_BOSON_GAIN_SWITCH_PARAMS_T *parm_struct);
FLR_RESULT CLIENT_pkgBosonGetGainSwitchRadiometricParamsCATS(FLR_BOSON_GAIN_SWITCH_RADIOMETRIC_PARAMS_T *parm_struct);
FLR_RESULT CLIENT_pkgBosonSetGainSwitchRadiometricParamsCATS(const FLR_BOSON_GAIN_SWITCH_RADIOMETRIC_PARAMS_T parm_struct);
FLR_RESULT CLIENT_pkgBosonGetCLowToHighPercentCATS(uint32_t *cLowToHighPercent);
// End Module: boson

// Begin Module: bpr
FLR_RESULT CLIENT_pkgBprGetState(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgBprSetState(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgBprGetStats(uint32_t *threeby, uint32_t *fiveby, uint32_t *rows, uint32_t *budget, uint32_t *used);
FLR_RESULT CLIENT_pkgBprGetDisplayMode(FLR_BPR_DISPLAY_MODE_E *data);
FLR_RESULT CLIENT_pkgBprSetDisplayMode(const FLR_BPR_DISPLAY_MODE_E data);
FLR_RESULT CLIENT_pkgBprGetDisplayModeMinValue(uint16_t *data);
FLR_RESULT CLIENT_pkgBprSetDisplayModeMinValue(const uint16_t data);
FLR_RESULT CLIENT_pkgBprGetDisplayModeMaxValue(uint16_t *data);
FLR_RESULT CLIENT_pkgBprSetDisplayModeMaxValue(const uint16_t data);
FLR_RESULT CLIENT_pkgBprGetWorkBufIndex(uint32_t *data);
FLR_RESULT CLIENT_pkgBprSetWorkBufIndex(const uint32_t data);
FLR_RESULT CLIENT_pkgBprGetWorkBufStats(uint32_t *threeby, uint32_t *fiveby, uint32_t *rows, uint32_t *budget, uint32_t *used);
// End Module: bpr

// Begin Module: capture
FLR_RESULT CLIENT_pkgCaptureSingleFrame();
FLR_RESULT CLIENT_pkgCaptureFrames(const FLR_CAPTURE_SETTINGS_T data);
FLR_RESULT CLIENT_pkgCaptureSingleFrameWithSrc(const FLR_CAPTURE_SRC_E data);
FLR_RESULT CLIENT_pkgCaptureSingleFrameToFile();
FLR_RESULT CLIENT_pkgCaptureGetStatus(FLR_CAPTURE_STATUS_T *status);
// End Module: capture

// Begin Module: colorLut
FLR_RESULT CLIENT_pkgColorlutSetControl(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgColorlutGetControl(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgColorlutSetId(const FLR_COLORLUT_ID_E data);
FLR_RESULT CLIENT_pkgColorlutGetId(FLR_COLORLUT_ID_E *data);
FLR_RESULT CLIENT_pkgColorlutSetOutlineColor(const uint8_t red, const uint8_t green, const uint8_t blue);
FLR_RESULT CLIENT_pkgColorlutGetOutlineColor(uint8_t *red, uint8_t *green, uint8_t *blue);
// End Module: colorLut

// Begin Module: dummy
FLR_RESULT CLIENT_pkgDummyBadCommand();
// End Module: dummy

// Begin Module: dvo
FLR_RESULT CLIENT_pkgDvoSetAnalogVideoState(const FLR_ENABLE_E analogVideoState);
FLR_RESULT CLIENT_pkgDvoGetAnalogVideoState(FLR_ENABLE_E *analogVideoState);
FLR_RESULT CLIENT_pkgDvoSetOutputFormat(const FLR_DVO_OUTPUT_FORMAT_E format);
FLR_RESULT CLIENT_pkgDvoGetOutputFormat(FLR_DVO_OUTPUT_FORMAT_E *format);
FLR_RESULT CLIENT_pkgDvoSetOutputYCbCrSettings(const FLR_DVO_YCBCR_SETTINGS_T settings);
FLR_RESULT CLIENT_pkgDvoGetOutputYCbCrSettings(FLR_DVO_YCBCR_SETTINGS_T *settings);
FLR_RESULT CLIENT_pkgDvoSetOutputRGBSettings(const FLR_DVO_RGB_SETTINGS_T settings);
FLR_RESULT CLIENT_pkgDvoGetOutputRGBSettings(FLR_DVO_RGB_SETTINGS_T *settings);
FLR_RESULT CLIENT_pkgDvoApplyCustomSettings();
FLR_RESULT CLIENT_pkgDvoSetDisplayMode(const FLR_DVO_DISPLAY_MODE_E displayMode);
FLR_RESULT CLIENT_pkgDvoGetDisplayMode(FLR_DVO_DISPLAY_MODE_E *displayMode);
FLR_RESULT CLIENT_pkgDvoSetType(const FLR_DVO_TYPE_E tap);
FLR_RESULT CLIENT_pkgDvoGetType(FLR_DVO_TYPE_E *tap);
FLR_RESULT CLIENT_pkgDvoSetVideoStandard(const FLR_DVO_VIDEO_STANDARD_E videoStandard);
FLR_RESULT CLIENT_pkgDvoGetVideoStandard(FLR_DVO_VIDEO_STANDARD_E *videoStandard);
FLR_RESULT CLIENT_pkgDvoSetCheckVideoDacPresent(const FLR_ENABLE_E checkVideoDacPresent);
FLR_RESULT CLIENT_pkgDvoGetCheckVideoDacPresent(FLR_ENABLE_E *checkVideoDacPresent);
FLR_RESULT CLIENT_pkgDvoSetCustomLcdConfig(const FLR_DVO_LCD_CONFIG_ID_E id, const FLR_DVO_LCD_CONFIG_T config);
FLR_RESULT CLIENT_pkgDvoGetCustomLcdConfig(const FLR_DVO_LCD_CONFIG_ID_E id, FLR_DVO_LCD_CONFIG_T *config);
FLR_RESULT CLIENT_pkgDvoSetLCDConfig(const FLR_DVO_LCD_CONFIG_ID_E id);
FLR_RESULT CLIENT_pkgDvoGetLCDConfig(FLR_DVO_LCD_CONFIG_ID_E *id);
FLR_RESULT CLIENT_pkgDvoGetClockInfo(uint32_t *horizontalSyncWidth, uint32_t *verticalSyncWidth, uint32_t *clocksPerRowPeriod, uint32_t *horizontalFrontPorch, uint32_t *horizontalBackPorch, uint32_t *frontTelemetryPixels, uint32_t *rearTelemetryPixels, uint32_t *videoColumns, uint32_t *validColumns, uint32_t *telemetryRows, uint32_t *videoRows, uint32_t *validRows, uint32_t *verticalFrontPorch, uint32_t *verticalBackPorch, uint32_t *rowPeriodsPerFrame, uint32_t *clocksPerFrame, float *clockRateInMHz, float *frameRateInHz, uint32_t *validOnRisingEdge, uint32_t *dataWidthInBits);
FLR_RESULT CLIENT_pkgDvoSetAllCustomLcdConfigs(const FLR_DVO_LCD_CONFIG_T config0, const FLR_DVO_LCD_CONFIG_T config1);
FLR_RESULT CLIENT_pkgDvoGetAllCustomLcdConfigs(FLR_DVO_LCD_CONFIG_T *config0, FLR_DVO_LCD_CONFIG_T *config1);
FLR_RESULT CLIENT_pkgDvoSetOutputIr16Format(const FLR_DVO_OUTPUT_IR16_FORMAT_E format);
FLR_RESULT CLIENT_pkgDvoGetOutputIr16Format(FLR_DVO_OUTPUT_IR16_FORMAT_E *format);
FLR_RESULT CLIENT_pkgDvoSetLcdClockRate(const FLR_DVO_LCD_CLOCK_RATE_E clockRate);
FLR_RESULT CLIENT_pkgDvoGetLcdClockRate(FLR_DVO_LCD_CLOCK_RATE_E *clockRate);
FLR_RESULT CLIENT_pkgDvoSetLcdVideoFrameRate(const uint32_t framerate);
FLR_RESULT CLIENT_pkgDvoGetLcdVideoFrameRate(uint32_t *framerate);
FLR_RESULT CLIENT_pkgDvoSetMipiStartState(const FLR_DVO_MIPI_STATE_E state);
FLR_RESULT CLIENT_pkgDvoGetMipiStartState(FLR_DVO_MIPI_STATE_E *state);
FLR_RESULT CLIENT_pkgDvoSetMipiState(const FLR_DVO_MIPI_STATE_E state);
FLR_RESULT CLIENT_pkgDvoGetMipiState(FLR_DVO_MIPI_STATE_E *state);
FLR_RESULT CLIENT_pkgDvoSetMipiClockLaneMode(const FLR_DVO_MIPI_CLOCK_LANE_MODE_E mode);
FLR_RESULT CLIENT_pkgDvoGetMipiClockLaneMode(FLR_DVO_MIPI_CLOCK_LANE_MODE_E *mode);
FLR_RESULT CLIENT_pkgDvoSetOutputInterface(const FLR_DVO_OUTPUT_INTERFACE_E format);
FLR_RESULT CLIENT_pkgDvoGetOutputInterface(FLR_DVO_OUTPUT_INTERFACE_E *format);
FLR_RESULT CLIENT_pkgDvoSetOutputFormatVC1(const FLR_DVO_OUTPUT_FORMAT_E format);
FLR_RESULT CLIENT_pkgDvoGetOutputFormatVC1(FLR_DVO_OUTPUT_FORMAT_E *format);
// End Module: dvo

// Begin Module: dvoMux
FLR_RESULT CLIENT_pkgDvomuxSetType(const FLR_DVOMUX_OUTPUT_IF_E output, const FLR_DVOMUX_SOURCE_E source, const FLR_DVOMUX_TYPE_E type);
FLR_RESULT CLIENT_pkgDvomuxGetType(const FLR_DVOMUX_OUTPUT_IF_E output, FLR_DVOMUX_SOURCE_E *source, FLR_DVOMUX_TYPE_E *type);
// End Module: dvoMux

// Begin Module: fileOps
FLR_RESULT CLIENT_pkgFileopsDir(uint8_t dirent[]);
FLR_RESULT CLIENT_pkgFileopsCd(const uint8_t path[128], uint8_t pwd[]);
FLR_RESULT CLIENT_pkgFileopsMd(const uint8_t path[128]);
FLR_RESULT CLIENT_pkgFileopsFopen(const uint8_t path[128], const uint8_t mode[128], uint32_t *id);
FLR_RESULT CLIENT_pkgFileopsFclose(const uint32_t id);
FLR_RESULT CLIENT_pkgFileopsFread(const uint32_t id, const uint32_t length, uint8_t buf[], uint32_t *ret);
FLR_RESULT CLIENT_pkgFileopsFwrite(const uint32_t id, const uint32_t length, const uint8_t buf[128], uint32_t *ret);
FLR_RESULT CLIENT_pkgFileopsFtell(const uint32_t id, uint32_t *offset);
FLR_RESULT CLIENT_pkgFileopsFseek(const uint32_t id, const uint32_t offset, const uint32_t origin);
FLR_RESULT CLIENT_pkgFileopsFtruncate(const uint32_t id, const uint32_t length);
FLR_RESULT CLIENT_pkgFileopsRmdir(const uint8_t path[128]);
FLR_RESULT CLIENT_pkgFileopsRm(const uint8_t path[128]);
FLR_RESULT CLIENT_pkgFileopsRename(const uint8_t oldpath[128], const uint8_t newpath[128]);
FLR_RESULT CLIENT_pkgFileopsGetFileSize(const uint8_t path[128], uint32_t *fileLength);
// End Module: fileOps

// Begin Module: flashIO
FLR_RESULT CLIENT_pkgFlashioSetProtectionState(const FLR_ENABLE_E protectionState);
FLR_RESULT CLIENT_pkgFlashioGetProtectionState(FLR_ENABLE_E *protectionState);
// End Module: flashIO

// Begin Module: flashMapFs
FLR_RESULT CLIENT_pkgFlashmapfsGetHeaderVersion(uint32_t *major, uint32_t *minor, uint32_t *patch);
// End Module: flashMapFs

// Begin Module: gao
FLR_RESULT CLIENT_pkgGaoSetGainState(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgGaoGetGainState(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgGaoSetFfcState(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgGaoGetFfcState(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgGaoSetTempCorrectionState(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgGaoGetTempCorrectionState(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgGaoSetIConstL(const int16_t data);
FLR_RESULT CLIENT_pkgGaoGetIConstL(int16_t *data);
FLR_RESULT CLIENT_pkgGaoSetIConstM(const int16_t data);
FLR_RESULT CLIENT_pkgGaoGetIConstM(int16_t *data);
FLR_RESULT CLIENT_pkgGaoSetAveragerState(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgGaoGetAveragerState(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgGaoSetNumFFCFrames(const uint16_t data);
FLR_RESULT CLIENT_pkgGaoGetNumFFCFrames(uint16_t *data);
FLR_RESULT CLIENT_pkgGaoGetAveragerThreshold(uint16_t *data);
FLR_RESULT CLIENT_pkgGaoSetTestRampState(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgGaoGetTestRampState(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgGaoSetSffcState(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgGaoGetSffcState(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgGaoSetNucType(const FLR_GAO_NUC_TYPE_E nucType);
FLR_RESULT CLIENT_pkgGaoGetNucType(FLR_GAO_NUC_TYPE_E *nucType);
FLR_RESULT CLIENT_pkgGaoSetFfcZeroMeanState(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgGaoGetFfcZeroMeanState(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgGaoGetAveragerDesiredState(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgGaoGetAppliedClip(uint16_t *data);
FLR_RESULT CLIENT_pkgGaoSetAppliedClipEnable(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgGaoGetAppliedClipEnable(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgGaoSetFfcShutterSimulationState(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgGaoGetFfcShutterSimulationState(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgGaoSetFfcShutterSimulatorValue(const uint16_t value);
FLR_RESULT CLIENT_pkgGaoGetFfcShutterSimulatorValue(uint16_t *value);
FLR_RESULT CLIENT_pkgGaoSetBcnrState(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgGaoGetBcnrState(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgGaoGetAppliedSffcScaleFactor(float *data);
FLR_RESULT CLIENT_pkgGaoSetSffcMode(const FLR_GAO_SFFC_MODE_E mode);
FLR_RESULT CLIENT_pkgGaoGetSffcMode(FLR_GAO_SFFC_MODE_E *mode);
// End Module: gao

// Begin Module: imageStats
FLR_RESULT CLIENT_pkgImagestatsGetTotalHistPixelsInROI(uint32_t *totalPixelsInROI);
FLR_RESULT CLIENT_pkgImagestatsGetPopBelowLowToHighThresh(uint32_t *popBelowLowToHighThresh);
FLR_RESULT CLIENT_pkgImagestatsGetPopAboveHighToLowThresh(uint32_t *popAboveHighToLowThresh);
FLR_RESULT CLIENT_pkgImagestatsSetROI(const FLR_ROI_T roi);
FLR_RESULT CLIENT_pkgImagestatsGetROI(FLR_ROI_T *roi);
FLR_RESULT CLIENT_pkgImagestatsGetFirstBin(uint16_t *firstBin);
FLR_RESULT CLIENT_pkgImagestatsGetLastBin(uint16_t *lastBin);
FLR_RESULT CLIENT_pkgImagestatsGetMean(uint16_t *mean);
FLR_RESULT CLIENT_pkgImagestatsGetFirstBinInROI(uint16_t *firstBinInROI);
FLR_RESULT CLIENT_pkgImagestatsGetLastBinInROI(uint16_t *lastBinInROI);
FLR_RESULT CLIENT_pkgImagestatsGetMeanInROI(uint16_t *meanInROI);
FLR_RESULT CLIENT_pkgImagestatsGetImageStats(uint16_t *meanIntensity, uint16_t *peakIntensity, uint16_t *baseIntensity);
FLR_RESULT CLIENT_pkgImagestatsGetPopAboveLowToHighThreshCATS(uint32_t *popAboveLowToHighThresh);
FLR_RESULT CLIENT_pkgImagestatsGetPopBelowHighToLowThreshCATS(uint32_t *popBelowHighToLowThresh);
FLR_RESULT CLIENT_pkgImagestatsGetPopBetweenLthCATSAndLthSATS(uint32_t *popBetweenCatsAndSats);
// End Module: imageStats

// Begin Module: isotherm
FLR_RESULT CLIENT_pkgIsothermGetEnable(FLR_ENABLE_E *isothermEnable);
FLR_RESULT CLIENT_pkgIsothermSetEnable(const FLR_ENABLE_E isothermEnable);
FLR_RESULT CLIENT_pkgIsothermSetTemps(const FLR_ISOTHERM_GAIN_E table, const int32_t thIsoT1, const int32_t thIsoT2, const int32_t thIsoT3, const int32_t thIsoT4, const int32_t thIsoT5);
FLR_RESULT CLIENT_pkgIsothermGetTemps(const FLR_ISOTHERM_GAIN_E table, int32_t *thIsoT1, int32_t *thIsoT2, int32_t *thIsoT3, int32_t *thIsoT4, int32_t *thIsoT5);
FLR_RESULT CLIENT_pkgIsothermSetIsoColorValues(const FLR_ISOTHERM_GAIN_E table, const FLR_ISOTHERM_COLORS_T region0, const FLR_ISOTHERM_COLORS_T region1, const FLR_ISOTHERM_COLORS_T region2, const FLR_ISOTHERM_COLORS_T region3, const FLR_ISOTHERM_COLORS_T region4, const FLR_ISOTHERM_COLORS_T region5);
FLR_RESULT CLIENT_pkgIsothermGetIsoColorValues(const FLR_ISOTHERM_GAIN_E table, FLR_ISOTHERM_COLORS_T *region0, FLR_ISOTHERM_COLORS_T *region1, FLR_ISOTHERM_COLORS_T *region2, FLR_ISOTHERM_COLORS_T *region3, FLR_ISOTHERM_COLORS_T *region4, FLR_ISOTHERM_COLORS_T *region5);
FLR_RESULT CLIENT_pkgIsothermSetRegionMode(const FLR_ISOTHERM_GAIN_E table, const FLR_ISOTHERM_REGION_E region0, const FLR_ISOTHERM_REGION_E region1, const FLR_ISOTHERM_REGION_E region2, const FLR_ISOTHERM_REGION_E region3, const FLR_ISOTHERM_REGION_E region4, const FLR_ISOTHERM_REGION_E region5);
FLR_RESULT CLIENT_pkgIsothermGetRegionMode(const FLR_ISOTHERM_GAIN_E table, FLR_ISOTHERM_REGION_E *region0, FLR_ISOTHERM_REGION_E *region1, FLR_ISOTHERM_REGION_E *region2, FLR_ISOTHERM_REGION_E *region3, FLR_ISOTHERM_REGION_E *region4, FLR_ISOTHERM_REGION_E *region5);
FLR_RESULT CLIENT_pkgIsothermGetUnit(FLR_ISOTHERM_UNIT_E *unit);
FLR_RESULT CLIENT_pkgIsothermSetUnit(const FLR_ISOTHERM_UNIT_E unit);
FLR_RESULT CLIENT_pkgIsothermGetSettingsLowGain(FLR_ISOTHERM_SETTINGS_T *settings);
FLR_RESULT CLIENT_pkgIsothermSetSettingsLowGain(const FLR_ISOTHERM_SETTINGS_T settings);
FLR_RESULT CLIENT_pkgIsothermGetSettingsHighGain(FLR_ISOTHERM_SETTINGS_T *settings);
FLR_RESULT CLIENT_pkgIsothermSetSettingsHighGain(const FLR_ISOTHERM_SETTINGS_T settings);
FLR_RESULT CLIENT_pkgIsothermSetColorLutId(const FLR_COLORLUT_ID_E colorLutIdLowGain, const FLR_COLORLUT_ID_E colorLutIdHighGain);
FLR_RESULT CLIENT_pkgIsothermGetColorLutId(FLR_COLORLUT_ID_E *colorLutIdLowGain, FLR_COLORLUT_ID_E *colorLutIdHighGain);
// End Module: isotherm

// Begin Module: jffs2
FLR_RESULT CLIENT_pkgJffs2Mount();
FLR_RESULT CLIENT_pkgJffs2Unmount();
FLR_RESULT CLIENT_pkgJffs2GetState(FLR_JFFS2_STATE_E *state);
// End Module: jffs2

// Begin Module: lagrange
// End Module: lagrange

// Begin Module: latencyCtrl
FLR_RESULT CLIENT_pkgLatencyctrlSetLowLatencyState(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgLatencyctrlGetLowLatencyState(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgLatencyctrlSetJitterReduction(const FLR_ENABLE_E enable, const int32_t line);
FLR_RESULT CLIENT_pkgLatencyctrlGetJitterReduction(FLR_ENABLE_E *enable, int32_t *line);
FLR_RESULT CLIENT_pkgLatencyctrlLatencyResetStats();
FLR_RESULT CLIENT_pkgLatencyctrlGetJitter(float *jitterMin, float *jitterMax);
FLR_RESULT CLIENT_pkgLatencyctrlGetLatency(float *latencyMin, float *latencyMax);
FLR_RESULT CLIENT_pkgLatencyctrlSetUsbVideoLatencyReduction(const int32_t line);
FLR_RESULT CLIENT_pkgLatencyctrlGetUsbVideoLatencyReduction(int32_t *line);
// End Module: latencyCtrl

// Begin Module: lfsr
FLR_RESULT CLIENT_pkgLfsrSetApplyOffsetEnableState(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgLfsrGetApplyOffsetEnableState(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgLfsrSetMaxIterations(const uint32_t data);
FLR_RESULT CLIENT_pkgLfsrGetMaxIterations(uint32_t *data);
FLR_RESULT CLIENT_pkgLfsrSetDf(const uint32_t data);
FLR_RESULT CLIENT_pkgLfsrGetDf(uint32_t *data);
FLR_RESULT CLIENT_pkgLfsrSetLambda1(const float data);
FLR_RESULT CLIENT_pkgLfsrGetLambda1(float *data);
FLR_RESULT CLIENT_pkgLfsrSetLambda2(const float data);
FLR_RESULT CLIENT_pkgLfsrGetLambda2(float *data);
FLR_RESULT CLIENT_pkgLfsrSetHaltEnable(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgLfsrGetHaltEnable(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgLfsrSetRandomMethod(const uint32_t data);
FLR_RESULT CLIENT_pkgLfsrGetRandomMethod(uint32_t *data);
FLR_RESULT CLIENT_pkgLfsrSetSingleStepEnable(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgLfsrGetSingleStepEnable(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgLfsrSetR_LocalBump(const float data);
FLR_RESULT CLIENT_pkgLfsrGetR_LocalBump(float *data);
FLR_RESULT CLIENT_pkgLfsrSetR_CornerBump(const float data);
FLR_RESULT CLIENT_pkgLfsrGetR_CornerBump(float *data);
FLR_RESULT CLIENT_pkgLfsrSetFFC_ResetEnable(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgLfsrGetFFC_ResetEnable(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgLfsrSetNormalizeAtCenterSpotState(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgLfsrGetNormalizeAtCenterSpotState(FLR_ENABLE_E *data);
// End Module: lfsr

// Begin Module: mem
FLR_RESULT CLIENT_pkgMemReadCapture(const uint8_t bufferNum, const uint32_t offset, const uint16_t sizeInBytes, uint8_t *data);
FLR_RESULT CLIENT_pkgMemGetCaptureSize(uint32_t *bytes, uint16_t *rows, uint16_t *columns);
FLR_RESULT CLIENT_pkgMemWriteFlash(const FLR_MEM_LOCATION_E location, const uint8_t index, const uint32_t offset, const uint16_t sizeInBytes, uint8_t *data);
FLR_RESULT CLIENT_pkgMemReadFlash(const FLR_MEM_LOCATION_E location, const uint8_t index, const uint32_t offset, const uint16_t sizeInBytes, uint8_t *data);
FLR_RESULT CLIENT_pkgMemGetFlashSize(const FLR_MEM_LOCATION_E location, uint32_t *bytes);
FLR_RESULT CLIENT_pkgMemEraseFlash(const FLR_MEM_LOCATION_E location, const uint8_t index);
FLR_RESULT CLIENT_pkgMemEraseFlashPartial(const FLR_MEM_LOCATION_E location, const uint8_t index, const uint32_t offset, const uint32_t length);
FLR_RESULT CLIENT_pkgMemReadCurrentGain(const uint32_t offset, const uint16_t sizeInBytes, uint8_t *data);
FLR_RESULT CLIENT_pkgMemGetGainSize(uint32_t *bytes, uint16_t *rows, uint16_t *columns);
FLR_RESULT CLIENT_pkgMemGetCaptureSizeSrc(const FLR_CAPTURE_SRC_E src, uint32_t *bytes, uint16_t *rows, uint16_t *columns);
FLR_RESULT CLIENT_pkgMemReadCaptureSrc(const FLR_CAPTURE_SRC_E src, const uint8_t bufferNum, const uint32_t offset, const uint16_t sizeInBytes, uint8_t *data);
// End Module: mem

// Begin Module: normalize
// End Module: normalize

// Begin Module: radiometry
FLR_RESULT CLIENT_pkgRadiometrySetTempStableEnable(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgRadiometryGetTempStableEnable(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgRadiometrySetFNumberLens0(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetFNumberLens0(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetFNumberLens1(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetFNumberLens1(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetTauLens0(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetTauLens0(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetTauLens1(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetTauLens1(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetGlobalGainDesired(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetGlobalOffsetDesired(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetGlobalGainApplied(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetGlobalOffsetApplied(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetTComponentOverrideMode(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgRadiometryGetTComponentOverrideMode(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgRadiometrySetGlobalGainOverride(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetGlobalGainOverride(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetGlobalOffsetOverride(const uint16_t data);
FLR_RESULT CLIENT_pkgRadiometryGetGlobalOffsetOverride(uint16_t *data);
FLR_RESULT CLIENT_pkgRadiometrySetGlobalParamOverrideMode(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgRadiometryGetGlobalParamOverrideMode(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgRadiometrySetRBFOHighGainDefault(const FLR_RADIOMETRY_RBFO_PARAMS_T data);
FLR_RESULT CLIENT_pkgRadiometryGetRBFOHighGainDefault(FLR_RADIOMETRY_RBFO_PARAMS_T *data);
FLR_RESULT CLIENT_pkgRadiometrySetRBFOLowGainDefault(const FLR_RADIOMETRY_RBFO_PARAMS_T data);
FLR_RESULT CLIENT_pkgRadiometryGetRBFOLowGainDefault(FLR_RADIOMETRY_RBFO_PARAMS_T *data);
FLR_RESULT CLIENT_pkgRadiometrySetRBFOHighGainFactory(const FLR_RADIOMETRY_RBFO_PARAMS_T data);
FLR_RESULT CLIENT_pkgRadiometryGetRBFOHighGainFactory(FLR_RADIOMETRY_RBFO_PARAMS_T *data);
FLR_RESULT CLIENT_pkgRadiometrySetRBFOLowGainFactory(const FLR_RADIOMETRY_RBFO_PARAMS_T data);
FLR_RESULT CLIENT_pkgRadiometryGetRBFOLowGainFactory(FLR_RADIOMETRY_RBFO_PARAMS_T *data);
FLR_RESULT CLIENT_pkgRadiometrySetDampingFactor(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetDampingFactor(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetGoMEQ(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetGoMShutter(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetGoMLens(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetGoMLG(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetGoMFFC(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetTempLensHousing(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetTempShutterHousing(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetTempShutterPaddle(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetFNumberShutterHousing(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetFNumberShutterHousing(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetEmissivityShutterHousing(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetEmissivityShutterHousing(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetM_DTfpa_Lens(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetM_DTfpa_Lens(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetOffset_Lens(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetOffset_Lens(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetM_Recursive_Lens(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetM_Recursive_Lens(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetGgFfc(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetCountsFromTemp(const FLR_RADIOMETRY_RBFO_TYPE_E rbfoType, const float temp, uint16_t *counts);
FLR_RESULT CLIENT_pkgRadiometryGetTempFromCounts(const FLR_RADIOMETRY_RBFO_TYPE_E rbfoType, const uint16_t counts, float *temp);
FLR_RESULT CLIENT_pkgRadiometrySetTempLensHousingOverride(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetTempLensHousingOverride(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetTempShutterHousingOverride(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetTempShutterHousingOverride(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetTempShutterPaddleOverride(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetTempShutterPaddleOverride(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetSignalFactorLut(const FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_LUT_T data);
FLR_RESULT CLIENT_pkgRadiometryGetSignalFactorLut(FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_LUT_T *data);
FLR_RESULT CLIENT_pkgRadiometrySetNoiseFactorLut(const FLR_RADIOMETRY_NOISE_COMP_FACTOR_LUT_T data);
FLR_RESULT CLIENT_pkgRadiometryGetNoiseFactorLut(FLR_RADIOMETRY_NOISE_COMP_FACTOR_LUT_T *data);
FLR_RESULT CLIENT_pkgRadiometrySetM_tfpaK(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetM_tfpaK(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetB_tfpaK(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetB_tfpaK(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetTAuxParams(const FLR_RADIOMETRY_TAUX_PARAMS_T data);
FLR_RESULT CLIENT_pkgRadiometryGetTAuxParams(FLR_RADIOMETRY_TAUX_PARAMS_T *data);
FLR_RESULT CLIENT_pkgRadiometrySetM_tAux(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetM_tAux(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetB_tAux(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetB_tAux(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetTsource_FFC(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetTsource_FFC(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetM_DTfpa_Sh_h(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetM_DTfpa_Sh_h(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetOffset_Sh_h(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetOffset_Sh_h(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetM_Recursive_Sh_h(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetM_Recursive_Sh_h(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetM_DTfpa_Sh_p(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetM_DTfpa_Sh_p(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetOffset_Sh_p(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetOffset_Sh_p(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetM_Recursive_Sh_p(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetM_Recursive_Sh_p(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetM_Delta_Sh_p(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetM_Delta_Sh_p(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetB_Delta_Sh_p(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetB_Delta_Sh_p(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetDtTfpaK(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetDtTfpaK_Damp(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetTAuxK(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetExternalFfcUpdateMode(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgRadiometryGetExternalFfcUpdateMode(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgRadiometryGetGG_scale(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetTempWindow(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetTempWindow(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetTransmissionWindow(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetTransmissionWindow(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetReflectivityWindow(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetReflectivityWindow(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetTempWindowReflection(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetTempWindowReflection(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetTransmissionAtmosphere(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetTransmissionAtmosphere(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetTempAtmosphere(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetTempAtmosphere(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetEmissivityTarget(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetEmissivityTarget(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetTempBackground(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetTempBackground(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetRadiometryCapable(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgRadiometrySetdeltaTempDampingFactor(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetdeltaTempDampingFactor(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetdeltaTempIntervalTime(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetdeltaTempIntervalTime(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetdeltaTempMaxValue(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetdeltaTempMaxValue(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetdeltaTempMaxIncrement(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetdeltaTempMaxIncrement(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetdeltaTempDampingTime(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetdeltaTempDampingTime(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetResponsivityFpaTemp(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetM_Delta_Lens(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetM_Delta_Lens(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetB_Delta_Lens(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetB_Delta_Lens(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetM_Delta_Sh_h(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetM_Delta_Sh_h(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetB_Delta_Sh_h(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetB_Delta_Sh_h(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetGG_Scale_HG(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetGG_Scale_HG(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetGG_Scale_LG(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetGG_Scale_LG(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetRbfoScaledMode(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgRadiometryGetRbfoScaledMode(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgRadiometryGetUncertaintyFactor(FLR_RADIOMETRY_UNCERTAINTY_FACTOR_E *data);
FLR_RESULT CLIENT_pkgRadiometryGetTRoomMinThresh(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetTRoomMaxThresh(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetTOperatingMinThresh(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetTOperatingMaxThresh(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetStableTempThresh(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetSlowDriftThresh(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetFfcTempThresh(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetTargetTempMinThreshLG(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetTargetTempMaxThreshLG(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetMFactorThresh(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetTargetTempMinThreshHG(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetTargetTempMaxThreshHG(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetUncertaintyStatusBits(uint16_t *data);
FLR_RESULT CLIENT_pkgRadiometrySetTemperatureOffset_HG(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetTemperatureOffset_HG(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetTemperatureOffset_LG(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetTemperatureOffset_LG(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetM_Delta_Lens_HG(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetM_Delta_Lens_HG(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetB_Delta_Lens_HG(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetB_Delta_Lens_HG(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetM_Delta_Lens_LG(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetM_Delta_Lens_LG(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetB_Delta_Lens_LG(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetB_Delta_Lens_LG(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetOffset_Lens_HG(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetOffset_Lens_HG(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetOffset_Lens_LG(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetOffset_Lens_LG(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetM_Delta_Sh_p_HG(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetM_Delta_Sh_p_HG(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetB_Delta_Sh_p_HG(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetB_Delta_Sh_p_HG(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetM_Delta_Sh_p_LG(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetM_Delta_Sh_p_LG(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetB_Delta_Sh_p_LG(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetB_Delta_Sh_p_LG(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetM_Delta_Sh_h_HG(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetM_Delta_Sh_h_HG(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetB_Delta_Sh_h_HG(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetB_Delta_Sh_h_HG(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetM_Delta_Sh_h_LG(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetM_Delta_Sh_h_LG(float *data);
FLR_RESULT CLIENT_pkgRadiometrySetB_Delta_Sh_h_LG(const float data);
FLR_RESULT CLIENT_pkgRadiometryGetB_Delta_Sh_h_LG(float *data);
FLR_RESULT CLIENT_pkgRadiometryGetGG_RoomTemp(float *data);
// End Module: radiometry

// Begin Module: roic
FLR_RESULT CLIENT_pkgRoicGetFPATemp(uint16_t *data);
FLR_RESULT CLIENT_pkgRoicGetFrameCount(uint32_t *data);
FLR_RESULT CLIENT_pkgRoicGetActiveNormalizationTarget(uint16_t *data);
FLR_RESULT CLIENT_pkgRoicSetFPARampState(const FLR_ENABLE_E state);
FLR_RESULT CLIENT_pkgRoicGetFPARampState(FLR_ENABLE_E *state);
FLR_RESULT CLIENT_pkgRoicGetSensorADC1(uint16_t *data);
FLR_RESULT CLIENT_pkgRoicGetSensorADC2(uint16_t *data);
FLR_RESULT CLIENT_pkgRoicSetFPATempOffset(const int16_t data);
FLR_RESULT CLIENT_pkgRoicGetFPATempOffset(int16_t *data);
FLR_RESULT CLIENT_pkgRoicSetFPATempMode(const FLR_ROIC_TEMP_MODE_E data);
FLR_RESULT CLIENT_pkgRoicGetFPATempMode(FLR_ROIC_TEMP_MODE_E *data);
FLR_RESULT CLIENT_pkgRoicGetFPATempTable(FLR_ROIC_FPATEMP_TABLE_T *table);
FLR_RESULT CLIENT_pkgRoicSetFPATempValue(const uint16_t data);
FLR_RESULT CLIENT_pkgRoicGetFPATempValue(uint16_t *data);
FLR_RESULT CLIENT_pkgRoicGetPreambleError(uint32_t *preambleError);
FLR_RESULT CLIENT_pkgRoicInducePreambleError(const uint32_t everyNthFrame);
FLR_RESULT CLIENT_pkgRoicGetRoicStarted(FLR_ENABLE_E *roicStarted);
FLR_RESULT CLIENT_pkgRoicSetFrameSkip(const uint16_t data);
FLR_RESULT CLIENT_pkgRoicGetFrameSkip(uint16_t *data);
FLR_RESULT CLIENT_pkgRoicSetFrameOneShot();
// End Module: roic

// Begin Module: scaler
FLR_RESULT CLIENT_pkgScalerGetMaxZoom(uint32_t *zoom);
FLR_RESULT CLIENT_pkgScalerSetZoom(const FLR_SCALER_ZOOM_PARAMS_T zoomParams);
FLR_RESULT CLIENT_pkgScalerGetZoom(FLR_SCALER_ZOOM_PARAMS_T *zoomParams);
FLR_RESULT CLIENT_pkgScalerSetFractionalZoom(const uint32_t zoomNumerator, const uint32_t zoomDenominator, const uint32_t zoomXCenter, const uint32_t zoomYCenter, const FLR_ENABLE_E inChangeEnable, const uint32_t zoomOutXCenter, const uint32_t zoomOutYCenter, const FLR_ENABLE_E outChangeEnable);
FLR_RESULT CLIENT_pkgScalerSetIndexZoom(const uint32_t zoomIndex, const uint32_t zoomXCenter, const uint32_t zoomYCenter, const FLR_ENABLE_E inChangeEnable, const uint32_t zoomOutXCenter, const uint32_t zoomOutYCenter, const FLR_ENABLE_E outChangeEnable);
// End Module: scaler

// Begin Module: scnr
FLR_RESULT CLIENT_pkgScnrSetEnableState(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgScnrGetEnableState(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgScnrSetThColSum(const uint16_t data);
FLR_RESULT CLIENT_pkgScnrGetThColSum(uint16_t *data);
FLR_RESULT CLIENT_pkgScnrSetThPixel(const uint16_t data);
FLR_RESULT CLIENT_pkgScnrGetThPixel(uint16_t *data);
FLR_RESULT CLIENT_pkgScnrSetMaxCorr(const uint16_t data);
FLR_RESULT CLIENT_pkgScnrGetMaxCorr(uint16_t *data);
FLR_RESULT CLIENT_pkgScnrGetThPixelApplied(uint16_t *data);
FLR_RESULT CLIENT_pkgScnrGetMaxCorrApplied(uint16_t *data);
FLR_RESULT CLIENT_pkgScnrSetThColSumSafe(const uint16_t data);
FLR_RESULT CLIENT_pkgScnrGetThColSumSafe(uint16_t *data);
FLR_RESULT CLIENT_pkgScnrSetThPixelSafe(const uint16_t data);
FLR_RESULT CLIENT_pkgScnrGetThPixelSafe(uint16_t *data);
FLR_RESULT CLIENT_pkgScnrSetMaxCorrSafe(const uint16_t data);
FLR_RESULT CLIENT_pkgScnrGetMaxCorrSafe(uint16_t *data);
FLR_RESULT CLIENT_pkgScnrSetCorrectionMethod(const FLR_SCNR_CORR_SELECT_E data);
FLR_RESULT CLIENT_pkgScnrGetCorrectionMethod(FLR_SCNR_CORR_SELECT_E *data);
FLR_RESULT CLIENT_pkgScnrSetStdThreshold(const uint16_t data);
FLR_RESULT CLIENT_pkgScnrGetStdThreshold(uint16_t *data);
FLR_RESULT CLIENT_pkgScnrSetNFrames(const uint32_t data);
FLR_RESULT CLIENT_pkgScnrGetNFrames(uint32_t *data);
FLR_RESULT CLIENT_pkgScnrSetResetDesired(const uint32_t data);
FLR_RESULT CLIENT_pkgScnrGetResetDesired(uint32_t *data);
FLR_RESULT CLIENT_pkgScnrSetM_modeOnly(const uint32_t data);
FLR_RESULT CLIENT_pkgScnrGetM_modeOnly(uint32_t *data);
FLR_RESULT CLIENT_pkgScnrGetMode(FLR_SCNR_MODE_E *data);
FLR_RESULT CLIENT_pkgScnrSetSpecklesEnableState(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgScnrGetSpecklesEnableState(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgScnrSetSpecklesThreshold(const uint32_t data);
FLR_RESULT CLIENT_pkgScnrGetSpecklesThreshold(uint32_t *data);
FLR_RESULT CLIENT_pkgScnrSetSpecklesRatio(const float data);
FLR_RESULT CLIENT_pkgScnrGetSpecklesRatio(float *data);
FLR_RESULT CLIENT_pkgScnrSetSpecklesDF(const float data);
FLR_RESULT CLIENT_pkgScnrGetSpecklesDF(float *data);
FLR_RESULT CLIENT_pkgScnrGetSpecklesDiffsBufferAddr(uint32_t *data);
FLR_RESULT CLIENT_pkgScnrGetSpecklesOffsBufferAddr(uint32_t *data);
FLR_RESULT CLIENT_pkgScnrSetSpecklesResetDesired(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgScnrGetSpecklesResetDesired(FLR_ENABLE_E *data);
// End Module: scnr

// Begin Module: sffc
FLR_RESULT CLIENT_pkgSffcGetScaleFactor(float *data);
FLR_RESULT CLIENT_pkgSffcGetDeltaTempLinearCoeff(float *data);
FLR_RESULT CLIENT_pkgSffcSetDeltaTempLinearCoeff(const float data);
FLR_RESULT CLIENT_pkgSffcGetDeltaTempOffsetCoeff(float *data);
FLR_RESULT CLIENT_pkgSffcSetDeltaTempOffsetCoeff(const float data);
FLR_RESULT CLIENT_pkgSffcGetFpaTempLinearCoeff(float *data);
FLR_RESULT CLIENT_pkgSffcSetFpaTempLinearCoeff(const float data);
FLR_RESULT CLIENT_pkgSffcGetFpaTempOffsetCoeff(float *data);
FLR_RESULT CLIENT_pkgSffcSetFpaTempOffsetCoeff(const float data);
FLR_RESULT CLIENT_pkgSffcGetDeltaTempTimeLimitInSecs(uint32_t *data);
FLR_RESULT CLIENT_pkgSffcSetDeltaTempTimeLimitInSecs(const uint32_t data);
// End Module: sffc

// Begin Module: spnr
FLR_RESULT CLIENT_pkgSpnrSetEnableState(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgSpnrGetEnableState(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgSpnrGetState(FLR_SPNR_STATE_E *data);
FLR_RESULT CLIENT_pkgSpnrSetFrameDelay(const uint32_t data);
FLR_RESULT CLIENT_pkgSpnrGetFrameDelay(uint32_t *data);
FLR_RESULT CLIENT_pkgSpnrSetSF(const float sf);
FLR_RESULT CLIENT_pkgSpnrGetSF(float *sf);
FLR_RESULT CLIENT_pkgSpnrGetSFApplied(float *sf);
FLR_RESULT CLIENT_pkgSpnrSetPSDKernel(const FLR_SPNR_PSD_KERNEL_T data);
FLR_RESULT CLIENT_pkgSpnrGetPSDKernel(FLR_SPNR_PSD_KERNEL_T *data);
FLR_RESULT CLIENT_pkgSpnrSetSFMin(const float sfmin);
FLR_RESULT CLIENT_pkgSpnrGetSFMin(float *sfmin);
FLR_RESULT CLIENT_pkgSpnrSetSFMax(const float sfmax);
FLR_RESULT CLIENT_pkgSpnrGetSFMax(float *sfmax);
FLR_RESULT CLIENT_pkgSpnrSetDFMin(const float dfmin);
FLR_RESULT CLIENT_pkgSpnrGetDFMin(float *dfmin);
FLR_RESULT CLIENT_pkgSpnrSetDFMax(const float dfmax);
FLR_RESULT CLIENT_pkgSpnrGetDFMax(float *dfmax);
FLR_RESULT CLIENT_pkgSpnrSetNormTarget(const float normTarget);
FLR_RESULT CLIENT_pkgSpnrGetNormTarget(float *normTarget);
FLR_RESULT CLIENT_pkgSpnrGetNormTargetApplied(float *normTargetApplied);
FLR_RESULT CLIENT_pkgSpnrSetThPix(const uint16_t th_pix);
FLR_RESULT CLIENT_pkgSpnrGetThPix(uint16_t *th_pix);
FLR_RESULT CLIENT_pkgSpnrSetThPixSum(const uint16_t th_pixSum);
FLR_RESULT CLIENT_pkgSpnrGetThPixSum(uint16_t *th_pixSum);
FLR_RESULT CLIENT_pkgSpnrSetMaxcorr(const uint16_t maxcorr);
FLR_RESULT CLIENT_pkgSpnrGetMaxcorr(uint16_t *maxcorr);
FLR_RESULT CLIENT_pkgSpnrGetAlgorithm(FLR_SPNR_ALGORITHM_E *data);
FLR_RESULT CLIENT_pkgSpnrSetAlgorithmDesired(const FLR_SPNR_ALGORITHM_E data);
FLR_RESULT CLIENT_pkgSpnrGetAlgorithmDesired(FLR_SPNR_ALGORITHM_E *data);
FLR_RESULT CLIENT_pkgSpnrSetDFFast(const float dffast);
FLR_RESULT CLIENT_pkgSpnrGetDFFast(float *dffast);
FLR_RESULT CLIENT_pkgSpnrSetDFSlow(const float dfslow);
FLR_RESULT CLIENT_pkgSpnrGetDFSlow(float *dfslow);
FLR_RESULT CLIENT_pkgSpnrSetSensitivityThreshold(const float threshold);
FLR_RESULT CLIENT_pkgSpnrGetSensitivityThreshold(float *threshold);
FLR_RESULT CLIENT_pkgSpnrSetReset(const FLR_SPNR_RESET_E resetType);
// End Module: spnr

// Begin Module: spotMeter
FLR_RESULT CLIENT_pkgSpotmeterSetEnable(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgSpotmeterGetEnable(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgSpotmeterGetRoiMaxSize(uint16_t *width, uint16_t *height);
FLR_RESULT CLIENT_pkgSpotmeterSetRoi(const FLR_ROI_T roi);
FLR_RESULT CLIENT_pkgSpotmeterGetRoi(FLR_ROI_T *roi);
FLR_RESULT CLIENT_pkgSpotmeterGetSpotStats(uint16_t *mean, uint16_t *deviation, FLR_SPOTMETER_SPOT_PARAM_T *min, FLR_SPOTMETER_SPOT_PARAM_T *max);
FLR_RESULT CLIENT_pkgSpotmeterSetStatsMode(const FLR_SPOTMETER_STATS_TEMP_MODE_E mode);
FLR_RESULT CLIENT_pkgSpotmeterGetStatsMode(FLR_SPOTMETER_STATS_TEMP_MODE_E *mode);
FLR_RESULT CLIENT_pkgSpotmeterGetTempStats(float *mean, float *deviation, FLR_SPOTMETER_STAT_PARAM_TEMP_T *min, FLR_SPOTMETER_STAT_PARAM_TEMP_T *max);
// End Module: spotMeter

// Begin Module: srnr
FLR_RESULT CLIENT_pkgSrnrSetEnableState(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgSrnrGetEnableState(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgSrnrSetThRowSum(const uint16_t data);
FLR_RESULT CLIENT_pkgSrnrGetThRowSum(uint16_t *data);
FLR_RESULT CLIENT_pkgSrnrSetThPixel(const uint16_t data);
FLR_RESULT CLIENT_pkgSrnrGetThPixel(uint16_t *data);
FLR_RESULT CLIENT_pkgSrnrSetMaxCorr(const uint16_t data);
FLR_RESULT CLIENT_pkgSrnrGetMaxCorr(uint16_t *data);
FLR_RESULT CLIENT_pkgSrnrGetThPixelApplied(uint16_t *data);
FLR_RESULT CLIENT_pkgSrnrGetMaxCorrApplied(uint16_t *data);
// End Module: srnr

// Begin Module: symbology
FLR_RESULT CLIENT_pkgSymbologySetEnable(const FLR_ENABLE_E draw_symbols);
FLR_RESULT CLIENT_pkgSymbologyCreateBitmap(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t width, const int16_t height);
FLR_RESULT CLIENT_pkgSymbologySendData(const uint8_t ID, const int16_t size, const uint8_t text[128]);
FLR_RESULT CLIENT_pkgSymbologyCreateArc(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t width, const int16_t height, const float start_angle, const float end_angle, const uint32_t color);
FLR_RESULT CLIENT_pkgSymbologyCreateText(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t width, const int16_t height, const int8_t font, const int16_t size, const FLR_SYMBOLOGY_TEXT_ALIGNMENT_E alignment, const uint32_t color, const uint8_t text[128]);
FLR_RESULT CLIENT_pkgSymbologyMoveSprite(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y);
FLR_RESULT CLIENT_pkgSymbologyAddToGroup(const uint8_t ID, const uint8_t group_ID);
FLR_RESULT CLIENT_pkgSymbologyRemoveFromGroup(const uint8_t ID, const uint8_t group_ID);
FLR_RESULT CLIENT_pkgSymbologyUpdateAndShow(const uint8_t ID, const uint8_t visible);
FLR_RESULT CLIENT_pkgSymbologyUpdateAndShowGroup(const uint8_t group_ID, const uint8_t visible);
FLR_RESULT CLIENT_pkgSymbologyDelete(const uint8_t ID);
FLR_RESULT CLIENT_pkgSymbologyDeleteGroup(const uint8_t group_ID);
FLR_RESULT CLIENT_pkgSymbologyCreateFilledRectangle(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t width, const int16_t height, const uint32_t color);
FLR_RESULT CLIENT_pkgSymbologyCreateOutlinedRectangle(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t width, const int16_t height, const uint32_t color);
FLR_RESULT CLIENT_pkgSymbologyCreateBitmapFromPng(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t size);
FLR_RESULT CLIENT_pkgSymbologyCreateCompressedBitmap(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t width, const int16_t height);
FLR_RESULT CLIENT_pkgSymbologyCreateBitmapFromPngFile(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const uint8_t path[128]);
FLR_RESULT CLIENT_pkgSymbologyCreateBitmapFromFile(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const uint8_t path[128], const FLR_SYMBOLOGY_IMAGE_TYPE_E imageType);
FLR_RESULT CLIENT_pkgSymbologyResetWritePosition(const uint8_t ID);
FLR_RESULT CLIENT_pkgSymbologyMoveByOffset(const uint8_t ID, const int16_t off_X, const int16_t off_Y);
FLR_RESULT CLIENT_pkgSymbologyMoveGroupByOffset(const uint8_t ID, const int16_t off_X, const int16_t off_Y);
FLR_RESULT CLIENT_pkgSymbologyCreateFilledEllipse(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t width, const int16_t height, const uint32_t color);
FLR_RESULT CLIENT_pkgSymbologyCreateLine(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t pos_X2, const int16_t pos_Y2, const uint32_t color);
FLR_RESULT CLIENT_pkgSymbologySetZorder(const uint8_t ID, const uint8_t zorder);
FLR_RESULT CLIENT_pkgSymbologySaveConfiguration();
FLR_RESULT CLIENT_pkgSymbologyReloadConfiguration();
FLR_RESULT CLIENT_pkgSymbologyGetEnable(FLR_ENABLE_E *draw_symbols);
FLR_RESULT CLIENT_pkgSymbologySetClonesNumber(const uint8_t ID, const uint8_t numberOfClones);
FLR_RESULT CLIENT_pkgSymbologyMoveCloneByOffset(const uint8_t ID, const uint8_t cloneID, const int16_t pos_X, const int16_t pos_Y);
FLR_RESULT CLIENT_pkgSymbologyMoveCloneSprite(const uint8_t ID, const uint8_t cloneID, const int16_t pos_X, const int16_t pos_Y);
FLR_RESULT CLIENT_pkgSymbologySetTransformation(const FLR_SYMBOLOGY_TRANSFORMATION_E transformation);
FLR_RESULT CLIENT_pkgSymbologyUpdateAllVisible();
FLR_RESULT CLIENT_pkgSymbologySetSizeAndScalingMode(const uint8_t ID, const int16_t width, const int16_t height, const FLR_SYMBOLOGY_SCALING_MODE_E scalingMode);
FLR_RESULT CLIENT_pkgSymbologyCreateLineHVT(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t pos_X2, const int16_t pos_Y2, const uint32_t color1, const uint32_t color2, const uint16_t dashLen, const uint16_t thickness);
FLR_RESULT CLIENT_pkgSymbologyCreateTextHVT(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t width, const int16_t height, const int8_t font, const int16_t size, const FLR_SYMBOLOGY_TEXT_ALIGNMENT_E alignment, const uint32_t color1, const uint32_t color2, const uint8_t dashLen, const uint8_t text[128]);
FLR_RESULT CLIENT_pkgSymbologyCreateTextBg(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t width, const int16_t height, const int8_t font, const int16_t size, const FLR_SYMBOLOGY_TEXT_ALIGNMENT_E alignment, const uint32_t color, const uint32_t bgColor, const uint8_t text[128]);
FLR_RESULT CLIENT_pkgSymbologyCreateScaledBitmapFromFile(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t width, const int16_t height, const FLR_SYMBOLOGY_SCALING_MODE_E scalingMode, const uint8_t path[128], const FLR_SYMBOLOGY_IMAGE_TYPE_E imageType);
FLR_RESULT CLIENT_pkgSymbologyGetLocation(const uint8_t ID, int16_t *pos_X, int16_t *pos_Y);
FLR_RESULT CLIENT_pkgSymbologyGetSize(const uint8_t ID, int16_t *width, int16_t *height);
FLR_RESULT CLIENT_pkgSymbologyGetZorder(const uint8_t ID, uint8_t *zorder);
FLR_RESULT CLIENT_pkgSymbologyGetColor(const uint8_t ID, uint32_t *color);
FLR_RESULT CLIENT_pkgSymbologyGetType(const uint8_t ID, FLR_SYMBOLOGY_TYPE_E *type);
FLR_RESULT CLIENT_pkgSymbologyCopySymbol(const uint8_t source, const uint8_t destination);
FLR_RESULT CLIENT_pkgSymbologyGetTextFontSize(const uint8_t ID, int16_t *size);
// End Module: symbology

// Begin Module: sysctrl
FLR_RESULT CLIENT_pkgSysctrlSetFreezeState(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgSysctrlGetFreezeState(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgSysctrlGetCameraFrameRate(uint32_t *frameRate);
FLR_RESULT CLIENT_pkgSysctrlGetUptimeSecs(uint32_t *uptime);
FLR_RESULT CLIENT_pkgSysctrlSetUsbVideoIR16Mode(const FLR_SYSCTRL_USBIR16_MODE_E data);
FLR_RESULT CLIENT_pkgSysctrlGetUsbVideoIR16Mode(FLR_SYSCTRL_USBIR16_MODE_E *data);
FLR_RESULT CLIENT_pkgSysctrlSetOperatingMode(const FLR_SYSCTRL_OPERATING_MODE_E data);
FLR_RESULT CLIENT_pkgSysctrlGetOperatingMode(FLR_SYSCTRL_OPERATING_MODE_E *data);
FLR_RESULT CLIENT_pkgSysctrlGetAvgFpaTempCounts(float *data);
FLR_RESULT CLIENT_pkgSysctrlSetFpaTempFrames(const uint16_t data);
FLR_RESULT CLIENT_pkgSysctrlGetFpaTempFrames(uint16_t *data);
// End Module: sysctrl

// Begin Module: sysinfo
FLR_RESULT CLIENT_pkgSysinfoGetMonitorSoftwareRev(uint32_t *major, uint32_t *minor, uint32_t *patch);
FLR_RESULT CLIENT_pkgSysinfoGetMonitorBuildVariant(FLR_SYSINFO_MONITOR_BUILD_VARIANT_T *monitorBuildVariant);
FLR_RESULT CLIENT_pkgSysinfoGetProductName(uint8_t name[]);
FLR_RESULT CLIENT_pkgSysinfoGetCameraSN(uint8_t number[]);
FLR_RESULT CLIENT_pkgSysinfoGetBootLocation(uint32_t *bootSwLocation);
FLR_RESULT CLIENT_pkgSysinfoGetSwConfigID(FLR_SYSINFO_SW_CONFIG_ID_E *swConfigID);
FLR_RESULT CLIENT_pkgSysinfoGetSwPermissions(FLR_SYSINFO_SW_PERMISSIONS_E *swPermissions);
FLR_RESULT CLIENT_pkgSysinfoGetIs9HzBuild(uint32_t *is9HzBuild);
FLR_RESULT CLIENT_pkgSysinfoGetProductVersion(uint32_t *major, uint32_t *minor, uint32_t *patch);
FLR_RESULT CLIENT_pkgSysinfoGetMonitorProductRev(uint32_t *major, uint32_t *minor, uint32_t *patch);
FLR_RESULT CLIENT_pkgSysinfoGetOpticalRevision(uint16_t *revision);
FLR_RESULT CLIENT_pkgSysinfoGetSensorRevision(uint16_t *revision);
FLR_RESULT CLIENT_pkgSysinfoGetProbeTipSN(uint8_t number[]);
FLR_RESULT CLIENT_pkgSysinfoGetMechanicalRevision(uint16_t *revision);
FLR_RESULT CLIENT_pkgSysinfoGetProbeTipType(FLR_SYSINFO_PROBE_TIP_TYPE *type);
// End Module: sysinfo

// Begin Module: systemSymbols
FLR_RESULT CLIENT_pkgSystemsymbolsGetID(const FLR_SYSTEMSYMBOLS_SYMBOL_E symbol, uint8_t *id, FLR_SYSTEMSYMBOLS_ID_TYPE_E *id_type);
FLR_RESULT CLIENT_pkgSystemsymbolsSetID(const FLR_SYSTEMSYMBOLS_SYMBOL_E symbol, const uint8_t id, const FLR_SYSTEMSYMBOLS_ID_TYPE_E id_type);
FLR_RESULT CLIENT_pkgSystemsymbolsGetEnable(const FLR_SYSTEMSYMBOLS_SYMBOL_E symbol, FLR_ENABLE_E *enabled);
FLR_RESULT CLIENT_pkgSystemsymbolsSetEnable(const FLR_SYSTEMSYMBOLS_SYMBOL_E symbol, const FLR_ENABLE_E enabled);
FLR_RESULT CLIENT_pkgSystemsymbolsGetSpotConfig(FLR_SYSTEMSYMBOLS_SPOTCONFIG_T *config);
FLR_RESULT CLIENT_pkgSystemsymbolsSetSpotConfig(const FLR_SYSTEMSYMBOLS_SPOTCONFIG_T config);
FLR_RESULT CLIENT_pkgSystemsymbolsGetIsoConfig(FLR_SYSTEMSYMBOLS_ISOCONFIG_T *config);
FLR_RESULT CLIENT_pkgSystemsymbolsSetIsoConfig(const FLR_SYSTEMSYMBOLS_ISOCONFIG_T config);
FLR_RESULT CLIENT_pkgSystemsymbolsGetBarConfig(FLR_SYSTEMSYMBOLS_BARCONFIG_T *lowGainConfig, FLR_SYSTEMSYMBOLS_BARCONFIG_T *highGainConfig, FLR_TEMPERATURE_UNIT_E *unit);
FLR_RESULT CLIENT_pkgSystemsymbolsSetBarConfig(const FLR_SYSTEMSYMBOLS_BARCONFIG_T lowGainConfig, const FLR_SYSTEMSYMBOLS_BARCONFIG_T highGainConfig, const FLR_TEMPERATURE_UNIT_E unit);
FLR_RESULT CLIENT_pkgSystemsymbolsGetSpotConfigIds(FLR_SYSTEMSYMBOLS_SPOTCONFIG_ID_T *config);
FLR_RESULT CLIENT_pkgSystemsymbolsSetSpotConfigIds(const FLR_SYSTEMSYMBOLS_SPOTCONFIG_ID_T config);
FLR_RESULT CLIENT_pkgSystemsymbolsGetIsoConfigIds(FLR_SYSTEMSYMBOLS_ISOCONFIG_ID_T *config);
FLR_RESULT CLIENT_pkgSystemsymbolsSetIsoConfigIds(const FLR_SYSTEMSYMBOLS_ISOCONFIG_ID_T config);
// End Module: systemSymbols

// Begin Module: telemetry
FLR_RESULT CLIENT_pkgTelemetrySetState(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgTelemetryGetState(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgTelemetrySetLocation(const FLR_TELEMETRY_LOC_E data);
FLR_RESULT CLIENT_pkgTelemetryGetLocation(FLR_TELEMETRY_LOC_E *data);
FLR_RESULT CLIENT_pkgTelemetrySetPacking(const FLR_TELEMETRY_PACKING_E data);
FLR_RESULT CLIENT_pkgTelemetryGetPacking(FLR_TELEMETRY_PACKING_E *data);
FLR_RESULT CLIENT_pkgTelemetrySetOrder(const FLR_TELEMETRY_ORDER_E data);
FLR_RESULT CLIENT_pkgTelemetryGetOrder(FLR_TELEMETRY_ORDER_E *data);
FLR_RESULT CLIENT_pkgTelemetrySetPackingVC1(const FLR_TELEMETRY_PACKING_E data);
FLR_RESULT CLIENT_pkgTelemetryGetPackingVC1(FLR_TELEMETRY_PACKING_E *data);
FLR_RESULT CLIENT_pkgTelemetrySetMipiEmbeddedDataTag(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgTelemetryGetMipiEmbeddedDataTag(FLR_ENABLE_E *data);
// End Module: telemetry

// Begin Module: testRamp
FLR_RESULT CLIENT_pkgTestrampSetType(const uint8_t index, const FLR_TESTRAMP_TYPE_E data);
FLR_RESULT CLIENT_pkgTestrampGetType(const uint8_t index, FLR_TESTRAMP_TYPE_E *data);
FLR_RESULT CLIENT_pkgTestrampSetSettings(const uint8_t index, const FLR_TESTRAMP_SETTINGS_T data);
FLR_RESULT CLIENT_pkgTestrampGetSettings(const uint8_t index, FLR_TESTRAMP_SETTINGS_T *data);
FLR_RESULT CLIENT_pkgTestrampSetMotionState(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgTestrampGetMotionState(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgTestrampSetIndex(const uint8_t data);
FLR_RESULT CLIENT_pkgTestrampGetIndex(uint8_t *data);
FLR_RESULT CLIENT_pkgTestrampGetMaxIndex(uint8_t *data);
FLR_RESULT CLIENT_pkgTestrampSetPN9ContinuousMode(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgTestrampGetPN9ContinuousMode(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgTestrampSetAnimationSettings(const FLR_TESTRAMP_ANIMATION_SETTINGS_T settings);
FLR_RESULT CLIENT_pkgTestrampGetAnimationSettings(FLR_TESTRAMP_ANIMATION_SETTINGS_T *settings);
// End Module: testRamp

// Begin Module: tf
FLR_RESULT CLIENT_pkgTfSetEnableState(const FLR_ENABLE_E data);
FLR_RESULT CLIENT_pkgTfGetEnableState(FLR_ENABLE_E *data);
FLR_RESULT CLIENT_pkgTfSetDelta_nf(const uint16_t data);
FLR_RESULT CLIENT_pkgTfGetDelta_nf(uint16_t *data);
FLR_RESULT CLIENT_pkgTfSetTHDeltaMotion(const uint16_t data);
FLR_RESULT CLIENT_pkgTfGetTHDeltaMotion(uint16_t *data);
FLR_RESULT CLIENT_pkgTfSetWLut(const FLR_TF_WLUT_T data);
FLR_RESULT CLIENT_pkgTfGetWLut(FLR_TF_WLUT_T *data);
FLR_RESULT CLIENT_pkgTfGetMotionCount(uint32_t *data);
FLR_RESULT CLIENT_pkgTfSetMotionThreshold(const uint32_t data);
FLR_RESULT CLIENT_pkgTfGetMotionThreshold(uint32_t *data);
FLR_RESULT CLIENT_pkgTfGetDelta_nfApplied(uint16_t *data);
FLR_RESULT CLIENT_pkgTfGetTHDeltaMotionApplied(uint16_t *data);
FLR_RESULT CLIENT_pkgTfSetTempSignalCompFactorLut(const FLR_TF_TEMP_SIGNAL_COMP_FACTOR_LUT_T data);
FLR_RESULT CLIENT_pkgTfGetTempSignalCompFactorLut(FLR_TF_TEMP_SIGNAL_COMP_FACTOR_LUT_T *data);
FLR_RESULT CLIENT_pkgTfGetRnf(uint16_t *rnf);
// End Module: tf

// Begin Module: uart
FLR_RESULT CLIENT_pkgUartSetStartupBaudRate(const FLR_UART_STARTUP_BAUDRATE_E data);
FLR_RESULT CLIENT_pkgUartGetStartupBaudRate(FLR_UART_STARTUP_BAUDRATE_E *data);
// End Module: uart

#endif // CLIENT_PACKAGER_H

