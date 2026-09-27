/*
 * capture.c -- DCMIPP (CSI-2 PIPE1) raw capture into a ring of frame buffers.
 *
 * VD66GY RAW10 over 2-lane CSI-2 -> DCMIPP PIPE1 with the ISP demosaic off and the pixel
 * packer in 8-bit mono mode -> one byte per Bayer pixel in AXISRAM1 (the 8 most significant
 * bits). Runs from ITCM (see the linker script).
 */
#include <string.h>
#include "main.h"
#include "capture.h"
#include "timebase.h"

#define PIPE            DCMIPP_PIPE1
#define FB_SIZE         (CAP_MAX_W * CAP_MAX_H)

enum { B_FREE = 0, B_WRITING, B_QUEUED, B_BUSY };

DCMIPP_HandleTypeDef hdcmipp;

static uint8_t s_fb[CAP_NBUF][FB_SIZE] __attribute__((section(".frame_buffers"), aligned(32)));

static volatile uint8_t  s_state[CAP_NBUF];
static volatile uint32_t s_ts[CAP_NBUF];
static volatile uint32_t s_seq[CAP_NBUF];
static volatile uint8_t  s_q[CAP_NBUF];
static volatile uint32_t s_q_head;          // ISR
static volatile uint32_t s_q_tail;          // main loop
static volatile int      s_wr;              // buffer the DMA is writing
static volatile uint32_t s_frames, s_dropped, s_errors, s_last_error;
static uint16_t s_w, s_h;
static bool s_running;

int cap_init(void) {
    hdcmipp.Instance = DCMIPP;
    if (HAL_DCMIPP_Init(&hdcmipp) != HAL_OK) {
        return -1;
    }
    DCMIPP_CSI_ConfTypeDef csi = {
        .NumberOfLanes = DCMIPP_CSI_TWO_DATA_LANES,
        .DataLaneMapping = DCMIPP_CSI_PHYSICAL_DATA_LANES,
        .PHYBitrate = DCMIPP_CSI_PHY_BT_800,
    };
    if (HAL_DCMIPP_CSI_SetConfig(&hdcmipp, &csi) != HAL_OK) {
        return -2;
    }
    if (HAL_DCMIPP_CSI_SetVCConfig(&hdcmipp, DCMIPP_VIRTUAL_CHANNEL0, DCMIPP_CSI_DT_BPP10) != HAL_OK) {
        return -3;
    }
    DCMIPP_CSI_PIPE_ConfTypeDef cp = {
        .DataTypeMode = DCMIPP_DTMODE_DTIDA,
        .DataTypeIDA = DCMIPP_DT_RAW10,
        .DataTypeIDB = DCMIPP_DT_RAW10,
    };
    if (HAL_DCMIPP_CSI_PIPE_SetConfig(&hdcmipp, PIPE, &cp) != HAL_OK) {
        return -4;
    }
    return 0;
}

int cap_start(uint16_t w, uint16_t h) {
    if (w == 0 || h == 0 || w > CAP_MAX_W || h > CAP_MAX_H || (w & 15)) {
        return -1;
    }
    if (s_running) {
        cap_stop();
    }
    s_w = w;
    s_h = h;
    DCMIPP_PipeConfTypeDef pc = {
        .FrameRate = DCMIPP_FRAME_RATE_ALL,
        .PixelPipePitch = w,
        .PixelPackerFormat = DCMIPP_PIXEL_PACKER_FORMAT_MONO_Y8_G8_1,
    };
    if (HAL_DCMIPP_PIPE_SetConfig(&hdcmipp, PIPE, &pc) != HAL_OK) {
        return -2;
    }
    // Raw output: no demosaic, no exposure/contrast blocks.
    HAL_DCMIPP_PIPE_DisableISPRawBayer2RGB(&hdcmipp, PIPE);
    HAL_DCMIPP_PIPE_DisableISPExposure(&hdcmipp, PIPE);
    HAL_DCMIPP_PIPE_DisableISPCtrlContrast(&hdcmipp, PIPE);
    // Crop to exactly w x h: if the sensor ever sends a larger frame (e.g. after a failed
    // reconfiguration) the DMA still cannot write past this buffer into the next one.
    DCMIPP_CropConfTypeDef crop = { .HStart = 0, .VStart = 0, .HSize = w, .VSize = h };
    if (HAL_DCMIPP_PIPE_SetCropConfig(&hdcmipp, PIPE, &crop) != HAL_OK ||
        HAL_DCMIPP_PIPE_EnableCrop(&hdcmipp, PIPE) != HAL_OK) {
        s_errors++;
    }

    for (int i = 0; i < CAP_NBUF; i++) {
        s_state[i] = B_FREE;
    }
    s_q_head = s_q_tail = 0;
    s_wr = 0;
    s_state[0] = B_WRITING;
    if (HAL_DCMIPP_CSI_PIPE_Start(&hdcmipp, PIPE, DCMIPP_VIRTUAL_CHANNEL0,
                                  (uint32_t) s_fb[0], DCMIPP_MODE_CONTINUOUS) != HAL_OK) {
        return -3;
    }
    s_running = true;
    return 0;
}

