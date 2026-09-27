/*
 * capture.h -- DCMIPP (CSI-2 PIPE1) raw capture into a ring of frame buffers.
 *
 * The camera DMA writes whole frames into AXISRAM1 buffers. The frame-end interrupt
 * timestamps each frame and hands it to the main loop in FIFO order; a frame is only
 * dropped (and counted) when every buffer is still queued or in use -- a buffer being
 * read is never overwritten.
 */
#ifndef CAPTURE_H
#define CAPTURE_H

#include <stdint.h>
#include <stdbool.h>

#define CAP_NBUF        4
#define CAP_MAX_W       320
#define CAP_MAX_H       240

typedef struct {
    const uint8_t *data;        // 8-bit raw Bayer, pitch = width
    uint16_t w, h;
    uint32_t ts_us;             // frame-end time (tb_us)
    uint32_t seq;               // frame number from the camera (counts dropped frames too)
    int      idx;               // buffer index, pass back to cap_release()
} cap_frame_t;

typedef struct {
    uint32_t frames;            // frame-end events
    uint32_t dropped;           // frames overwritten because no buffer was free
    uint32_t errors;            // DCMIPP / CSI error events
    uint32_t last_error;        // last HAL error code
} cap_stats_t;

int  cap_init(void);                        // DCMIPP + CSI host setup (clocks, RIF, IRQs)
int  cap_start(uint16_t w, uint16_t h);     // start continuous capture
void cap_stop(void);
// Wait up to timeout_ms for the next frame. Returns false on timeout.
bool cap_get(cap_frame_t *f, uint32_t timeout_ms);
void cap_release(const cap_frame_t *f);
void cap_flush(void);                       // drop all queued frames
void cap_get_stats(cap_stats_t *st);
bool cap_running(void);

#endif
