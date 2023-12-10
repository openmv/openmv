/**
 * @copyright (C) 2017 Melexis N.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "omv_i2c.h"
#include <MLX90641_I2C_Driver.h>
#include <MLX90641_API.h>
#include <math.h>
#define fabs(x) fabsf(x)
#define pow(a,b) powf(a,b)
#define sqrt(x) sqrtf(x)

static void ExtractVDDParameters(uint16_t *eeData, paramsMLX90641 *mlx90641);
static void ExtractPTATParameters(uint16_t *eeData, paramsMLX90641 *mlx90641);
static void ExtractGainParameters(uint16_t *eeData, paramsMLX90641 *mlx90641);
static void ExtractTgcParameters(uint16_t *eeData, paramsMLX90641 *mlx90641);
static void ExtractEmissivityParameters(uint16_t *eeData, paramsMLX90641 *mlx90641);
static void ExtractResolutionParameters(uint16_t *eeData, paramsMLX90641 *mlx90641);
static void ExtractKsTaParameters(uint16_t *eeData, paramsMLX90641 *mlx90641);
static void ExtractKsToParameters(uint16_t *eeData, paramsMLX90641 *mlx90641);
static void ExtractAlphaParameters(uint16_t *eeData, paramsMLX90641 *mlx90641);
static void ExtractOffsetParameters(uint16_t *eeData, paramsMLX90641 *mlx90641);
static void ExtractKtaPixelParameters(uint16_t *eeData, paramsMLX90641 *mlx90641);
static void ExtractKvPixelParameters(uint16_t *eeData, paramsMLX90641 *mlx90641);
static void ExtractCPParameters(uint16_t *eeData, paramsMLX90641 *mlx90641);
static int ExtractDeviatingPixels(uint16_t *eeData, paramsMLX90641 *mlx90641);
static int CheckEEPROMValid(uint16_t *eeData); 
static int HammingDecode(uint16_t *eeData);  
static int ValidateFrameData(uint16_t *frameData);
static int ValidateAuxData(uint16_t *auxData);

//------------------------------------------------------------------------------
  
int MLX90641_DumpEE(uint8_t slaveAddr, uint16_t *eeData)
{
     int error = 1;
     error = MLX90641_I2CRead(slaveAddr, 0x2400, 832, eeData);
     if (error == 0)
     {
        error = HammingDecode(eeData);  
     }
         
     return error;
}

//------------------------------------------------------------------------------

int HammingDecode(uint16_t *eeData)
{
    int error = 0;
    int16_t parity[5];
    int8_t D[16];
    int16_t check;
    uint16_t data;
    uint16_t mask;
    
    for (int addr=16; addr<832; addr++)
    {   
        parity[0] = -1;
        parity[1] = -1;
        parity[2] = -1;
        parity[3] = -1;
        parity[4] = -1;
        
        data = eeData[addr];
        
        mask = 1;
        for( int i = 0; i < 16; i++)
        {          
          D[i] = (data & mask) >> i;
          mask = mask << 1;
        }
        
        parity[0] = D[0]^D[1]^D[3]^D[4]^D[6]^D[8]^D[10]^D[11];
        parity[1] = D[0]^D[2]^D[3]^D[5]^D[6]^D[9]^D[10]^D[12];
        parity[2] = D[1]^D[2]^D[3]^D[7]^D[8]^D[9]^D[10]^D[13];
        parity[3] = D[4]^D[5]^D[6]^D[7]^D[8]^D[9]^D[10]^D[14];
        parity[4] = D[0]^D[1]^D[2]^D[3]^D[4]^D[5]^D[6]^D[7]^D[8]^D[9]^D[10]^D[11]^D[12]^D[13]^D[14]^D[15];
        
        
        if ((parity[0]!=0) || (parity[1]!=0) || (parity[2]!=0) || (parity[3]!=0) || (parity[4]!=0))
        {        
            check = (parity[0]<<0) + (parity[1]<<1) + (parity[2]<<2) + (parity[3]<<3) + (parity[4]<<4);
    
            if ((check > 15)&&(check < 32))
            {
                switch (check)
                {    
                    case 16:
                        D[15] = 1 - D[15];
                        break;
                    
                    case 24:
                        D[14] = 1 - D[14];
                        break;
                        
                    case 20:
                        D[13] = 1 - D[13];
                        break;
                        
                    case 18:
                        D[12] = 1 - D[12];
                        break;                                
                        
                    case 17:
                        D[11] = 1 - D[11];
                        break;
                        
                    case 31:
                        D[10] = 1 - D[10];
                        break;
                        
                    case 30:
                        D[9] = 1 - D[9];
                        break;
                    
                    case 29:
                        D[8] = 1 - D[8];
                        break;                
                    
                    case 28:
                        D[7] = 1 - D[7];
                        break;
                        
                    case 27:
                        D[6] = 1 - D[6];
                        break;
                            
                    case 26:
                        D[5] = 1 - D[5];
                        break;    
                        
                    case 25:
                        D[4] = 1 - D[4];
                        break;     
                        
                    case 23:
                        D[3] = 1 - D[3];
                        break; 
                        
                    case 22:
                        D[2] = 1 - D[2];
                        break; 
                            
                    case 21:
                        D[1] = 1 - D[1];
                        break; 
                        
                    case 19:
                        D[0] = 1 - D[0];
                        break;     
                                     
                }
               
                if(error == 0)
                {
                    error = -9;
                   
                }
                
                data = 0;
                mask = 1;
                for( int i = 0; i < 16; i++)
                {                    
                    data = data + D[i]*mask;
                    mask = mask << 1;
                }
       
            }
            else
            {
                error = -10;                
            }   
        }
        
        eeData[addr] = data & 0x07FF;
    }
    
    return error;
}

//------------------------------------------------------------------------------

int MLX90641_SynchFrame(uint8_t slaveAddr)
{
    uint16_t dataReady = 0;
    uint16_t statusRegister;
    int error = 1;
    
    error = MLX90641_I2CWrite(slaveAddr, 0x8000, 0x0030);
    if(error == -1)
    {
        return error;
    }
    
    while(dataReady == 0)
    {
        error = MLX90641_I2CRead(slaveAddr, 0x8000, 1, &statusRegister);
        if(error != 0)
        {
            return error;
        }    
        dataReady = statusRegister & 0x0008;
    }      
    
   return 0;   
}

//------------------------------------------------------------------------------

int MLX90641_TriggerMeasurement(uint8_t slaveAddr)
{
    int error = 1;
    uint16_t ctrlReg;
    
    error = MLX90641_I2CRead(slaveAddr, 0x800D, 1, &ctrlReg);
    
    if ( error != 0) 
    {
        return error;
    }    
                                                
    ctrlReg |= 0x8000;
    error = MLX90641_I2CWrite(slaveAddr, 0x800D, ctrlReg);
    
    if ( error != 0)
    {
        return error;
    }    
    
    error = MLX90641_I2CGeneralReset();
    
    if ( error != 0)
    {
        return error;
    }    
    
    error = MLX90641_I2CRead(slaveAddr, 0x800D, 1, &ctrlReg);
    
    if ( error != 0)
    {
        return error;
    }    
    
    if ((ctrlReg & 0x8000) != 0)
    {
        return -11;
    }
    
    return 0;    
}

//------------------------------------------------------------------------------

int MLX90641_GetFrameData(uint8_t slaveAddr, uint16_t *frameData)
{
    uint16_t dataReady = 1;
    uint16_t controlRegister1;
    uint16_t statusRegister;
    uint16_t data[48];
    int error = 1;
    uint8_t cnt = 0;
    uint8_t subPage = 0;
    
    dataReady = 0;
    while(dataReady == 0)
    {
        error = MLX90641_I2CRead(slaveAddr, 0x8000, 1, &statusRegister);
        if(error != 0)
        {
            return error;
        }    
        dataReady = statusRegister & 0x0008;
    }   
    subPage = statusRegister & 0x0001;
        
    error = MLX90641_I2CWrite(slaveAddr, 0x8000, 0x0030);
    if(error == -1)
    {
        return error;
    }
                
    if(subPage == 0)
    { 
        error = MLX90641_I2CRead(slaveAddr, 0x0400, 32, frameData); 
        if(error != 0)
        {
            return error;
        }
        error = MLX90641_I2CRead(slaveAddr, 0x0440, 32, frameData+32); 
        if(error != 0)
        {
            return error;
        }
        error = MLX90641_I2CRead(slaveAddr, 0x0480, 32, frameData+64); 
        if(error != 0)
        {
            return error;
        }
        error = MLX90641_I2CRead(slaveAddr, 0x04C0, 32, frameData+96); 
        if(error != 0)
        {
            return error;
        }
        error = MLX90641_I2CRead(slaveAddr, 0x0500, 32, frameData+128); 
        if(error != 0)
        {
            return error;
        }
        error = MLX90641_I2CRead(slaveAddr, 0x0540, 32, frameData+160); 
        if(error != 0)
        {
            return error;
        }
    }    
    else
    {
        error = MLX90641_I2CRead(slaveAddr, 0x0420, 32, frameData); 
        if(error != 0)
        {
            return error;
        }
        error = MLX90641_I2CRead(slaveAddr, 0x0460, 32, frameData+32); 
        if(error != 0)
        {
            return error;
        }
        error = MLX90641_I2CRead(slaveAddr, 0x04A0, 32, frameData+64); 
        if(error != 0)
        {
            return error;
        }
        error = MLX90641_I2CRead(slaveAddr, 0x04E0, 32, frameData+96); 
        if(error != 0)
        {
            return error;
        }
        error = MLX90641_I2CRead(slaveAddr, 0x0520, 32, frameData+128); 
        if(error != 0)
        {
            return error;
        }
        error = MLX90641_I2CRead(slaveAddr, 0x0560, 32, frameData+160); 
        if(error != 0)
        {
            return error;
        }
    }   
    
    error = MLX90641_I2CRead(slaveAddr, 0x0580, 48, data); 
    if(error != 0)
    {
        return error;
    }            
    
    
    error = MLX90641_I2CRead(slaveAddr, 0x800D, 1, &controlRegister1);
    frameData[240] = controlRegister1;
    frameData[241] = subPage;

    if(error != 0)
    {
        return error;
    }
    
    error = ValidateAuxData(data);
    if(error == 0)
    {
        for(cnt=0; cnt<48; cnt++)
        {
            frameData[cnt+192] = data[cnt];
        }
    }        
    
    error = ValidateFrameData(frameData);
    if (error != 0)
    {
        return error;
    }                                 
    
    return frameData[241];    
}

int ValidateFrameData(uint16_t *frameData)
{
    uint8_t line = 0;
    
    for(int i=0; i<192; i+=16)
    {
        if(frameData[i] == 0x7FFF) return -8;
        line = line + 1;
    }    
        
    return 0;    
}

int ValidateAuxData(uint16_t *auxData) 
{
    
    if(auxData[0] == 0x7FFF) return -8;    
    
    for(int i=8; i<19; i++)
    {
        if(auxData[i] == 0x7FFF) return -8;
    }
    
    for(int i=20; i<23; i++)
    {
        if(auxData[i] == 0x7FFF) return -8;
    }
    
    for(int i=24; i<33; i++)
    {
        if(auxData[i] == 0x7FFF) return -8;
    }
    
    for(int i=40; i<48; i++)
    {
        if(auxData[i] == 0x7FFF) return -8;
    }
    
    return 0;
    
}

//------------------------------------------------------------------------------

int MLX90641_ExtractParameters(uint16_t *eeData, paramsMLX90641 *mlx90641)
{
    int error = CheckEEPROMValid(eeData);
    
    if(error == 0)
    {
        ExtractVDDParameters(eeData, mlx90641);
        ExtractPTATParameters(eeData, mlx90641);
        ExtractGainParameters(eeData, mlx90641);
        ExtractTgcParameters(eeData, mlx90641);
        ExtractEmissivityParameters(eeData, mlx90641);
        ExtractResolutionParameters(eeData, mlx90641);
        ExtractKsTaParameters(eeData, mlx90641);
        ExtractKsToParameters(eeData, mlx90641);
        ExtractCPParameters(eeData, mlx90641);
        ExtractAlphaParameters(eeData, mlx90641);
        ExtractOffsetParameters(eeData, mlx90641);
        ExtractKtaPixelParameters(eeData, mlx90641);
        ExtractKvPixelParameters(eeData, mlx90641);        
        error = ExtractDeviatingPixels(eeData, mlx90641);  
    }
    
    return error;

}

//------------------------------------------------------------------------------

int MLX90641_SetResolution(uint8_t slaveAddr, uint8_t resolution)
{
    uint16_t controlRegister1;
    int value;
    int error;
    
    value = (resolution & 0x03) << 10;
    
    error = MLX90641_I2CRead(slaveAddr, 0x800D, 1, &controlRegister1);
    
    if(error == 0)
    {
        value = (controlRegister1 & 0xF3FF) | value;
        error = MLX90641_I2CWrite(slaveAddr, 0x800D, value);        
    }    
    
    return error;
}

//------------------------------------------------------------------------------

int MLX90641_GetCurResolution(uint8_t slaveAddr)
{
    uint16_t controlRegister1;
    int resolutionRAM;
    int error;
    
    error = MLX90641_I2CRead(slaveAddr, 0x800D, 1, &controlRegister1);
    if(error != 0)
    {
        return error;
    }    
    resolutionRAM = (controlRegister1 & 0x0C00) >> 10;
    
    return resolutionRAM; 
}

//------------------------------------------------------------------------------

int MLX90641_SetRefreshRate(uint8_t slaveAddr, uint8_t refreshRate)
{
    uint16_t controlRegister1;
    int value;
    int error;
    
    value = (refreshRate & 0x07)<<7;
    
    error = MLX90641_I2CRead(slaveAddr, 0x800D, 1, &controlRegister1);
    if(error == 0)
    {
        value = (controlRegister1 & 0xFC7F) | value;
        error = MLX90641_I2CWrite(slaveAddr, 0x800D, value);
    }    
    
    return error;
}

//------------------------------------------------------------------------------

int MLX90641_GetRefreshRate(uint8_t slaveAddr)
{
    uint16_t controlRegister1;
    int refreshRate;
    int error;
    
    error = MLX90641_I2CRead(slaveAddr, 0x800D, 1, &controlRegister1);
    if(error != 0)
    {
        return error;
    }    
    refreshRate = (controlRegister1 & 0x0380) >> 7;
    
    return refreshRate;
}

//------------------------------------------------------------------------------

void MLX90641_CalculateTo(uint16_t *frameData, const paramsMLX90641 *params, float emissivity, float tr, float *result)
{
    float vdd;
    float ta;
    float ta4;
    float tr4;
    float taTr;
    float gain;
    float irDataCP;
    float irData;
    float alphaCompensated;
    float Sx;
    float To;
    float alphaCorrR[8];
    int8_t range;
    uint16_t subPage;
    float ktaScale;
    float kvScale;
    float alphaScale;
    float kta;
    float kv;
    
    subPage = frameData[241];
    vdd = MLX90641_GetVdd(frameData, params);
    ta = MLX90641_GetTa(frameData, params);    
    ta4 = (ta + 273.15);
    ta4 = ta4 * ta4;
    ta4 = ta4 * ta4;
    tr4 = (tr + 273.15);
    tr4 = tr4 * tr4;
    tr4 = tr4 * tr4;
    
    taTr = tr4 - (tr4-ta4)/emissivity;
    
    ktaScale = pow(2,(double)params->ktaScale);
    kvScale = pow(2,(double)params->kvScale);
    alphaScale = pow(2,(double)params->alphaScale);
    
    alphaCorrR[1] = 1 / (1 + params->ksTo[1] * 20);
    alphaCorrR[0] = alphaCorrR[1] / (1 + params->ksTo[0] * 20);
    alphaCorrR[2] = 1 ;
    alphaCorrR[3] = (1 + params->ksTo[2] * params->ct[3]);
    alphaCorrR[4] = alphaCorrR[3] * (1 + params->ksTo[3] * (params->ct[4] - params->ct[3]));
    alphaCorrR[5] = alphaCorrR[4] * (1 + params->ksTo[4] * (params->ct[5] - params->ct[4]));
    alphaCorrR[6] = alphaCorrR[5] * (1 + params->ksTo[5] * (params->ct[6] - params->ct[5]));
    alphaCorrR[7] = alphaCorrR[6] * (1 + params->ksTo[6] * (params->ct[7] - params->ct[6]));
    
//------------------------- Gain calculation -----------------------------------    
    gain = frameData[202];
    if(gain > 32767)
    {
        gain = gain - 65536;
    }
    
    gain = params->gainEE / gain; 
  
//------------------------- To calculation -------------------------------------        
    irDataCP = frameData[200];  
    if(irDataCP > 32767)
    {
        irDataCP = irDataCP - 65536;
    }
    irDataCP = irDataCP * gain;

    irDataCP = irDataCP - params->cpOffset * (1 + params->cpKta * (ta - 25)) * (1 + params->cpKv * (vdd - 3.3));
    
    for( int pixelNumber = 0; pixelNumber < 192; pixelNumber++)
    {      
        irData = frameData[pixelNumber];
        if(irData > 32767)
        {
            irData = irData - 65536;
        }
        irData = irData * gain;
        
        kta = (float)params->kta[pixelNumber]/ktaScale;
        kv = (float)params->kv[pixelNumber]/kvScale;
            
        irData = irData - params->offset[subPage][pixelNumber]*(1 + kta*(ta - 25))*(1 + kv*(vdd - 3.3));                
    
        irData = irData - params->tgc * irDataCP;
        
        irData = irData / emissivity;
        
        alphaCompensated = SCALEALPHA*alphaScale/params->alpha[pixelNumber];
        alphaCompensated = alphaCompensated*(1 + params->KsTa * (ta - 25));
        
        Sx = alphaCompensated * alphaCompensated * alphaCompensated * (irData + alphaCompensated * taTr);
        Sx = sqrt(sqrt(Sx)) * params->ksTo[2];
        
        To = sqrt(sqrt(irData/(alphaCompensated * (1 - params->ksTo[2] * 273.15) + Sx) + taTr)) - 273.15;
                
        if(To < params->ct[1])
        {
            range = 0;
        }
        else if(To < params->ct[2])   
        {
            range = 1;            
        }   
        else if(To < params->ct[3])
        {
            range = 2;            
        }
        else if(To < params->ct[4])
        {
            range = 3;            
        }
        else if(To < params->ct[5])
        {
            range = 4;            
        }
        else if(To < params->ct[6])
        {
            range = 5;            
        }
        else if(To < params->ct[7])
        {
            range = 6;            
        }
        else
        {
            range = 7;            
        }      
        
        To = sqrt(sqrt(irData / (alphaCompensated * alphaCorrR[range] * (1 + params->ksTo[range] * (To - params->ct[range]))) + taTr)) - 273.15;
        
        result[pixelNumber] = To;
    }
}

//------------------------------------------------------------------------------

void MLX90641_GetImage(uint16_t *frameData, const paramsMLX90641 *params, float *result)
{
    float vdd;
    float ta;
    float gain;
    float irDataCP;
    float irData;
    float alphaCompensated;
    float image;
    uint16_t subPage;
    float ktaScale;
    float kvScale;
    float kta;
    float kv;
    
    subPage = frameData[241];
    
    vdd = MLX90641_GetVdd(frameData, params);
    ta = MLX90641_GetTa(frameData, params);
    ktaScale = pow(2,(double)params->ktaScale);
    kvScale = pow(2,(double)params->kvScale);
    
//------------------------- Gain calculation -----------------------------------    
    gain = frameData[202];
    if(gain > 32767)
    {
        gain = gain - 65536;
    }
    
    gain = params->gainEE / gain; 
  
//------------------------- Image calculation -------------------------------------    
    irDataCP = frameData[200];  
    if(irDataCP > 32767)
    {
        irDataCP = irDataCP - 65536;
    }
    irDataCP = irDataCP * gain;

    irDataCP = irDataCP - params->cpOffset * (1 + params->cpKta * (ta - 25)) * (1 + params->cpKv * (vdd - 3.3));
    
    for( int pixelNumber = 0; pixelNumber < 192; pixelNumber++)
    {
        irData = frameData[pixelNumber];
        if(irData > 32767)
        {
            irData = irData - 65536;
        }
        irData = irData * gain;
        
        kta = (float)params->kta[pixelNumber]/ktaScale;
        kv = (float)params->kv[pixelNumber]/kvScale;
            
        irData = irData - params->offset[subPage][pixelNumber]*(1 + kta*(ta - 25))*(1 + kv*(vdd - 3.3));                
        
        irData = irData - params->tgc * irDataCP;
            
        alphaCompensated = (params->alpha[pixelNumber] - params->tgc * params->cpAlpha);
            
        image = irData*alphaCompensated;
            
        result[pixelNumber] = image;
    }
}

//------------------------------------------------------------------------------

float MLX90641_GetVdd(uint16_t *frameData, const paramsMLX90641 *params)
{
    float vdd;
    float resolutionCorrection;
    
    int resolutionRAM;    
    
    vdd = frameData[234];
    if(vdd > 32767)
    {
        vdd = vdd - 65536;
    }
    resolutionRAM = (frameData[240] & 0x0C00) >> 10;
    resolutionCorrection = pow(2, (double)params->resolutionEE) / pow(2, (double)resolutionRAM);
    vdd = (resolutionCorrection * vdd - params->vdd25) / params->kVdd + 3.3;
    
    return vdd;
}

//------------------------------------------------------------------------------

float MLX90641_GetTa(uint16_t *frameData, const paramsMLX90641 *params)
{
    float ptat;
    float ptatArt;
    float vdd;
    float ta;
    
    vdd = MLX90641_GetVdd(frameData, params);
    
    ptat = frameData[224];
    if(ptat > 32767)
    {
        ptat = ptat - 65536;
    }
    
    ptatArt = frameData[192];
    if(ptatArt > 32767)
    {
        ptatArt = ptatArt - 65536;
    }
    ptatArt = (ptat / (ptat * params->alphaPTAT + ptatArt)) * pow(2, (double)18);
    
    ta = (ptatArt / (1 + params->KvPTAT * (vdd - 3.3)) - params->vPTAT25);
    ta = ta / params->KtPTAT + 25;
    
    return ta;
}

//------------------------------------------------------------------------------

int MLX90641_GetSubPageNumber(uint16_t *frameData)
{
    return frameData[241];    

}    

//------------------------------------------------------------------------------
void MLX90641_BadPixelsCorrection(uint16_t pixel, float *to)
{   
    float ap[2];
    uint8_t line;
    uint8_t column;
    
    if(pixel<192)
    {
        line = pixel>>4;
        column = pixel - (line<<4);
               
        if(column == 0)
        {
            to[pixel] = to[pixel+1];            
        }
        else if(column == 1 || column == 14)
        {
            to[pixel] = (to[pixel-1]+to[pixel+1])/2.0;                
        } 
        else if(column == 15)
        {
            to[pixel] = to[pixel-1];
        } 
        else
        {            
            ap[0] = to[pixel+1] - to[pixel+2];
            ap[1] = to[pixel-1] - to[pixel-2];
            if(fabs(ap[0]) > fabs(ap[1]))
            {
                to[pixel] = to[pixel-1] + ap[1];                        
            }
            else
            {
                to[pixel] = to[pixel+1] + ap[0];                        
            }
                    
        }                      
    }   
}    

//------------------------------------------------------------------------------
void ExtractVDDParameters(uint16_t *eeData, paramsMLX90641 *mlx90641)
{
    int16_t kVdd;
    int16_t vdd25;
    
    kVdd = eeData[39];
    if(kVdd > 1023)
    {
        kVdd = kVdd - 2048;
    }
    kVdd = 32 * kVdd;
    
    vdd25 = eeData[38];
    if(vdd25 > 1023)
    {
        vdd25 = vdd25 - 2048;
    }
    vdd25 = 32 * vdd25;
    
    mlx90641->kVdd = kVdd;
    mlx90641->vdd25 = vdd25; 
}

//------------------------------------------------------------------------------

void ExtractPTATParameters(uint16_t *eeData, paramsMLX90641 *mlx90641)
{
    float KvPTAT;
    float KtPTAT;
    int16_t vPTAT25;
    float alphaPTAT;
    
    KvPTAT = eeData[43];
    if(KvPTAT > 1023)
    {
        KvPTAT = KvPTAT - 2048;
    }
    KvPTAT = KvPTAT/4096;
    
    KtPTAT = eeData[42];
    if(KtPTAT > 1023)
    {
        KtPTAT = KtPTAT - 2048;
    }
    KtPTAT = KtPTAT/8;
    
    vPTAT25 = 32 * eeData[40] + eeData[41];
    
    alphaPTAT = eeData[44] / 128.0f;
    
    mlx90641->KvPTAT = KvPTAT;
    mlx90641->KtPTAT = KtPTAT;    
    mlx90641->vPTAT25 = vPTAT25;
    mlx90641->alphaPTAT = alphaPTAT;   
}

//------------------------------------------------------------------------------

void ExtractGainParameters(uint16_t *eeData, paramsMLX90641 *mlx90641)
{
    int16_t gainEE;
    
    gainEE = 32 * eeData[36] + eeData[37];

    mlx90641->gainEE = gainEE;    
}

//------------------------------------------------------------------------------

void ExtractTgcParameters(uint16_t *eeData, paramsMLX90641 *mlx90641)
{
    float tgc;
    tgc = eeData[51] & 0x01FF;
    if(tgc > 255)
    {
        tgc = tgc - 512;
    }
    tgc = tgc / 64.0f;
    
    mlx90641->tgc = tgc;        
}

//------------------------------------------------------------------------------

void ExtractEmissivityParameters(uint16_t *eeData, paramsMLX90641 *mlx90641)
{
    float emissivity;
    emissivity = eeData[35];
       
    if(emissivity > 1023)
    {
        emissivity = emissivity - 2048;
    }
    emissivity = emissivity/512;
    
    mlx90641->emissivityEE = emissivity;
}
    
//------------------------------------------------------------------------------

void ExtractResolutionParameters(uint16_t *eeData, paramsMLX90641 *mlx90641)
{
    uint8_t resolutionEE;
    resolutionEE = (eeData[51] & 0x0600) >> 9;    
    
    mlx90641->resolutionEE = resolutionEE;
}

//------------------------------------------------------------------------------

void ExtractKsTaParameters(uint16_t *eeData, paramsMLX90641 *mlx90641)
{
    float KsTa;
    KsTa = eeData[34];
    if(KsTa > 1023)
    {
        KsTa = KsTa - 2048;
    }
    KsTa = KsTa / 32768.0f;
    
    mlx90641->KsTa = KsTa;
}

//------------------------------------------------------------------------------

void ExtractKsToParameters(uint16_t *eeData, paramsMLX90641 *mlx90641)
{
    int KsToScale;
    
    mlx90641->ct[0] = -40;
    mlx90641->ct[1] = -20;
    mlx90641->ct[2] = 0;
    mlx90641->ct[3] = 80;
    mlx90641->ct[4] = 120;
    mlx90641->ct[5] = eeData[58];
    mlx90641->ct[6] = eeData[60];
    mlx90641->ct[7] = eeData[62];
     
    KsToScale = eeData[52];
    KsToScale = 1 << KsToScale;
    
    mlx90641->ksTo[0] = eeData[53];
    mlx90641->ksTo[1] = eeData[54];
    mlx90641->ksTo[2] = eeData[55];
    mlx90641->ksTo[3] = eeData[56];
    mlx90641->ksTo[4] = eeData[57];
    mlx90641->ksTo[5] = eeData[59];
    mlx90641->ksTo[6] = eeData[61];
    mlx90641->ksTo[7] = eeData[63];
    
    
    for(int i = 0; i < 8; i++)
    {
        if(mlx90641->ksTo[i] > 1023)
        {
            mlx90641->ksTo[i] = mlx90641->ksTo[i] - 2048;
        }
        mlx90641->ksTo[i] = mlx90641->ksTo[i] / KsToScale;
    } 
}

//------------------------------------------------------------------------------

void ExtractAlphaParameters(uint16_t *eeData, paramsMLX90641 *mlx90641)
{
    float rowMaxAlphaNorm[6];
    uint16_t scaleRowAlpha[6];
    uint8_t alphaScale;
    float alphaTemp[192];
    float temp;
    int p = 0;

    scaleRowAlpha[0] = (eeData[25] >> 5) + 20;
    scaleRowAlpha[1] = (eeData[25] & 0x001F) + 20;
    scaleRowAlpha[2] = (eeData[26] >> 5) + 20;
    scaleRowAlpha[3] = (eeData[26] & 0x001F) + 20;
    scaleRowAlpha[4] = (eeData[27] >> 5) + 20;
    scaleRowAlpha[5] = (eeData[27] & 0x001F) + 20;

    
    for(int i = 0; i < 6; i++)
    {
        rowMaxAlphaNorm[i] = eeData[28 + i] / pow(2,(double)scaleRowAlpha[i]);
        rowMaxAlphaNorm[i] = rowMaxAlphaNorm[i] / 2047.0f;
    }

    for(int i = 0; i < 6; i++)
    {
        for(int j = 0; j < 32; j ++)
        {
            p = 32 * i +j;
            alphaTemp[p] = eeData[256 + p] * rowMaxAlphaNorm[i]; 
            alphaTemp[p] = alphaTemp[p] - mlx90641->tgc * mlx90641->cpAlpha;
            alphaTemp[p] = SCALEALPHA/alphaTemp[p];
        }
    }
    
    temp = alphaTemp[0];
    for(int i = 1; i < 192; i++)
    {
        if (alphaTemp[i] > temp)
        {
            temp = alphaTemp[i];
        }
    }
    
    alphaScale = 0;
    while(temp < 32768)
    {
        temp = temp*2;
        alphaScale = alphaScale + 1;
    } 
    
    for(int i = 0; i < 192; i++)
    {
        temp = alphaTemp[i] * pow(2,(double)alphaScale);        
        mlx90641->alpha[i] = (temp + 0.5);        
        
    } 
    
    mlx90641->alphaScale = alphaScale;      
}

//------------------------------------------------------------------------------

void ExtractOffsetParameters(uint16_t *eeData, paramsMLX90641 *mlx90641)
{
    int scaleOffset;
    int16_t offsetRef;
    int16_t tempOffset; 
    
    scaleOffset = eeData[16] >> 5;
    scaleOffset = 1 << scaleOffset;

    offsetRef = 32 * eeData[17] + eeData[18];
    if (offsetRef > 32767)
    {
        offsetRef = offsetRef - 65536;
    }

    for(int i = 0; i < 192; i++)
    {
        tempOffset = eeData[64 + i];
        if(tempOffset > 1023)
        {
           tempOffset = eeData[64 + i] - 2048; 
        }
        mlx90641->offset[0][i] = tempOffset * scaleOffset + offsetRef;
        
        tempOffset = eeData[640 + i];
        if(tempOffset > 1023)
        {
           tempOffset = eeData[640 + i] - 2048; 
        }
        mlx90641->offset[1][i] = tempOffset * scaleOffset + offsetRef;
    }
}

//------------------------------------------------------------------------------

void ExtractKtaPixelParameters(uint16_t *eeData, paramsMLX90641 *mlx90641)
{
    uint8_t ktaScale1;
    uint8_t ktaScale2;
    int16_t ktaAvg;
    int16_t tempKta;
    float ktaTemp[192];
    float temp;

    ktaAvg = eeData[21];
    if (ktaAvg > 1023)
    {
        ktaAvg = ktaAvg - 2048;
    }
  
    ktaScale1 = eeData[22] >> 5;
    ktaScale2 = eeData[22] & 0x001F;

    for(int i = 0; i < 192; i++)
    {
        tempKta = (eeData[448 + i] >> 5);
        if (tempKta > 31)
        {
            tempKta = tempKta - 64;
        }

        ktaTemp[i] = tempKta * pow(2,(double)ktaScale2);
        ktaTemp[i] = ktaTemp[i] + ktaAvg;
        ktaTemp[i] = ktaTemp[i] / pow(2,(double)ktaScale1);
    }
    
    temp = fabs(ktaTemp[0]);
    for(int i = 1; i < 192; i++)
    {
        if (fabs(ktaTemp[i]) > temp)
        {
            temp = fabs(ktaTemp[i]);
        }
    }
    
    ktaScale1 = 0;
    while(temp < 64)
    {
        temp = temp*2;
        ktaScale1 = ktaScale1 + 1;
    }    
     
    for(int i = 0; i < 192; i++)
    {
        temp = ktaTemp[i] * pow(2,(double)ktaScale1);
        if (temp < 0)
        {
            mlx90641->kta[i] = (temp - 0.5);
        }
        else
        {
            mlx90641->kta[i] = (temp + 0.5);
        }        
        
    } 
    
    mlx90641->ktaScale = ktaScale1;
}

//------------------------------------------------------------------------------

void ExtractKvPixelParameters(uint16_t *eeData, paramsMLX90641 *mlx90641)
{
    uint8_t kvScale1;
    uint8_t kvScale2;
    int16_t kvAvg;
    int16_t tempKv;
    float kvTemp[192];
    float temp;

    kvAvg = eeData[23];
    if (kvAvg > 1023)
    {
        kvAvg = kvAvg - 2048;
    }
  
    kvScale1 = eeData[24] >> 5;
    kvScale2 = eeData[24] & 0x001F;

    for(int i = 0; i < 192; i++)
    {
        tempKv = (eeData[448 + i] & 0x001F);
        if (tempKv > 15)
        {
            tempKv = tempKv - 32;
        }

        kvTemp[i] = tempKv * pow(2,(double)kvScale2);
        kvTemp[i] = kvTemp[i] + kvAvg;
        kvTemp[i] = kvTemp[i] / pow(2,(double)kvScale1);
    }
    
    temp = fabs(kvTemp[0]);
    for(int i = 1; i < 192; i++)
    {
        if (fabs(kvTemp[i]) > temp)
        {
            temp = fabs(kvTemp[i]);
        }
    }
    
    kvScale1 = 0;
    while(temp < 64)
    {
        temp = temp*2;
        kvScale1 = kvScale1 + 1;
    }    
     
    for(int i = 0; i < 192; i++)
    {
        temp = kvTemp[i] * pow(2,(double)kvScale1);
        if (temp < 0)
        {
            mlx90641->kv[i] = (temp - 0.5);
        }
        else
        {
            mlx90641->kv[i] = (temp + 0.5);
        }        
        
    } 
    
    mlx90641->kvScale = kvScale1;        
}

//------------------------------------------------------------------------------

void ExtractCPParameters(uint16_t *eeData, paramsMLX90641 *mlx90641)
{
    float alphaCP;
    int16_t offsetCP;
    float cpKv;
    float cpKta;
    uint8_t alphaScale;
    uint8_t ktaScale1;
    uint8_t kvScale;

    alphaScale = eeData[46];
    
    offsetCP = 32 * eeData[47] + eeData[48];
    if (offsetCP > 32767)
    {
        offsetCP = offsetCP - 65536;
    }
       
    alphaCP = eeData[45];
    if (alphaCP > 1023)
    {
        alphaCP = alphaCP - 2048;
    }
    
    alphaCP = alphaCP /  pow(2,(double)alphaScale);
    
    
    cpKta = eeData[49] & 0x001F;
    if (cpKta > 31)
    {
        cpKta = cpKta - 64;
    }
    ktaScale1 = eeData[49] >> 6;    
    mlx90641->cpKta = cpKta / pow(2,(double)ktaScale1);
    
    cpKv = eeData[50] & 0x001F;
    if (cpKv > 31)
    {
        cpKv = cpKv - 64;
    }
    kvScale = eeData[50] >> 6;
    mlx90641->cpKv = cpKv / pow(2,(double)kvScale);
       
    mlx90641->cpAlpha = alphaCP;
    mlx90641->cpOffset = offsetCP;
}

//------------------------------------------------------------------------------

float MLX90641_GetEmissivity(const paramsMLX90641 *mlx90641)
{
    return  mlx90641->emissivityEE;
}

//------------------------------------------------------------------------------

int ExtractDeviatingPixels(uint16_t *eeData, paramsMLX90641 *mlx90641)
{
    uint16_t pixCnt = 0;
    uint16_t brokenPixCnt = 0;

    int warn = 0;
    
    mlx90641->brokenPixel = 0xFFFF;
        
    pixCnt = 0;    
    while (pixCnt < 192 && brokenPixCnt < 2)
    {
        if((eeData[pixCnt+64] == 0) && (eeData[pixCnt+256] == 0) && (eeData[pixCnt+448] == 0) && (eeData[pixCnt+640] == 0))
        {
            mlx90641->brokenPixel = pixCnt;
            brokenPixCnt = brokenPixCnt + 1;
        }    
        
        pixCnt = pixCnt + 1;
    } 
    
    if(brokenPixCnt > 1)  
    {
        warn = -3;
    }         
    
    return warn;
       
}
 
 //------------------------------------------------------------------------------
 
 int CheckEEPROMValid(uint16_t *eeData)  
 {
     int deviceSelect;
     deviceSelect = eeData[10] & 0x0040;
     if(deviceSelect != 0)
     {
         return 0;
     }
     
     return -7;    
 }        