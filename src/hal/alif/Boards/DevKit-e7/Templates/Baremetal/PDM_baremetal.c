/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/******************************************************************************
 * @file     PDM_baremetal.c
 * @author   Nisarga A M
 * @email    nisarga.am@alifsemi.com
 * @version  V1.0.0
 * @date     15-Jan-2023
 * @brief    Baremetal code for PDM(Pulse Density Modulation).
 *           -> Select the PDM channel
 *           -> Select the mode of operation in the Control API.
 *           -> Select the mode of operation in the Control API.
 *           -> Then build the project and flash the generated axf file on the target
 *           -> Then Start playing some audio or speak near to the PDM microphone which
 *              is on B0 Flat board.
 *           -> Once the sample count reaches the maximum value,the PCM samples will be
 *              stored in the particular buffer.
 *           -> Export the memory and To play the PCM data, use pcmplay.c file which
 *              will generate the pcm_samples.pcm audio file
 *           -> Use ffplay command to play the audio.
 *           Hardware setup:
 *           -> Flat board has internal PDM Mics which is connected to channel 4
 *              and channel 5 internally, no hardware connection is required for
 *              channel 4 and 5.
 *              For channel 4 and channel 5
 *           -> Clock line:
 *               PDM_C2 (PDM_C2_A) -> P6_7
 *           -> Data line:
 *               PDM_D2 (PDM_D2_B) -> P5_4
 *
 ******************************************************************************/
/* System Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Project Includes */
/* include for PDM Driver */
#include "Driver_PDM.h"
#include "pinconf.h"
#include "RTE_Components.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */

#define ENABLE              1
#define DISABLE             0

/* Store the number of samples */
/* For 40000 samples user can hear maximum up to 4 sec of audio
 * to store maximum samples then change the scatter file and increase the memory */
#define NUM_SAMPLE  40000

/* channel number used for channel configuration and status register */
#define CHANNEL_4  4
#define CHANNEL_5  5

/* PDM Channel 4 configurations */
#define CH4_PHASE             0x0000001F
#define CH4_GAIN              0x0000000D
#define CH4_PEAK_DETECT_TH    0x00060002
#define CH4_PEAK_DETECT_ITV   0x0004002D

/* PDM Channel 5 configurations */
#define CH5_PHASE             0x00000003
#define CH5_GAIN              0x00000013
#define CH5_PEAK_DETECT_TH    0x00060002
#define CH5_PEAK_DETECT_ITV   0x00020027

/* PDM driver instance */
extern ARM_DRIVER_PDM Driver_PDM;
static ARM_DRIVER_PDM *PDMdrv = &Driver_PDM;

PDM_CH_CONFIG pdm_coef_reg;

/* For Demo purpose use channel 4  and channel 5 */
/* To store the PCM samples for any Channel */
uint16_t sample_buf[NUM_SAMPLE];

/* Channel 4 FIR coefficient */
uint32_t ch4_fir[18] = { 0x00000001,0x00000003,0x00000003,0x000007F4,0x00000004,0x000007ED,0x000007F5,0x000007F4,0x000007D3,
                        0x000007FE,0x000007BC,0x000007E5,0x000007D9,0x00000793,0x00000029,0x0000072C,0x00000072,0x000002FD};

/* Channel 5 FIR coefficient */
uint32_t ch5_fir[18] = {0x00000000, 0x000007FF,0x00000000,0x00000004,0x00000004,0x000007FC,0x00000000,0x000007FB,0x000007E4,
                       0x00000000,0x0000002B,0x00000009,0x00000016,0x00000049,0x00000793,0x000006F8,0x00000045,0x00000178};

/* PDM callback events */
typedef enum {
    PDM_CALLBACK_ERROR_EVENT            = (1 << 0),
    PDM_CALLBACK_WARNING_EVENT          = (1 << 1),
    PDM_CALLBACK_AUDIO_DETECTION_EVENT  = (1 << 2)
}PDM_CB_EVENTS;

volatile int32_t call_back_event = 0;

void pdm_demo();

