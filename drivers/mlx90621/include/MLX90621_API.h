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
#ifndef _MLX621_API_H_
#define _MLX621_API_H_

#define SCALEALPHA 0.000001
    
typedef struct
    {
        int16_t vTh25;
        float kT1;
        float kT2;        
        float tgc;
        float KsTa;
        float ksTo;
        float alpha[64];    
        float ai[64];
        float bi[64];    
        float cpAlpha;
        float cpA;
        float cpB;
        uint16_t brokenPixels[5];
        uint16_t outlierPixels[5];  
    } paramsMLX90621;
    
    int MLX90621_DumpEE(uint8_t *eeData);
    int MLX90621_GetFrameData(uint16_t *frameData);
    int MLX90621_Configure(uint8_t *eeData);
    int MLX90621_GetOscillatorTrim(uint16_t *oscTrim);
    int MLX90621_GetConfiguration(uint16_t *cfgReg);
    int MLX90621_ExtractParameters(uint8_t *eeData, paramsMLX90621 *mlx90621);
    float MLX90621_GetTa(uint16_t *frameData, const paramsMLX90621 *params);
    void MLX90621_GetImage(uint16_t *frameData, const paramsMLX90621 *params, float *result);
    void MLX90621_CalculateTo(uint16_t *frameData, const paramsMLX90621 *params, float emissivity, float tr, float *result);
    int MLX90621_SetResolution(uint8_t resolution);
    int MLX90621_GetCurResolution();
    int MLX90621_SetRefreshRate(uint8_t refreshRate);   
    int MLX90621_GetRefreshRate();  
 
#endif
