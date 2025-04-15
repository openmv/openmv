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


#include "Client_API.h"

static const uint16_t MaxMemoryChunk = 256;

FLR_RESULT TLinearSetControl(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgTlinearSetControl(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetControl()

FLR_RESULT TLinearGetControl(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgTlinearGetControl(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetControl()

FLR_RESULT TLinearGetLUT(const FLR_BOSON_TABLETYPE_E mode, const uint16_t offset, float a[], float b[]){
    FLR_RESULT returncode = CLIENT_pkgTlinearGetLUT(mode, offset, a, b);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetLUT()

FLR_RESULT TLinearRefreshLUT(const FLR_BOSON_TABLETYPE_E mode){
    FLR_RESULT returncode = CLIENT_pkgTlinearRefreshLUT(mode);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of RefreshLUT()

FLR_RESULT agcSetPercentPerBin(const float data){
    FLR_RESULT returncode = CLIENT_pkgAgcSetPercentPerBin(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetPercentPerBin()

FLR_RESULT agcGetPercentPerBin(float *data){
    FLR_RESULT returncode = CLIENT_pkgAgcGetPercentPerBin(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetPercentPerBin()

FLR_RESULT agcSetLinearPercent(const float data){
    FLR_RESULT returncode = CLIENT_pkgAgcSetLinearPercent(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetLinearPercent()

FLR_RESULT agcGetLinearPercent(float *data){
    FLR_RESULT returncode = CLIENT_pkgAgcGetLinearPercent(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetLinearPercent()

FLR_RESULT agcSetOutlierCut(const float data){
    FLR_RESULT returncode = CLIENT_pkgAgcSetOutlierCut(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetOutlierCut()

FLR_RESULT agcGetOutlierCut(float *data){
    FLR_RESULT returncode = CLIENT_pkgAgcGetOutlierCut(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetOutlierCut()

FLR_RESULT agcGetDrOut(float *data){
    FLR_RESULT returncode = CLIENT_pkgAgcGetDrOut(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDrOut()

FLR_RESULT agcSetMaxGain(const float data){
    FLR_RESULT returncode = CLIENT_pkgAgcSetMaxGain(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetMaxGain()

FLR_RESULT agcGetMaxGain(float *data){
    FLR_RESULT returncode = CLIENT_pkgAgcGetMaxGain(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMaxGain()

FLR_RESULT agcSetdf(const float data){
    FLR_RESULT returncode = CLIENT_pkgAgcSetdf(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of Setdf()

FLR_RESULT agcGetdf(float *data){
    FLR_RESULT returncode = CLIENT_pkgAgcGetdf(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of Getdf()

FLR_RESULT agcSetGamma(const float data){
    FLR_RESULT returncode = CLIENT_pkgAgcSetGamma(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetGamma()

FLR_RESULT agcGetGamma(float *data){
    FLR_RESULT returncode = CLIENT_pkgAgcGetGamma(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGamma()

FLR_RESULT agcGetFirstBin(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgAgcGetFirstBin(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFirstBin()

FLR_RESULT agcGetLastBin(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgAgcGetLastBin(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetLastBin()

FLR_RESULT agcSetDetailHeadroom(const float data){
    FLR_RESULT returncode = CLIENT_pkgAgcSetDetailHeadroom(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetDetailHeadroom()

FLR_RESULT agcGetDetailHeadroom(float *data){
    FLR_RESULT returncode = CLIENT_pkgAgcGetDetailHeadroom(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDetailHeadroom()

FLR_RESULT agcSetd2br(const float data){
    FLR_RESULT returncode = CLIENT_pkgAgcSetd2br(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of Setd2br()

FLR_RESULT agcGetd2br(float *data){
    FLR_RESULT returncode = CLIENT_pkgAgcGetd2br(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of Getd2br()

FLR_RESULT agcSetSigmaR(const float data){
    FLR_RESULT returncode = CLIENT_pkgAgcSetSigmaR(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetSigmaR()

FLR_RESULT agcGetSigmaR(float *data){
    FLR_RESULT returncode = CLIENT_pkgAgcGetSigmaR(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSigmaR()

FLR_RESULT agcSetUseEntropy(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgAgcSetUseEntropy(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetUseEntropy()

FLR_RESULT agcGetUseEntropy(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgAgcGetUseEntropy(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetUseEntropy()

FLR_RESULT agcSetROI(const FLR_ROI_T roi){
    FLR_RESULT returncode = CLIENT_pkgAgcSetROI(roi);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetROI()

FLR_RESULT agcGetROI(FLR_ROI_T *roi){
    FLR_RESULT returncode = CLIENT_pkgAgcGetROI(roi);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetROI()

FLR_RESULT agcGetMaxGainApplied(float *data){
    FLR_RESULT returncode = CLIENT_pkgAgcGetMaxGainApplied(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMaxGainApplied()

FLR_RESULT agcGetSigmaRApplied(float *data){
    FLR_RESULT returncode = CLIENT_pkgAgcGetSigmaRApplied(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSigmaRApplied()

FLR_RESULT agcSetOutlierCutBalance(const float data){
    FLR_RESULT returncode = CLIENT_pkgAgcSetOutlierCutBalance(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetOutlierCutBalance()

FLR_RESULT agcGetOutlierCutBalance(float *data){
    FLR_RESULT returncode = CLIENT_pkgAgcGetOutlierCutBalance(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetOutlierCutBalance()

FLR_RESULT agcGetOutlierCutApplied(float *percentHigh, float *percentLow){
    FLR_RESULT returncode = CLIENT_pkgAgcGetOutlierCutApplied(percentHigh, percentLow);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetOutlierCutApplied()

FLR_RESULT agcSetDetailHeadroomBalance(const float data){
    FLR_RESULT returncode = CLIENT_pkgAgcSetDetailHeadroomBalance(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetDetailHeadroomBalance()

FLR_RESULT agcGetDetailHeadroomBalance(float *data){
    FLR_RESULT returncode = CLIENT_pkgAgcGetDetailHeadroomBalance(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDetailHeadroomBalance()

FLR_RESULT agcGetDetailHeadroomApplied(float *countsHigh, float *countsLow){
    FLR_RESULT returncode = CLIENT_pkgAgcGetDetailHeadroomApplied(countsHigh, countsLow);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDetailHeadroomApplied()

FLR_RESULT agcGetTfThresholds(uint16_t *tf_thresholdMin, uint16_t *tf_thresholdMax){
    FLR_RESULT returncode = CLIENT_pkgAgcGetTfThresholds(tf_thresholdMin, tf_thresholdMax);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTfThresholds()

FLR_RESULT agcSetTfThresholds(const uint16_t tf_thresholdMin, const uint16_t tf_thresholdMax){
    FLR_RESULT returncode = CLIENT_pkgAgcSetTfThresholds(tf_thresholdMin, tf_thresholdMax);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTfThresholds()

FLR_RESULT agcGetMode(FLR_AGC_MODE_E *mode){
    FLR_RESULT returncode = CLIENT_pkgAgcGetMode(mode);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMode()

FLR_RESULT agcSetMode(const FLR_AGC_MODE_E mode){
    FLR_RESULT returncode = CLIENT_pkgAgcSetMode(mode);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetMode()

FLR_RESULT agcSetHighTempAlarmValues(const uint32_t lowGain, const uint32_t highGain, const uint32_t pixPopulation){
    FLR_RESULT returncode = CLIENT_pkgAgcSetHighTempAlarmValues(lowGain, highGain, pixPopulation);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetHighTempAlarmValues()

FLR_RESULT agcGetContrast(int32_t *contrast){
    FLR_RESULT returncode = CLIENT_pkgAgcGetContrast(contrast);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetContrast()

FLR_RESULT agcSetContrast(const int32_t contrast){
    FLR_RESULT returncode = CLIENT_pkgAgcSetContrast(contrast);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetContrast()

FLR_RESULT agcGetBrightnessBias(int32_t *brightnessBias){
    FLR_RESULT returncode = CLIENT_pkgAgcGetBrightnessBias(brightnessBias);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetBrightnessBias()

FLR_RESULT agcSetBrightnessBias(const int32_t brightnessBias){
    FLR_RESULT returncode = CLIENT_pkgAgcSetBrightnessBias(brightnessBias);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetBrightnessBias()

FLR_RESULT agcGetBrightness(int32_t *brightness){
    FLR_RESULT returncode = CLIENT_pkgAgcGetBrightness(brightness);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetBrightness()

FLR_RESULT agcSetBrightness(const int32_t brightness){
    FLR_RESULT returncode = CLIENT_pkgAgcSetBrightness(brightness);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetBrightness()

FLR_RESULT agcSetMaxGainForLowGain(const float data){
    FLR_RESULT returncode = CLIENT_pkgAgcSetMaxGainForLowGain(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetMaxGainForLowGain()

FLR_RESULT agcGetMaxGainForLowGain(float *data){
    FLR_RESULT returncode = CLIENT_pkgAgcGetMaxGainForLowGain(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMaxGainForLowGain()

FLR_RESULT agcSetRadius(const uint32_t data){
    FLR_RESULT returncode = CLIENT_pkgAgcSetRadius(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetRadius()

FLR_RESULT agcGetRadius(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgAgcGetRadius(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetRadius()

FLR_RESULT agcSetGmax(const float data){
    FLR_RESULT returncode = CLIENT_pkgAgcSetGmax(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetGmax()

FLR_RESULT agcGetGmax(float *data){
    FLR_RESULT returncode = CLIENT_pkgAgcGetGmax(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGmax()

FLR_RESULT agcSetGmin(const float data){
    FLR_RESULT returncode = CLIENT_pkgAgcSetGmin(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetGmin()

FLR_RESULT agcGetGmin(float *data){
    FLR_RESULT returncode = CLIENT_pkgAgcGetGmin(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGmin()

FLR_RESULT bosonGetCameraSN(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetCameraSN(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetCameraSN()

FLR_RESULT bosonGetCameraPN(FLR_BOSON_PARTNUMBER_T *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetCameraPN(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetCameraPN()

FLR_RESULT bosonGetSensorSN(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetSensorSN(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSensorSN()

FLR_RESULT bosonRunFFC(){
    FLR_RESULT returncode = CLIENT_pkgBosonRunFFC();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of RunFFC()

FLR_RESULT bosonSetFFCTempThreshold(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgBosonSetFFCTempThreshold(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFFCTempThreshold()

FLR_RESULT bosonGetFFCTempThreshold(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetFFCTempThreshold(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFFCTempThreshold()

FLR_RESULT bosonSetFFCFrameThreshold(const uint32_t data){
    FLR_RESULT returncode = CLIENT_pkgBosonSetFFCFrameThreshold(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFFCFrameThreshold()

FLR_RESULT bosonGetFFCFrameThreshold(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetFFCFrameThreshold(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFFCFrameThreshold()

FLR_RESULT bosonGetFFCInProgress(int16_t *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetFFCInProgress(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFFCInProgress()

FLR_RESULT bosonReboot(){
    FLR_RESULT returncode = CLIENT_pkgBosonReboot();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of Reboot()

FLR_RESULT bosonSetFFCMode(const FLR_BOSON_FFCMODE_E ffcMode){
    FLR_RESULT returncode = CLIENT_pkgBosonSetFFCMode(ffcMode);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFFCMode()

FLR_RESULT bosonGetFFCMode(FLR_BOSON_FFCMODE_E *ffcMode){
    FLR_RESULT returncode = CLIENT_pkgBosonGetFFCMode(ffcMode);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFFCMode()

FLR_RESULT bosonSetGainMode(const FLR_BOSON_GAINMODE_E gainMode){
    FLR_RESULT returncode = CLIENT_pkgBosonSetGainMode(gainMode);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetGainMode()

FLR_RESULT bosonGetGainMode(FLR_BOSON_GAINMODE_E *gainMode){
    FLR_RESULT returncode = CLIENT_pkgBosonGetGainMode(gainMode);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGainMode()

FLR_RESULT bosonWriteDynamicHeaderToFlash(){
    FLR_RESULT returncode = CLIENT_pkgBosonWriteDynamicHeaderToFlash();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of WriteDynamicHeaderToFlash()

FLR_RESULT bosonReadDynamicHeaderFromFlash(){
    FLR_RESULT returncode = CLIENT_pkgBosonReadDynamicHeaderFromFlash();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of ReadDynamicHeaderFromFlash()

FLR_RESULT bosonRestoreFactoryDefaultsFromFlash(){
    FLR_RESULT returncode = CLIENT_pkgBosonRestoreFactoryDefaultsFromFlash();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of RestoreFactoryDefaultsFromFlash()

FLR_RESULT bosonRestoreFactoryBadPixelsFromFlash(){
    FLR_RESULT returncode = CLIENT_pkgBosonRestoreFactoryBadPixelsFromFlash();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of RestoreFactoryBadPixelsFromFlash()

FLR_RESULT bosonWriteBadPixelsToFlash(){
    FLR_RESULT returncode = CLIENT_pkgBosonWriteBadPixelsToFlash();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of WriteBadPixelsToFlash()

FLR_RESULT bosonGetSoftwareRev(uint32_t *major, uint32_t *minor, uint32_t *patch){
    FLR_RESULT returncode = CLIENT_pkgBosonGetSoftwareRev(major, minor, patch);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSoftwareRev()

FLR_RESULT bosonSetBadPixelLocation(const uint32_t row, const uint32_t col){
    FLR_RESULT returncode = CLIENT_pkgBosonSetBadPixelLocation(row, col);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetBadPixelLocation()

FLR_RESULT bosonlookupFPATempDegCx10(int16_t *data){
    FLR_RESULT returncode = CLIENT_pkgBosonlookupFPATempDegCx10(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of lookupFPATempDegCx10()

FLR_RESULT bosonlookupFPATempDegKx10(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgBosonlookupFPATempDegKx10(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of lookupFPATempDegKx10()

FLR_RESULT bosonWriteLensNvFfcToFlash(){
    FLR_RESULT returncode = CLIENT_pkgBosonWriteLensNvFfcToFlash();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of WriteLensNvFfcToFlash()

FLR_RESULT bosonWriteLensGainToFlash(){
    FLR_RESULT returncode = CLIENT_pkgBosonWriteLensGainToFlash();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of WriteLensGainToFlash()

FLR_RESULT bosonSetLensNumber(const uint32_t lensNumber){
    FLR_RESULT returncode = CLIENT_pkgBosonSetLensNumber(lensNumber);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetLensNumber()

FLR_RESULT bosonGetLensNumber(uint32_t *lensNumber){
    FLR_RESULT returncode = CLIENT_pkgBosonGetLensNumber(lensNumber);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetLensNumber()

FLR_RESULT bosonSetTableNumber(const uint32_t tableNumber){
    FLR_RESULT returncode = CLIENT_pkgBosonSetTableNumber(tableNumber);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTableNumber()

FLR_RESULT bosonGetTableNumber(uint32_t *tableNumber){
    FLR_RESULT returncode = CLIENT_pkgBosonGetTableNumber(tableNumber);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTableNumber()

FLR_RESULT bosonGetSensorPN(FLR_BOSON_SENSOR_PARTNUMBER_T *sensorPN){
    FLR_RESULT returncode = CLIENT_pkgBosonGetSensorPN(sensorPN);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSensorPN()

FLR_RESULT bosonSetGainSwitchParams(const FLR_BOSON_GAIN_SWITCH_PARAMS_T parm_struct){
    FLR_RESULT returncode = CLIENT_pkgBosonSetGainSwitchParams(parm_struct);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetGainSwitchParams()

FLR_RESULT bosonGetGainSwitchParams(FLR_BOSON_GAIN_SWITCH_PARAMS_T *parm_struct){
    FLR_RESULT returncode = CLIENT_pkgBosonGetGainSwitchParams(parm_struct);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGainSwitchParams()

FLR_RESULT bosonGetSwitchToHighGainFlag(uint8_t *switchToHighGainFlag){
    FLR_RESULT returncode = CLIENT_pkgBosonGetSwitchToHighGainFlag(switchToHighGainFlag);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSwitchToHighGainFlag()

FLR_RESULT bosonGetSwitchToLowGainFlag(uint8_t *switchToLowGainFlag){
    FLR_RESULT returncode = CLIENT_pkgBosonGetSwitchToLowGainFlag(switchToLowGainFlag);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSwitchToLowGainFlag()

FLR_RESULT bosonGetCLowToHighPercent(uint32_t *cLowToHighPercent){
    FLR_RESULT returncode = CLIENT_pkgBosonGetCLowToHighPercent(cLowToHighPercent);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetCLowToHighPercent()

FLR_RESULT bosonGetMaxNUCTables(uint32_t *maxNUCTables){
    FLR_RESULT returncode = CLIENT_pkgBosonGetMaxNUCTables(maxNUCTables);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMaxNUCTables()

FLR_RESULT bosonGetMaxLensTables(uint32_t *maxLensTables){
    FLR_RESULT returncode = CLIENT_pkgBosonGetMaxLensTables(maxLensTables);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMaxLensTables()

FLR_RESULT bosonGetFfcWaitCloseFrames(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetFfcWaitCloseFrames(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFfcWaitCloseFrames()

FLR_RESULT bosonSetFfcWaitCloseFrames(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgBosonSetFfcWaitCloseFrames(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFfcWaitCloseFrames()

FLR_RESULT bosonCheckForTableSwitch(){
    FLR_RESULT returncode = CLIENT_pkgBosonCheckForTableSwitch();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of CheckForTableSwitch()

FLR_RESULT bosonGetDesiredTableNumber(uint32_t *desiredTableNumber){
    FLR_RESULT returncode = CLIENT_pkgBosonGetDesiredTableNumber(desiredTableNumber);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDesiredTableNumber()

FLR_RESULT bosonGetFfcStatus(FLR_BOSON_FFCSTATUS_E *ffcStatus){
    FLR_RESULT returncode = CLIENT_pkgBosonGetFfcStatus(ffcStatus);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFfcStatus()

FLR_RESULT bosonGetFfcDesired(uint32_t *ffcDesired){
    FLR_RESULT returncode = CLIENT_pkgBosonGetFfcDesired(ffcDesired);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFfcDesired()

FLR_RESULT bosonGetSwRevInHeader(uint32_t *major, uint32_t *minor, uint32_t *patch){
    FLR_RESULT returncode = CLIENT_pkgBosonGetSwRevInHeader(major, minor, patch);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSwRevInHeader()

FLR_RESULT bosonGetLastFFCFrameCount(uint32_t *frameCount){
    FLR_RESULT returncode = CLIENT_pkgBosonGetLastFFCFrameCount(frameCount);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetLastFFCFrameCount()

FLR_RESULT bosonGetLastFFCTempDegKx10(uint16_t *temp){
    FLR_RESULT returncode = CLIENT_pkgBosonGetLastFFCTempDegKx10(temp);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetLastFFCTempDegKx10()

FLR_RESULT bosonGetTableSwitchDesired(uint16_t *tableSwitchDesired){
    FLR_RESULT returncode = CLIENT_pkgBosonGetTableSwitchDesired(tableSwitchDesired);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTableSwitchDesired()

FLR_RESULT bosonGetOverTempThreshold(float *temperatureInC){
    FLR_RESULT returncode = CLIENT_pkgBosonGetOverTempThreshold(temperatureInC);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetOverTempThreshold()

FLR_RESULT bosonGetLowPowerMode(uint16_t *lowPowerMode){
    FLR_RESULT returncode = CLIENT_pkgBosonGetLowPowerMode(lowPowerMode);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetLowPowerMode()

FLR_RESULT bosonGetOverTempEventOccurred(uint16_t *overTempEventOccurred){
    FLR_RESULT returncode = CLIENT_pkgBosonGetOverTempEventOccurred(overTempEventOccurred);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetOverTempEventOccurred()

FLR_RESULT bosonSetPermitThermalShutdownOverride(const FLR_ENABLE_E permitThermalShutdownOverride){
    FLR_RESULT returncode = CLIENT_pkgBosonSetPermitThermalShutdownOverride(permitThermalShutdownOverride);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetPermitThermalShutdownOverride()

FLR_RESULT bosonGetPermitThermalShutdownOverride(FLR_ENABLE_E *permitThermalShutdownOverride){
    FLR_RESULT returncode = CLIENT_pkgBosonGetPermitThermalShutdownOverride(permitThermalShutdownOverride);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetPermitThermalShutdownOverride()

FLR_RESULT bosonGetMyriadTemp(float *myriadTemp){
    FLR_RESULT returncode = CLIENT_pkgBosonGetMyriadTemp(myriadTemp);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMyriadTemp()

FLR_RESULT bosonGetNvFFCNucTableNumberLens0(int32_t *nvFFCNucTableNumberLens0){
    FLR_RESULT returncode = CLIENT_pkgBosonGetNvFFCNucTableNumberLens0(nvFFCNucTableNumberLens0);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetNvFFCNucTableNumberLens0()

FLR_RESULT bosonGetNvFFCNucTableNumberLens1(int32_t *nvFFCNucTableNumberLens1){
    FLR_RESULT returncode = CLIENT_pkgBosonGetNvFFCNucTableNumberLens1(nvFFCNucTableNumberLens1);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetNvFFCNucTableNumberLens1()

FLR_RESULT bosonGetNvFFCFPATempDegKx10Lens0(uint16_t *nvFFCFPATempDegKx10Lens0){
    FLR_RESULT returncode = CLIENT_pkgBosonGetNvFFCFPATempDegKx10Lens0(nvFFCFPATempDegKx10Lens0);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetNvFFCFPATempDegKx10Lens0()

FLR_RESULT bosonGetNvFFCFPATempDegKx10Lens1(uint16_t *nvFFCFPATempDegKx10Lens1){
    FLR_RESULT returncode = CLIENT_pkgBosonGetNvFFCFPATempDegKx10Lens1(nvFFCFPATempDegKx10Lens1);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetNvFFCFPATempDegKx10Lens1()

FLR_RESULT bosonSetFFCWarnTimeInSecx10(const uint16_t ffcWarnTime){
    FLR_RESULT returncode = CLIENT_pkgBosonSetFFCWarnTimeInSecx10(ffcWarnTime);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFFCWarnTimeInSecx10()

FLR_RESULT bosonGetFFCWarnTimeInSecx10(uint16_t *ffcWarnTime){
    FLR_RESULT returncode = CLIENT_pkgBosonGetFFCWarnTimeInSecx10(ffcWarnTime);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFFCWarnTimeInSecx10()

FLR_RESULT bosonGetOverTempEventCounter(uint32_t *overTempEventCounter){
    FLR_RESULT returncode = CLIENT_pkgBosonGetOverTempEventCounter(overTempEventCounter);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetOverTempEventCounter()

FLR_RESULT bosonSetOverTempTimerInSec(const uint16_t overTempTimerInSec){
    FLR_RESULT returncode = CLIENT_pkgBosonSetOverTempTimerInSec(overTempTimerInSec);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetOverTempTimerInSec()

FLR_RESULT bosonGetOverTempTimerInSec(uint16_t *overTempTimerInSec){
    FLR_RESULT returncode = CLIENT_pkgBosonGetOverTempTimerInSec(overTempTimerInSec);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetOverTempTimerInSec()

FLR_RESULT bosonUnloadCurrentLensCorrections(){
    FLR_RESULT returncode = CLIENT_pkgBosonUnloadCurrentLensCorrections();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of UnloadCurrentLensCorrections()

FLR_RESULT bosonSetTimeForQuickFFCsInSecs(const uint32_t timeForQuickFFCsInSecs){
    FLR_RESULT returncode = CLIENT_pkgBosonSetTimeForQuickFFCsInSecs(timeForQuickFFCsInSecs);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTimeForQuickFFCsInSecs()

FLR_RESULT bosonGetTimeForQuickFFCsInSecs(uint32_t *timeForQuickFFCsInSecs){
    FLR_RESULT returncode = CLIENT_pkgBosonGetTimeForQuickFFCsInSecs(timeForQuickFFCsInSecs);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTimeForQuickFFCsInSecs()

FLR_RESULT bosonReloadCurrentLensCorrections(){
    FLR_RESULT returncode = CLIENT_pkgBosonReloadCurrentLensCorrections();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of ReloadCurrentLensCorrections()

FLR_RESULT bosonGetBootTimestamps(float *FirstLight, float *StartInit, float *BosonExecDone, float *Timestamp4){
    FLR_RESULT returncode = CLIENT_pkgBosonGetBootTimestamps(FirstLight, StartInit, BosonExecDone, Timestamp4);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetBootTimestamps()

FLR_RESULT bosonSetExtSyncMode(const FLR_BOSON_EXT_SYNC_MODE_E mode){
    FLR_RESULT returncode = CLIENT_pkgBosonSetExtSyncMode(mode);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetExtSyncMode()

FLR_RESULT bosonGetExtSyncMode(FLR_BOSON_EXT_SYNC_MODE_E *mode){
    FLR_RESULT returncode = CLIENT_pkgBosonGetExtSyncMode(mode);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetExtSyncMode()

FLR_RESULT bosonGetLastCommand(uint32_t *sequenceNum, uint32_t *cmdID){
    FLR_RESULT returncode = CLIENT_pkgBosonGetLastCommand(sequenceNum, cmdID);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetLastCommand()

FLR_RESULT bosonGetSensorHostCalVersion(uint32_t *version){
    FLR_RESULT returncode = CLIENT_pkgBosonGetSensorHostCalVersion(version);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSensorHostCalVersion()

FLR_RESULT bosonSetDesiredStartupTableNumber(const int32_t table){
    FLR_RESULT returncode = CLIENT_pkgBosonSetDesiredStartupTableNumber(table);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetDesiredStartupTableNumber()

FLR_RESULT bosonGetDesiredStartupTableNumber(int32_t *table){
    FLR_RESULT returncode = CLIENT_pkgBosonGetDesiredStartupTableNumber(table);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDesiredStartupTableNumber()

FLR_RESULT bosonSetNvFFCMeanValueLens0(const float meanValue){
    FLR_RESULT returncode = CLIENT_pkgBosonSetNvFFCMeanValueLens0(meanValue);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetNvFFCMeanValueLens0()

FLR_RESULT bosonGetNvFFCMeanValueLens0(float *meanValue){
    FLR_RESULT returncode = CLIENT_pkgBosonGetNvFFCMeanValueLens0(meanValue);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetNvFFCMeanValueLens0()

FLR_RESULT bosonSetNvFFCMeanValueLens1(const float meanValue){
    FLR_RESULT returncode = CLIENT_pkgBosonSetNvFFCMeanValueLens1(meanValue);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetNvFFCMeanValueLens1()

FLR_RESULT bosonGetNvFFCMeanValueLens1(float *meanValue){
    FLR_RESULT returncode = CLIENT_pkgBosonGetNvFFCMeanValueLens1(meanValue);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetNvFFCMeanValueLens1()

FLR_RESULT bosonSetInvertImage(const FLR_ENABLE_E invertImage){
    FLR_RESULT returncode = CLIENT_pkgBosonSetInvertImage(invertImage);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetInvertImage()

FLR_RESULT bosonGetInvertImage(FLR_ENABLE_E *invertImage){
    FLR_RESULT returncode = CLIENT_pkgBosonGetInvertImage(invertImage);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetInvertImage()

FLR_RESULT bosonSetRevertImage(const FLR_ENABLE_E revertImage){
    FLR_RESULT returncode = CLIENT_pkgBosonSetRevertImage(revertImage);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetRevertImage()

FLR_RESULT bosonGetRevertImage(FLR_ENABLE_E *revertImage){
    FLR_RESULT returncode = CLIENT_pkgBosonGetRevertImage(revertImage);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetRevertImage()

FLR_RESULT bosonGetTimeStamp(const FLR_BOSON_TIMESTAMPTYPE_E timeStampType, float *timeStamp){
    FLR_RESULT returncode = CLIENT_pkgBosonGetTimeStamp(timeStampType, timeStamp);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTimeStamp()

FLR_RESULT bosonGetISPFrameCount(uint32_t *ispFrameCount){
    FLR_RESULT returncode = CLIENT_pkgBosonGetISPFrameCount(ispFrameCount);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetISPFrameCount()

FLR_RESULT bosonWriteUserBadPixelsToAllTables(){
    FLR_RESULT returncode = CLIENT_pkgBosonWriteUserBadPixelsToAllTables();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of WriteUserBadPixelsToAllTables()

FLR_RESULT bosonWriteFactoryBadPixelsToAllTables(){
    FLR_RESULT returncode = CLIENT_pkgBosonWriteFactoryBadPixelsToAllTables();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of WriteFactoryBadPixelsToAllTables()

FLR_RESULT bosonGetTempDiodeStatus(FLR_BOSON_TEMP_DIODE_STATUS_E *status){
    FLR_RESULT returncode = CLIENT_pkgBosonGetTempDiodeStatus(status);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTempDiodeStatus()

FLR_RESULT bosonClearFactoryBadPixelsInDDR(){
    FLR_RESULT returncode = CLIENT_pkgBosonClearFactoryBadPixelsInDDR();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of ClearFactoryBadPixelsInDDR()

FLR_RESULT bosonGetFfcWaitOpenFrames(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetFfcWaitOpenFrames(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFfcWaitOpenFrames()

FLR_RESULT bosonSetFfcWaitOpenFrames(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgBosonSetFfcWaitOpenFrames(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFfcWaitOpenFrames()

FLR_RESULT bosonGetFfcWaitOpenFlagSettleFrames(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetFfcWaitOpenFlagSettleFrames(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFfcWaitOpenFlagSettleFrames()

FLR_RESULT bosonSetFfcWaitOpenFlagSettleFrames(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgBosonSetFfcWaitOpenFlagSettleFrames(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFfcWaitOpenFlagSettleFrames()

FLR_RESULT bosonGetTauExtFfcCompatibilityMode(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetTauExtFfcCompatibilityMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTauExtFfcCompatibilityMode()

FLR_RESULT bosonSetTauExtFfcCompatibilityMode(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgBosonSetTauExtFfcCompatibilityMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTauExtFfcCompatibilityMode()

FLR_RESULT bosonGetInitialTableSelectionTempOffset(int16_t *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetInitialTableSelectionTempOffset(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetInitialTableSelectionTempOffset()

FLR_RESULT bosonSetInitialTableSelectionTempOffset(const int16_t data){
    FLR_RESULT returncode = CLIENT_pkgBosonSetInitialTableSelectionTempOffset(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetInitialTableSelectionTempOffset()

FLR_RESULT bosonGetImageValid(int16_t *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetImageValid(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetImageValid()

FLR_RESULT bosonGetCurrentTableType(FLR_BOSON_TABLETYPE_E *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetCurrentTableType(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetCurrentTableType()

FLR_RESULT bosonGetGainSwitchFrameThreshold(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetGainSwitchFrameThreshold(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGainSwitchFrameThreshold()

FLR_RESULT bosonSetGainSwitchFrameThreshold(const uint32_t data){
    FLR_RESULT returncode = CLIENT_pkgBosonSetGainSwitchFrameThreshold(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetGainSwitchFrameThreshold()

FLR_RESULT bosonGetGainSwitchHysteresisTime(float *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetGainSwitchHysteresisTime(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGainSwitchHysteresisTime()

FLR_RESULT bosonSetGainSwitchHysteresisTime(const float data){
    FLR_RESULT returncode = CLIENT_pkgBosonSetGainSwitchHysteresisTime(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetGainSwitchHysteresisTime()

FLR_RESULT bosonGetGainSwitchDesired(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetGainSwitchDesired(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGainSwitchDesired()

FLR_RESULT bosonGetGainSwitchRadiometricParams(FLR_BOSON_GAIN_SWITCH_RADIOMETRIC_PARAMS_T *parm_struct){
    FLR_RESULT returncode = CLIENT_pkgBosonGetGainSwitchRadiometricParams(parm_struct);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGainSwitchRadiometricParams()

FLR_RESULT bosonSetGainSwitchRadiometricParams(const FLR_BOSON_GAIN_SWITCH_RADIOMETRIC_PARAMS_T parm_struct){
    FLR_RESULT returncode = CLIENT_pkgBosonSetGainSwitchRadiometricParams(parm_struct);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetGainSwitchRadiometricParams()

FLR_RESULT bosonSetSaturationOverrideMode(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgBosonSetSaturationOverrideMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetSaturationOverrideMode()

FLR_RESULT bosonGetSaturationOverrideMode(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetSaturationOverrideMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSaturationOverrideMode()

FLR_RESULT bosonSetSaturationOverrideValue(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgBosonSetSaturationOverrideValue(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetSaturationOverrideValue()

FLR_RESULT bosonGetSaturationOverrideValue(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetSaturationOverrideValue(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSaturationOverrideValue()

FLR_RESULT bosonSetffcHighLowGainThresholdMode(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgBosonSetffcHighLowGainThresholdMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetffcHighLowGainThresholdMode()

FLR_RESULT bosonGetffcHighLowGainThresholdMode(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetffcHighLowGainThresholdMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetffcHighLowGainThresholdMode()

FLR_RESULT bosonSetFFCTempThresholdLowGain(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgBosonSetFFCTempThresholdLowGain(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFFCTempThresholdLowGain()

FLR_RESULT bosonGetFFCTempThresholdLowGain(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetFFCTempThresholdLowGain(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFFCTempThresholdLowGain()

FLR_RESULT bosonSetFFCFrameThresholdLowGain(const uint32_t data){
    FLR_RESULT returncode = CLIENT_pkgBosonSetFFCFrameThresholdLowGain(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFFCFrameThresholdLowGain()

FLR_RESULT bosonGetFFCFrameThresholdLowGain(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetFFCFrameThresholdLowGain(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFFCFrameThresholdLowGain()

FLR_RESULT bosonGetBoardID(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetBoardID(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetBoardID()

FLR_RESULT bosonSetAutoGainSwitchConditions(const FLR_BOSON_AUTOGAIN_SWITCH_CONDITION_E data){
    FLR_RESULT returncode = CLIENT_pkgBosonSetAutoGainSwitchConditions(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetAutoGainSwitchConditions()

FLR_RESULT bosonGetAutoGainSwitchConditions(FLR_BOSON_AUTOGAIN_SWITCH_CONDITION_E *data){
    FLR_RESULT returncode = CLIENT_pkgBosonGetAutoGainSwitchConditions(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetAutoGainSwitchConditions()

FLR_RESULT bosonSetGainSwitchParamsCATS(const FLR_BOSON_GAIN_SWITCH_PARAMS_T parm_struct){
    FLR_RESULT returncode = CLIENT_pkgBosonSetGainSwitchParamsCATS(parm_struct);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetGainSwitchParamsCATS()

FLR_RESULT bosonGetGainSwitchParamsCATS(FLR_BOSON_GAIN_SWITCH_PARAMS_T *parm_struct){
    FLR_RESULT returncode = CLIENT_pkgBosonGetGainSwitchParamsCATS(parm_struct);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGainSwitchParamsCATS()

FLR_RESULT bosonGetGainSwitchRadiometricParamsCATS(FLR_BOSON_GAIN_SWITCH_RADIOMETRIC_PARAMS_T *parm_struct){
    FLR_RESULT returncode = CLIENT_pkgBosonGetGainSwitchRadiometricParamsCATS(parm_struct);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGainSwitchRadiometricParamsCATS()

FLR_RESULT bosonSetGainSwitchRadiometricParamsCATS(const FLR_BOSON_GAIN_SWITCH_RADIOMETRIC_PARAMS_T parm_struct){
    FLR_RESULT returncode = CLIENT_pkgBosonSetGainSwitchRadiometricParamsCATS(parm_struct);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetGainSwitchRadiometricParamsCATS()

FLR_RESULT bosonGetCLowToHighPercentCATS(uint32_t *cLowToHighPercent){
    FLR_RESULT returncode = CLIENT_pkgBosonGetCLowToHighPercentCATS(cLowToHighPercent);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetCLowToHighPercentCATS()

FLR_RESULT bprGetState(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgBprGetState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetState()

FLR_RESULT bprSetState(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgBprSetState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetState()

FLR_RESULT bprGetStats(uint32_t *threeby, uint32_t *fiveby, uint32_t *rows, uint32_t *budget, uint32_t *used){
    FLR_RESULT returncode = CLIENT_pkgBprGetStats(threeby, fiveby, rows, budget, used);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetStats()

FLR_RESULT bprGetDisplayMode(FLR_BPR_DISPLAY_MODE_E *data){
    FLR_RESULT returncode = CLIENT_pkgBprGetDisplayMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDisplayMode()

FLR_RESULT bprSetDisplayMode(const FLR_BPR_DISPLAY_MODE_E data){
    FLR_RESULT returncode = CLIENT_pkgBprSetDisplayMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetDisplayMode()

FLR_RESULT bprGetDisplayModeMinValue(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgBprGetDisplayModeMinValue(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDisplayModeMinValue()

FLR_RESULT bprSetDisplayModeMinValue(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgBprSetDisplayModeMinValue(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetDisplayModeMinValue()

FLR_RESULT bprGetDisplayModeMaxValue(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgBprGetDisplayModeMaxValue(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDisplayModeMaxValue()

FLR_RESULT bprSetDisplayModeMaxValue(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgBprSetDisplayModeMaxValue(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetDisplayModeMaxValue()

FLR_RESULT bprGetWorkBufIndex(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgBprGetWorkBufIndex(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetWorkBufIndex()

FLR_RESULT bprSetWorkBufIndex(const uint32_t data){
    FLR_RESULT returncode = CLIENT_pkgBprSetWorkBufIndex(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetWorkBufIndex()

FLR_RESULT bprGetWorkBufStats(uint32_t *threeby, uint32_t *fiveby, uint32_t *rows, uint32_t *budget, uint32_t *used){
    FLR_RESULT returncode = CLIENT_pkgBprGetWorkBufStats(threeby, fiveby, rows, budget, used);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetWorkBufStats()

FLR_RESULT captureSingleFrame(){
    FLR_RESULT returncode = CLIENT_pkgCaptureSingleFrame();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SingleFrame()

FLR_RESULT captureFrames(const FLR_CAPTURE_SETTINGS_T data){
    FLR_RESULT returncode = CLIENT_pkgCaptureFrames(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of Frames()

FLR_RESULT captureSingleFrameWithSrc(const FLR_CAPTURE_SRC_E data){
    FLR_RESULT returncode = CLIENT_pkgCaptureSingleFrameWithSrc(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SingleFrameWithSrc()

FLR_RESULT captureSingleFrameToFile(){
    FLR_RESULT returncode = CLIENT_pkgCaptureSingleFrameToFile();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SingleFrameToFile()

FLR_RESULT captureGetStatus(FLR_CAPTURE_STATUS_T *status){
    FLR_RESULT returncode = CLIENT_pkgCaptureGetStatus(status);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetStatus()

FLR_RESULT colorLutSetControl(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgColorlutSetControl(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetControl()

FLR_RESULT colorLutGetControl(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgColorlutGetControl(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetControl()

FLR_RESULT colorLutSetId(const FLR_COLORLUT_ID_E data){
    FLR_RESULT returncode = CLIENT_pkgColorlutSetId(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetId()

FLR_RESULT colorLutGetId(FLR_COLORLUT_ID_E *data){
    FLR_RESULT returncode = CLIENT_pkgColorlutGetId(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetId()

FLR_RESULT colorLutSetOutlineColor(const uint8_t red, const uint8_t green, const uint8_t blue){
    FLR_RESULT returncode = CLIENT_pkgColorlutSetOutlineColor(red, green, blue);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetOutlineColor()

FLR_RESULT colorLutGetOutlineColor(uint8_t *red, uint8_t *green, uint8_t *blue){
    FLR_RESULT returncode = CLIENT_pkgColorlutGetOutlineColor(red, green, blue);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetOutlineColor()

FLR_RESULT dummyBadCommand(){
    FLR_RESULT returncode = CLIENT_pkgDummyBadCommand();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of BadCommand()

FLR_RESULT dvoSetAnalogVideoState(const FLR_ENABLE_E analogVideoState){
    FLR_RESULT returncode = CLIENT_pkgDvoSetAnalogVideoState(analogVideoState);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetAnalogVideoState()

FLR_RESULT dvoGetAnalogVideoState(FLR_ENABLE_E *analogVideoState){
    FLR_RESULT returncode = CLIENT_pkgDvoGetAnalogVideoState(analogVideoState);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetAnalogVideoState()

FLR_RESULT dvoSetOutputFormat(const FLR_DVO_OUTPUT_FORMAT_E format){
    FLR_RESULT returncode = CLIENT_pkgDvoSetOutputFormat(format);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetOutputFormat()

FLR_RESULT dvoGetOutputFormat(FLR_DVO_OUTPUT_FORMAT_E *format){
    FLR_RESULT returncode = CLIENT_pkgDvoGetOutputFormat(format);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetOutputFormat()

FLR_RESULT dvoSetOutputYCbCrSettings(const FLR_DVO_YCBCR_SETTINGS_T settings){
    FLR_RESULT returncode = CLIENT_pkgDvoSetOutputYCbCrSettings(settings);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetOutputYCbCrSettings()

FLR_RESULT dvoGetOutputYCbCrSettings(FLR_DVO_YCBCR_SETTINGS_T *settings){
    FLR_RESULT returncode = CLIENT_pkgDvoGetOutputYCbCrSettings(settings);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetOutputYCbCrSettings()

FLR_RESULT dvoSetOutputRGBSettings(const FLR_DVO_RGB_SETTINGS_T settings){
    FLR_RESULT returncode = CLIENT_pkgDvoSetOutputRGBSettings(settings);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetOutputRGBSettings()

FLR_RESULT dvoGetOutputRGBSettings(FLR_DVO_RGB_SETTINGS_T *settings){
    FLR_RESULT returncode = CLIENT_pkgDvoGetOutputRGBSettings(settings);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetOutputRGBSettings()

FLR_RESULT dvoApplyCustomSettings(){
    FLR_RESULT returncode = CLIENT_pkgDvoApplyCustomSettings();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of ApplyCustomSettings()

FLR_RESULT dvoSetDisplayMode(const FLR_DVO_DISPLAY_MODE_E displayMode){
    FLR_RESULT returncode = CLIENT_pkgDvoSetDisplayMode(displayMode);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetDisplayMode()

FLR_RESULT dvoGetDisplayMode(FLR_DVO_DISPLAY_MODE_E *displayMode){
    FLR_RESULT returncode = CLIENT_pkgDvoGetDisplayMode(displayMode);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDisplayMode()

FLR_RESULT dvoSetType(const FLR_DVO_TYPE_E tap){
    FLR_RESULT returncode = CLIENT_pkgDvoSetType(tap);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetType()

FLR_RESULT dvoGetType(FLR_DVO_TYPE_E *tap){
    FLR_RESULT returncode = CLIENT_pkgDvoGetType(tap);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetType()

FLR_RESULT dvoSetVideoStandard(const FLR_DVO_VIDEO_STANDARD_E videoStandard){
    FLR_RESULT returncode = CLIENT_pkgDvoSetVideoStandard(videoStandard);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetVideoStandard()

FLR_RESULT dvoGetVideoStandard(FLR_DVO_VIDEO_STANDARD_E *videoStandard){
    FLR_RESULT returncode = CLIENT_pkgDvoGetVideoStandard(videoStandard);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetVideoStandard()

FLR_RESULT dvoSetCheckVideoDacPresent(const FLR_ENABLE_E checkVideoDacPresent){
    FLR_RESULT returncode = CLIENT_pkgDvoSetCheckVideoDacPresent(checkVideoDacPresent);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetCheckVideoDacPresent()

FLR_RESULT dvoGetCheckVideoDacPresent(FLR_ENABLE_E *checkVideoDacPresent){
    FLR_RESULT returncode = CLIENT_pkgDvoGetCheckVideoDacPresent(checkVideoDacPresent);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetCheckVideoDacPresent()

FLR_RESULT dvoSetCustomLcdConfig(const FLR_DVO_LCD_CONFIG_ID_E id, const FLR_DVO_LCD_CONFIG_T config){
    FLR_RESULT returncode = CLIENT_pkgDvoSetCustomLcdConfig(id, config);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetCustomLcdConfig()

FLR_RESULT dvoGetCustomLcdConfig(const FLR_DVO_LCD_CONFIG_ID_E id, FLR_DVO_LCD_CONFIG_T *config){
    FLR_RESULT returncode = CLIENT_pkgDvoGetCustomLcdConfig(id, config);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetCustomLcdConfig()

FLR_RESULT dvoSetLCDConfig(const FLR_DVO_LCD_CONFIG_ID_E id){
    FLR_RESULT returncode = CLIENT_pkgDvoSetLCDConfig(id);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetLCDConfig()

FLR_RESULT dvoGetLCDConfig(FLR_DVO_LCD_CONFIG_ID_E *id){
    FLR_RESULT returncode = CLIENT_pkgDvoGetLCDConfig(id);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetLCDConfig()

FLR_RESULT dvoGetClockInfo(uint32_t *horizontalSyncWidth, uint32_t *verticalSyncWidth, uint32_t *clocksPerRowPeriod, uint32_t *horizontalFrontPorch, uint32_t *horizontalBackPorch, uint32_t *frontTelemetryPixels, uint32_t *rearTelemetryPixels, uint32_t *videoColumns, uint32_t *validColumns, uint32_t *telemetryRows, uint32_t *videoRows, uint32_t *validRows, uint32_t *verticalFrontPorch, uint32_t *verticalBackPorch, uint32_t *rowPeriodsPerFrame, uint32_t *clocksPerFrame, float *clockRateInMHz, float *frameRateInHz, uint32_t *validOnRisingEdge, uint32_t *dataWidthInBits){
    FLR_RESULT returncode = CLIENT_pkgDvoGetClockInfo(horizontalSyncWidth, verticalSyncWidth, clocksPerRowPeriod, horizontalFrontPorch, horizontalBackPorch, frontTelemetryPixels, rearTelemetryPixels, videoColumns, validColumns, telemetryRows, videoRows, validRows, verticalFrontPorch, verticalBackPorch, rowPeriodsPerFrame, clocksPerFrame, clockRateInMHz, frameRateInHz, validOnRisingEdge, dataWidthInBits);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetClockInfo()

FLR_RESULT dvoSetAllCustomLcdConfigs(const FLR_DVO_LCD_CONFIG_T config0, const FLR_DVO_LCD_CONFIG_T config1){
    FLR_RESULT returncode = CLIENT_pkgDvoSetAllCustomLcdConfigs(config0, config1);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetAllCustomLcdConfigs()

FLR_RESULT dvoGetAllCustomLcdConfigs(FLR_DVO_LCD_CONFIG_T *config0, FLR_DVO_LCD_CONFIG_T *config1){
    FLR_RESULT returncode = CLIENT_pkgDvoGetAllCustomLcdConfigs(config0, config1);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetAllCustomLcdConfigs()

FLR_RESULT dvoSetOutputIr16Format(const FLR_DVO_OUTPUT_IR16_FORMAT_E format){
    FLR_RESULT returncode = CLIENT_pkgDvoSetOutputIr16Format(format);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetOutputIr16Format()

FLR_RESULT dvoGetOutputIr16Format(FLR_DVO_OUTPUT_IR16_FORMAT_E *format){
    FLR_RESULT returncode = CLIENT_pkgDvoGetOutputIr16Format(format);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetOutputIr16Format()

FLR_RESULT dvoSetLcdClockRate(const FLR_DVO_LCD_CLOCK_RATE_E clockRate){
    FLR_RESULT returncode = CLIENT_pkgDvoSetLcdClockRate(clockRate);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetLcdClockRate()

FLR_RESULT dvoGetLcdClockRate(FLR_DVO_LCD_CLOCK_RATE_E *clockRate){
    FLR_RESULT returncode = CLIENT_pkgDvoGetLcdClockRate(clockRate);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetLcdClockRate()

FLR_RESULT dvoSetLcdVideoFrameRate(const uint32_t framerate){
    FLR_RESULT returncode = CLIENT_pkgDvoSetLcdVideoFrameRate(framerate);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetLcdVideoFrameRate()

FLR_RESULT dvoGetLcdVideoFrameRate(uint32_t *framerate){
    FLR_RESULT returncode = CLIENT_pkgDvoGetLcdVideoFrameRate(framerate);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetLcdVideoFrameRate()

FLR_RESULT dvoSetMipiStartState(const FLR_DVO_MIPI_STATE_E state){
    FLR_RESULT returncode = CLIENT_pkgDvoSetMipiStartState(state);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetMipiStartState()

FLR_RESULT dvoGetMipiStartState(FLR_DVO_MIPI_STATE_E *state){
    FLR_RESULT returncode = CLIENT_pkgDvoGetMipiStartState(state);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMipiStartState()

FLR_RESULT dvoSetMipiState(const FLR_DVO_MIPI_STATE_E state){
    FLR_RESULT returncode = CLIENT_pkgDvoSetMipiState(state);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetMipiState()

FLR_RESULT dvoGetMipiState(FLR_DVO_MIPI_STATE_E *state){
    FLR_RESULT returncode = CLIENT_pkgDvoGetMipiState(state);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMipiState()

FLR_RESULT dvoSetMipiClockLaneMode(const FLR_DVO_MIPI_CLOCK_LANE_MODE_E mode){
    FLR_RESULT returncode = CLIENT_pkgDvoSetMipiClockLaneMode(mode);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetMipiClockLaneMode()

FLR_RESULT dvoGetMipiClockLaneMode(FLR_DVO_MIPI_CLOCK_LANE_MODE_E *mode){
    FLR_RESULT returncode = CLIENT_pkgDvoGetMipiClockLaneMode(mode);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMipiClockLaneMode()

FLR_RESULT dvoSetOutputInterface(const FLR_DVO_OUTPUT_INTERFACE_E format){
    FLR_RESULT returncode = CLIENT_pkgDvoSetOutputInterface(format);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetOutputInterface()

FLR_RESULT dvoGetOutputInterface(FLR_DVO_OUTPUT_INTERFACE_E *format){
    FLR_RESULT returncode = CLIENT_pkgDvoGetOutputInterface(format);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetOutputInterface()

FLR_RESULT dvoSetOutputFormatVC1(const FLR_DVO_OUTPUT_FORMAT_E format){
    FLR_RESULT returncode = CLIENT_pkgDvoSetOutputFormatVC1(format);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetOutputFormatVC1()

FLR_RESULT dvoGetOutputFormatVC1(FLR_DVO_OUTPUT_FORMAT_E *format){
    FLR_RESULT returncode = CLIENT_pkgDvoGetOutputFormatVC1(format);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetOutputFormatVC1()

FLR_RESULT dvoMuxSetType(const FLR_DVOMUX_OUTPUT_IF_E output, const FLR_DVOMUX_SOURCE_E source, const FLR_DVOMUX_TYPE_E type){
    FLR_RESULT returncode = CLIENT_pkgDvomuxSetType(output, source, type);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetType()

FLR_RESULT dvoMuxGetType(const FLR_DVOMUX_OUTPUT_IF_E output, FLR_DVOMUX_SOURCE_E *source, FLR_DVOMUX_TYPE_E *type){
    FLR_RESULT returncode = CLIENT_pkgDvomuxGetType(output, source, type);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetType()

FLR_RESULT fileOpsDir(uint8_t dirent[]){
    FLR_RESULT returncode = CLIENT_pkgFileopsDir(dirent);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of Dir()

FLR_RESULT fileOpsCd(const uint8_t path[], uint8_t pwd[]){
    FLR_RESULT returncode = CLIENT_pkgFileopsCd(path, pwd);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of Cd()

FLR_RESULT fileOpsMd(const uint8_t path[]){
    FLR_RESULT returncode = CLIENT_pkgFileopsMd(path);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of Md()

FLR_RESULT fileOpsFopen(const uint8_t path[], const uint8_t mode[], uint32_t *id){
    FLR_RESULT returncode = CLIENT_pkgFileopsFopen(path, mode, id);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of Fopen()

FLR_RESULT fileOpsFclose(const uint32_t id){
    FLR_RESULT returncode = CLIENT_pkgFileopsFclose(id);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of Fclose()

FLR_RESULT fileOpsFread(const uint32_t id, const uint32_t length, uint8_t buf[], uint32_t *ret){
    FLR_RESULT returncode = CLIENT_pkgFileopsFread(id, length, buf, ret);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of Fread()

FLR_RESULT fileOpsFwrite(const uint32_t id, const uint32_t length, const uint8_t buf[], uint32_t *ret){
    FLR_RESULT returncode = CLIENT_pkgFileopsFwrite(id, length, buf, ret);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of Fwrite()

FLR_RESULT fileOpsFtell(const uint32_t id, uint32_t *offset){
    FLR_RESULT returncode = CLIENT_pkgFileopsFtell(id, offset);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of Ftell()

FLR_RESULT fileOpsFseek(const uint32_t id, const uint32_t offset, const uint32_t origin){
    FLR_RESULT returncode = CLIENT_pkgFileopsFseek(id, offset, origin);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of Fseek()

FLR_RESULT fileOpsFtruncate(const uint32_t id, const uint32_t length){
    FLR_RESULT returncode = CLIENT_pkgFileopsFtruncate(id, length);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of Ftruncate()

FLR_RESULT fileOpsRmdir(const uint8_t path[]){
    FLR_RESULT returncode = CLIENT_pkgFileopsRmdir(path);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of Rmdir()

FLR_RESULT fileOpsRm(const uint8_t path[]){
    FLR_RESULT returncode = CLIENT_pkgFileopsRm(path);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of Rm()

FLR_RESULT fileOpsRename(const uint8_t oldpath[], const uint8_t newpath[]){
    FLR_RESULT returncode = CLIENT_pkgFileopsRename(oldpath, newpath);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of Rename()

FLR_RESULT fileOpsGetFileSize(const uint8_t path[], uint32_t *fileLength){
    FLR_RESULT returncode = CLIENT_pkgFileopsGetFileSize(path, fileLength);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFileSize()

FLR_RESULT flashIOSetProtectionState(const FLR_ENABLE_E protectionState){
    FLR_RESULT returncode = CLIENT_pkgFlashioSetProtectionState(protectionState);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetProtectionState()

FLR_RESULT flashIOGetProtectionState(FLR_ENABLE_E *protectionState){
    FLR_RESULT returncode = CLIENT_pkgFlashioGetProtectionState(protectionState);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetProtectionState()

FLR_RESULT flashMapFsGetHeaderVersion(uint32_t *major, uint32_t *minor, uint32_t *patch){
    FLR_RESULT returncode = CLIENT_pkgFlashmapfsGetHeaderVersion(major, minor, patch);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetHeaderVersion()

FLR_RESULT gaoSetGainState(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgGaoSetGainState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetGainState()

FLR_RESULT gaoGetGainState(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgGaoGetGainState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGainState()

FLR_RESULT gaoSetFfcState(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgGaoSetFfcState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFfcState()

FLR_RESULT gaoGetFfcState(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgGaoGetFfcState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFfcState()

FLR_RESULT gaoSetTempCorrectionState(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgGaoSetTempCorrectionState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTempCorrectionState()

FLR_RESULT gaoGetTempCorrectionState(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgGaoGetTempCorrectionState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTempCorrectionState()

FLR_RESULT gaoSetIConstL(const int16_t data){
    FLR_RESULT returncode = CLIENT_pkgGaoSetIConstL(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetIConstL()

FLR_RESULT gaoGetIConstL(int16_t *data){
    FLR_RESULT returncode = CLIENT_pkgGaoGetIConstL(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetIConstL()

FLR_RESULT gaoSetIConstM(const int16_t data){
    FLR_RESULT returncode = CLIENT_pkgGaoSetIConstM(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetIConstM()

FLR_RESULT gaoGetIConstM(int16_t *data){
    FLR_RESULT returncode = CLIENT_pkgGaoGetIConstM(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetIConstM()

FLR_RESULT gaoSetAveragerState(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgGaoSetAveragerState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetAveragerState()

FLR_RESULT gaoGetAveragerState(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgGaoGetAveragerState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetAveragerState()

FLR_RESULT gaoSetNumFFCFrames(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgGaoSetNumFFCFrames(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetNumFFCFrames()

FLR_RESULT gaoGetNumFFCFrames(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgGaoGetNumFFCFrames(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetNumFFCFrames()

FLR_RESULT gaoGetAveragerThreshold(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgGaoGetAveragerThreshold(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetAveragerThreshold()

FLR_RESULT gaoSetTestRampState(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgGaoSetTestRampState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTestRampState()

FLR_RESULT gaoGetTestRampState(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgGaoGetTestRampState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTestRampState()

FLR_RESULT gaoSetSffcState(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgGaoSetSffcState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetSffcState()

FLR_RESULT gaoGetSffcState(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgGaoGetSffcState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSffcState()

FLR_RESULT gaoSetNucType(const FLR_GAO_NUC_TYPE_E nucType){
    FLR_RESULT returncode = CLIENT_pkgGaoSetNucType(nucType);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetNucType()

FLR_RESULT gaoGetNucType(FLR_GAO_NUC_TYPE_E *nucType){
    FLR_RESULT returncode = CLIENT_pkgGaoGetNucType(nucType);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetNucType()

FLR_RESULT gaoSetFfcZeroMeanState(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgGaoSetFfcZeroMeanState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFfcZeroMeanState()

FLR_RESULT gaoGetFfcZeroMeanState(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgGaoGetFfcZeroMeanState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFfcZeroMeanState()

FLR_RESULT gaoGetAveragerDesiredState(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgGaoGetAveragerDesiredState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetAveragerDesiredState()

FLR_RESULT gaoGetAppliedClip(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgGaoGetAppliedClip(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetAppliedClip()

FLR_RESULT gaoSetAppliedClipEnable(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgGaoSetAppliedClipEnable(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetAppliedClipEnable()

FLR_RESULT gaoGetAppliedClipEnable(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgGaoGetAppliedClipEnable(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetAppliedClipEnable()

FLR_RESULT gaoSetFfcShutterSimulationState(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgGaoSetFfcShutterSimulationState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFfcShutterSimulationState()

FLR_RESULT gaoGetFfcShutterSimulationState(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgGaoGetFfcShutterSimulationState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFfcShutterSimulationState()

FLR_RESULT gaoSetFfcShutterSimulatorValue(const uint16_t value){
    FLR_RESULT returncode = CLIENT_pkgGaoSetFfcShutterSimulatorValue(value);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFfcShutterSimulatorValue()

FLR_RESULT gaoGetFfcShutterSimulatorValue(uint16_t *value){
    FLR_RESULT returncode = CLIENT_pkgGaoGetFfcShutterSimulatorValue(value);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFfcShutterSimulatorValue()

FLR_RESULT gaoSetBcnrState(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgGaoSetBcnrState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetBcnrState()

FLR_RESULT gaoGetBcnrState(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgGaoGetBcnrState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetBcnrState()

FLR_RESULT gaoGetAppliedSffcScaleFactor(float *data){
    FLR_RESULT returncode = CLIENT_pkgGaoGetAppliedSffcScaleFactor(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetAppliedSffcScaleFactor()

FLR_RESULT gaoSetSffcMode(const FLR_GAO_SFFC_MODE_E mode){
    FLR_RESULT returncode = CLIENT_pkgGaoSetSffcMode(mode);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetSffcMode()

FLR_RESULT gaoGetSffcMode(FLR_GAO_SFFC_MODE_E *mode){
    FLR_RESULT returncode = CLIENT_pkgGaoGetSffcMode(mode);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSffcMode()

FLR_RESULT imageStatsGetTotalHistPixelsInROI(uint32_t *totalPixelsInROI){
    FLR_RESULT returncode = CLIENT_pkgImagestatsGetTotalHistPixelsInROI(totalPixelsInROI);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTotalHistPixelsInROI()

FLR_RESULT imageStatsGetPopBelowLowToHighThresh(uint32_t *popBelowLowToHighThresh){
    FLR_RESULT returncode = CLIENT_pkgImagestatsGetPopBelowLowToHighThresh(popBelowLowToHighThresh);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetPopBelowLowToHighThresh()

FLR_RESULT imageStatsGetPopAboveHighToLowThresh(uint32_t *popAboveHighToLowThresh){
    FLR_RESULT returncode = CLIENT_pkgImagestatsGetPopAboveHighToLowThresh(popAboveHighToLowThresh);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetPopAboveHighToLowThresh()

FLR_RESULT imageStatsSetROI(const FLR_ROI_T roi){
    FLR_RESULT returncode = CLIENT_pkgImagestatsSetROI(roi);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetROI()

FLR_RESULT imageStatsGetROI(FLR_ROI_T *roi){
    FLR_RESULT returncode = CLIENT_pkgImagestatsGetROI(roi);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetROI()

FLR_RESULT imageStatsGetFirstBin(uint16_t *firstBin){
    FLR_RESULT returncode = CLIENT_pkgImagestatsGetFirstBin(firstBin);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFirstBin()

FLR_RESULT imageStatsGetLastBin(uint16_t *lastBin){
    FLR_RESULT returncode = CLIENT_pkgImagestatsGetLastBin(lastBin);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetLastBin()

FLR_RESULT imageStatsGetMean(uint16_t *mean){
    FLR_RESULT returncode = CLIENT_pkgImagestatsGetMean(mean);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMean()

FLR_RESULT imageStatsGetFirstBinInROI(uint16_t *firstBinInROI){
    FLR_RESULT returncode = CLIENT_pkgImagestatsGetFirstBinInROI(firstBinInROI);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFirstBinInROI()

FLR_RESULT imageStatsGetLastBinInROI(uint16_t *lastBinInROI){
    FLR_RESULT returncode = CLIENT_pkgImagestatsGetLastBinInROI(lastBinInROI);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetLastBinInROI()

FLR_RESULT imageStatsGetMeanInROI(uint16_t *meanInROI){
    FLR_RESULT returncode = CLIENT_pkgImagestatsGetMeanInROI(meanInROI);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMeanInROI()

FLR_RESULT imageStatsGetImageStats(uint16_t *meanIntensity, uint16_t *peakIntensity, uint16_t *baseIntensity){
    FLR_RESULT returncode = CLIENT_pkgImagestatsGetImageStats(meanIntensity, peakIntensity, baseIntensity);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetImageStats()

FLR_RESULT imageStatsGetPopAboveLowToHighThreshCATS(uint32_t *popAboveLowToHighThresh){
    FLR_RESULT returncode = CLIENT_pkgImagestatsGetPopAboveLowToHighThreshCATS(popAboveLowToHighThresh);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetPopAboveLowToHighThreshCATS()

FLR_RESULT imageStatsGetPopBelowHighToLowThreshCATS(uint32_t *popBelowHighToLowThresh){
    FLR_RESULT returncode = CLIENT_pkgImagestatsGetPopBelowHighToLowThreshCATS(popBelowHighToLowThresh);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetPopBelowHighToLowThreshCATS()

FLR_RESULT imageStatsGetPopBetweenLthCATSAndLthSATS(uint32_t *popBetweenCatsAndSats){
    FLR_RESULT returncode = CLIENT_pkgImagestatsGetPopBetweenLthCATSAndLthSATS(popBetweenCatsAndSats);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetPopBetweenLthCATSAndLthSATS()

FLR_RESULT isothermGetEnable(FLR_ENABLE_E *isothermEnable){
    FLR_RESULT returncode = CLIENT_pkgIsothermGetEnable(isothermEnable);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetEnable()

FLR_RESULT isothermSetEnable(const FLR_ENABLE_E isothermEnable){
    FLR_RESULT returncode = CLIENT_pkgIsothermSetEnable(isothermEnable);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetEnable()

FLR_RESULT isothermSetTemps(const FLR_ISOTHERM_GAIN_E table, const int32_t thIsoT1, const int32_t thIsoT2, const int32_t thIsoT3, const int32_t thIsoT4, const int32_t thIsoT5){
    FLR_RESULT returncode = CLIENT_pkgIsothermSetTemps(table, thIsoT1, thIsoT2, thIsoT3, thIsoT4, thIsoT5);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTemps()

FLR_RESULT isothermGetTemps(const FLR_ISOTHERM_GAIN_E table, int32_t *thIsoT1, int32_t *thIsoT2, int32_t *thIsoT3, int32_t *thIsoT4, int32_t *thIsoT5){
    FLR_RESULT returncode = CLIENT_pkgIsothermGetTemps(table, thIsoT1, thIsoT2, thIsoT3, thIsoT4, thIsoT5);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTemps()

FLR_RESULT isothermSetIsoColorValues(const FLR_ISOTHERM_GAIN_E table, const FLR_ISOTHERM_COLORS_T region0, const FLR_ISOTHERM_COLORS_T region1, const FLR_ISOTHERM_COLORS_T region2, const FLR_ISOTHERM_COLORS_T region3, const FLR_ISOTHERM_COLORS_T region4, const FLR_ISOTHERM_COLORS_T region5){
    FLR_RESULT returncode = CLIENT_pkgIsothermSetIsoColorValues(table, region0, region1, region2, region3, region4, region5);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetIsoColorValues()

FLR_RESULT isothermGetIsoColorValues(const FLR_ISOTHERM_GAIN_E table, FLR_ISOTHERM_COLORS_T *region0, FLR_ISOTHERM_COLORS_T *region1, FLR_ISOTHERM_COLORS_T *region2, FLR_ISOTHERM_COLORS_T *region3, FLR_ISOTHERM_COLORS_T *region4, FLR_ISOTHERM_COLORS_T *region5){
    FLR_RESULT returncode = CLIENT_pkgIsothermGetIsoColorValues(table, region0, region1, region2, region3, region4, region5);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetIsoColorValues()

FLR_RESULT isothermSetRegionMode(const FLR_ISOTHERM_GAIN_E table, const FLR_ISOTHERM_REGION_E region0, const FLR_ISOTHERM_REGION_E region1, const FLR_ISOTHERM_REGION_E region2, const FLR_ISOTHERM_REGION_E region3, const FLR_ISOTHERM_REGION_E region4, const FLR_ISOTHERM_REGION_E region5){
    FLR_RESULT returncode = CLIENT_pkgIsothermSetRegionMode(table, region0, region1, region2, region3, region4, region5);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetRegionMode()

FLR_RESULT isothermGetRegionMode(const FLR_ISOTHERM_GAIN_E table, FLR_ISOTHERM_REGION_E *region0, FLR_ISOTHERM_REGION_E *region1, FLR_ISOTHERM_REGION_E *region2, FLR_ISOTHERM_REGION_E *region3, FLR_ISOTHERM_REGION_E *region4, FLR_ISOTHERM_REGION_E *region5){
    FLR_RESULT returncode = CLIENT_pkgIsothermGetRegionMode(table, region0, region1, region2, region3, region4, region5);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetRegionMode()

FLR_RESULT isothermGetUnit(FLR_ISOTHERM_UNIT_E *unit){
    FLR_RESULT returncode = CLIENT_pkgIsothermGetUnit(unit);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetUnit()

FLR_RESULT isothermSetUnit(const FLR_ISOTHERM_UNIT_E unit){
    FLR_RESULT returncode = CLIENT_pkgIsothermSetUnit(unit);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetUnit()

FLR_RESULT isothermGetSettingsLowGain(FLR_ISOTHERM_SETTINGS_T *settings){
    FLR_RESULT returncode = CLIENT_pkgIsothermGetSettingsLowGain(settings);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSettingsLowGain()

FLR_RESULT isothermSetSettingsLowGain(const FLR_ISOTHERM_SETTINGS_T settings){
    FLR_RESULT returncode = CLIENT_pkgIsothermSetSettingsLowGain(settings);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetSettingsLowGain()

FLR_RESULT isothermGetSettingsHighGain(FLR_ISOTHERM_SETTINGS_T *settings){
    FLR_RESULT returncode = CLIENT_pkgIsothermGetSettingsHighGain(settings);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSettingsHighGain()

FLR_RESULT isothermSetSettingsHighGain(const FLR_ISOTHERM_SETTINGS_T settings){
    FLR_RESULT returncode = CLIENT_pkgIsothermSetSettingsHighGain(settings);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetSettingsHighGain()

FLR_RESULT isothermSetColorLutId(const FLR_COLORLUT_ID_E colorLutIdLowGain, const FLR_COLORLUT_ID_E colorLutIdHighGain){
    FLR_RESULT returncode = CLIENT_pkgIsothermSetColorLutId(colorLutIdLowGain, colorLutIdHighGain);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetColorLutId()

FLR_RESULT isothermGetColorLutId(FLR_COLORLUT_ID_E *colorLutIdLowGain, FLR_COLORLUT_ID_E *colorLutIdHighGain){
    FLR_RESULT returncode = CLIENT_pkgIsothermGetColorLutId(colorLutIdLowGain, colorLutIdHighGain);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetColorLutId()

FLR_RESULT jffs2Mount(){
    FLR_RESULT returncode = CLIENT_pkgJffs2Mount();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of Mount()

FLR_RESULT jffs2Unmount(){
    FLR_RESULT returncode = CLIENT_pkgJffs2Unmount();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of Unmount()

FLR_RESULT jffs2GetState(FLR_JFFS2_STATE_E *state){
    FLR_RESULT returncode = CLIENT_pkgJffs2GetState(state);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetState()

FLR_RESULT latencyCtrlSetLowLatencyState(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgLatencyctrlSetLowLatencyState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetLowLatencyState()

FLR_RESULT latencyCtrlGetLowLatencyState(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgLatencyctrlGetLowLatencyState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetLowLatencyState()

FLR_RESULT latencyCtrlSetJitterReduction(const FLR_ENABLE_E enable, const int32_t line){
    FLR_RESULT returncode = CLIENT_pkgLatencyctrlSetJitterReduction(enable, line);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetJitterReduction()

FLR_RESULT latencyCtrlGetJitterReduction(FLR_ENABLE_E *enable, int32_t *line){
    FLR_RESULT returncode = CLIENT_pkgLatencyctrlGetJitterReduction(enable, line);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetJitterReduction()

FLR_RESULT latencyCtrlLatencyResetStats(){
    FLR_RESULT returncode = CLIENT_pkgLatencyctrlLatencyResetStats();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of LatencyResetStats()

FLR_RESULT latencyCtrlGetJitter(float *jitterMin, float *jitterMax){
    FLR_RESULT returncode = CLIENT_pkgLatencyctrlGetJitter(jitterMin, jitterMax);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetJitter()

FLR_RESULT latencyCtrlGetLatency(float *latencyMin, float *latencyMax){
    FLR_RESULT returncode = CLIENT_pkgLatencyctrlGetLatency(latencyMin, latencyMax);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetLatency()

FLR_RESULT latencyCtrlSetUsbVideoLatencyReduction(const int32_t line){
    FLR_RESULT returncode = CLIENT_pkgLatencyctrlSetUsbVideoLatencyReduction(line);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetUsbVideoLatencyReduction()

FLR_RESULT latencyCtrlGetUsbVideoLatencyReduction(int32_t *line){
    FLR_RESULT returncode = CLIENT_pkgLatencyctrlGetUsbVideoLatencyReduction(line);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetUsbVideoLatencyReduction()

FLR_RESULT lfsrSetApplyOffsetEnableState(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgLfsrSetApplyOffsetEnableState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetApplyOffsetEnableState()

FLR_RESULT lfsrGetApplyOffsetEnableState(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgLfsrGetApplyOffsetEnableState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetApplyOffsetEnableState()

FLR_RESULT lfsrSetMaxIterations(const uint32_t data){
    FLR_RESULT returncode = CLIENT_pkgLfsrSetMaxIterations(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetMaxIterations()

FLR_RESULT lfsrGetMaxIterations(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgLfsrGetMaxIterations(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMaxIterations()

FLR_RESULT lfsrSetDf(const uint32_t data){
    FLR_RESULT returncode = CLIENT_pkgLfsrSetDf(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetDf()

FLR_RESULT lfsrGetDf(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgLfsrGetDf(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDf()

FLR_RESULT lfsrSetLambda1(const float data){
    FLR_RESULT returncode = CLIENT_pkgLfsrSetLambda1(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetLambda1()

FLR_RESULT lfsrGetLambda1(float *data){
    FLR_RESULT returncode = CLIENT_pkgLfsrGetLambda1(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetLambda1()

FLR_RESULT lfsrSetLambda2(const float data){
    FLR_RESULT returncode = CLIENT_pkgLfsrSetLambda2(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetLambda2()

FLR_RESULT lfsrGetLambda2(float *data){
    FLR_RESULT returncode = CLIENT_pkgLfsrGetLambda2(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetLambda2()

FLR_RESULT lfsrSetHaltEnable(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgLfsrSetHaltEnable(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetHaltEnable()

FLR_RESULT lfsrGetHaltEnable(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgLfsrGetHaltEnable(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetHaltEnable()

FLR_RESULT lfsrSetRandomMethod(const uint32_t data){
    FLR_RESULT returncode = CLIENT_pkgLfsrSetRandomMethod(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetRandomMethod()

FLR_RESULT lfsrGetRandomMethod(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgLfsrGetRandomMethod(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetRandomMethod()

FLR_RESULT lfsrSetSingleStepEnable(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgLfsrSetSingleStepEnable(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetSingleStepEnable()

FLR_RESULT lfsrGetSingleStepEnable(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgLfsrGetSingleStepEnable(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSingleStepEnable()

FLR_RESULT lfsrSetR_LocalBump(const float data){
    FLR_RESULT returncode = CLIENT_pkgLfsrSetR_LocalBump(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetR_LocalBump()

FLR_RESULT lfsrGetR_LocalBump(float *data){
    FLR_RESULT returncode = CLIENT_pkgLfsrGetR_LocalBump(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetR_LocalBump()

FLR_RESULT lfsrSetR_CornerBump(const float data){
    FLR_RESULT returncode = CLIENT_pkgLfsrSetR_CornerBump(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetR_CornerBump()

FLR_RESULT lfsrGetR_CornerBump(float *data){
    FLR_RESULT returncode = CLIENT_pkgLfsrGetR_CornerBump(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetR_CornerBump()

FLR_RESULT lfsrSetFFC_ResetEnable(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgLfsrSetFFC_ResetEnable(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFFC_ResetEnable()

FLR_RESULT lfsrGetFFC_ResetEnable(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgLfsrGetFFC_ResetEnable(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFFC_ResetEnable()

FLR_RESULT lfsrSetNormalizeAtCenterSpotState(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgLfsrSetNormalizeAtCenterSpotState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetNormalizeAtCenterSpotState()

FLR_RESULT lfsrGetNormalizeAtCenterSpotState(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgLfsrGetNormalizeAtCenterSpotState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetNormalizeAtCenterSpotState()

FLR_RESULT memReadCapture(const uint8_t bufferNum, const uint32_t offset, const uint16_t sizeInBytes, uint8_t *data){
    if (sizeInBytes > MaxMemoryChunk)
    {
        return FLR_DATA_SIZE_ERROR;
    }
    FLR_RESULT returncode = CLIENT_pkgMemReadCapture(bufferNum, offset, sizeInBytes, data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of ReadCapture()

FLR_RESULT memGetCaptureSize(uint32_t *bytes, uint16_t *rows, uint16_t *columns){
    FLR_RESULT returncode = CLIENT_pkgMemGetCaptureSize(bytes, rows, columns);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetCaptureSize()

FLR_RESULT memWriteFlash(const FLR_MEM_LOCATION_E location, const uint8_t index, const uint32_t offset, const uint16_t sizeInBytes, uint8_t *data){
    if (sizeInBytes > MaxMemoryChunk)
    {
        return FLR_DATA_SIZE_ERROR;
    }
    FLR_RESULT returncode = CLIENT_pkgMemWriteFlash(location, index, offset, sizeInBytes, data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of WriteFlash()

FLR_RESULT memReadFlash(const FLR_MEM_LOCATION_E location, const uint8_t index, const uint32_t offset, const uint16_t sizeInBytes, uint8_t *data){
    if (sizeInBytes > MaxMemoryChunk)
    {
        return FLR_DATA_SIZE_ERROR;
    }
    FLR_RESULT returncode = CLIENT_pkgMemReadFlash(location, index, offset, sizeInBytes, data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of ReadFlash()

FLR_RESULT memGetFlashSize(const FLR_MEM_LOCATION_E location, uint32_t *bytes){
    FLR_RESULT returncode = CLIENT_pkgMemGetFlashSize(location, bytes);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFlashSize()

FLR_RESULT memEraseFlash(const FLR_MEM_LOCATION_E location, const uint8_t index){
    FLR_RESULT returncode = CLIENT_pkgMemEraseFlash(location, index);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of EraseFlash()

FLR_RESULT memEraseFlashPartial(const FLR_MEM_LOCATION_E location, const uint8_t index, const uint32_t offset, const uint32_t length){
    FLR_RESULT returncode = CLIENT_pkgMemEraseFlashPartial(location, index, offset, length);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of EraseFlashPartial()

FLR_RESULT memReadCurrentGain(const uint32_t offset, const uint16_t sizeInBytes, uint8_t *data){
    if (sizeInBytes > MaxMemoryChunk)
    {
        return FLR_DATA_SIZE_ERROR;
    }
    FLR_RESULT returncode = CLIENT_pkgMemReadCurrentGain(offset, sizeInBytes, data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of ReadCurrentGain()

FLR_RESULT memGetGainSize(uint32_t *bytes, uint16_t *rows, uint16_t *columns){
    FLR_RESULT returncode = CLIENT_pkgMemGetGainSize(bytes, rows, columns);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGainSize()

FLR_RESULT memGetCaptureSizeSrc(const FLR_CAPTURE_SRC_E src, uint32_t *bytes, uint16_t *rows, uint16_t *columns){
    FLR_RESULT returncode = CLIENT_pkgMemGetCaptureSizeSrc(src, bytes, rows, columns);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetCaptureSizeSrc()

FLR_RESULT memReadCaptureSrc(const FLR_CAPTURE_SRC_E src, const uint8_t bufferNum, const uint32_t offset, const uint16_t sizeInBytes, uint8_t *data){
    if (sizeInBytes > MaxMemoryChunk)
    {
        return FLR_DATA_SIZE_ERROR;
    }
    FLR_RESULT returncode = CLIENT_pkgMemReadCaptureSrc(src, bufferNum, offset, sizeInBytes, data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of ReadCaptureSrc()

FLR_RESULT radiometrySetTempStableEnable(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetTempStableEnable(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTempStableEnable()

FLR_RESULT radiometryGetTempStableEnable(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTempStableEnable(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTempStableEnable()

FLR_RESULT radiometrySetFNumberLens0(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetFNumberLens0(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFNumberLens0()

FLR_RESULT radiometryGetFNumberLens0(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetFNumberLens0(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFNumberLens0()

FLR_RESULT radiometrySetFNumberLens1(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetFNumberLens1(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFNumberLens1()

FLR_RESULT radiometryGetFNumberLens1(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetFNumberLens1(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFNumberLens1()

FLR_RESULT radiometrySetTauLens0(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetTauLens0(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTauLens0()

FLR_RESULT radiometryGetTauLens0(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTauLens0(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTauLens0()

FLR_RESULT radiometrySetTauLens1(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetTauLens1(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTauLens1()

FLR_RESULT radiometryGetTauLens1(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTauLens1(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTauLens1()

FLR_RESULT radiometryGetGlobalGainDesired(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetGlobalGainDesired(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGlobalGainDesired()

FLR_RESULT radiometryGetGlobalOffsetDesired(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetGlobalOffsetDesired(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGlobalOffsetDesired()

FLR_RESULT radiometryGetGlobalGainApplied(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetGlobalGainApplied(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGlobalGainApplied()

FLR_RESULT radiometryGetGlobalOffsetApplied(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetGlobalOffsetApplied(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGlobalOffsetApplied()

FLR_RESULT radiometrySetTComponentOverrideMode(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetTComponentOverrideMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTComponentOverrideMode()

FLR_RESULT radiometryGetTComponentOverrideMode(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTComponentOverrideMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTComponentOverrideMode()

FLR_RESULT radiometrySetGlobalGainOverride(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetGlobalGainOverride(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetGlobalGainOverride()

FLR_RESULT radiometryGetGlobalGainOverride(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetGlobalGainOverride(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGlobalGainOverride()

FLR_RESULT radiometrySetGlobalOffsetOverride(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetGlobalOffsetOverride(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetGlobalOffsetOverride()

FLR_RESULT radiometryGetGlobalOffsetOverride(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetGlobalOffsetOverride(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGlobalOffsetOverride()

FLR_RESULT radiometrySetGlobalParamOverrideMode(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetGlobalParamOverrideMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetGlobalParamOverrideMode()

FLR_RESULT radiometryGetGlobalParamOverrideMode(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetGlobalParamOverrideMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGlobalParamOverrideMode()

FLR_RESULT radiometrySetRBFOHighGainDefault(const FLR_RADIOMETRY_RBFO_PARAMS_T data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetRBFOHighGainDefault(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetRBFOHighGainDefault()

FLR_RESULT radiometryGetRBFOHighGainDefault(FLR_RADIOMETRY_RBFO_PARAMS_T *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetRBFOHighGainDefault(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetRBFOHighGainDefault()

FLR_RESULT radiometrySetRBFOLowGainDefault(const FLR_RADIOMETRY_RBFO_PARAMS_T data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetRBFOLowGainDefault(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetRBFOLowGainDefault()

FLR_RESULT radiometryGetRBFOLowGainDefault(FLR_RADIOMETRY_RBFO_PARAMS_T *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetRBFOLowGainDefault(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetRBFOLowGainDefault()

FLR_RESULT radiometrySetRBFOHighGainFactory(const FLR_RADIOMETRY_RBFO_PARAMS_T data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetRBFOHighGainFactory(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetRBFOHighGainFactory()

FLR_RESULT radiometryGetRBFOHighGainFactory(FLR_RADIOMETRY_RBFO_PARAMS_T *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetRBFOHighGainFactory(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetRBFOHighGainFactory()

FLR_RESULT radiometrySetRBFOLowGainFactory(const FLR_RADIOMETRY_RBFO_PARAMS_T data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetRBFOLowGainFactory(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetRBFOLowGainFactory()

FLR_RESULT radiometryGetRBFOLowGainFactory(FLR_RADIOMETRY_RBFO_PARAMS_T *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetRBFOLowGainFactory(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetRBFOLowGainFactory()

FLR_RESULT radiometrySetDampingFactor(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetDampingFactor(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetDampingFactor()

FLR_RESULT radiometryGetDampingFactor(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetDampingFactor(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDampingFactor()

FLR_RESULT radiometryGetGoMEQ(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetGoMEQ(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGoMEQ()

FLR_RESULT radiometryGetGoMShutter(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetGoMShutter(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGoMShutter()

FLR_RESULT radiometryGetGoMLens(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetGoMLens(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGoMLens()

FLR_RESULT radiometryGetGoMLG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetGoMLG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGoMLG()

FLR_RESULT radiometryGetGoMFFC(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetGoMFFC(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGoMFFC()

FLR_RESULT radiometryGetTempLensHousing(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTempLensHousing(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTempLensHousing()

FLR_RESULT radiometryGetTempShutterHousing(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTempShutterHousing(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTempShutterHousing()

FLR_RESULT radiometryGetTempShutterPaddle(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTempShutterPaddle(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTempShutterPaddle()

FLR_RESULT radiometrySetFNumberShutterHousing(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetFNumberShutterHousing(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFNumberShutterHousing()

FLR_RESULT radiometryGetFNumberShutterHousing(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetFNumberShutterHousing(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFNumberShutterHousing()

FLR_RESULT radiometrySetEmissivityShutterHousing(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetEmissivityShutterHousing(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetEmissivityShutterHousing()

FLR_RESULT radiometryGetEmissivityShutterHousing(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetEmissivityShutterHousing(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetEmissivityShutterHousing()

FLR_RESULT radiometrySetM_DTfpa_Lens(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetM_DTfpa_Lens(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetM_DTfpa_Lens()

FLR_RESULT radiometryGetM_DTfpa_Lens(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetM_DTfpa_Lens(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetM_DTfpa_Lens()

FLR_RESULT radiometrySetOffset_Lens(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetOffset_Lens(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetOffset_Lens()

FLR_RESULT radiometryGetOffset_Lens(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetOffset_Lens(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetOffset_Lens()

FLR_RESULT radiometrySetM_Recursive_Lens(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetM_Recursive_Lens(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetM_Recursive_Lens()

FLR_RESULT radiometryGetM_Recursive_Lens(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetM_Recursive_Lens(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetM_Recursive_Lens()

FLR_RESULT radiometryGetGgFfc(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetGgFfc(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGgFfc()

FLR_RESULT radiometryGetCountsFromTemp(const FLR_RADIOMETRY_RBFO_TYPE_E rbfoType, const float temp, uint16_t *counts){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetCountsFromTemp(rbfoType, temp, counts);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetCountsFromTemp()

FLR_RESULT radiometryGetTempFromCounts(const FLR_RADIOMETRY_RBFO_TYPE_E rbfoType, const uint16_t counts, float *temp){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTempFromCounts(rbfoType, counts, temp);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTempFromCounts()

FLR_RESULT radiometrySetTempLensHousingOverride(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetTempLensHousingOverride(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTempLensHousingOverride()

FLR_RESULT radiometryGetTempLensHousingOverride(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTempLensHousingOverride(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTempLensHousingOverride()

FLR_RESULT radiometrySetTempShutterHousingOverride(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetTempShutterHousingOverride(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTempShutterHousingOverride()

FLR_RESULT radiometryGetTempShutterHousingOverride(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTempShutterHousingOverride(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTempShutterHousingOverride()

FLR_RESULT radiometrySetTempShutterPaddleOverride(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetTempShutterPaddleOverride(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTempShutterPaddleOverride()

FLR_RESULT radiometryGetTempShutterPaddleOverride(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTempShutterPaddleOverride(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTempShutterPaddleOverride()

FLR_RESULT radiometrySetSignalFactorLut(const FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_LUT_T data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetSignalFactorLut(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetSignalFactorLut()

FLR_RESULT radiometryGetSignalFactorLut(FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_LUT_T *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetSignalFactorLut(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSignalFactorLut()

FLR_RESULT radiometrySetNoiseFactorLut(const FLR_RADIOMETRY_NOISE_COMP_FACTOR_LUT_T data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetNoiseFactorLut(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetNoiseFactorLut()

FLR_RESULT radiometryGetNoiseFactorLut(FLR_RADIOMETRY_NOISE_COMP_FACTOR_LUT_T *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetNoiseFactorLut(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetNoiseFactorLut()

FLR_RESULT radiometrySetM_tfpaK(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetM_tfpaK(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetM_tfpaK()

FLR_RESULT radiometryGetM_tfpaK(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetM_tfpaK(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetM_tfpaK()

FLR_RESULT radiometrySetB_tfpaK(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetB_tfpaK(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetB_tfpaK()

FLR_RESULT radiometryGetB_tfpaK(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetB_tfpaK(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetB_tfpaK()

FLR_RESULT radiometrySetTAuxParams(const FLR_RADIOMETRY_TAUX_PARAMS_T data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetTAuxParams(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTAuxParams()

FLR_RESULT radiometryGetTAuxParams(FLR_RADIOMETRY_TAUX_PARAMS_T *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTAuxParams(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTAuxParams()

FLR_RESULT radiometrySetM_tAux(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetM_tAux(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetM_tAux()

FLR_RESULT radiometryGetM_tAux(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetM_tAux(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetM_tAux()

FLR_RESULT radiometrySetB_tAux(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetB_tAux(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetB_tAux()

FLR_RESULT radiometryGetB_tAux(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetB_tAux(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetB_tAux()

FLR_RESULT radiometrySetTsource_FFC(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetTsource_FFC(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTsource_FFC()

FLR_RESULT radiometryGetTsource_FFC(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTsource_FFC(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTsource_FFC()

FLR_RESULT radiometrySetM_DTfpa_Sh_h(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetM_DTfpa_Sh_h(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetM_DTfpa_Sh_h()

FLR_RESULT radiometryGetM_DTfpa_Sh_h(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetM_DTfpa_Sh_h(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetM_DTfpa_Sh_h()

FLR_RESULT radiometrySetOffset_Sh_h(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetOffset_Sh_h(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetOffset_Sh_h()

FLR_RESULT radiometryGetOffset_Sh_h(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetOffset_Sh_h(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetOffset_Sh_h()

FLR_RESULT radiometrySetM_Recursive_Sh_h(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetM_Recursive_Sh_h(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetM_Recursive_Sh_h()

FLR_RESULT radiometryGetM_Recursive_Sh_h(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetM_Recursive_Sh_h(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetM_Recursive_Sh_h()

FLR_RESULT radiometrySetM_DTfpa_Sh_p(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetM_DTfpa_Sh_p(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetM_DTfpa_Sh_p()

FLR_RESULT radiometryGetM_DTfpa_Sh_p(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetM_DTfpa_Sh_p(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetM_DTfpa_Sh_p()

FLR_RESULT radiometrySetOffset_Sh_p(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetOffset_Sh_p(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetOffset_Sh_p()

FLR_RESULT radiometryGetOffset_Sh_p(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetOffset_Sh_p(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetOffset_Sh_p()

FLR_RESULT radiometrySetM_Recursive_Sh_p(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetM_Recursive_Sh_p(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetM_Recursive_Sh_p()

FLR_RESULT radiometryGetM_Recursive_Sh_p(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetM_Recursive_Sh_p(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetM_Recursive_Sh_p()

FLR_RESULT radiometrySetM_Delta_Sh_p(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetM_Delta_Sh_p(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetM_Delta_Sh_p()

FLR_RESULT radiometryGetM_Delta_Sh_p(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetM_Delta_Sh_p(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetM_Delta_Sh_p()

FLR_RESULT radiometrySetB_Delta_Sh_p(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetB_Delta_Sh_p(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetB_Delta_Sh_p()

FLR_RESULT radiometryGetB_Delta_Sh_p(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetB_Delta_Sh_p(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetB_Delta_Sh_p()

FLR_RESULT radiometryGetDtTfpaK(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetDtTfpaK(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDtTfpaK()

FLR_RESULT radiometryGetDtTfpaK_Damp(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetDtTfpaK_Damp(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDtTfpaK_Damp()

FLR_RESULT radiometryGetTAuxK(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTAuxK(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTAuxK()

FLR_RESULT radiometrySetExternalFfcUpdateMode(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetExternalFfcUpdateMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetExternalFfcUpdateMode()

FLR_RESULT radiometryGetExternalFfcUpdateMode(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetExternalFfcUpdateMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetExternalFfcUpdateMode()

FLR_RESULT radiometryGetGG_scale(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetGG_scale(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGG_scale()

FLR_RESULT radiometrySetTempWindow(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetTempWindow(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTempWindow()

FLR_RESULT radiometryGetTempWindow(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTempWindow(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTempWindow()

FLR_RESULT radiometrySetTransmissionWindow(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetTransmissionWindow(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTransmissionWindow()

FLR_RESULT radiometryGetTransmissionWindow(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTransmissionWindow(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTransmissionWindow()

FLR_RESULT radiometrySetReflectivityWindow(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetReflectivityWindow(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetReflectivityWindow()

FLR_RESULT radiometryGetReflectivityWindow(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetReflectivityWindow(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetReflectivityWindow()

FLR_RESULT radiometrySetTempWindowReflection(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetTempWindowReflection(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTempWindowReflection()

FLR_RESULT radiometryGetTempWindowReflection(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTempWindowReflection(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTempWindowReflection()

FLR_RESULT radiometrySetTransmissionAtmosphere(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetTransmissionAtmosphere(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTransmissionAtmosphere()

FLR_RESULT radiometryGetTransmissionAtmosphere(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTransmissionAtmosphere(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTransmissionAtmosphere()

FLR_RESULT radiometrySetTempAtmosphere(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetTempAtmosphere(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTempAtmosphere()

FLR_RESULT radiometryGetTempAtmosphere(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTempAtmosphere(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTempAtmosphere()

FLR_RESULT radiometrySetEmissivityTarget(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetEmissivityTarget(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetEmissivityTarget()

FLR_RESULT radiometryGetEmissivityTarget(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetEmissivityTarget(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetEmissivityTarget()

FLR_RESULT radiometrySetTempBackground(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetTempBackground(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTempBackground()

FLR_RESULT radiometryGetTempBackground(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTempBackground(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTempBackground()

FLR_RESULT radiometryGetRadiometryCapable(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetRadiometryCapable(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetRadiometryCapable()

FLR_RESULT radiometrySetdeltaTempDampingFactor(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetdeltaTempDampingFactor(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetdeltaTempDampingFactor()

FLR_RESULT radiometryGetdeltaTempDampingFactor(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetdeltaTempDampingFactor(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetdeltaTempDampingFactor()

FLR_RESULT radiometrySetdeltaTempIntervalTime(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetdeltaTempIntervalTime(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetdeltaTempIntervalTime()

FLR_RESULT radiometryGetdeltaTempIntervalTime(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetdeltaTempIntervalTime(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetdeltaTempIntervalTime()

FLR_RESULT radiometrySetdeltaTempMaxValue(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetdeltaTempMaxValue(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetdeltaTempMaxValue()

FLR_RESULT radiometryGetdeltaTempMaxValue(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetdeltaTempMaxValue(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetdeltaTempMaxValue()

FLR_RESULT radiometrySetdeltaTempMaxIncrement(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetdeltaTempMaxIncrement(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetdeltaTempMaxIncrement()

FLR_RESULT radiometryGetdeltaTempMaxIncrement(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetdeltaTempMaxIncrement(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetdeltaTempMaxIncrement()

FLR_RESULT radiometrySetdeltaTempDampingTime(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetdeltaTempDampingTime(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetdeltaTempDampingTime()

FLR_RESULT radiometryGetdeltaTempDampingTime(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetdeltaTempDampingTime(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetdeltaTempDampingTime()

FLR_RESULT radiometryGetResponsivityFpaTemp(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetResponsivityFpaTemp(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetResponsivityFpaTemp()

FLR_RESULT radiometrySetM_Delta_Lens(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetM_Delta_Lens(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetM_Delta_Lens()

FLR_RESULT radiometryGetM_Delta_Lens(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetM_Delta_Lens(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetM_Delta_Lens()

FLR_RESULT radiometrySetB_Delta_Lens(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetB_Delta_Lens(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetB_Delta_Lens()

FLR_RESULT radiometryGetB_Delta_Lens(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetB_Delta_Lens(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetB_Delta_Lens()

FLR_RESULT radiometrySetM_Delta_Sh_h(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetM_Delta_Sh_h(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetM_Delta_Sh_h()

FLR_RESULT radiometryGetM_Delta_Sh_h(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetM_Delta_Sh_h(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetM_Delta_Sh_h()

FLR_RESULT radiometrySetB_Delta_Sh_h(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetB_Delta_Sh_h(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetB_Delta_Sh_h()

FLR_RESULT radiometryGetB_Delta_Sh_h(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetB_Delta_Sh_h(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetB_Delta_Sh_h()

FLR_RESULT radiometrySetGG_Scale_HG(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetGG_Scale_HG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetGG_Scale_HG()

FLR_RESULT radiometryGetGG_Scale_HG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetGG_Scale_HG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGG_Scale_HG()

FLR_RESULT radiometrySetGG_Scale_LG(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetGG_Scale_LG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetGG_Scale_LG()

FLR_RESULT radiometryGetGG_Scale_LG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetGG_Scale_LG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGG_Scale_LG()

FLR_RESULT radiometrySetRbfoScaledMode(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetRbfoScaledMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetRbfoScaledMode()

FLR_RESULT radiometryGetRbfoScaledMode(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetRbfoScaledMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetRbfoScaledMode()

FLR_RESULT radiometryGetUncertaintyFactor(FLR_RADIOMETRY_UNCERTAINTY_FACTOR_E *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetUncertaintyFactor(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetUncertaintyFactor()

FLR_RESULT radiometryGetTRoomMinThresh(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTRoomMinThresh(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTRoomMinThresh()

FLR_RESULT radiometryGetTRoomMaxThresh(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTRoomMaxThresh(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTRoomMaxThresh()

FLR_RESULT radiometryGetTOperatingMinThresh(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTOperatingMinThresh(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTOperatingMinThresh()

FLR_RESULT radiometryGetTOperatingMaxThresh(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTOperatingMaxThresh(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTOperatingMaxThresh()

FLR_RESULT radiometryGetStableTempThresh(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetStableTempThresh(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetStableTempThresh()

FLR_RESULT radiometryGetSlowDriftThresh(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetSlowDriftThresh(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSlowDriftThresh()

FLR_RESULT radiometryGetFfcTempThresh(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetFfcTempThresh(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFfcTempThresh()

FLR_RESULT radiometryGetTargetTempMinThreshLG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTargetTempMinThreshLG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTargetTempMinThreshLG()

FLR_RESULT radiometryGetTargetTempMaxThreshLG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTargetTempMaxThreshLG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTargetTempMaxThreshLG()

FLR_RESULT radiometryGetMFactorThresh(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetMFactorThresh(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMFactorThresh()

FLR_RESULT radiometryGetTargetTempMinThreshHG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTargetTempMinThreshHG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTargetTempMinThreshHG()

FLR_RESULT radiometryGetTargetTempMaxThreshHG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTargetTempMaxThreshHG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTargetTempMaxThreshHG()

FLR_RESULT radiometryGetUncertaintyStatusBits(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetUncertaintyStatusBits(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetUncertaintyStatusBits()

FLR_RESULT radiometrySetTemperatureOffset_HG(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetTemperatureOffset_HG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTemperatureOffset_HG()

FLR_RESULT radiometryGetTemperatureOffset_HG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTemperatureOffset_HG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTemperatureOffset_HG()

FLR_RESULT radiometrySetTemperatureOffset_LG(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetTemperatureOffset_LG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTemperatureOffset_LG()

FLR_RESULT radiometryGetTemperatureOffset_LG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetTemperatureOffset_LG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTemperatureOffset_LG()

FLR_RESULT radiometrySetM_Delta_Lens_HG(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetM_Delta_Lens_HG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetM_Delta_Lens_HG()

FLR_RESULT radiometryGetM_Delta_Lens_HG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetM_Delta_Lens_HG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetM_Delta_Lens_HG()

FLR_RESULT radiometrySetB_Delta_Lens_HG(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetB_Delta_Lens_HG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetB_Delta_Lens_HG()

FLR_RESULT radiometryGetB_Delta_Lens_HG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetB_Delta_Lens_HG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetB_Delta_Lens_HG()

FLR_RESULT radiometrySetM_Delta_Lens_LG(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetM_Delta_Lens_LG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetM_Delta_Lens_LG()

FLR_RESULT radiometryGetM_Delta_Lens_LG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetM_Delta_Lens_LG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetM_Delta_Lens_LG()

FLR_RESULT radiometrySetB_Delta_Lens_LG(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetB_Delta_Lens_LG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetB_Delta_Lens_LG()

FLR_RESULT radiometryGetB_Delta_Lens_LG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetB_Delta_Lens_LG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetB_Delta_Lens_LG()

FLR_RESULT radiometrySetOffset_Lens_HG(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetOffset_Lens_HG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetOffset_Lens_HG()

FLR_RESULT radiometryGetOffset_Lens_HG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetOffset_Lens_HG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetOffset_Lens_HG()

FLR_RESULT radiometrySetOffset_Lens_LG(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetOffset_Lens_LG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetOffset_Lens_LG()

FLR_RESULT radiometryGetOffset_Lens_LG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetOffset_Lens_LG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetOffset_Lens_LG()

FLR_RESULT radiometrySetM_Delta_Sh_p_HG(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetM_Delta_Sh_p_HG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetM_Delta_Sh_p_HG()

FLR_RESULT radiometryGetM_Delta_Sh_p_HG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetM_Delta_Sh_p_HG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetM_Delta_Sh_p_HG()

FLR_RESULT radiometrySetB_Delta_Sh_p_HG(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetB_Delta_Sh_p_HG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetB_Delta_Sh_p_HG()

FLR_RESULT radiometryGetB_Delta_Sh_p_HG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetB_Delta_Sh_p_HG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetB_Delta_Sh_p_HG()

FLR_RESULT radiometrySetM_Delta_Sh_p_LG(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetM_Delta_Sh_p_LG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetM_Delta_Sh_p_LG()

FLR_RESULT radiometryGetM_Delta_Sh_p_LG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetM_Delta_Sh_p_LG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetM_Delta_Sh_p_LG()

FLR_RESULT radiometrySetB_Delta_Sh_p_LG(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetB_Delta_Sh_p_LG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetB_Delta_Sh_p_LG()

FLR_RESULT radiometryGetB_Delta_Sh_p_LG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetB_Delta_Sh_p_LG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetB_Delta_Sh_p_LG()

FLR_RESULT radiometrySetM_Delta_Sh_h_HG(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetM_Delta_Sh_h_HG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetM_Delta_Sh_h_HG()

FLR_RESULT radiometryGetM_Delta_Sh_h_HG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetM_Delta_Sh_h_HG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetM_Delta_Sh_h_HG()

FLR_RESULT radiometrySetB_Delta_Sh_h_HG(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetB_Delta_Sh_h_HG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetB_Delta_Sh_h_HG()

FLR_RESULT radiometryGetB_Delta_Sh_h_HG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetB_Delta_Sh_h_HG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetB_Delta_Sh_h_HG()

FLR_RESULT radiometrySetM_Delta_Sh_h_LG(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetM_Delta_Sh_h_LG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetM_Delta_Sh_h_LG()

FLR_RESULT radiometryGetM_Delta_Sh_h_LG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetM_Delta_Sh_h_LG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetM_Delta_Sh_h_LG()

FLR_RESULT radiometrySetB_Delta_Sh_h_LG(const float data){
    FLR_RESULT returncode = CLIENT_pkgRadiometrySetB_Delta_Sh_h_LG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetB_Delta_Sh_h_LG()

FLR_RESULT radiometryGetB_Delta_Sh_h_LG(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetB_Delta_Sh_h_LG(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetB_Delta_Sh_h_LG()

FLR_RESULT radiometryGetGG_RoomTemp(float *data){
    FLR_RESULT returncode = CLIENT_pkgRadiometryGetGG_RoomTemp(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetGG_RoomTemp()

FLR_RESULT roicGetFPATemp(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgRoicGetFPATemp(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFPATemp()

FLR_RESULT roicGetFrameCount(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgRoicGetFrameCount(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFrameCount()

FLR_RESULT roicGetActiveNormalizationTarget(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgRoicGetActiveNormalizationTarget(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetActiveNormalizationTarget()

FLR_RESULT roicSetFPARampState(const FLR_ENABLE_E state){
    FLR_RESULT returncode = CLIENT_pkgRoicSetFPARampState(state);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFPARampState()

FLR_RESULT roicGetFPARampState(FLR_ENABLE_E *state){
    FLR_RESULT returncode = CLIENT_pkgRoicGetFPARampState(state);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFPARampState()

FLR_RESULT roicGetSensorADC1(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgRoicGetSensorADC1(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSensorADC1()

FLR_RESULT roicGetSensorADC2(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgRoicGetSensorADC2(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSensorADC2()

FLR_RESULT roicSetFPATempOffset(const int16_t data){
    FLR_RESULT returncode = CLIENT_pkgRoicSetFPATempOffset(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFPATempOffset()

FLR_RESULT roicGetFPATempOffset(int16_t *data){
    FLR_RESULT returncode = CLIENT_pkgRoicGetFPATempOffset(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFPATempOffset()

FLR_RESULT roicSetFPATempMode(const FLR_ROIC_TEMP_MODE_E data){
    FLR_RESULT returncode = CLIENT_pkgRoicSetFPATempMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFPATempMode()

FLR_RESULT roicGetFPATempMode(FLR_ROIC_TEMP_MODE_E *data){
    FLR_RESULT returncode = CLIENT_pkgRoicGetFPATempMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFPATempMode()

FLR_RESULT roicGetFPATempTable(FLR_ROIC_FPATEMP_TABLE_T *table){
    FLR_RESULT returncode = CLIENT_pkgRoicGetFPATempTable(table);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFPATempTable()

FLR_RESULT roicSetFPATempValue(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgRoicSetFPATempValue(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFPATempValue()

FLR_RESULT roicGetFPATempValue(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgRoicGetFPATempValue(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFPATempValue()

FLR_RESULT roicGetPreambleError(uint32_t *preambleError){
    FLR_RESULT returncode = CLIENT_pkgRoicGetPreambleError(preambleError);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetPreambleError()

FLR_RESULT roicInducePreambleError(const uint32_t everyNthFrame){
    FLR_RESULT returncode = CLIENT_pkgRoicInducePreambleError(everyNthFrame);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of InducePreambleError()

FLR_RESULT roicGetRoicStarted(FLR_ENABLE_E *roicStarted){
    FLR_RESULT returncode = CLIENT_pkgRoicGetRoicStarted(roicStarted);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetRoicStarted()

FLR_RESULT roicSetFrameSkip(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgRoicSetFrameSkip(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFrameSkip()

FLR_RESULT roicGetFrameSkip(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgRoicGetFrameSkip(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFrameSkip()

FLR_RESULT roicSetFrameOneShot(){
    FLR_RESULT returncode = CLIENT_pkgRoicSetFrameOneShot();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFrameOneShot()

FLR_RESULT scalerGetMaxZoom(uint32_t *zoom){
    FLR_RESULT returncode = CLIENT_pkgScalerGetMaxZoom(zoom);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMaxZoom()

FLR_RESULT scalerSetZoom(const FLR_SCALER_ZOOM_PARAMS_T zoomParams){
    FLR_RESULT returncode = CLIENT_pkgScalerSetZoom(zoomParams);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetZoom()

FLR_RESULT scalerGetZoom(FLR_SCALER_ZOOM_PARAMS_T *zoomParams){
    FLR_RESULT returncode = CLIENT_pkgScalerGetZoom(zoomParams);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetZoom()

FLR_RESULT scalerSetFractionalZoom(const uint32_t zoomNumerator, const uint32_t zoomDenominator, const uint32_t zoomXCenter, const uint32_t zoomYCenter, const FLR_ENABLE_E inChangeEnable, const uint32_t zoomOutXCenter, const uint32_t zoomOutYCenter, const FLR_ENABLE_E outChangeEnable){
    FLR_RESULT returncode = CLIENT_pkgScalerSetFractionalZoom(zoomNumerator, zoomDenominator, zoomXCenter, zoomYCenter, inChangeEnable, zoomOutXCenter, zoomOutYCenter, outChangeEnable);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFractionalZoom()

FLR_RESULT scalerSetIndexZoom(const uint32_t zoomIndex, const uint32_t zoomXCenter, const uint32_t zoomYCenter, const FLR_ENABLE_E inChangeEnable, const uint32_t zoomOutXCenter, const uint32_t zoomOutYCenter, const FLR_ENABLE_E outChangeEnable){
    FLR_RESULT returncode = CLIENT_pkgScalerSetIndexZoom(zoomIndex, zoomXCenter, zoomYCenter, inChangeEnable, zoomOutXCenter, zoomOutYCenter, outChangeEnable);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetIndexZoom()

FLR_RESULT scnrSetEnableState(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgScnrSetEnableState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetEnableState()

FLR_RESULT scnrGetEnableState(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgScnrGetEnableState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetEnableState()

FLR_RESULT scnrSetThColSum(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgScnrSetThColSum(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetThColSum()

FLR_RESULT scnrGetThColSum(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgScnrGetThColSum(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetThColSum()

FLR_RESULT scnrSetThPixel(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgScnrSetThPixel(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetThPixel()

FLR_RESULT scnrGetThPixel(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgScnrGetThPixel(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetThPixel()

FLR_RESULT scnrSetMaxCorr(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgScnrSetMaxCorr(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetMaxCorr()

FLR_RESULT scnrGetMaxCorr(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgScnrGetMaxCorr(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMaxCorr()

FLR_RESULT scnrGetThPixelApplied(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgScnrGetThPixelApplied(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetThPixelApplied()

FLR_RESULT scnrGetMaxCorrApplied(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgScnrGetMaxCorrApplied(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMaxCorrApplied()

FLR_RESULT scnrSetThColSumSafe(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgScnrSetThColSumSafe(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetThColSumSafe()

FLR_RESULT scnrGetThColSumSafe(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgScnrGetThColSumSafe(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetThColSumSafe()

FLR_RESULT scnrSetThPixelSafe(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgScnrSetThPixelSafe(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetThPixelSafe()

FLR_RESULT scnrGetThPixelSafe(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgScnrGetThPixelSafe(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetThPixelSafe()

FLR_RESULT scnrSetMaxCorrSafe(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgScnrSetMaxCorrSafe(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetMaxCorrSafe()

FLR_RESULT scnrGetMaxCorrSafe(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgScnrGetMaxCorrSafe(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMaxCorrSafe()

FLR_RESULT scnrSetCorrectionMethod(const FLR_SCNR_CORR_SELECT_E data){
    FLR_RESULT returncode = CLIENT_pkgScnrSetCorrectionMethod(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetCorrectionMethod()

FLR_RESULT scnrGetCorrectionMethod(FLR_SCNR_CORR_SELECT_E *data){
    FLR_RESULT returncode = CLIENT_pkgScnrGetCorrectionMethod(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetCorrectionMethod()

FLR_RESULT scnrSetStdThreshold(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgScnrSetStdThreshold(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetStdThreshold()

FLR_RESULT scnrGetStdThreshold(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgScnrGetStdThreshold(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetStdThreshold()

FLR_RESULT scnrSetNFrames(const uint32_t data){
    FLR_RESULT returncode = CLIENT_pkgScnrSetNFrames(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetNFrames()

FLR_RESULT scnrGetNFrames(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgScnrGetNFrames(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetNFrames()

FLR_RESULT scnrSetResetDesired(const uint32_t data){
    FLR_RESULT returncode = CLIENT_pkgScnrSetResetDesired(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetResetDesired()

FLR_RESULT scnrGetResetDesired(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgScnrGetResetDesired(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetResetDesired()

FLR_RESULT scnrSetM_modeOnly(const uint32_t data){
    FLR_RESULT returncode = CLIENT_pkgScnrSetM_modeOnly(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetM_modeOnly()

FLR_RESULT scnrGetM_modeOnly(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgScnrGetM_modeOnly(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetM_modeOnly()

FLR_RESULT scnrGetMode(FLR_SCNR_MODE_E *data){
    FLR_RESULT returncode = CLIENT_pkgScnrGetMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMode()

FLR_RESULT scnrSetSpecklesEnableState(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgScnrSetSpecklesEnableState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetSpecklesEnableState()

FLR_RESULT scnrGetSpecklesEnableState(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgScnrGetSpecklesEnableState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSpecklesEnableState()

FLR_RESULT scnrSetSpecklesThreshold(const uint32_t data){
    FLR_RESULT returncode = CLIENT_pkgScnrSetSpecklesThreshold(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetSpecklesThreshold()

FLR_RESULT scnrGetSpecklesThreshold(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgScnrGetSpecklesThreshold(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSpecklesThreshold()

FLR_RESULT scnrSetSpecklesRatio(const float data){
    FLR_RESULT returncode = CLIENT_pkgScnrSetSpecklesRatio(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetSpecklesRatio()

FLR_RESULT scnrGetSpecklesRatio(float *data){
    FLR_RESULT returncode = CLIENT_pkgScnrGetSpecklesRatio(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSpecklesRatio()

FLR_RESULT scnrSetSpecklesDF(const float data){
    FLR_RESULT returncode = CLIENT_pkgScnrSetSpecklesDF(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetSpecklesDF()

FLR_RESULT scnrGetSpecklesDF(float *data){
    FLR_RESULT returncode = CLIENT_pkgScnrGetSpecklesDF(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSpecklesDF()

FLR_RESULT scnrGetSpecklesDiffsBufferAddr(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgScnrGetSpecklesDiffsBufferAddr(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSpecklesDiffsBufferAddr()

FLR_RESULT scnrGetSpecklesOffsBufferAddr(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgScnrGetSpecklesOffsBufferAddr(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSpecklesOffsBufferAddr()

FLR_RESULT scnrSetSpecklesResetDesired(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgScnrSetSpecklesResetDesired(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetSpecklesResetDesired()

FLR_RESULT scnrGetSpecklesResetDesired(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgScnrGetSpecklesResetDesired(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSpecklesResetDesired()

FLR_RESULT sffcGetScaleFactor(float *data){
    FLR_RESULT returncode = CLIENT_pkgSffcGetScaleFactor(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetScaleFactor()

FLR_RESULT sffcGetDeltaTempLinearCoeff(float *data){
    FLR_RESULT returncode = CLIENT_pkgSffcGetDeltaTempLinearCoeff(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDeltaTempLinearCoeff()

FLR_RESULT sffcSetDeltaTempLinearCoeff(const float data){
    FLR_RESULT returncode = CLIENT_pkgSffcSetDeltaTempLinearCoeff(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetDeltaTempLinearCoeff()

FLR_RESULT sffcGetDeltaTempOffsetCoeff(float *data){
    FLR_RESULT returncode = CLIENT_pkgSffcGetDeltaTempOffsetCoeff(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDeltaTempOffsetCoeff()

FLR_RESULT sffcSetDeltaTempOffsetCoeff(const float data){
    FLR_RESULT returncode = CLIENT_pkgSffcSetDeltaTempOffsetCoeff(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetDeltaTempOffsetCoeff()

FLR_RESULT sffcGetFpaTempLinearCoeff(float *data){
    FLR_RESULT returncode = CLIENT_pkgSffcGetFpaTempLinearCoeff(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFpaTempLinearCoeff()

FLR_RESULT sffcSetFpaTempLinearCoeff(const float data){
    FLR_RESULT returncode = CLIENT_pkgSffcSetFpaTempLinearCoeff(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFpaTempLinearCoeff()

FLR_RESULT sffcGetFpaTempOffsetCoeff(float *data){
    FLR_RESULT returncode = CLIENT_pkgSffcGetFpaTempOffsetCoeff(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFpaTempOffsetCoeff()

FLR_RESULT sffcSetFpaTempOffsetCoeff(const float data){
    FLR_RESULT returncode = CLIENT_pkgSffcSetFpaTempOffsetCoeff(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFpaTempOffsetCoeff()

FLR_RESULT sffcGetDeltaTempTimeLimitInSecs(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgSffcGetDeltaTempTimeLimitInSecs(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDeltaTempTimeLimitInSecs()

FLR_RESULT sffcSetDeltaTempTimeLimitInSecs(const uint32_t data){
    FLR_RESULT returncode = CLIENT_pkgSffcSetDeltaTempTimeLimitInSecs(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetDeltaTempTimeLimitInSecs()

FLR_RESULT spnrSetEnableState(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgSpnrSetEnableState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetEnableState()

FLR_RESULT spnrGetEnableState(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgSpnrGetEnableState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetEnableState()

FLR_RESULT spnrGetState(FLR_SPNR_STATE_E *data){
    FLR_RESULT returncode = CLIENT_pkgSpnrGetState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetState()

FLR_RESULT spnrSetFrameDelay(const uint32_t data){
    FLR_RESULT returncode = CLIENT_pkgSpnrSetFrameDelay(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFrameDelay()

FLR_RESULT spnrGetFrameDelay(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgSpnrGetFrameDelay(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFrameDelay()

FLR_RESULT spnrSetSF(const float sf){
    FLR_RESULT returncode = CLIENT_pkgSpnrSetSF(sf);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetSF()

FLR_RESULT spnrGetSF(float *sf){
    FLR_RESULT returncode = CLIENT_pkgSpnrGetSF(sf);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSF()

FLR_RESULT spnrGetSFApplied(float *sf){
    FLR_RESULT returncode = CLIENT_pkgSpnrGetSFApplied(sf);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSFApplied()

FLR_RESULT spnrSetPSDKernel(const FLR_SPNR_PSD_KERNEL_T data){
    FLR_RESULT returncode = CLIENT_pkgSpnrSetPSDKernel(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetPSDKernel()

FLR_RESULT spnrGetPSDKernel(FLR_SPNR_PSD_KERNEL_T *data){
    FLR_RESULT returncode = CLIENT_pkgSpnrGetPSDKernel(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetPSDKernel()

FLR_RESULT spnrSetSFMin(const float sfmin){
    FLR_RESULT returncode = CLIENT_pkgSpnrSetSFMin(sfmin);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetSFMin()

FLR_RESULT spnrGetSFMin(float *sfmin){
    FLR_RESULT returncode = CLIENT_pkgSpnrGetSFMin(sfmin);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSFMin()

FLR_RESULT spnrSetSFMax(const float sfmax){
    FLR_RESULT returncode = CLIENT_pkgSpnrSetSFMax(sfmax);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetSFMax()

FLR_RESULT spnrGetSFMax(float *sfmax){
    FLR_RESULT returncode = CLIENT_pkgSpnrGetSFMax(sfmax);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSFMax()

FLR_RESULT spnrSetDFMin(const float dfmin){
    FLR_RESULT returncode = CLIENT_pkgSpnrSetDFMin(dfmin);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetDFMin()

FLR_RESULT spnrGetDFMin(float *dfmin){
    FLR_RESULT returncode = CLIENT_pkgSpnrGetDFMin(dfmin);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDFMin()

FLR_RESULT spnrSetDFMax(const float dfmax){
    FLR_RESULT returncode = CLIENT_pkgSpnrSetDFMax(dfmax);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetDFMax()

FLR_RESULT spnrGetDFMax(float *dfmax){
    FLR_RESULT returncode = CLIENT_pkgSpnrGetDFMax(dfmax);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDFMax()

FLR_RESULT spnrSetNormTarget(const float normTarget){
    FLR_RESULT returncode = CLIENT_pkgSpnrSetNormTarget(normTarget);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetNormTarget()

FLR_RESULT spnrGetNormTarget(float *normTarget){
    FLR_RESULT returncode = CLIENT_pkgSpnrGetNormTarget(normTarget);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetNormTarget()

FLR_RESULT spnrGetNormTargetApplied(float *normTargetApplied){
    FLR_RESULT returncode = CLIENT_pkgSpnrGetNormTargetApplied(normTargetApplied);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetNormTargetApplied()

FLR_RESULT spnrSetThPix(const uint16_t th_pix){
    FLR_RESULT returncode = CLIENT_pkgSpnrSetThPix(th_pix);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetThPix()

FLR_RESULT spnrGetThPix(uint16_t *th_pix){
    FLR_RESULT returncode = CLIENT_pkgSpnrGetThPix(th_pix);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetThPix()

FLR_RESULT spnrSetThPixSum(const uint16_t th_pixSum){
    FLR_RESULT returncode = CLIENT_pkgSpnrSetThPixSum(th_pixSum);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetThPixSum()

FLR_RESULT spnrGetThPixSum(uint16_t *th_pixSum){
    FLR_RESULT returncode = CLIENT_pkgSpnrGetThPixSum(th_pixSum);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetThPixSum()

FLR_RESULT spnrSetMaxcorr(const uint16_t maxcorr){
    FLR_RESULT returncode = CLIENT_pkgSpnrSetMaxcorr(maxcorr);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetMaxcorr()

FLR_RESULT spnrGetMaxcorr(uint16_t *maxcorr){
    FLR_RESULT returncode = CLIENT_pkgSpnrGetMaxcorr(maxcorr);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMaxcorr()

FLR_RESULT spnrGetAlgorithm(FLR_SPNR_ALGORITHM_E *data){
    FLR_RESULT returncode = CLIENT_pkgSpnrGetAlgorithm(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetAlgorithm()

FLR_RESULT spnrSetAlgorithmDesired(const FLR_SPNR_ALGORITHM_E data){
    FLR_RESULT returncode = CLIENT_pkgSpnrSetAlgorithmDesired(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetAlgorithmDesired()

FLR_RESULT spnrGetAlgorithmDesired(FLR_SPNR_ALGORITHM_E *data){
    FLR_RESULT returncode = CLIENT_pkgSpnrGetAlgorithmDesired(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetAlgorithmDesired()

FLR_RESULT spnrSetDFFast(const float dffast){
    FLR_RESULT returncode = CLIENT_pkgSpnrSetDFFast(dffast);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetDFFast()

FLR_RESULT spnrGetDFFast(float *dffast){
    FLR_RESULT returncode = CLIENT_pkgSpnrGetDFFast(dffast);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDFFast()

FLR_RESULT spnrSetDFSlow(const float dfslow){
    FLR_RESULT returncode = CLIENT_pkgSpnrSetDFSlow(dfslow);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetDFSlow()

FLR_RESULT spnrGetDFSlow(float *dfslow){
    FLR_RESULT returncode = CLIENT_pkgSpnrGetDFSlow(dfslow);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDFSlow()

FLR_RESULT spnrSetSensitivityThreshold(const float threshold){
    FLR_RESULT returncode = CLIENT_pkgSpnrSetSensitivityThreshold(threshold);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetSensitivityThreshold()

FLR_RESULT spnrGetSensitivityThreshold(float *threshold){
    FLR_RESULT returncode = CLIENT_pkgSpnrGetSensitivityThreshold(threshold);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSensitivityThreshold()

FLR_RESULT spnrSetReset(const FLR_SPNR_RESET_E resetType){
    FLR_RESULT returncode = CLIENT_pkgSpnrSetReset(resetType);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetReset()

FLR_RESULT spotMeterSetEnable(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgSpotmeterSetEnable(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetEnable()

FLR_RESULT spotMeterGetEnable(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgSpotmeterGetEnable(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetEnable()

FLR_RESULT spotMeterGetRoiMaxSize(uint16_t *width, uint16_t *height){
    FLR_RESULT returncode = CLIENT_pkgSpotmeterGetRoiMaxSize(width, height);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetRoiMaxSize()

FLR_RESULT spotMeterSetRoi(const FLR_ROI_T roi){
    FLR_RESULT returncode = CLIENT_pkgSpotmeterSetRoi(roi);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetRoi()

FLR_RESULT spotMeterGetRoi(FLR_ROI_T *roi){
    FLR_RESULT returncode = CLIENT_pkgSpotmeterGetRoi(roi);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetRoi()

FLR_RESULT spotMeterGetSpotStats(uint16_t *mean, uint16_t *deviation, FLR_SPOTMETER_SPOT_PARAM_T *min, FLR_SPOTMETER_SPOT_PARAM_T *max){
    FLR_RESULT returncode = CLIENT_pkgSpotmeterGetSpotStats(mean, deviation, min, max);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSpotStats()

FLR_RESULT spotMeterSetStatsMode(const FLR_SPOTMETER_STATS_TEMP_MODE_E mode){
    FLR_RESULT returncode = CLIENT_pkgSpotmeterSetStatsMode(mode);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetStatsMode()

FLR_RESULT spotMeterGetStatsMode(FLR_SPOTMETER_STATS_TEMP_MODE_E *mode){
    FLR_RESULT returncode = CLIENT_pkgSpotmeterGetStatsMode(mode);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetStatsMode()

FLR_RESULT spotMeterGetTempStats(float *mean, float *deviation, FLR_SPOTMETER_STAT_PARAM_TEMP_T *min, FLR_SPOTMETER_STAT_PARAM_TEMP_T *max){
    FLR_RESULT returncode = CLIENT_pkgSpotmeterGetTempStats(mean, deviation, min, max);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTempStats()

FLR_RESULT srnrSetEnableState(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgSrnrSetEnableState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetEnableState()

FLR_RESULT srnrGetEnableState(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgSrnrGetEnableState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetEnableState()

FLR_RESULT srnrSetThRowSum(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgSrnrSetThRowSum(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetThRowSum()

FLR_RESULT srnrGetThRowSum(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgSrnrGetThRowSum(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetThRowSum()

FLR_RESULT srnrSetThPixel(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgSrnrSetThPixel(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetThPixel()

FLR_RESULT srnrGetThPixel(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgSrnrGetThPixel(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetThPixel()

FLR_RESULT srnrSetMaxCorr(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgSrnrSetMaxCorr(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetMaxCorr()

FLR_RESULT srnrGetMaxCorr(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgSrnrGetMaxCorr(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMaxCorr()

FLR_RESULT srnrGetThPixelApplied(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgSrnrGetThPixelApplied(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetThPixelApplied()

FLR_RESULT srnrGetMaxCorrApplied(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgSrnrGetMaxCorrApplied(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMaxCorrApplied()

FLR_RESULT symbologySetEnable(const FLR_ENABLE_E draw_symbols){
    FLR_RESULT returncode = CLIENT_pkgSymbologySetEnable(draw_symbols);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetEnable()

FLR_RESULT symbologyCreateBitmap(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t width, const int16_t height){
    FLR_RESULT returncode = CLIENT_pkgSymbologyCreateBitmap(ID, pos_X, pos_Y, width, height);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of CreateBitmap()

FLR_RESULT symbologySendData(const uint8_t ID, const int16_t size, const uint8_t text[]){
    FLR_RESULT returncode = CLIENT_pkgSymbologySendData(ID, size, text);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SendData()

FLR_RESULT symbologyCreateArc(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t width, const int16_t height, const float start_angle, const float end_angle, const uint32_t color){
    FLR_RESULT returncode = CLIENT_pkgSymbologyCreateArc(ID, pos_X, pos_Y, width, height, start_angle, end_angle, color);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of CreateArc()

FLR_RESULT symbologyCreateText(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t width, const int16_t height, const int8_t font, const int16_t size, const FLR_SYMBOLOGY_TEXT_ALIGNMENT_E alignment, const uint32_t color, const uint8_t text[]){
    FLR_RESULT returncode = CLIENT_pkgSymbologyCreateText(ID, pos_X, pos_Y, width, height, font, size, alignment, color, text);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of CreateText()

FLR_RESULT symbologyMoveSprite(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y){
    FLR_RESULT returncode = CLIENT_pkgSymbologyMoveSprite(ID, pos_X, pos_Y);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of MoveSprite()

FLR_RESULT symbologyAddToGroup(const uint8_t ID, const uint8_t group_ID){
    FLR_RESULT returncode = CLIENT_pkgSymbologyAddToGroup(ID, group_ID);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of AddToGroup()

FLR_RESULT symbologyRemoveFromGroup(const uint8_t ID, const uint8_t group_ID){
    FLR_RESULT returncode = CLIENT_pkgSymbologyRemoveFromGroup(ID, group_ID);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of RemoveFromGroup()

FLR_RESULT symbologyUpdateAndShow(const uint8_t ID, const uint8_t visible){
    FLR_RESULT returncode = CLIENT_pkgSymbologyUpdateAndShow(ID, visible);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of UpdateAndShow()

FLR_RESULT symbologyUpdateAndShowGroup(const uint8_t group_ID, const uint8_t visible){
    FLR_RESULT returncode = CLIENT_pkgSymbologyUpdateAndShowGroup(group_ID, visible);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of UpdateAndShowGroup()

FLR_RESULT symbologyDelete(const uint8_t ID){
    FLR_RESULT returncode = CLIENT_pkgSymbologyDelete(ID);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of Delete()

FLR_RESULT symbologyDeleteGroup(const uint8_t group_ID){
    FLR_RESULT returncode = CLIENT_pkgSymbologyDeleteGroup(group_ID);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of DeleteGroup()

FLR_RESULT symbologyCreateFilledRectangle(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t width, const int16_t height, const uint32_t color){
    FLR_RESULT returncode = CLIENT_pkgSymbologyCreateFilledRectangle(ID, pos_X, pos_Y, width, height, color);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of CreateFilledRectangle()

FLR_RESULT symbologyCreateOutlinedRectangle(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t width, const int16_t height, const uint32_t color){
    FLR_RESULT returncode = CLIENT_pkgSymbologyCreateOutlinedRectangle(ID, pos_X, pos_Y, width, height, color);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of CreateOutlinedRectangle()

FLR_RESULT symbologyCreateBitmapFromPng(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t size){
    FLR_RESULT returncode = CLIENT_pkgSymbologyCreateBitmapFromPng(ID, pos_X, pos_Y, size);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of CreateBitmapFromPng()

FLR_RESULT symbologyCreateCompressedBitmap(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t width, const int16_t height){
    FLR_RESULT returncode = CLIENT_pkgSymbologyCreateCompressedBitmap(ID, pos_X, pos_Y, width, height);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of CreateCompressedBitmap()

FLR_RESULT symbologyCreateBitmapFromPngFile(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const uint8_t path[]){
    FLR_RESULT returncode = CLIENT_pkgSymbologyCreateBitmapFromPngFile(ID, pos_X, pos_Y, path);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of CreateBitmapFromPngFile()

FLR_RESULT symbologyCreateBitmapFromFile(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const uint8_t path[], const FLR_SYMBOLOGY_IMAGE_TYPE_E imageType){
    FLR_RESULT returncode = CLIENT_pkgSymbologyCreateBitmapFromFile(ID, pos_X, pos_Y, path, imageType);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of CreateBitmapFromFile()

FLR_RESULT symbologyResetWritePosition(const uint8_t ID){
    FLR_RESULT returncode = CLIENT_pkgSymbologyResetWritePosition(ID);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of ResetWritePosition()

FLR_RESULT symbologyMoveByOffset(const uint8_t ID, const int16_t off_X, const int16_t off_Y){
    FLR_RESULT returncode = CLIENT_pkgSymbologyMoveByOffset(ID, off_X, off_Y);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of MoveByOffset()

FLR_RESULT symbologyMoveGroupByOffset(const uint8_t ID, const int16_t off_X, const int16_t off_Y){
    FLR_RESULT returncode = CLIENT_pkgSymbologyMoveGroupByOffset(ID, off_X, off_Y);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of MoveGroupByOffset()

FLR_RESULT symbologyCreateFilledEllipse(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t width, const int16_t height, const uint32_t color){
    FLR_RESULT returncode = CLIENT_pkgSymbologyCreateFilledEllipse(ID, pos_X, pos_Y, width, height, color);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of CreateFilledEllipse()

FLR_RESULT symbologyCreateLine(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t pos_X2, const int16_t pos_Y2, const uint32_t color){
    FLR_RESULT returncode = CLIENT_pkgSymbologyCreateLine(ID, pos_X, pos_Y, pos_X2, pos_Y2, color);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of CreateLine()

FLR_RESULT symbologySetZorder(const uint8_t ID, const uint8_t zorder){
    FLR_RESULT returncode = CLIENT_pkgSymbologySetZorder(ID, zorder);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetZorder()

FLR_RESULT symbologySaveConfiguration(){
    FLR_RESULT returncode = CLIENT_pkgSymbologySaveConfiguration();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SaveConfiguration()

FLR_RESULT symbologyReloadConfiguration(){
    FLR_RESULT returncode = CLIENT_pkgSymbologyReloadConfiguration();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of ReloadConfiguration()

FLR_RESULT symbologyGetEnable(FLR_ENABLE_E *draw_symbols){
    FLR_RESULT returncode = CLIENT_pkgSymbologyGetEnable(draw_symbols);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetEnable()

FLR_RESULT symbologySetClonesNumber(const uint8_t ID, const uint8_t numberOfClones){
    FLR_RESULT returncode = CLIENT_pkgSymbologySetClonesNumber(ID, numberOfClones);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetClonesNumber()

FLR_RESULT symbologyMoveCloneByOffset(const uint8_t ID, const uint8_t cloneID, const int16_t pos_X, const int16_t pos_Y){
    FLR_RESULT returncode = CLIENT_pkgSymbologyMoveCloneByOffset(ID, cloneID, pos_X, pos_Y);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of MoveCloneByOffset()

FLR_RESULT symbologyMoveCloneSprite(const uint8_t ID, const uint8_t cloneID, const int16_t pos_X, const int16_t pos_Y){
    FLR_RESULT returncode = CLIENT_pkgSymbologyMoveCloneSprite(ID, cloneID, pos_X, pos_Y);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of MoveCloneSprite()

FLR_RESULT symbologySetTransformation(const FLR_SYMBOLOGY_TRANSFORMATION_E transformation){
    FLR_RESULT returncode = CLIENT_pkgSymbologySetTransformation(transformation);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTransformation()

FLR_RESULT symbologyUpdateAllVisible(){
    FLR_RESULT returncode = CLIENT_pkgSymbologyUpdateAllVisible();
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of UpdateAllVisible()

FLR_RESULT symbologySetSizeAndScalingMode(const uint8_t ID, const int16_t width, const int16_t height, const FLR_SYMBOLOGY_SCALING_MODE_E scalingMode){
    FLR_RESULT returncode = CLIENT_pkgSymbologySetSizeAndScalingMode(ID, width, height, scalingMode);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetSizeAndScalingMode()

FLR_RESULT symbologyCreateLineHVT(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t pos_X2, const int16_t pos_Y2, const uint32_t color1, const uint32_t color2, const uint16_t dashLen, const uint16_t thickness){
    FLR_RESULT returncode = CLIENT_pkgSymbologyCreateLineHVT(ID, pos_X, pos_Y, pos_X2, pos_Y2, color1, color2, dashLen, thickness);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of CreateLineHVT()

FLR_RESULT symbologyCreateTextHVT(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t width, const int16_t height, const int8_t font, const int16_t size, const FLR_SYMBOLOGY_TEXT_ALIGNMENT_E alignment, const uint32_t color1, const uint32_t color2, const uint8_t dashLen, const uint8_t text[]){
    FLR_RESULT returncode = CLIENT_pkgSymbologyCreateTextHVT(ID, pos_X, pos_Y, width, height, font, size, alignment, color1, color2, dashLen, text);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of CreateTextHVT()

FLR_RESULT symbologyCreateTextBg(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t width, const int16_t height, const int8_t font, const int16_t size, const FLR_SYMBOLOGY_TEXT_ALIGNMENT_E alignment, const uint32_t color, const uint32_t bgColor, const uint8_t text[]){
    FLR_RESULT returncode = CLIENT_pkgSymbologyCreateTextBg(ID, pos_X, pos_Y, width, height, font, size, alignment, color, bgColor, text);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of CreateTextBg()

FLR_RESULT symbologyCreateScaledBitmapFromFile(const uint8_t ID, const int16_t pos_X, const int16_t pos_Y, const int16_t width, const int16_t height, const FLR_SYMBOLOGY_SCALING_MODE_E scalingMode, const uint8_t path[], const FLR_SYMBOLOGY_IMAGE_TYPE_E imageType){
    FLR_RESULT returncode = CLIENT_pkgSymbologyCreateScaledBitmapFromFile(ID, pos_X, pos_Y, width, height, scalingMode, path, imageType);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of CreateScaledBitmapFromFile()

FLR_RESULT symbologyGetLocation(const uint8_t ID, int16_t *pos_X, int16_t *pos_Y){
    FLR_RESULT returncode = CLIENT_pkgSymbologyGetLocation(ID, pos_X, pos_Y);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetLocation()

FLR_RESULT symbologyGetSize(const uint8_t ID, int16_t *width, int16_t *height){
    FLR_RESULT returncode = CLIENT_pkgSymbologyGetSize(ID, width, height);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSize()

FLR_RESULT symbologyGetZorder(const uint8_t ID, uint8_t *zorder){
    FLR_RESULT returncode = CLIENT_pkgSymbologyGetZorder(ID, zorder);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetZorder()

FLR_RESULT symbologyGetColor(const uint8_t ID, uint32_t *color){
    FLR_RESULT returncode = CLIENT_pkgSymbologyGetColor(ID, color);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetColor()

FLR_RESULT symbologyGetType(const uint8_t ID, FLR_SYMBOLOGY_TYPE_E *type){
    FLR_RESULT returncode = CLIENT_pkgSymbologyGetType(ID, type);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetType()

FLR_RESULT symbologyCopySymbol(const uint8_t source, const uint8_t destination){
    FLR_RESULT returncode = CLIENT_pkgSymbologyCopySymbol(source, destination);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of CopySymbol()

FLR_RESULT symbologyGetTextFontSize(const uint8_t ID, int16_t *size){
    FLR_RESULT returncode = CLIENT_pkgSymbologyGetTextFontSize(ID, size);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTextFontSize()

FLR_RESULT sysctrlSetFreezeState(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgSysctrlSetFreezeState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFreezeState()

FLR_RESULT sysctrlGetFreezeState(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgSysctrlGetFreezeState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFreezeState()

FLR_RESULT sysctrlGetCameraFrameRate(uint32_t *frameRate){
    FLR_RESULT returncode = CLIENT_pkgSysctrlGetCameraFrameRate(frameRate);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetCameraFrameRate()

FLR_RESULT sysctrlGetUptimeSecs(uint32_t *uptime){
    FLR_RESULT returncode = CLIENT_pkgSysctrlGetUptimeSecs(uptime);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetUptimeSecs()

FLR_RESULT sysctrlSetUsbVideoIR16Mode(const FLR_SYSCTRL_USBIR16_MODE_E data){
    FLR_RESULT returncode = CLIENT_pkgSysctrlSetUsbVideoIR16Mode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetUsbVideoIR16Mode()

FLR_RESULT sysctrlGetUsbVideoIR16Mode(FLR_SYSCTRL_USBIR16_MODE_E *data){
    FLR_RESULT returncode = CLIENT_pkgSysctrlGetUsbVideoIR16Mode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetUsbVideoIR16Mode()

FLR_RESULT sysctrlSetOperatingMode(const FLR_SYSCTRL_OPERATING_MODE_E data){
    FLR_RESULT returncode = CLIENT_pkgSysctrlSetOperatingMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetOperatingMode()

FLR_RESULT sysctrlGetOperatingMode(FLR_SYSCTRL_OPERATING_MODE_E *data){
    FLR_RESULT returncode = CLIENT_pkgSysctrlGetOperatingMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetOperatingMode()

FLR_RESULT sysctrlGetAvgFpaTempCounts(float *data){
    FLR_RESULT returncode = CLIENT_pkgSysctrlGetAvgFpaTempCounts(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetAvgFpaTempCounts()

FLR_RESULT sysctrlSetFpaTempFrames(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgSysctrlSetFpaTempFrames(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetFpaTempFrames()

FLR_RESULT sysctrlGetFpaTempFrames(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgSysctrlGetFpaTempFrames(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetFpaTempFrames()

FLR_RESULT sysinfoGetMonitorSoftwareRev(uint32_t *major, uint32_t *minor, uint32_t *patch){
    FLR_RESULT returncode = CLIENT_pkgSysinfoGetMonitorSoftwareRev(major, minor, patch);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMonitorSoftwareRev()

FLR_RESULT sysinfoGetMonitorBuildVariant(FLR_SYSINFO_MONITOR_BUILD_VARIANT_T *monitorBuildVariant){
    FLR_RESULT returncode = CLIENT_pkgSysinfoGetMonitorBuildVariant(monitorBuildVariant);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMonitorBuildVariant()

FLR_RESULT sysinfoGetProductName(uint8_t name[]){
    FLR_RESULT returncode = CLIENT_pkgSysinfoGetProductName(name);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetProductName()

FLR_RESULT sysinfoGetCameraSN(uint8_t number[]){
    FLR_RESULT returncode = CLIENT_pkgSysinfoGetCameraSN(number);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetCameraSN()

FLR_RESULT sysinfoGetBootLocation(uint32_t *bootSwLocation){
    FLR_RESULT returncode = CLIENT_pkgSysinfoGetBootLocation(bootSwLocation);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetBootLocation()

FLR_RESULT sysinfoGetSwConfigID(FLR_SYSINFO_SW_CONFIG_ID_E *swConfigID){
    FLR_RESULT returncode = CLIENT_pkgSysinfoGetSwConfigID(swConfigID);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSwConfigID()

FLR_RESULT sysinfoGetSwPermissions(FLR_SYSINFO_SW_PERMISSIONS_E *swPermissions){
    FLR_RESULT returncode = CLIENT_pkgSysinfoGetSwPermissions(swPermissions);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSwPermissions()

FLR_RESULT sysinfoGetIs9HzBuild(uint32_t *is9HzBuild){
    FLR_RESULT returncode = CLIENT_pkgSysinfoGetIs9HzBuild(is9HzBuild);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetIs9HzBuild()

FLR_RESULT sysinfoGetProductVersion(uint32_t *major, uint32_t *minor, uint32_t *patch){
    FLR_RESULT returncode = CLIENT_pkgSysinfoGetProductVersion(major, minor, patch);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetProductVersion()

FLR_RESULT sysinfoGetMonitorProductRev(uint32_t *major, uint32_t *minor, uint32_t *patch){
    FLR_RESULT returncode = CLIENT_pkgSysinfoGetMonitorProductRev(major, minor, patch);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMonitorProductRev()

FLR_RESULT sysinfoGetOpticalRevision(uint16_t *revision){
    FLR_RESULT returncode = CLIENT_pkgSysinfoGetOpticalRevision(revision);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetOpticalRevision()

FLR_RESULT sysinfoGetSensorRevision(uint16_t *revision){
    FLR_RESULT returncode = CLIENT_pkgSysinfoGetSensorRevision(revision);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSensorRevision()

FLR_RESULT sysinfoGetProbeTipSN(uint8_t number[]){
    FLR_RESULT returncode = CLIENT_pkgSysinfoGetProbeTipSN(number);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetProbeTipSN()

FLR_RESULT sysinfoGetMechanicalRevision(uint16_t *revision){
    FLR_RESULT returncode = CLIENT_pkgSysinfoGetMechanicalRevision(revision);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMechanicalRevision()

FLR_RESULT sysinfoGetProbeTipType(FLR_SYSINFO_PROBE_TIP_TYPE *type){
    FLR_RESULT returncode = CLIENT_pkgSysinfoGetProbeTipType(type);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetProbeTipType()

FLR_RESULT systemSymbolsGetID(const FLR_SYSTEMSYMBOLS_SYMBOL_E symbol, uint8_t *id, FLR_SYSTEMSYMBOLS_ID_TYPE_E *id_type){
    FLR_RESULT returncode = CLIENT_pkgSystemsymbolsGetID(symbol, id, id_type);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetID()

FLR_RESULT systemSymbolsSetID(const FLR_SYSTEMSYMBOLS_SYMBOL_E symbol, const uint8_t id, const FLR_SYSTEMSYMBOLS_ID_TYPE_E id_type){
    FLR_RESULT returncode = CLIENT_pkgSystemsymbolsSetID(symbol, id, id_type);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetID()

FLR_RESULT systemSymbolsGetEnable(const FLR_SYSTEMSYMBOLS_SYMBOL_E symbol, FLR_ENABLE_E *enabled){
    FLR_RESULT returncode = CLIENT_pkgSystemsymbolsGetEnable(symbol, enabled);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetEnable()

FLR_RESULT systemSymbolsSetEnable(const FLR_SYSTEMSYMBOLS_SYMBOL_E symbol, const FLR_ENABLE_E enabled){
    FLR_RESULT returncode = CLIENT_pkgSystemsymbolsSetEnable(symbol, enabled);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetEnable()

FLR_RESULT systemSymbolsGetSpotConfig(FLR_SYSTEMSYMBOLS_SPOTCONFIG_T *config){
    FLR_RESULT returncode = CLIENT_pkgSystemsymbolsGetSpotConfig(config);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSpotConfig()

FLR_RESULT systemSymbolsSetSpotConfig(const FLR_SYSTEMSYMBOLS_SPOTCONFIG_T config){
    FLR_RESULT returncode = CLIENT_pkgSystemsymbolsSetSpotConfig(config);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetSpotConfig()

FLR_RESULT systemSymbolsGetIsoConfig(FLR_SYSTEMSYMBOLS_ISOCONFIG_T *config){
    FLR_RESULT returncode = CLIENT_pkgSystemsymbolsGetIsoConfig(config);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetIsoConfig()

FLR_RESULT systemSymbolsSetIsoConfig(const FLR_SYSTEMSYMBOLS_ISOCONFIG_T config){
    FLR_RESULT returncode = CLIENT_pkgSystemsymbolsSetIsoConfig(config);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetIsoConfig()

FLR_RESULT systemSymbolsGetBarConfig(FLR_SYSTEMSYMBOLS_BARCONFIG_T *lowGainConfig, FLR_SYSTEMSYMBOLS_BARCONFIG_T *highGainConfig, FLR_TEMPERATURE_UNIT_E *unit){
    FLR_RESULT returncode = CLIENT_pkgSystemsymbolsGetBarConfig(lowGainConfig, highGainConfig, unit);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetBarConfig()

FLR_RESULT systemSymbolsSetBarConfig(const FLR_SYSTEMSYMBOLS_BARCONFIG_T lowGainConfig, const FLR_SYSTEMSYMBOLS_BARCONFIG_T highGainConfig, const FLR_TEMPERATURE_UNIT_E unit){
    FLR_RESULT returncode = CLIENT_pkgSystemsymbolsSetBarConfig(lowGainConfig, highGainConfig, unit);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetBarConfig()

FLR_RESULT systemSymbolsGetSpotConfigIds(FLR_SYSTEMSYMBOLS_SPOTCONFIG_ID_T *config){
    FLR_RESULT returncode = CLIENT_pkgSystemsymbolsGetSpotConfigIds(config);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSpotConfigIds()

FLR_RESULT systemSymbolsSetSpotConfigIds(const FLR_SYSTEMSYMBOLS_SPOTCONFIG_ID_T config){
    FLR_RESULT returncode = CLIENT_pkgSystemsymbolsSetSpotConfigIds(config);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetSpotConfigIds()

FLR_RESULT systemSymbolsGetIsoConfigIds(FLR_SYSTEMSYMBOLS_ISOCONFIG_ID_T *config){
    FLR_RESULT returncode = CLIENT_pkgSystemsymbolsGetIsoConfigIds(config);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetIsoConfigIds()

FLR_RESULT systemSymbolsSetIsoConfigIds(const FLR_SYSTEMSYMBOLS_ISOCONFIG_ID_T config){
    FLR_RESULT returncode = CLIENT_pkgSystemsymbolsSetIsoConfigIds(config);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetIsoConfigIds()

FLR_RESULT telemetrySetState(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgTelemetrySetState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetState()

FLR_RESULT telemetryGetState(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgTelemetryGetState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetState()

FLR_RESULT telemetrySetLocation(const FLR_TELEMETRY_LOC_E data){
    FLR_RESULT returncode = CLIENT_pkgTelemetrySetLocation(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetLocation()

FLR_RESULT telemetryGetLocation(FLR_TELEMETRY_LOC_E *data){
    FLR_RESULT returncode = CLIENT_pkgTelemetryGetLocation(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetLocation()

FLR_RESULT telemetrySetPacking(const FLR_TELEMETRY_PACKING_E data){
    FLR_RESULT returncode = CLIENT_pkgTelemetrySetPacking(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetPacking()

FLR_RESULT telemetryGetPacking(FLR_TELEMETRY_PACKING_E *data){
    FLR_RESULT returncode = CLIENT_pkgTelemetryGetPacking(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetPacking()

FLR_RESULT telemetrySetOrder(const FLR_TELEMETRY_ORDER_E data){
    FLR_RESULT returncode = CLIENT_pkgTelemetrySetOrder(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetOrder()

FLR_RESULT telemetryGetOrder(FLR_TELEMETRY_ORDER_E *data){
    FLR_RESULT returncode = CLIENT_pkgTelemetryGetOrder(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetOrder()

FLR_RESULT telemetrySetPackingVC1(const FLR_TELEMETRY_PACKING_E data){
    FLR_RESULT returncode = CLIENT_pkgTelemetrySetPackingVC1(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetPackingVC1()

FLR_RESULT telemetryGetPackingVC1(FLR_TELEMETRY_PACKING_E *data){
    FLR_RESULT returncode = CLIENT_pkgTelemetryGetPackingVC1(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetPackingVC1()

FLR_RESULT telemetrySetMipiEmbeddedDataTag(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgTelemetrySetMipiEmbeddedDataTag(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetMipiEmbeddedDataTag()

FLR_RESULT telemetryGetMipiEmbeddedDataTag(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgTelemetryGetMipiEmbeddedDataTag(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMipiEmbeddedDataTag()

FLR_RESULT testRampSetType(const uint8_t index, const FLR_TESTRAMP_TYPE_E data){
    FLR_RESULT returncode = CLIENT_pkgTestrampSetType(index, data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetType()

FLR_RESULT testRampGetType(const uint8_t index, FLR_TESTRAMP_TYPE_E *data){
    FLR_RESULT returncode = CLIENT_pkgTestrampGetType(index, data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetType()

FLR_RESULT testRampSetSettings(const uint8_t index, const FLR_TESTRAMP_SETTINGS_T data){
    FLR_RESULT returncode = CLIENT_pkgTestrampSetSettings(index, data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetSettings()

FLR_RESULT testRampGetSettings(const uint8_t index, FLR_TESTRAMP_SETTINGS_T *data){
    FLR_RESULT returncode = CLIENT_pkgTestrampGetSettings(index, data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetSettings()

FLR_RESULT testRampSetMotionState(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgTestrampSetMotionState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetMotionState()

FLR_RESULT testRampGetMotionState(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgTestrampGetMotionState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMotionState()

FLR_RESULT testRampSetIndex(const uint8_t data){
    FLR_RESULT returncode = CLIENT_pkgTestrampSetIndex(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetIndex()

FLR_RESULT testRampGetIndex(uint8_t *data){
    FLR_RESULT returncode = CLIENT_pkgTestrampGetIndex(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetIndex()

FLR_RESULT testRampGetMaxIndex(uint8_t *data){
    FLR_RESULT returncode = CLIENT_pkgTestrampGetMaxIndex(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMaxIndex()

FLR_RESULT testRampSetPN9ContinuousMode(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgTestrampSetPN9ContinuousMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetPN9ContinuousMode()

FLR_RESULT testRampGetPN9ContinuousMode(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgTestrampGetPN9ContinuousMode(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetPN9ContinuousMode()

FLR_RESULT testRampSetAnimationSettings(const FLR_TESTRAMP_ANIMATION_SETTINGS_T settings){
    FLR_RESULT returncode = CLIENT_pkgTestrampSetAnimationSettings(settings);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetAnimationSettings()

FLR_RESULT testRampGetAnimationSettings(FLR_TESTRAMP_ANIMATION_SETTINGS_T *settings){
    FLR_RESULT returncode = CLIENT_pkgTestrampGetAnimationSettings(settings);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetAnimationSettings()

FLR_RESULT tfSetEnableState(const FLR_ENABLE_E data){
    FLR_RESULT returncode = CLIENT_pkgTfSetEnableState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetEnableState()

FLR_RESULT tfGetEnableState(FLR_ENABLE_E *data){
    FLR_RESULT returncode = CLIENT_pkgTfGetEnableState(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetEnableState()

FLR_RESULT tfSetDelta_nf(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgTfSetDelta_nf(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetDelta_nf()

FLR_RESULT tfGetDelta_nf(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgTfGetDelta_nf(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDelta_nf()

FLR_RESULT tfSetTHDeltaMotion(const uint16_t data){
    FLR_RESULT returncode = CLIENT_pkgTfSetTHDeltaMotion(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTHDeltaMotion()

FLR_RESULT tfGetTHDeltaMotion(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgTfGetTHDeltaMotion(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTHDeltaMotion()

FLR_RESULT tfSetWLut(const FLR_TF_WLUT_T data){
    FLR_RESULT returncode = CLIENT_pkgTfSetWLut(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetWLut()

FLR_RESULT tfGetWLut(FLR_TF_WLUT_T *data){
    FLR_RESULT returncode = CLIENT_pkgTfGetWLut(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetWLut()

FLR_RESULT tfGetMotionCount(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgTfGetMotionCount(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMotionCount()

FLR_RESULT tfSetMotionThreshold(const uint32_t data){
    FLR_RESULT returncode = CLIENT_pkgTfSetMotionThreshold(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetMotionThreshold()

FLR_RESULT tfGetMotionThreshold(uint32_t *data){
    FLR_RESULT returncode = CLIENT_pkgTfGetMotionThreshold(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetMotionThreshold()

FLR_RESULT tfGetDelta_nfApplied(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgTfGetDelta_nfApplied(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetDelta_nfApplied()

FLR_RESULT tfGetTHDeltaMotionApplied(uint16_t *data){
    FLR_RESULT returncode = CLIENT_pkgTfGetTHDeltaMotionApplied(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTHDeltaMotionApplied()

FLR_RESULT tfSetTempSignalCompFactorLut(const FLR_TF_TEMP_SIGNAL_COMP_FACTOR_LUT_T data){
    FLR_RESULT returncode = CLIENT_pkgTfSetTempSignalCompFactorLut(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetTempSignalCompFactorLut()

FLR_RESULT tfGetTempSignalCompFactorLut(FLR_TF_TEMP_SIGNAL_COMP_FACTOR_LUT_T *data){
    FLR_RESULT returncode = CLIENT_pkgTfGetTempSignalCompFactorLut(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetTempSignalCompFactorLut()

FLR_RESULT tfGetRnf(uint16_t *rnf){
    FLR_RESULT returncode = CLIENT_pkgTfGetRnf(rnf);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetRnf()

FLR_RESULT uartSetStartupBaudRate(const FLR_UART_STARTUP_BAUDRATE_E data){
    FLR_RESULT returncode = CLIENT_pkgUartSetStartupBaudRate(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of SetStartupBaudRate()

FLR_RESULT uartGetStartupBaudRate(FLR_UART_STARTUP_BAUDRATE_E *data){
    FLR_RESULT returncode = CLIENT_pkgUartGetStartupBaudRate(data);
    // Check for any errorcode
    if((uint32_t) returncode){
        return returncode;
    }
    return R_SUCCESS;
} // End of GetStartupBaudRate()