static void PDM_fifo_callback(uint32_t event)
{
    if(event & ARM_PDM_EVENT_ERROR )
    {
        call_back_event = PDM_CALLBACK_ERROR_EVENT;
    }

    if(event & ARM_PDM_EVENT_CAPTURE_COMPLETE)
    {
        call_back_event = PDM_CALLBACK_WARNING_EVENT;
    }

    if(event & ARM_PDM_EVENT_AUDIO_DETECTION)
    {
        call_back_event = PDM_CALLBACK_AUDIO_DETECTION_EVENT;
    }
}

/**
 * @fn         : void PDM_fifo_callback(uint32_t event)
 * @brief      : PDM fifo callback
 *               -> Initialize the PDM module.
 *               -> Enable the Power for the PDM module
 *               -> Select the mode of operation in Control API.The
 *                    mode which user has selected will be applies to
 *                    all the channel which user has selected.
 *               -> Select the Bypass DC blocking IIR filter for reference.
 *               -> Select the PDM channel and use the selected channel
 *                    configuration and status register values.
 *               -> Play some audio and start capturing the data.
 *               -> Once all data has stored in the particular buffer ,
 *                  call back event will be set and it will stop capturing
 *                  data.
 *               -> Once all the data capture is done , go to the particular
 *                  memory location which user has given for storing PCM data
 *                  samples.
 *               -> Then export the memory and give the total size of
 *                  the buffer memory and select the particular bin file and export
 *                  the memory.
 *               -> Play the PCM sample file using ffplay command.
 * @return     : none
 */
