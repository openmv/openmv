/*
 * cashew_core.h -- portable per-frame kernel detection for raw Bayer frames.
 *
 * Pipeline per frame (no MicroPython / HAL dependencies, host-testable):
 *   raw Bayer -> 2x2 sliding luma (phase independent) -> threshold -> bit mask
 *   -> 3x3 majority filter (replicate padding) -> run extraction
 *   -> 8-connected run labelling (union-find) with running moments
 *   -> per-blob: 4-edge touch flag, area, ellipse length/width, hull solidity,
 *      per-channel colour means -> accept / reject status.
 *
 * Coordinates: x = sensor column (along belt travel), y = sensor row (across belt).
 */
#ifndef CASHEW_CORE_H
#define CASHEW_CORE_H

#include <stdint.h>
#include <stdbool.h>

#define CZ_MAX_W        656     // up to VGA width (+margin), multiple of 16
#define CZ_MAX_H        416
#define CZ_MAX_RUNS     12000   // runs per frame
#define CZ_MAX_LABELS   2048    // provisional labels per frame
#define CZ_MAX_CAND     24      // blobs examined in detail per frame
#define CZ_MAX_OUT      24      // blobs reported per frame

typedef enum {
    CZ_OK = 0,          // valid single kernel, fully inside ROI
    CZ_EDGE,            // touches one of the 4 ROI edges (overpass / retry next frame)
    CZ_SMALL,           // below min_pix (dust, fragments)
    CZ_LONG,            // length > lmax * 1.1        (touching pair)
    CZ_ASPECT,          // L/W outside [ar_min, ar_max] or W > wmax (touching pair)
    CZ_CONCAVE,         // solidity < sol_min         (touching pair)
    CZ_BIG,             // area > max_pix             (touching group)
    CZ_NECK,            // splits into >= 2 pieces after neck_px erosion (touching pair)
    CZ_CROWDED,         // another object overlaps its span along travel (+ gap): not ejectable alone
    CZ_NSTATUS
} cz_status_t;

typedef enum { CZ_BGGR = 0, CZ_GBRG, CZ_GRBG, CZ_RGGB } cz_cfa_t;

typedef struct {
    uint16_t threshold;     // on 2x2 luma sum (0..1020)
    bool     invert;        // true: kernels darker than belt
    bool     filter;        // 3x3 majority filter on/off
    uint8_t  edge_px;       // foreground within this many px of ROI edge => touching
    uint16_t roi_x0, roi_y0, roi_x1, roi_y1;   // inclusive ROI in frame px
    uint32_t min_pix;
    uint32_t max_pix;
    float    mm_px;         // mm per pixel
    float    lmax_mm;       // longest valid single kernel
    float    ar_max;        // max length / width   (end-to-end pairs)
    float    ar_min;        // min length / width   (side-by-side pairs look round)
    float    wmax_mm;       // max width of a single kernel
    float    sol_min;       // min area / convex-hull area
    uint8_t  neck_px;       // max erosion depth for the neck (touching) split test, 0 = off
    float    gap_mm;        // min clear belt gap (along travel) to any other object; < 0 = off
    uint32_t crowd_min_pix; // objects smaller than this (dust) do not count as neighbours
    uint8_t  cfa;           // cz_cfa_t of pixel (0,0)
} cz_params_t;

typedef struct {
    uint8_t  status;        // cz_status_t
    uint16_t x0, y0, x1, y1;// bounding box
    uint32_t area;          // pixels
    float    cx, cy;        // centroid (px)
    float    len_mm, wid_mm;// ellipse-equivalent major / minor axis
    float    angle_deg;     // major axis angle vs x (travel) axis
    float    solidity;      // area / hull area (0 if not computed)
    float    r, g, b;       // mean raw value per channel over blob pixels (0 if not computed)
} cz_blob_t;

typedef struct {
    uint32_t nruns;
    uint32_t nblobs;        // all connected components
    uint32_t nout;          // entries written to out[]
    uint32_t count[CZ_NSTATUS];
    uint32_t overflow;      // run/label/candidate table overflows (should stay 0)
} cz_frame_t;

#include <stddef.h>

// Work buffers (~430 KB) are supplied by the caller once: cz_ws_set(malloc(cz_ws_size())).
size_t cz_ws_size(void);
void   cz_ws_set(void *mem);   // memory must be zeroed
bool   cz_ws_ready(void);

void cz_default_params(cz_params_t *p, int w, int h);

// Process one raw 8-bit Bayer frame of w x h (w multiple of 32, <= CZ_MAX_W).
// Returns number of blobs written to out (<= max_out); fills fi.
int cz_process(const uint8_t *raw, int w, int h, const cz_params_t *p,
               cz_blob_t *out, int max_out, cz_frame_t *fi);

// Access the filtered mask of the last processed frame (bit-packed, LSB = lowest x).
const uint32_t *cz_mask_row(int y);

// ---- de-duplication by belt position ------------------------------------
// pos_mm: kernel belt coordinate (constant for one kernel across frames).
// Returns true if this is the first sighting (and remembers it).
// A kernel is a repeat sighting only if BOTH its belt position (along travel) and its
// across-belt position match a remembered kernel -- so several valid kernels in one
// frame, including side-by-side ones, are all kept.
#define CZ_DEDUP_N  32
typedef struct { float pos[CZ_DEDUP_N], y[CZ_DEDUP_N]; int head; float tol_mm, tol_y_mm; } cz_dedup_t;
void cz_dedup_init(cz_dedup_t *d, float tol_mm, float tol_y_mm);
bool cz_dedup_first(cz_dedup_t *d, float pos_mm, float y_mm);

#endif
