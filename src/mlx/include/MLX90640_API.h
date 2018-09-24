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
#ifndef _MLX640_API_H_
#define _MLX640_API_H_
    
  typedef struct
    {
        int16_t kVdd;
        int16_t vdd25;
        float KvPTAT;
        float KtPTAT;
        uint16_t vPTAT25;
        float alphaPTAT;
        int16_t gainEE;
        float tgc;
        float cpKv;
        float cpKta;
        uint8_t resolutionEE;
        uint8_t calibrationModeEE;
        float KsTa;
        float ksTo[4];
        int16_t ct[4];
        float alpha[768];    
        int16_t offset[768];    
        float kta[768];    
        float kv[768];
        float cpAlpha[2];
        int16_t cpOffset[2];
        float ilChessC[3]; 
        uint16_t brokenPixels[5];
        uint16_t outlierPixels[5];  
    } paramsMLX90640;
    
    int MLX90640_DumpEE(uint8_t slaveAddr, uint16_t *eeData);
    int MLX90640_GetFrameData(uint8_t slaveAddr, uint16_t *frameData);
    int MLX90640_ExtractParameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
    float MLX90640_GetVdd(uint16_t *frameData, const paramsMLX90640 *params);
    float MLX90640_GetTa(uint16_t *frameData, const paramsMLX90640 *params);
    void MLX90640_GetImage(uint16_t *frameData, const paramsMLX90640 *params, float *result);
    void MLX90640_CalculateTo(uint16_t *frameData, const paramsMLX90640 *params, float emissivity, float tr, float *result);
    int MLX90640_SetResolution(uint8_t slaveAddr, uint8_t resolution);
    int MLX90640_GetCurResolution(uint8_t slaveAddr);
    int MLX90640_SetRefreshRate(uint8_t slaveAddr, uint8_t refreshRate);   
    int MLX90640_GetRefreshRate(uint8_t slaveAddr);  
    int MLX90640_GetSubPageNumber(uint16_t *frameData);
    int MLX90640_GetCurMode(uint8_t slaveAddr); 
    int MLX90640_SetInterleavedMode(uint8_t slaveAddr);
    int MLX90640_SetChessMode(uint8_t slaveAddr);
    
#endif