void pdm_demo()
{
    int32_t ret = 0;
    ARM_DRIVER_VERSION version;
    int32_t retval;

    printf("\r\n >>> PDM demo starting up!!! <<< \r\n");

    version = PDMdrv->GetVersion();
    printf("\r\n PDM version api:%X driver:%X...\r\n", version.api, version.drv);

    retval = pinconf_set(PORT_5, PIN_4, PINMUX_ALTERNATE_FUNCTION_3, PADCTRL_READ_ENABLE);
    if (retval)
        printf("pinconf_set failed\n");

    retval = pinconf_set(PORT_6, PIN_7, PINMUX_ALTERNATE_FUNCTION_3, 0x0);
    if (retval)
        printf("pinconf_set failed\n");

    /* Initialize PDM driver */
    ret = PDMdrv->Initialize(PDM_fifo_callback);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: PDM init failed\n");
        return;
    }

    /* Enable the power for PDM */
    ret = PDMdrv->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: PDM Power up failed\n");
        goto error_uninitialize;
    }

    /* To select the PDM channel 4 and channel 5 */
    ret = PDMdrv->Control(ARM_PDM_SELECT_CHANNEL, ARM_PDM_MASK_CHANNEL_4 |ARM_PDM_MASK_CHANNEL_5, NULL);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: PDM channel select control failed\n");
        goto error_poweroff;
    }

    /* Select Standard voice PDM mode */
    ret = PDMdrv->Control(ARM_PDM_MODE, ARM_PDM_MODE_AUDIOFREQ_8K_DECM_64, NULL);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: PDM Standard voice control mode failed\n");
        goto error_poweroff;
    }

    /* Select the DC blocking IIR filter */
    ret = PDMdrv->Control(ARM_PDM_BYPASS_IIR_FILTER, ENABLE, NULL);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: PDM DC blocking IIR control failed\n");
        goto error_poweroff;
    }

    /* Set Channel 4 Phase value */
    ret = PDMdrv->Control(ARM_PDM_CHANNEL_PHASE, CHANNEL_4, CH4_PHASE);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: PDM Channel_Config failed\n");
        goto error_uninitialize;
    }

    /* Set Channel 4 Gain value */
    ret = PDMdrv->Control(ARM_PDM_CHANNEL_GAIN, CHANNEL_4, CH4_GAIN);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: PDM Channel_Config failed\n");
        goto error_uninitialize;
    }

    /* Set Channel 4 Peak detect threshold value */
    ret = PDMdrv->Control(ARM_PDM_CHANNEL_PEAK_DETECT_TH, CHANNEL_4, CH4_PEAK_DETECT_TH);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: PDM Channel_Config failed\n");
        goto error_uninitialize;
    }

    /* Set Channel 4 Peak detect ITV value */
    ret = PDMdrv->Control(ARM_PDM_CHANNEL_PEAK_DETECT_ITV, CHANNEL_4, CH4_PEAK_DETECT_ITV);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: PDM Channel_Config failed\n");
        goto error_uninitialize;
    }

    pdm_coef_reg.ch_num  = 4;
    memcpy(pdm_coef_reg.ch_fir_coef, ch4_fir, sizeof(pdm_coef_reg.ch_fir_coef)); /* Channel 4 fir coefficient */
    pdm_coef_reg.ch_iir_coef         = 0x00000004;   /* Channel IIR Filter Coefficient */

    ret = PDMdrv->Config(&pdm_coef_reg);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: PDM Channel_Config failed\n");
        goto error_uninitialize;
    }

    /* Set Channel 5 Phase value */
    ret = PDMdrv->Control(ARM_PDM_CHANNEL_PHASE, CHANNEL_5, CH5_PHASE);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: PDM Channel_Config failed\n");
        goto error_uninitialize;
    }

    /* Set Channel 5 Gain value */
    ret = PDMdrv->Control(ARM_PDM_CHANNEL_GAIN, CHANNEL_5, CH5_GAIN);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: PDM Channel_Config failed\n");
        goto error_uninitialize;
    }

    /* Set Channel 5 Peak detect threshold value */
    ret = PDMdrv->Control(ARM_PDM_CHANNEL_PEAK_DETECT_TH, CHANNEL_5, CH5_PEAK_DETECT_TH);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: PDM Channel_Config failed\n");
        goto error_uninitialize;
    }

    /* Set Channel 5 Peak detect ITV value */
    ret = PDMdrv->Control(ARM_PDM_CHANNEL_PEAK_DETECT_ITV, CHANNEL_5, CH5_PEAK_DETECT_ITV);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: PDM Channel_Config failed\n");
        goto error_uninitialize;
    }

    pdm_coef_reg.ch_num  = 5;
    memcpy(pdm_coef_reg.ch_fir_coef, ch5_fir, sizeof(pdm_coef_reg.ch_fir_coef)); /* Channel 5 fir coefficient */
    pdm_coef_reg.ch_iir_coef         = 0x00000004;   /* Channel IIR Filter Coefficient */

    ret = PDMdrv->Config(&pdm_coef_reg);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: PDM Channel_Config failed\n");
        goto error_uninitialize;
    }

    printf("\n------> Start Speaking or Play some Audio!------> \n");

    /* Receive the audio samples */
    ret = PDMdrv->Receive((uint16_t*)sample_buf, NUM_SAMPLE);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: PDM Receive failed\n");
       goto error_capture;
    }

    /* wait for the call back event */
    while(call_back_event == 0);

    /* PDM fifo alomost full warning event */
    if(call_back_event == PDM_CALLBACK_WARNING_EVENT)
    {
        printf("\n PDM warning event : Fifo almost full\n");
    }

    /* PDM channel audio detection event */
    if(call_back_event == PDM_CALLBACK_AUDIO_DETECTION_EVENT)
    {
        printf("\n PDM audio detect event: data in the audio channel");
    }

    /* PDM fifo overflow error event */
    if(call_back_event == PDM_CALLBACK_ERROR_EVENT)
    {
        printf("\n PDM error event: Fifo overflow \n");
    }

    call_back_event = 0;

    printf("\n------> Stop recording ------> \n");
    printf("\n--> PCM samples will be stored in 0x%p address and size of buffer is %d\n", sample_buf, sizeof(sample_buf));
    printf("\n ---END--- \r\n <<< wait forever >>> \n");
    while(1);

error_capture:
error_poweroff:
    ret = PDMdrv->PowerControl(ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\n Error: PDM power off failed\n");
    }

error_uninitialize:
    ret = PDMdrv->Uninitialize();
    if(ret != ARM_DRIVER_OK)
    {
        printf("\n Error: PDM Uninitialize failed\n");
    }

    printf("\r\n XXX PDM demo exiting XXX...\r\n");
}

/* Define main entry point.  */
int main()
{
    #if defined(RTE_Compiler_IO_STDOUT_User)
    int32_t ret;
    ret = stdout_init();
    if(ret != ARM_DRIVER_OK)
    {
        while(1)
        {
        }
    }
    #endif
    /* Enter the demo Application.  */
    pdm_demo();

    return 0;
}