void cap_stop(void) {
    if (s_running) {
        if (HAL_DCMIPP_CSI_PIPE_Stop(&hdcmipp, PIPE, DCMIPP_VIRTUAL_CHANNEL0) != HAL_OK) {
            // The HAL leaves the pipe "busy" when the stop times out (e.g. no frames arriving);
            // force it idle so the next cap_start() can run.
            CLEAR_BIT(DCMIPP->P1FCTCR, DCMIPP_P1FCTCR_CPTREQ);
            hdcmipp.PipeState[PIPE] = HAL_DCMIPP_PIPE_STATE_READY;
            hdcmipp.State = HAL_DCMIPP_STATE_READY;
            s_errors++;
        }
        s_running = false;
    }
}

bool cap_running(void) {
    return s_running;
}

// Frame end: hand the finished buffer to the main loop and point the DMA at a free one.
// If none is free the finished frame is dropped (its buffer is written again).
void HAL_DCMIPP_PIPE_FrameEventCallback(DCMIPP_HandleTypeDef *h, uint32_t pipe) {
    if (pipe != PIPE) {
        return;
    }
    uint32_t ts = tb_us();
    uint32_t seq = s_frames++;
    int done = s_wr;
    int nxt = -1;
    for (int i = 0; i < CAP_NBUF; i++) {
        if (s_state[i] == B_FREE) {
            nxt = i;
            break;
        }
    }
    if (nxt < 0) {
        s_dropped++;
        return;
    }
    HAL_DCMIPP_PIPE_SetMemoryAddress(h, PIPE, DCMIPP_MEMORY_ADDRESS_0, (uint32_t) s_fb[nxt]);
    s_state[nxt] = B_WRITING;
    s_wr = nxt;
    s_ts[done] = ts;
    s_seq[done] = seq;
    s_state[done] = B_QUEUED;
    s_q[s_q_head % CAP_NBUF] = (uint8_t) done;
    __DMB();
    s_q_head = s_q_head + 1;
}

void HAL_DCMIPP_PIPE_ErrorCallback(DCMIPP_HandleTypeDef *h, uint32_t pipe) {
    s_errors++;
    s_last_error = h->ErrorCode;
    h->ErrorCode = HAL_DCMIPP_ERROR_NONE;
    if (pipe == PIPE) {
        __HAL_DCMIPP_ENABLE_IT(h, DCMIPP_IT_PIPE1_OVR);     // the HAL disables it on overrun
    }
}

void HAL_DCMIPP_ErrorCallback(DCMIPP_HandleTypeDef *h) {
    s_errors++;
    s_last_error = h->ErrorCode;
    h->ErrorCode = HAL_DCMIPP_ERROR_NONE;
}

bool cap_get(cap_frame_t *f, uint32_t timeout_ms) {
    uint32_t t0 = HAL_GetTick();
    while (s_q_tail == s_q_head) {
        if ((HAL_GetTick() - t0) >= timeout_ms) {
            return false;
        }
    }
    int idx = s_q[s_q_tail % CAP_NBUF];
    s_state[idx] = B_BUSY;
    __DMB();
    s_q_tail = s_q_tail + 1;
    // The DMA wrote behind the D-cache: drop any stale lines of this buffer.
    uint32_t size = ((uint32_t) s_w * s_h + 31u) & ~31u;
    SCB_InvalidateDCache_by_Addr((void *) s_fb[idx], (int32_t) size);
    f->data = s_fb[idx];
    f->w = s_w;
    f->h = s_h;
    f->ts_us = s_ts[idx];
    f->seq = s_seq[idx];
    f->idx = idx;
    return true;
}

void cap_release(const cap_frame_t *f) {
    if (f->idx >= 0 && f->idx < CAP_NBUF) {
        __DMB();
        s_state[f->idx] = B_FREE;
    }
}

void cap_flush(void) {
    if (s_q_tail == s_q_head) {
        return;
    }
    HAL_NVIC_DisableIRQ(DCMIPP_IRQn);
    while (s_q_tail != s_q_head) {
        int idx = s_q[s_q_tail % CAP_NBUF];
        s_state[idx] = B_FREE;
        s_q_tail = s_q_tail + 1;
    }
    HAL_NVIC_EnableIRQ(DCMIPP_IRQn);
}

void cap_get_stats(cap_stats_t *st) {
    st->frames = s_frames;
    st->dropped = s_dropped;
    st->errors = s_errors;
    st->last_error = s_last_error;
}
