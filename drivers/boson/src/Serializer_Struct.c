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


#include <stdint.h>
#include "Serializer_Struct.h"

void byteToFLR_ROI_T(const uint8_t *inBuff, FLR_ROI_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=8) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16((const uint8_t *)ptr,&(outVal->rowStart));
    ptr += 2;
    
    if(ptr-inBuff>=8) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16((const uint8_t *)ptr,&(outVal->rowStop));
    ptr += 2;
    
    if(ptr-inBuff>=8) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16((const uint8_t *)ptr,&(outVal->colStart));
    ptr += 2;
    
    if(ptr-inBuff>=8) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16((const uint8_t *)ptr,&(outVal->colStop));
    ptr += 2;
    
} //end byteToFLR_ROI_T()
void FLR_ROI_TToByte(const FLR_ROI_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=8) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ToByte((const uint16_t) (inVal->rowStart),(const uint8_t *)ptr);
    ptr += 2;
    
    if((ptr-outBuff)>=8) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ToByte((const uint16_t) (inVal->rowStop),(const uint8_t *)ptr);
    ptr += 2;
    
    if((ptr-outBuff)>=8) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ToByte((const uint16_t) (inVal->colStart),(const uint8_t *)ptr);
    ptr += 2;
    
    if((ptr-outBuff)>=8) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ToByte((const uint16_t) (inVal->colStop),(const uint8_t *)ptr);
    ptr += 2;
    
} //end FLR_ROI_TToByte()
void byteToFLR_BOSON_PARTNUMBER_T(const uint8_t *inBuff, FLR_BOSON_PARTNUMBER_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=20) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUCHARArray((const uint8_t *)ptr,(outVal->value),(uint16_t) 20);
    ptr += 20;
    
} //end byteToFLR_BOSON_PARTNUMBER_T()
void FLR_BOSON_PARTNUMBER_TToByte(const FLR_BOSON_PARTNUMBER_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=20) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UCHARArrayToByte((const uint8_t *)(inVal->value), (uint16_t) 20,(const uint8_t *)ptr);
    ptr += 20;
    
} //end FLR_BOSON_PARTNUMBER_TToByte()
void byteToFLR_BOSON_SENSOR_PARTNUMBER_T(const uint8_t *inBuff, FLR_BOSON_SENSOR_PARTNUMBER_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=32) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUCHARArray((const uint8_t *)ptr,(outVal->value),(uint16_t) 32);
    ptr += 32;
    
} //end byteToFLR_BOSON_SENSOR_PARTNUMBER_T()
void FLR_BOSON_SENSOR_PARTNUMBER_TToByte(const FLR_BOSON_SENSOR_PARTNUMBER_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=32) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UCHARArrayToByte((const uint8_t *)(inVal->value), (uint16_t) 32,(const uint8_t *)ptr);
    ptr += 32;
    
} //end FLR_BOSON_SENSOR_PARTNUMBER_TToByte()
void byteToFLR_BOSON_GAIN_SWITCH_PARAMS_T(const uint8_t *inBuff, FLR_BOSON_GAIN_SWITCH_PARAMS_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->pHighToLowPercent));
    ptr += 4;
    
    if(ptr-inBuff>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->cHighToLowPercent));
    ptr += 4;
    
    if(ptr-inBuff>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->pLowToHighPercent));
    ptr += 4;
    
    if(ptr-inBuff>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->hysteresisPercent));
    ptr += 4;
    
} //end byteToFLR_BOSON_GAIN_SWITCH_PARAMS_T()
void FLR_BOSON_GAIN_SWITCH_PARAMS_TToByte(const FLR_BOSON_GAIN_SWITCH_PARAMS_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->pHighToLowPercent),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->cHighToLowPercent),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->pLowToHighPercent),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->hysteresisPercent),(const uint8_t *)ptr);
    ptr += 4;
    
} //end FLR_BOSON_GAIN_SWITCH_PARAMS_TToByte()
void byteToFLR_BOSON_GAIN_SWITCH_RADIOMETRIC_PARAMS_T(const uint8_t *inBuff, FLR_BOSON_GAIN_SWITCH_RADIOMETRIC_PARAMS_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->pHighToLowPercent));
    ptr += 4;
    
    if(ptr-inBuff>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLOAT((const uint8_t *)ptr,&(outVal->TempHighToLowDegK));
    ptr += 4;
    
    if(ptr-inBuff>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->pLowToHighPercent));
    ptr += 4;
    
    if(ptr-inBuff>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLOAT((const uint8_t *)ptr,&(outVal->TempLowToHighDegK));
    ptr += 4;
    
} //end byteToFLR_BOSON_GAIN_SWITCH_RADIOMETRIC_PARAMS_T()
void FLR_BOSON_GAIN_SWITCH_RADIOMETRIC_PARAMS_TToByte(const FLR_BOSON_GAIN_SWITCH_RADIOMETRIC_PARAMS_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->pHighToLowPercent),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLOATToByte((const float) (inVal->TempHighToLowDegK),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->pLowToHighPercent),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLOATToByte((const float) (inVal->TempLowToHighDegK),(const uint8_t *)ptr);
    ptr += 4;
    
} //end FLR_BOSON_GAIN_SWITCH_RADIOMETRIC_PARAMS_TToByte()
void byteToFLR_BOSON_SATURATION_LUT_T(const uint8_t *inBuff, FLR_BOSON_SATURATION_LUT_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=34) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16Array((const uint8_t *)ptr,(outVal->value),(uint16_t) 17);
    ptr += 34;
    
} //end byteToFLR_BOSON_SATURATION_LUT_T()
void FLR_BOSON_SATURATION_LUT_TToByte(const FLR_BOSON_SATURATION_LUT_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=34) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ArrayToByte((const uint16_t *)(inVal->value), (uint16_t) 17,(const uint8_t *)ptr);
    ptr += 34;
    
} //end FLR_BOSON_SATURATION_LUT_TToByte()
void byteToFLR_BOSON_SATURATION_HEADER_LUT_T(const uint8_t *inBuff, FLR_BOSON_SATURATION_HEADER_LUT_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=36) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_BOSON_SATURATION_LUT_T((const uint8_t *)ptr,&(outVal->lut));
    ptr += 34;
    
    if(ptr-inBuff>=36) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16((const uint8_t *)ptr,&(outVal->tableIndex));
    ptr += 2;
    
} //end byteToFLR_BOSON_SATURATION_HEADER_LUT_T()
void FLR_BOSON_SATURATION_HEADER_LUT_TToByte(const FLR_BOSON_SATURATION_HEADER_LUT_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=36) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_BOSON_SATURATION_LUT_TToByte(&(inVal->lut),(const uint8_t *)ptr);
    ptr += 34;
    
    if((ptr-outBuff)>=36) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ToByte((const uint16_t) (inVal->tableIndex),(const uint8_t *)ptr);
    ptr += 2;
    
} //end FLR_BOSON_SATURATION_HEADER_LUT_TToByte()
void byteToFLR_CAPTURE_SETTINGS_T(const uint8_t *inBuff, FLR_CAPTURE_SETTINGS_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=10) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_32((const uint8_t *)ptr,(int32_t*)&(outVal->dataSrc));
    ptr += 4;
    
    if(ptr-inBuff>=10) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->numFrames));
    ptr += 4;
    
    if(ptr-inBuff>=10) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16((const uint8_t *)ptr,&(outVal->bufferIndex));
    ptr += 2;
    
} //end byteToFLR_CAPTURE_SETTINGS_T()
void FLR_CAPTURE_SETTINGS_TToByte(const FLR_CAPTURE_SETTINGS_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=10) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_32ToByte((const int32_t) (inVal->dataSrc), (const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=10) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->numFrames),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=10) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ToByte((const uint16_t) (inVal->bufferIndex),(const uint8_t *)ptr);
    ptr += 2;
    
} //end FLR_CAPTURE_SETTINGS_TToByte()
void byteToFLR_CAPTURE_FILE_SETTINGS_T(const uint8_t *inBuff, FLR_CAPTURE_FILE_SETTINGS_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=132) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_32((const uint8_t *)ptr,(int32_t*)&(outVal->captureFileType));
    ptr += 4;
    
    if(ptr-inBuff>=132) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUCHARArray((const uint8_t *)ptr,(outVal->filePath),(uint16_t) 128);
    ptr += 128;
    
} //end byteToFLR_CAPTURE_FILE_SETTINGS_T()
void FLR_CAPTURE_FILE_SETTINGS_TToByte(const FLR_CAPTURE_FILE_SETTINGS_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=132) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_32ToByte((const int32_t) (inVal->captureFileType), (const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=132) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UCHARArrayToByte((const uint8_t *)(inVal->filePath), (uint16_t) 128,(const uint8_t *)ptr);
    ptr += 128;
    
} //end FLR_CAPTURE_FILE_SETTINGS_TToByte()
void byteToFLR_CAPTURE_STATUS_T(const uint8_t *inBuff, FLR_CAPTURE_STATUS_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=24) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_32((const uint8_t *)ptr,(int32_t*)&(outVal->state));
    ptr += 4;
    
    if(ptr-inBuff>=24) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->result));
    ptr += 4;
    
    if(ptr-inBuff>=24) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->capturedFrames));
    ptr += 4;
    
    if(ptr-inBuff>=24) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->missedFrames));
    ptr += 4;
    
    if(ptr-inBuff>=24) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->savedFrames));
    ptr += 4;
    
    if(ptr-inBuff>=24) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->unsyncFrames));
    ptr += 4;
    
} //end byteToFLR_CAPTURE_STATUS_T()
void FLR_CAPTURE_STATUS_TToByte(const FLR_CAPTURE_STATUS_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=24) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_32ToByte((const int32_t) (inVal->state), (const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=24) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->result),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=24) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->capturedFrames),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=24) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->missedFrames),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=24) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->savedFrames),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=24) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->unsyncFrames),(const uint8_t *)ptr);
    ptr += 4;
    
} //end FLR_CAPTURE_STATUS_TToByte()
void byteToFLR_DVO_YCBCR_SETTINGS_T(const uint8_t *inBuff, FLR_DVO_YCBCR_SETTINGS_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=12) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_32((const uint8_t *)ptr,(int32_t*)&(outVal->ycbcrFormat));
    ptr += 4;
    
    if(ptr-inBuff>=12) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_32((const uint8_t *)ptr,(int32_t*)&(outVal->cbcrOrder));
    ptr += 4;
    
    if(ptr-inBuff>=12) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_32((const uint8_t *)ptr,(int32_t*)&(outVal->yOrder));
    ptr += 4;
    
} //end byteToFLR_DVO_YCBCR_SETTINGS_T()
void FLR_DVO_YCBCR_SETTINGS_TToByte(const FLR_DVO_YCBCR_SETTINGS_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=12) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_32ToByte((const int32_t) (inVal->ycbcrFormat), (const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=12) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_32ToByte((const int32_t) (inVal->cbcrOrder), (const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=12) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_32ToByte((const int32_t) (inVal->yOrder), (const uint8_t *)ptr);
    ptr += 4;
    
} //end FLR_DVO_YCBCR_SETTINGS_TToByte()
void byteToFLR_DVO_RGB_SETTINGS_T(const uint8_t *inBuff, FLR_DVO_RGB_SETTINGS_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=8) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_32((const uint8_t *)ptr,(int32_t*)&(outVal->rgbFormat));
    ptr += 4;
    
    if(ptr-inBuff>=8) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_32((const uint8_t *)ptr,(int32_t*)&(outVal->rgbOrder));
    ptr += 4;
    
} //end byteToFLR_DVO_RGB_SETTINGS_T()
void FLR_DVO_RGB_SETTINGS_TToByte(const FLR_DVO_RGB_SETTINGS_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=8) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_32ToByte((const int32_t) (inVal->rgbFormat), (const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=8) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_32ToByte((const int32_t) (inVal->rgbOrder), (const uint8_t *)ptr);
    ptr += 4;
    
} //end FLR_DVO_RGB_SETTINGS_TToByte()
void byteToFLR_DVO_LCD_CONFIG_T(const uint8_t *inBuff, FLR_DVO_LCD_CONFIG_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->width));
    ptr += 4;
    
    if(ptr-inBuff>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->hPulseWidth));
    ptr += 4;
    
    if(ptr-inBuff>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->hBackP));
    ptr += 4;
    
    if(ptr-inBuff>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->hFrontP));
    ptr += 4;
    
    if(ptr-inBuff>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->height));
    ptr += 4;
    
    if(ptr-inBuff>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->vPulseWidth));
    ptr += 4;
    
    if(ptr-inBuff>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->vBackP));
    ptr += 4;
    
    if(ptr-inBuff>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->vFrontP));
    ptr += 4;
    
    if(ptr-inBuff>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->outputFormat));
    ptr += 4;
    
    if(ptr-inBuff>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->control));
    ptr += 4;
    
    if(ptr-inBuff>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->rotation));
    ptr += 4;
    
    if(ptr-inBuff>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->pixelClockkHz));
    ptr += 4;
    
} //end byteToFLR_DVO_LCD_CONFIG_T()
void FLR_DVO_LCD_CONFIG_TToByte(const FLR_DVO_LCD_CONFIG_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->width),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->hPulseWidth),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->hBackP),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->hFrontP),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->height),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->vPulseWidth),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->vBackP),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->vFrontP),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->outputFormat),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->control),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->rotation),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=48) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->pixelClockkHz),(const uint8_t *)ptr);
    ptr += 4;
    
} //end FLR_DVO_LCD_CONFIG_TToByte()
void byteToFLR_GAO_RNS_COL_CORRECT_T(const uint8_t *inBuff, FLR_GAO_RNS_COL_CORRECT_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=40) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_16Array((const uint8_t *)ptr,(outVal->value),(uint16_t) 20);
    ptr += 40;
    
} //end byteToFLR_GAO_RNS_COL_CORRECT_T()
void FLR_GAO_RNS_COL_CORRECT_TToByte(const FLR_GAO_RNS_COL_CORRECT_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=40) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_16ArrayToByte((const int16_t *)(inVal->value), (uint16_t) 20,(const uint8_t *)ptr);
    ptr += 40;
    
} //end FLR_GAO_RNS_COL_CORRECT_TToByte()
void byteToFLR_ISOTHERM_COLOR_T(const uint8_t *inBuff, FLR_ISOTHERM_COLOR_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=6) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16((const uint8_t *)ptr,&(outVal->r));
    ptr += 2;
    
    if(ptr-inBuff>=6) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16((const uint8_t *)ptr,&(outVal->g));
    ptr += 2;
    
    if(ptr-inBuff>=6) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16((const uint8_t *)ptr,&(outVal->b));
    ptr += 2;
    
} //end byteToFLR_ISOTHERM_COLOR_T()
void FLR_ISOTHERM_COLOR_TToByte(const FLR_ISOTHERM_COLOR_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=6) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ToByte((const uint16_t) (inVal->r),(const uint8_t *)ptr);
    ptr += 2;
    
    if((ptr-outBuff)>=6) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ToByte((const uint16_t) (inVal->g),(const uint8_t *)ptr);
    ptr += 2;
    
    if((ptr-outBuff)>=6) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ToByte((const uint16_t) (inVal->b),(const uint8_t *)ptr);
    ptr += 2;
    
} //end FLR_ISOTHERM_COLOR_TToByte()
void byteToFLR_ISOTHERM_COLORS_T(const uint8_t *inBuff, FLR_ISOTHERM_COLORS_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=20) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_ISOTHERM_COLOR_T((const uint8_t *)ptr,&(outVal->range1));
    ptr += 6;
    
    if(ptr-inBuff>=20) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_ISOTHERM_COLOR_T((const uint8_t *)ptr,&(outVal->range2));
    ptr += 6;
    
    if(ptr-inBuff>=20) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_ISOTHERM_COLOR_T((const uint8_t *)ptr,&(outVal->range3));
    ptr += 6;
    
    if(ptr-inBuff>=20) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16((const uint8_t *)ptr,&(outVal->num));
    ptr += 2;
    
} //end byteToFLR_ISOTHERM_COLORS_T()
void FLR_ISOTHERM_COLORS_TToByte(const FLR_ISOTHERM_COLORS_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=20) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_ISOTHERM_COLOR_TToByte(&(inVal->range1),(const uint8_t *)ptr);
    ptr += 6;
    
    if((ptr-outBuff)>=20) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_ISOTHERM_COLOR_TToByte(&(inVal->range2),(const uint8_t *)ptr);
    ptr += 6;
    
    if((ptr-outBuff)>=20) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_ISOTHERM_COLOR_TToByte(&(inVal->range3),(const uint8_t *)ptr);
    ptr += 6;
    
    if((ptr-outBuff)>=20) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ToByte((const uint16_t) (inVal->num),(const uint8_t *)ptr);
    ptr += 2;
    
} //end FLR_ISOTHERM_COLORS_TToByte()
void byteToFLR_ISOTHERM_SETTINGS_T(const uint8_t *inBuff, FLR_ISOTHERM_SETTINGS_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_32((const uint8_t *)ptr,&(outVal->thIsoT1));
    ptr += 4;
    
    if(ptr-inBuff>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_32((const uint8_t *)ptr,&(outVal->thIsoT2));
    ptr += 4;
    
    if(ptr-inBuff>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_32((const uint8_t *)ptr,&(outVal->thIsoT3));
    ptr += 4;
    
    if(ptr-inBuff>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_32((const uint8_t *)ptr,&(outVal->thIsoT4));
    ptr += 4;
    
    if(ptr-inBuff>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_32((const uint8_t *)ptr,&(outVal->thIsoT5));
    ptr += 4;
    
    if(ptr-inBuff>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_ISOTHERM_COLORS_T((const uint8_t *)ptr,&(outVal->color0));
    ptr += 20;
    
    if(ptr-inBuff>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_ISOTHERM_COLORS_T((const uint8_t *)ptr,&(outVal->color1));
    ptr += 20;
    
    if(ptr-inBuff>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_ISOTHERM_COLORS_T((const uint8_t *)ptr,&(outVal->color2));
    ptr += 20;
    
    if(ptr-inBuff>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_ISOTHERM_COLORS_T((const uint8_t *)ptr,&(outVal->color3));
    ptr += 20;
    
    if(ptr-inBuff>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_ISOTHERM_COLORS_T((const uint8_t *)ptr,&(outVal->color4));
    ptr += 20;
    
    if(ptr-inBuff>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_ISOTHERM_COLORS_T((const uint8_t *)ptr,&(outVal->color5));
    ptr += 20;
    
    if(ptr-inBuff>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_32((const uint8_t *)ptr,(int32_t*)&(outVal->region0));
    ptr += 4;
    
    if(ptr-inBuff>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_32((const uint8_t *)ptr,(int32_t*)&(outVal->region1));
    ptr += 4;
    
    if(ptr-inBuff>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_32((const uint8_t *)ptr,(int32_t*)&(outVal->region2));
    ptr += 4;
    
    if(ptr-inBuff>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_32((const uint8_t *)ptr,(int32_t*)&(outVal->region3));
    ptr += 4;
    
    if(ptr-inBuff>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_32((const uint8_t *)ptr,(int32_t*)&(outVal->region4));
    ptr += 4;
    
    if(ptr-inBuff>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_32((const uint8_t *)ptr,(int32_t*)&(outVal->region5));
    ptr += 4;
    
} //end byteToFLR_ISOTHERM_SETTINGS_T()
void FLR_ISOTHERM_SETTINGS_TToByte(const FLR_ISOTHERM_SETTINGS_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_32ToByte((const int32_t) (inVal->thIsoT1),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_32ToByte((const int32_t) (inVal->thIsoT2),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_32ToByte((const int32_t) (inVal->thIsoT3),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_32ToByte((const int32_t) (inVal->thIsoT4),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_32ToByte((const int32_t) (inVal->thIsoT5),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_ISOTHERM_COLORS_TToByte(&(inVal->color0),(const uint8_t *)ptr);
    ptr += 20;
    
    if((ptr-outBuff)>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_ISOTHERM_COLORS_TToByte(&(inVal->color1),(const uint8_t *)ptr);
    ptr += 20;
    
    if((ptr-outBuff)>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_ISOTHERM_COLORS_TToByte(&(inVal->color2),(const uint8_t *)ptr);
    ptr += 20;
    
    if((ptr-outBuff)>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_ISOTHERM_COLORS_TToByte(&(inVal->color3),(const uint8_t *)ptr);
    ptr += 20;
    
    if((ptr-outBuff)>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_ISOTHERM_COLORS_TToByte(&(inVal->color4),(const uint8_t *)ptr);
    ptr += 20;
    
    if((ptr-outBuff)>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_ISOTHERM_COLORS_TToByte(&(inVal->color5),(const uint8_t *)ptr);
    ptr += 20;
    
    if((ptr-outBuff)>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_32ToByte((const int32_t) (inVal->region0), (const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_32ToByte((const int32_t) (inVal->region1), (const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_32ToByte((const int32_t) (inVal->region2), (const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_32ToByte((const int32_t) (inVal->region3), (const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_32ToByte((const int32_t) (inVal->region4), (const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=164) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_32ToByte((const int32_t) (inVal->region5), (const uint8_t *)ptr);
    ptr += 4;
    
} //end FLR_ISOTHERM_SETTINGS_TToByte()
void byteToFLR_RADIOMETRY_SIGNAL_COMP_FACTOR_LUT_T(const uint8_t *inBuff, FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_LUT_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=34) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16Array((const uint8_t *)ptr,(outVal->value),(uint16_t) 17);
    ptr += 34;
    
} //end byteToFLR_RADIOMETRY_SIGNAL_COMP_FACTOR_LUT_T()
void FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_LUT_TToByte(const FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_LUT_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=34) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ArrayToByte((const uint16_t *)(inVal->value), (uint16_t) 17,(const uint8_t *)ptr);
    ptr += 34;
    
} //end FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_LUT_TToByte()
void byteToFLR_RADIOMETRY_NOISE_COMP_FACTOR_LUT_T(const uint8_t *inBuff, FLR_RADIOMETRY_NOISE_COMP_FACTOR_LUT_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=34) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16Array((const uint8_t *)ptr,(outVal->value),(uint16_t) 17);
    ptr += 34;
    
} //end byteToFLR_RADIOMETRY_NOISE_COMP_FACTOR_LUT_T()
void FLR_RADIOMETRY_NOISE_COMP_FACTOR_LUT_TToByte(const FLR_RADIOMETRY_NOISE_COMP_FACTOR_LUT_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=34) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ArrayToByte((const uint16_t *)(inVal->value), (uint16_t) 17,(const uint8_t *)ptr);
    ptr += 34;
    
} //end FLR_RADIOMETRY_NOISE_COMP_FACTOR_LUT_TToByte()
void byteToFLR_RADIOMETRY_SIGNAL_COMP_FACTOR_HEADER_LUT_T(const uint8_t *inBuff, FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_HEADER_LUT_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=36) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_RADIOMETRY_SIGNAL_COMP_FACTOR_LUT_T((const uint8_t *)ptr,&(outVal->lut));
    ptr += 34;
    
    if(ptr-inBuff>=36) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16((const uint8_t *)ptr,&(outVal->tableIndex));
    ptr += 2;
    
} //end byteToFLR_RADIOMETRY_SIGNAL_COMP_FACTOR_HEADER_LUT_T()
void FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_HEADER_LUT_TToByte(const FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_HEADER_LUT_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=36) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_LUT_TToByte(&(inVal->lut),(const uint8_t *)ptr);
    ptr += 34;
    
    if((ptr-outBuff)>=36) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ToByte((const uint16_t) (inVal->tableIndex),(const uint8_t *)ptr);
    ptr += 2;
    
} //end FLR_RADIOMETRY_SIGNAL_COMP_FACTOR_HEADER_LUT_TToByte()
void byteToFLR_RADIOMETRY_NOISE_COMP_FACTOR_HEADER_LUT_T(const uint8_t *inBuff, FLR_RADIOMETRY_NOISE_COMP_FACTOR_HEADER_LUT_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=36) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_RADIOMETRY_NOISE_COMP_FACTOR_LUT_T((const uint8_t *)ptr,&(outVal->lut));
    ptr += 34;
    
    if(ptr-inBuff>=36) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16((const uint8_t *)ptr,&(outVal->tableIndex));
    ptr += 2;
    
} //end byteToFLR_RADIOMETRY_NOISE_COMP_FACTOR_HEADER_LUT_T()
void FLR_RADIOMETRY_NOISE_COMP_FACTOR_HEADER_LUT_TToByte(const FLR_RADIOMETRY_NOISE_COMP_FACTOR_HEADER_LUT_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=36) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_RADIOMETRY_NOISE_COMP_FACTOR_LUT_TToByte(&(inVal->lut),(const uint8_t *)ptr);
    ptr += 34;
    
    if((ptr-outBuff)>=36) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ToByte((const uint16_t) (inVal->tableIndex),(const uint8_t *)ptr);
    ptr += 2;
    
} //end FLR_RADIOMETRY_NOISE_COMP_FACTOR_HEADER_LUT_TToByte()
void byteToFLR_RADIOMETRY_RBFO_PARAMS_T(const uint8_t *inBuff, FLR_RADIOMETRY_RBFO_PARAMS_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLOAT((const uint8_t *)ptr,&(outVal->RBFO_R));
    ptr += 4;
    
    if(ptr-inBuff>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLOAT((const uint8_t *)ptr,&(outVal->RBFO_B));
    ptr += 4;
    
    if(ptr-inBuff>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLOAT((const uint8_t *)ptr,&(outVal->RBFO_F));
    ptr += 4;
    
    if(ptr-inBuff>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLOAT((const uint8_t *)ptr,&(outVal->RBFO_O));
    ptr += 4;
    
} //end byteToFLR_RADIOMETRY_RBFO_PARAMS_T()
void FLR_RADIOMETRY_RBFO_PARAMS_TToByte(const FLR_RADIOMETRY_RBFO_PARAMS_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLOATToByte((const float) (inVal->RBFO_R),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLOATToByte((const float) (inVal->RBFO_B),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLOATToByte((const float) (inVal->RBFO_F),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLOATToByte((const float) (inVal->RBFO_O),(const uint8_t *)ptr);
    ptr += 4;
    
} //end FLR_RADIOMETRY_RBFO_PARAMS_TToByte()
void byteToFLR_RADIOMETRY_TAUX_PARAMS_T(const uint8_t *inBuff, FLR_RADIOMETRY_TAUX_PARAMS_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLOAT((const uint8_t *)ptr,&(outVal->A3));
    ptr += 4;
    
    if(ptr-inBuff>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLOAT((const uint8_t *)ptr,&(outVal->A2));
    ptr += 4;
    
    if(ptr-inBuff>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLOAT((const uint8_t *)ptr,&(outVal->A1));
    ptr += 4;
    
    if(ptr-inBuff>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLOAT((const uint8_t *)ptr,&(outVal->A0));
    ptr += 4;
    
} //end byteToFLR_RADIOMETRY_TAUX_PARAMS_T()
void FLR_RADIOMETRY_TAUX_PARAMS_TToByte(const FLR_RADIOMETRY_TAUX_PARAMS_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLOATToByte((const float) (inVal->A3),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLOATToByte((const float) (inVal->A2),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLOATToByte((const float) (inVal->A1),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=16) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLOATToByte((const float) (inVal->A0),(const uint8_t *)ptr);
    ptr += 4;
    
} //end FLR_RADIOMETRY_TAUX_PARAMS_TToByte()
void byteToFLR_ROIC_FPATEMP_TABLE_T(const uint8_t *inBuff, FLR_ROIC_FPATEMP_TABLE_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=64) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_16Array((const uint8_t *)ptr,(outVal->value),(uint16_t) 32);
    ptr += 64;
    
} //end byteToFLR_ROIC_FPATEMP_TABLE_T()
void FLR_ROIC_FPATEMP_TABLE_TToByte(const FLR_ROIC_FPATEMP_TABLE_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=64) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_16ArrayToByte((const int16_t *)(inVal->value), (uint16_t) 32,(const uint8_t *)ptr);
    ptr += 64;
    
} //end FLR_ROIC_FPATEMP_TABLE_TToByte()
void byteToFLR_SCALER_ZOOM_PARAMS_T(const uint8_t *inBuff, FLR_SCALER_ZOOM_PARAMS_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=12) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->zoom));
    ptr += 4;
    
    if(ptr-inBuff>=12) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->xCenter));
    ptr += 4;
    
    if(ptr-inBuff>=12) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->yCenter));
    ptr += 4;
    
} //end byteToFLR_SCALER_ZOOM_PARAMS_T()
void FLR_SCALER_ZOOM_PARAMS_TToByte(const FLR_SCALER_ZOOM_PARAMS_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=12) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->zoom),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=12) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->xCenter),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=12) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->yCenter),(const uint8_t *)ptr);
    ptr += 4;
    
} //end FLR_SCALER_ZOOM_PARAMS_TToByte()
void byteToFLR_SPNR_PSD_KERNEL_T(const uint8_t *inBuff, FLR_SPNR_PSD_KERNEL_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=256) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLOATArray((const uint8_t *)ptr,(outVal->fvalue),(uint16_t) 64);
    ptr += 256;
    
} //end byteToFLR_SPNR_PSD_KERNEL_T()
void FLR_SPNR_PSD_KERNEL_TToByte(const FLR_SPNR_PSD_KERNEL_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=256) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLOATArrayToByte((const float *)(inVal->fvalue), (uint16_t) 64,(const uint8_t *)ptr);
    ptr += 256;
    
} //end FLR_SPNR_PSD_KERNEL_TToByte()
void byteToFLR_SPOTMETER_SPOT_PARAM_T(const uint8_t *inBuff, FLR_SPOTMETER_SPOT_PARAM_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=6) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16((const uint8_t *)ptr,&(outVal->row));
    ptr += 2;
    
    if(ptr-inBuff>=6) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16((const uint8_t *)ptr,&(outVal->column));
    ptr += 2;
    
    if(ptr-inBuff>=6) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16((const uint8_t *)ptr,&(outVal->value));
    ptr += 2;
    
} //end byteToFLR_SPOTMETER_SPOT_PARAM_T()
void FLR_SPOTMETER_SPOT_PARAM_TToByte(const FLR_SPOTMETER_SPOT_PARAM_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=6) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ToByte((const uint16_t) (inVal->row),(const uint8_t *)ptr);
    ptr += 2;
    
    if((ptr-outBuff)>=6) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ToByte((const uint16_t) (inVal->column),(const uint8_t *)ptr);
    ptr += 2;
    
    if((ptr-outBuff)>=6) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ToByte((const uint16_t) (inVal->value),(const uint8_t *)ptr);
    ptr += 2;
    
} //end FLR_SPOTMETER_SPOT_PARAM_TToByte()
void byteToFLR_SPOTMETER_STAT_PARAM_TEMP_T(const uint8_t *inBuff, FLR_SPOTMETER_STAT_PARAM_TEMP_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=8) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16((const uint8_t *)ptr,&(outVal->row));
    ptr += 2;
    
    if(ptr-inBuff>=8) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16((const uint8_t *)ptr,&(outVal->column));
    ptr += 2;
    
    if(ptr-inBuff>=8) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLOAT((const uint8_t *)ptr,&(outVal->value));
    ptr += 4;
    
} //end byteToFLR_SPOTMETER_STAT_PARAM_TEMP_T()
void FLR_SPOTMETER_STAT_PARAM_TEMP_TToByte(const FLR_SPOTMETER_STAT_PARAM_TEMP_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=8) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ToByte((const uint16_t) (inVal->row),(const uint8_t *)ptr);
    ptr += 2;
    
    if((ptr-outBuff)>=8) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ToByte((const uint16_t) (inVal->column),(const uint8_t *)ptr);
    ptr += 2;
    
    if((ptr-outBuff)>=8) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLOATToByte((const float) (inVal->value),(const uint8_t *)ptr);
    ptr += 4;
    
} //end FLR_SPOTMETER_STAT_PARAM_TEMP_TToByte()
void byteToFLR_SYSINFO_MONITOR_BUILD_VARIANT_T(const uint8_t *inBuff, FLR_SYSINFO_MONITOR_BUILD_VARIANT_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=50) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUCHARArray((const uint8_t *)ptr,(outVal->value),(uint16_t) 50);
    ptr += 50;
    
} //end byteToFLR_SYSINFO_MONITOR_BUILD_VARIANT_T()
void FLR_SYSINFO_MONITOR_BUILD_VARIANT_TToByte(const FLR_SYSINFO_MONITOR_BUILD_VARIANT_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=50) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UCHARArrayToByte((const uint8_t *)(inVal->value), (uint16_t) 50,(const uint8_t *)ptr);
    ptr += 50;
    
} //end FLR_SYSINFO_MONITOR_BUILD_VARIANT_TToByte()
void byteToFLR_SYSINFO_PROBE_TIP_TYPE(const uint8_t *inBuff, FLR_SYSINFO_PROBE_TIP_TYPE *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=5) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_32((const uint8_t *)ptr,(int32_t*)&(outVal->model));
    ptr += 4;
    
    if(ptr-inBuff>=5) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUCHAR((const uint8_t *)ptr,&(outVal->hwRevision));
    ptr += 1;
    
} //end byteToFLR_SYSINFO_PROBE_TIP_TYPE()
void FLR_SYSINFO_PROBE_TIP_TYPEToByte(const FLR_SYSINFO_PROBE_TIP_TYPE *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=5) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_32ToByte((const int32_t) (inVal->model), (const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=5) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UCHARToByte((const uint8_t) (inVal->hwRevision),(const uint8_t *)ptr);
    ptr += 1;
    
} //end FLR_SYSINFO_PROBE_TIP_TYPEToByte()
void byteToFLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T(const uint8_t *inBuff, FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=15) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUCHAR((const uint8_t *)ptr,&(outVal->id));
    ptr += 1;
    
    if(ptr-inBuff>=15) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_16((const uint8_t *)ptr,&(outVal->x));
    ptr += 2;
    
    if(ptr-inBuff>=15) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_16((const uint8_t *)ptr,&(outVal->y));
    ptr += 2;
    
    if(ptr-inBuff>=15) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_16((const uint8_t *)ptr,&(outVal->width));
    ptr += 2;
    
    if(ptr-inBuff>=15) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_16((const uint8_t *)ptr,&(outVal->height));
    ptr += 2;
    
    if(ptr-inBuff>=15) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_32((const uint8_t *)ptr,&(outVal->color));
    ptr += 4;
    
    if(ptr-inBuff>=15) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_16((const uint8_t *)ptr,&(outVal->size));
    ptr += 2;
    
} //end byteToFLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T()
void FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_TToByte(const FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=15) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UCHARToByte((const uint8_t) (inVal->id),(const uint8_t *)ptr);
    ptr += 1;
    
    if((ptr-outBuff)>=15) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_16ToByte((const int16_t) (inVal->x),(const uint8_t *)ptr);
    ptr += 2;
    
    if((ptr-outBuff)>=15) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_16ToByte((const int16_t) (inVal->y),(const uint8_t *)ptr);
    ptr += 2;
    
    if((ptr-outBuff)>=15) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_16ToByte((const int16_t) (inVal->width),(const uint8_t *)ptr);
    ptr += 2;
    
    if((ptr-outBuff)>=15) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_16ToByte((const int16_t) (inVal->height),(const uint8_t *)ptr);
    ptr += 2;
    
    if((ptr-outBuff)>=15) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_32ToByte((const uint32_t) (inVal->color),(const uint8_t *)ptr);
    ptr += 4;
    
    if((ptr-outBuff)>=15) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_16ToByte((const int16_t) (inVal->size),(const uint8_t *)ptr);
    ptr += 2;
    
} //end FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_TToByte()
void byteToFLR_SYSTEMSYMBOLS_SPOTCONFIG_T(const uint8_t *inBuff, FLR_SYSTEMSYMBOLS_SPOTCONFIG_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T((const uint8_t *)ptr,&(outVal->symbol));
    ptr += 15;
    
    if(ptr-inBuff>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T((const uint8_t *)ptr,&(outVal->area));
    ptr += 15;
    
    if(ptr-inBuff>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T((const uint8_t *)ptr,&(outVal->min));
    ptr += 15;
    
    if(ptr-inBuff>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T((const uint8_t *)ptr,&(outVal->max));
    ptr += 15;
    
    if(ptr-inBuff>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T((const uint8_t *)ptr,&(outVal->mean));
    ptr += 15;
    
    if(ptr-inBuff>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T((const uint8_t *)ptr,&(outVal->meanBar));
    ptr += 15;
    
    if(ptr-inBuff>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T((const uint8_t *)ptr,&(outVal->greenBarOutline));
    ptr += 15;
    
    if(ptr-inBuff>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T((const uint8_t *)ptr,&(outVal->greenBar));
    ptr += 15;
    
    if(ptr-inBuff>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T((const uint8_t *)ptr,&(outVal->greenBarText1));
    ptr += 15;
    
    if(ptr-inBuff>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T((const uint8_t *)ptr,&(outVal->greenBarText2));
    ptr += 15;
    
    if(ptr-inBuff>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T((const uint8_t *)ptr,&(outVal->greenBarText3));
    ptr += 15;
    
    if(ptr-inBuff>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T((const uint8_t *)ptr,&(outVal->greenBarText4));
    ptr += 15;
    
    if(ptr-inBuff>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T((const uint8_t *)ptr,&(outVal->greenBarText5));
    ptr += 15;
    
} //end byteToFLR_SYSTEMSYMBOLS_SPOTCONFIG_T()
void FLR_SYSTEMSYMBOLS_SPOTCONFIG_TToByte(const FLR_SYSTEMSYMBOLS_SPOTCONFIG_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_TToByte(&(inVal->symbol),(const uint8_t *)ptr);
    ptr += 15;
    
    if((ptr-outBuff)>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_TToByte(&(inVal->area),(const uint8_t *)ptr);
    ptr += 15;
    
    if((ptr-outBuff)>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_TToByte(&(inVal->min),(const uint8_t *)ptr);
    ptr += 15;
    
    if((ptr-outBuff)>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_TToByte(&(inVal->max),(const uint8_t *)ptr);
    ptr += 15;
    
    if((ptr-outBuff)>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_TToByte(&(inVal->mean),(const uint8_t *)ptr);
    ptr += 15;
    
    if((ptr-outBuff)>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_TToByte(&(inVal->meanBar),(const uint8_t *)ptr);
    ptr += 15;
    
    if((ptr-outBuff)>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_TToByte(&(inVal->greenBarOutline),(const uint8_t *)ptr);
    ptr += 15;
    
    if((ptr-outBuff)>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_TToByte(&(inVal->greenBar),(const uint8_t *)ptr);
    ptr += 15;
    
    if((ptr-outBuff)>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_TToByte(&(inVal->greenBarText1),(const uint8_t *)ptr);
    ptr += 15;
    
    if((ptr-outBuff)>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_TToByte(&(inVal->greenBarText2),(const uint8_t *)ptr);
    ptr += 15;
    
    if((ptr-outBuff)>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_TToByte(&(inVal->greenBarText3),(const uint8_t *)ptr);
    ptr += 15;
    
    if((ptr-outBuff)>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_TToByte(&(inVal->greenBarText4),(const uint8_t *)ptr);
    ptr += 15;
    
    if((ptr-outBuff)>=195) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_TToByte(&(inVal->greenBarText5),(const uint8_t *)ptr);
    ptr += 15;
    
} //end FLR_SYSTEMSYMBOLS_SPOTCONFIG_TToByte()
void byteToFLR_SYSTEMSYMBOLS_SPOTCONFIG_ID_T(const uint8_t *inBuff, FLR_SYSTEMSYMBOLS_SPOTCONFIG_ID_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUCHAR((const uint8_t *)ptr,&(outVal->symbol));
    ptr += 1;
    
    if(ptr-inBuff>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUCHAR((const uint8_t *)ptr,&(outVal->area));
    ptr += 1;
    
    if(ptr-inBuff>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUCHAR((const uint8_t *)ptr,&(outVal->min));
    ptr += 1;
    
    if(ptr-inBuff>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUCHAR((const uint8_t *)ptr,&(outVal->max));
    ptr += 1;
    
    if(ptr-inBuff>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUCHAR((const uint8_t *)ptr,&(outVal->mean));
    ptr += 1;
    
    if(ptr-inBuff>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUCHAR((const uint8_t *)ptr,&(outVal->meanBar));
    ptr += 1;
    
    if(ptr-inBuff>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUCHAR((const uint8_t *)ptr,&(outVal->greenBarOutline));
    ptr += 1;
    
    if(ptr-inBuff>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUCHAR((const uint8_t *)ptr,&(outVal->greenBar));
    ptr += 1;
    
    if(ptr-inBuff>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUCHAR((const uint8_t *)ptr,&(outVal->greenBarText1));
    ptr += 1;
    
    if(ptr-inBuff>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUCHAR((const uint8_t *)ptr,&(outVal->greenBarText2));
    ptr += 1;
    
    if(ptr-inBuff>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUCHAR((const uint8_t *)ptr,&(outVal->greenBarText3));
    ptr += 1;
    
    if(ptr-inBuff>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUCHAR((const uint8_t *)ptr,&(outVal->greenBarText4));
    ptr += 1;
    
    if(ptr-inBuff>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUCHAR((const uint8_t *)ptr,&(outVal->greenBarText5));
    ptr += 1;
    
} //end byteToFLR_SYSTEMSYMBOLS_SPOTCONFIG_ID_T()
void FLR_SYSTEMSYMBOLS_SPOTCONFIG_ID_TToByte(const FLR_SYSTEMSYMBOLS_SPOTCONFIG_ID_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UCHARToByte((const uint8_t) (inVal->symbol),(const uint8_t *)ptr);
    ptr += 1;
    
    if((ptr-outBuff)>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UCHARToByte((const uint8_t) (inVal->area),(const uint8_t *)ptr);
    ptr += 1;
    
    if((ptr-outBuff)>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UCHARToByte((const uint8_t) (inVal->min),(const uint8_t *)ptr);
    ptr += 1;
    
    if((ptr-outBuff)>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UCHARToByte((const uint8_t) (inVal->max),(const uint8_t *)ptr);
    ptr += 1;
    
    if((ptr-outBuff)>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UCHARToByte((const uint8_t) (inVal->mean),(const uint8_t *)ptr);
    ptr += 1;
    
    if((ptr-outBuff)>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UCHARToByte((const uint8_t) (inVal->meanBar),(const uint8_t *)ptr);
    ptr += 1;
    
    if((ptr-outBuff)>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UCHARToByte((const uint8_t) (inVal->greenBarOutline),(const uint8_t *)ptr);
    ptr += 1;
    
    if((ptr-outBuff)>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UCHARToByte((const uint8_t) (inVal->greenBar),(const uint8_t *)ptr);
    ptr += 1;
    
    if((ptr-outBuff)>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UCHARToByte((const uint8_t) (inVal->greenBarText1),(const uint8_t *)ptr);
    ptr += 1;
    
    if((ptr-outBuff)>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UCHARToByte((const uint8_t) (inVal->greenBarText2),(const uint8_t *)ptr);
    ptr += 1;
    
    if((ptr-outBuff)>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UCHARToByte((const uint8_t) (inVal->greenBarText3),(const uint8_t *)ptr);
    ptr += 1;
    
    if((ptr-outBuff)>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UCHARToByte((const uint8_t) (inVal->greenBarText4),(const uint8_t *)ptr);
    ptr += 1;
    
    if((ptr-outBuff)>=13) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UCHARToByte((const uint8_t) (inVal->greenBarText5),(const uint8_t *)ptr);
    ptr += 1;
    
} //end FLR_SYSTEMSYMBOLS_SPOTCONFIG_ID_TToByte()
void byteToFLR_SYSTEMSYMBOLS_ISOCONFIG_T(const uint8_t *inBuff, FLR_SYSTEMSYMBOLS_ISOCONFIG_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=30) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T((const uint8_t *)ptr,&(outVal->colorBar));
    ptr += 15;
    
    if(ptr-inBuff>=30) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToFLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_T((const uint8_t *)ptr,&(outVal->colorBarOutline));
    ptr += 15;
    
} //end byteToFLR_SYSTEMSYMBOLS_ISOCONFIG_T()
void FLR_SYSTEMSYMBOLS_ISOCONFIG_TToByte(const FLR_SYSTEMSYMBOLS_ISOCONFIG_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=30) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_TToByte(&(inVal->colorBar),(const uint8_t *)ptr);
    ptr += 15;
    
    if((ptr-outBuff)>=30) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    FLR_SYSTEMSYMBOLS_SPOT_ISO_ENTRY_TToByte(&(inVal->colorBarOutline),(const uint8_t *)ptr);
    ptr += 15;
    
} //end FLR_SYSTEMSYMBOLS_ISOCONFIG_TToByte()
void byteToFLR_SYSTEMSYMBOLS_ISOCONFIG_ID_T(const uint8_t *inBuff, FLR_SYSTEMSYMBOLS_ISOCONFIG_ID_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=2) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUCHAR((const uint8_t *)ptr,&(outVal->colorBar));
    ptr += 1;
    
    if(ptr-inBuff>=2) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUCHAR((const uint8_t *)ptr,&(outVal->colorBarOutline));
    ptr += 1;
    
} //end byteToFLR_SYSTEMSYMBOLS_ISOCONFIG_ID_T()
void FLR_SYSTEMSYMBOLS_ISOCONFIG_ID_TToByte(const FLR_SYSTEMSYMBOLS_ISOCONFIG_ID_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=2) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UCHARToByte((const uint8_t) (inVal->colorBar),(const uint8_t *)ptr);
    ptr += 1;
    
    if((ptr-outBuff)>=2) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UCHARToByte((const uint8_t) (inVal->colorBarOutline),(const uint8_t *)ptr);
    ptr += 1;
    
} //end FLR_SYSTEMSYMBOLS_ISOCONFIG_ID_TToByte()
void byteToFLR_SYSTEMSYMBOLS_BARCONFIG_T(const uint8_t *inBuff, FLR_SYSTEMSYMBOLS_BARCONFIG_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=10) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_16((const uint8_t *)ptr,&(outVal->val0));
    ptr += 2;
    
    if(ptr-inBuff>=10) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_16((const uint8_t *)ptr,&(outVal->val1));
    ptr += 2;
    
    if(ptr-inBuff>=10) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_16((const uint8_t *)ptr,&(outVal->val2));
    ptr += 2;
    
    if(ptr-inBuff>=10) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_16((const uint8_t *)ptr,&(outVal->val3));
    ptr += 2;
    
    if(ptr-inBuff>=10) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_16((const uint8_t *)ptr,&(outVal->val4));
    ptr += 2;
    
} //end byteToFLR_SYSTEMSYMBOLS_BARCONFIG_T()
void FLR_SYSTEMSYMBOLS_BARCONFIG_TToByte(const FLR_SYSTEMSYMBOLS_BARCONFIG_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=10) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_16ToByte((const int16_t) (inVal->val0),(const uint8_t *)ptr);
    ptr += 2;
    
    if((ptr-outBuff)>=10) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_16ToByte((const int16_t) (inVal->val1),(const uint8_t *)ptr);
    ptr += 2;
    
    if((ptr-outBuff)>=10) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_16ToByte((const int16_t) (inVal->val2),(const uint8_t *)ptr);
    ptr += 2;
    
    if((ptr-outBuff)>=10) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_16ToByte((const int16_t) (inVal->val3),(const uint8_t *)ptr);
    ptr += 2;
    
    if((ptr-outBuff)>=10) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_16ToByte((const int16_t) (inVal->val4),(const uint8_t *)ptr);
    ptr += 2;
    
} //end FLR_SYSTEMSYMBOLS_BARCONFIG_TToByte()
void byteToFLR_TESTRAMP_SETTINGS_T(const uint8_t *inBuff, FLR_TESTRAMP_SETTINGS_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=6) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16((const uint8_t *)ptr,&(outVal->start));
    ptr += 2;
    
    if(ptr-inBuff>=6) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16((const uint8_t *)ptr,&(outVal->end));
    ptr += 2;
    
    if(ptr-inBuff>=6) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16((const uint8_t *)ptr,&(outVal->increment));
    ptr += 2;
    
} //end byteToFLR_TESTRAMP_SETTINGS_T()
void FLR_TESTRAMP_SETTINGS_TToByte(const FLR_TESTRAMP_SETTINGS_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=6) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ToByte((const uint16_t) (inVal->start),(const uint8_t *)ptr);
    ptr += 2;
    
    if((ptr-outBuff)>=6) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ToByte((const uint16_t) (inVal->end),(const uint8_t *)ptr);
    ptr += 2;
    
    if((ptr-outBuff)>=6) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ToByte((const uint16_t) (inVal->increment),(const uint8_t *)ptr);
    ptr += 2;
    
} //end FLR_TESTRAMP_SETTINGS_TToByte()
void byteToFLR_TESTRAMP_ANIMATION_SETTINGS_T(const uint8_t *inBuff, FLR_TESTRAMP_ANIMATION_SETTINGS_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=4) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToINT_16((const uint8_t *)ptr,&(outVal->moveLines));
    ptr += 2;
    
    if(ptr-inBuff>=4) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16((const uint8_t *)ptr,&(outVal->moveFrames));
    ptr += 2;
    
} //end byteToFLR_TESTRAMP_ANIMATION_SETTINGS_T()
void FLR_TESTRAMP_ANIMATION_SETTINGS_TToByte(const FLR_TESTRAMP_ANIMATION_SETTINGS_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=4) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    INT_16ToByte((const int16_t) (inVal->moveLines),(const uint8_t *)ptr);
    ptr += 2;
    
    if((ptr-outBuff)>=4) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ToByte((const uint16_t) (inVal->moveFrames),(const uint8_t *)ptr);
    ptr += 2;
    
} //end FLR_TESTRAMP_ANIMATION_SETTINGS_TToByte()
void byteToFLR_TF_WLUT_T(const uint8_t *inBuff, FLR_TF_WLUT_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=32) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUCHARArray((const uint8_t *)ptr,(outVal->value),(uint16_t) 32);
    ptr += 32;
    
} //end byteToFLR_TF_WLUT_T()
void FLR_TF_WLUT_TToByte(const FLR_TF_WLUT_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=32) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UCHARArrayToByte((const uint8_t *)(inVal->value), (uint16_t) 32,(const uint8_t *)ptr);
    ptr += 32;
    
} //end FLR_TF_WLUT_TToByte()
void byteToFLR_TF_NF_LUT_T(const uint8_t *inBuff, FLR_TF_NF_LUT_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=34) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16Array((const uint8_t *)ptr,(outVal->value),(uint16_t) 17);
    ptr += 34;
    
} //end byteToFLR_TF_NF_LUT_T()
void FLR_TF_NF_LUT_TToByte(const FLR_TF_NF_LUT_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=34) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ArrayToByte((const uint16_t *)(inVal->value), (uint16_t) 17,(const uint8_t *)ptr);
    ptr += 34;
    
} //end FLR_TF_NF_LUT_TToByte()
void byteToFLR_TF_TEMP_SIGNAL_COMP_FACTOR_LUT_T(const uint8_t *inBuff, FLR_TF_TEMP_SIGNAL_COMP_FACTOR_LUT_T *outVal) {
    
    uint8_t *ptr = (uint8_t *)inBuff;
    if(ptr-inBuff>=34) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    byteToUINT_16Array((const uint8_t *)ptr,(outVal->value),(uint16_t) 17);
    ptr += 34;
    
} //end byteToFLR_TF_TEMP_SIGNAL_COMP_FACTOR_LUT_T()
void FLR_TF_TEMP_SIGNAL_COMP_FACTOR_LUT_TToByte(const FLR_TF_TEMP_SIGNAL_COMP_FACTOR_LUT_T *inVal, const uint8_t *outBuff) {
    
    uint8_t *ptr = (uint8_t *)outBuff;
    if((ptr-outBuff)>=34) {
        return;// R_CAM_PKG_BUFFER_OVERFLOW;
    }
    UINT_16ArrayToByte((const uint16_t *)(inVal->value), (uint16_t) 17,(const uint8_t *)ptr);
    ptr += 34;
    
} //end FLR_TF_TEMP_SIGNAL_COMP_FACTOR_LUT_TToByte()
