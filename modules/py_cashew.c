/*
 * py_cashew.c -- MicroPython module "cashew": C capture + detection loop for the
 * singulated-cashew camera, raw Bayer QVGA-class frames at up to ~250 fps.
 *
 * Supported cameras:
 *   - PAG7936 on the OpenMV AE3 (parallel bus), 320x200 QVGA.
 *   - ST VD66GY on the NUCLEO-N657X0-Q (MIPI CSI-2), 320 x height, optional 2x/4x binning.
 *
 *   import csi, cashew
 *   c = csi.CSI()                      # make sure the camera object exists
 *   cashew.setup(fps=240, expo_us=100, gain_db=6.0)
 *   cashew.param(threshold=240, mm_px=0.25)
 *   cashew.sweep(4166, 2000, 250)      # frame-time sweep
 *   cashew.run(frames=0, report=240)   # C loop; Ctrl-C (IDE stop) to exit
 *
 * The processing core (cashew_core.c) has no MicroPython dependencies.
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "py/mphal.h"
#include "py/runtime.h"
#include "board_config.h"

#if MICROPY_PY_CSI_NG

#include "omv_csi.h"
#include "imlib.h"
#include "py_image.h"
#include "cashew_core.h"
#include "umalloc.h"

// ---- state --------------------------------------------------------------------
#define TS_RING     64
#define KRING       64

typedef struct {
    uint32_t ts_us, frame;
    float cx, cy, len_mm, wid_mm, solidity, pos_mm, r, g, b;
    uint32_t area;
} kernel_rec_t;

static cz_params_t  s_p;
static bool         s_p_init;
static float        s_belt_mm_s = 3000.0f;
static int          s_dir = 1;
static cz_dedup_t   s_dd;
static uint32_t     s_fps_set = 240;

static volatile uint32_t s_ts_prod;
static uint32_t     s_ts_cons;
static volatile uint32_t s_ts_ring[TS_RING];

static kernel_rec_t s_kring[KRING];
static uint32_t     s_khead, s_kcount;

static const char *const status_name[CZ_NSTATUS] = {
    "ok", "edge", "small", "long", "aspect", "concave", "big", "neck", "crowded"
};

static void frame_cb(void *arg) {
    (void) arg;
    uint32_t i = s_ts_prod;
    s_ts_ring[i % TS_RING] = mp_hal_ticks_us();
    s_ts_prod = i + 1;
}

static omv_csi_t *get_csi(void) {
    omv_csi_t *csi = omv_csi_get(-1);
    if (!csi || !csi->detected) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("camera not detected"));
    }
    if (!csi->fb) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("create csi.CSI() first"));
    }
    return csi;
}

static void check(int err, const char *what) {
    if (err != 0) {
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("%s: %s"), what, omv_csi_strerror(err));
    }
}

// Workspace: the per-row hot block goes to DTCM when the port has a DTCM pool,
// the per-candidate warm block to fast SRAM. Re-allocated if the frame grows.
static void *s_hot, *s_warm;
static int s_ws_w, s_ws_h;
static size_t s_hot_size, s_warm_size;

static void *ws_alloc(size_t size, bool hot) {
    void *m = NULL;
    if (hot) {
        m = uma_malloc(size, UMA_DTCM | UMA_STRICT | UMA_PERSIST | UMA_MAYBE);
    }
    if (!m) {
        m = uma_malloc(size, UMA_FAST | UMA_PERSIST | UMA_MAYBE);
    }
    if (!m) {
        m = uma_malloc(size, UMA_PERSIST | UMA_MAYBE);
    }
    return m;
}

static void ensure_ws(int w, int h) {
    w = (w + 31) & ~31;
    if (cz_ws_ready() && s_hot && w <= s_ws_w && h <= s_ws_h) {
        return;
    }
    if (s_hot) { uma_free(s_hot); s_hot = NULL; }
    if (s_warm) { uma_free(s_warm); s_warm = NULL; }
    int runs = 20 * h;
    runs = runs < 4000 ? 4000 : runs > CZ_MAX_RUNS ? CZ_MAX_RUNS : runs;
    cz_dims_t d = { (uint16_t) w, (uint16_t) h, (uint16_t) runs,
                    (uint16_t) ((w * h <= 320 * 240) ? 512 : CZ_MAX_LABELS) };
    s_hot_size = cz_ws_hot_size(&d);
    s_warm_size = cz_ws_warm_size(&d);
    s_hot = ws_alloc(s_hot_size, true);
    s_warm = ws_alloc(s_warm_size, false);
    if (!s_hot || !s_warm || !cz_ws_init(&d, s_hot, s_warm)) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("cashew: no memory for work buffers"));
    }
    s_ws_w = w; s_ws_h = h;
}

static const char *mem_name(const void *p) {
    uintptr_t a = (uintptr_t) p;
    #if defined(OMV_DTCM_ORIGIN)
    if (a >= OMV_DTCM_ORIGIN && a < OMV_DTCM_ORIGIN + 0x40000) return "DTCM";
    #endif
    (void) a;
    return "SRAM";
}

static void ensure_params(int w, int h) {
    if (!s_p_init) {
        cz_default_params(&s_p, w, h);
        cz_dedup_init(&s_dd, 10.0f, 6.0f);
        s_p_init = true;
    }
}

static void pag_write(omv_csi_t *csi, uint16_t reg, uint8_t val) {
    omv_csi_write_reg(csi, reg, val);
}

static void pag_frame_time(omv_csi_t *csi, uint32_t us) {
    int h = omv_csi_read_reg(csi, 0x004E);
    pag_write(csi, 0x004C, us & 0xFF);
    pag_write(csi, 0x004D, (us >> 8) & 0xFF);
    pag_write(csi, 0x004E, (uint8_t) (((h < 0 ? 0 : h) & 0xE0) | ((us >> 16) & 0x1F)));
    pag_write(csi, 0x00EB, 0x80);   // commit
}

static bool is_pag7936(omv_csi_t *csi) {
    return csi->chip_id == 0x7936;
}

// Sensor frame time in microseconds: driver ioctl if supported (VD66GY), else PAG7936 registers.
static void set_frame_us(omv_csi_t *csi, uint32_t us) {
    if (omv_csi_ioctl(csi, OMV_CSI_IOCTL_SET_FRAME_TIME_US, (int) us) == 0) {
        return;
    }
    if (is_pag7936(csi)) {
        pag_frame_time(csi, us);
        return;
    }
    mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("sensor has no frame-time control"));
}

// ---- cashew.setup(fps=240, expo_us=100, gain_db=6.0, fb=4, denoise=False, lsc=True, clk_hz=0,
//                   height=0, bin=0, vblank=0, strobe_gpio=-1, strobe_inv=False)
// denoise/lsc/clk_hz: PAG7936 only. height/bin/vblank/strobe_*: VD66GY only.
static mp_obj_t py_cashew_setup(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_fps, ARG_expo, ARG_gain, ARG_fb, ARG_denoise, ARG_lsc, ARG_clk,
           ARG_height, ARG_bin, ARG_vblank, ARG_strobe, ARG_strobe_inv };
    static const mp_arg_t allowed[] = {
        { MP_QSTR_fps,     MP_ARG_INT | MP_ARG_KW_ONLY,  {.u_int = 240} },
        { MP_QSTR_expo_us, MP_ARG_INT | MP_ARG_KW_ONLY,  {.u_int = 100} },
        { MP_QSTR_gain_db, MP_ARG_OBJ | MP_ARG_KW_ONLY,  {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_fb,      MP_ARG_INT | MP_ARG_KW_ONLY,  {.u_int = 4} },
        { MP_QSTR_denoise, MP_ARG_BOOL | MP_ARG_KW_ONLY, {.u_bool = false} },
        { MP_QSTR_lsc,     MP_ARG_BOOL | MP_ARG_KW_ONLY, {.u_bool = true} },
        { MP_QSTR_clk_hz,  MP_ARG_INT | MP_ARG_KW_ONLY,  {.u_int = 0} },
        { MP_QSTR_height,  MP_ARG_INT | MP_ARG_KW_ONLY,  {.u_int = 0} },
        { MP_QSTR_bin,     MP_ARG_INT | MP_ARG_KW_ONLY,  {.u_int = 0} },
        { MP_QSTR_vblank,  MP_ARG_INT | MP_ARG_KW_ONLY,  {.u_int = 0} },
        { MP_QSTR_strobe_gpio, MP_ARG_INT | MP_ARG_KW_ONLY,  {.u_int = -1} },
        { MP_QSTR_strobe_inv,  MP_ARG_BOOL | MP_ARG_KW_ONLY, {.u_bool = false} },
    };
    mp_arg_val_t a[MP_ARRAY_SIZE(allowed)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed), allowed, a);

    omv_csi_t *csi = get_csi();
    float gain_db = (a[ARG_gain].u_obj == mp_const_none) ? 6.0f : mp_obj_get_float(a[ARG_gain].u_obj);

    const bool pag = is_pag7936(csi);
    const bool vd = (csi->chip_id == 0x5603);

    check(omv_csi_reset(csi, true), "reset");
    if (a[ARG_clk].u_int > 0) {
        check(omv_csi_set_clk_frequency(csi, a[ARG_clk].u_int), "clk");
    }
    check(omv_csi_set_pixformat(csi, PIXFORMAT_BAYER), "pixformat");
    if (vd) {
        // Output size 320 x height (even, <= CZ_MAX_H rows for the detector). The QVGA entry
        // of the resolution table is overridden until reboot. 320 x 200 = 80 x 50 mm field.
        int hh = a[ARG_height].u_int > 0 ? a[ARG_height].u_int : 200;
        if (hh < 32 || hh > CZ_MAX_H || (hh & 1)) {
            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("height must be even, 32..%d"), CZ_MAX_H);
        }
        csi->resolution[OMV_CSI_FRAMESIZE_QVGA][0] = 320;
        csi->resolution[OMV_CSI_FRAMESIZE_QVGA][1] = hh;
        check(omv_csi_ioctl(csi, OMV_CSI_IOCTL_VD66GY_SET_BINNING, (int) a[ARG_bin].u_int), "bin");
        if (a[ARG_vblank].u_int > 0) {
            check(omv_csi_ioctl(csi, OMV_CSI_IOCTL_VD66GY_SET_VBLANK, (int) a[ARG_vblank].u_int), "vblank");
        }
    }
    check(omv_csi_set_framesize(csi, OMV_CSI_FRAMESIZE_QVGA), "framesize");
    check(omv_csi_set_framerate(csi, a[ARG_fps].u_int), "framerate");
    check(omv_csi_set_auto_gain(csi, false, gain_db, NAN), "gain");
    check(omv_csi_set_auto_exposure(csi, false, a[ARG_expo].u_int), "exposure");
    // No ISP white-balance statistics: raw Bayer goes straight to the detector.
    omv_csi_set_auto_whitebal(csi, false, NAN, NAN, NAN);
    if (pag) {
        pag_write(csi, 0x0882, a[ARG_denoise].u_bool ? 0x03 : 0x00);
        pag_write(csi, 0x0810, a[ARG_lsc].u_bool ? 0x01 : 0x00);
        pag_write(csi, 0x00EB, 0x80);
    }
    if (vd && a[ARG_strobe].u_int >= 0) {
        // Sensor GPIO as strobe output (active during exposure), mode 2 in the VD6G GPIO control.
        int ctrl = 0x02 | (a[ARG_strobe_inv].u_bool ? 0x20 : 0x00);
        check(omv_csi_ioctl(csi, OMV_CSI_IOCTL_VD66GY_SET_GPIO, (int) a[ARG_strobe].u_int, ctrl), "strobe_gpio");
    }
    check(omv_csi_set_framebuffers(csi, a[ARG_fb].u_int), "framebuffers");

    omv_csi_cb_t cb = { .fun = frame_cb, .arg = NULL };
    omv_csi_set_frame_callback(csi, cb);
    s_fps_set = (uint32_t) a[ARG_fps].u_int;

    int w = csi->resolution[csi->framesize][0], h = csi->resolution[csi->framesize][1];
    ensure_params(w, h);
    ensure_ws(w, h);
    // Keep a user ROI that still fits; otherwise use the full frame.
    if (s_p.roi_x1 >= w || s_p.roi_y1 >= h || s_p.roi_x0 > s_p.roi_x1 || s_p.roi_y0 > s_p.roi_y1) {
        s_p.roi_x0 = 0; s_p.roi_y0 = 0;
        s_p.roi_x1 = (uint16_t) (w - 1); s_p.roi_y1 = (uint16_t) (h - 1);
    }
    if (vd) {
        s_p.cfa = (uint8_t) (csi->cfa_format & 3);   // GRBG unless mirrored/flipped
    }
    mp_hal_delay_ms(200);
    int expo_us = 0;
    omv_csi_get_exposure_us(csi, &expo_us);
    mp_printf(&mp_plat_print, "cashew.setup: %s %dx%d BAYER (cfa %d), %lu fps, expo %d us (asked %d), gain %.1f dB, fb %d\n",
              omv_csi_name(csi), w, h, (int) s_p.cfa, (unsigned long) s_fps_set, expo_us,
              (int) a[ARG_expo].u_int, (double) gain_db, (int) a[ARG_fb].u_int);
    mp_printf(&mp_plat_print, "  work buffers: hot %u B in %s, warm %u B in %s\n",
              (unsigned) s_hot_size, mem_name(s_hot), (unsigned) s_warm_size, mem_name(s_warm));
    if (vd) {
        int info[6] = { 0 };
        if (omv_csi_ioctl(csi, OMV_CSI_IOCTL_VD66GY_GET_INFO, info) == 0) {
            mp_printf(&mp_plat_print, "  VD66GY: line %d clk (%d ns), frame %d lines, rows %d, bin %d, min frame %d us (%.0f fps)\n",
                      info[0], info[4], info[1], info[2], info[3], info[5], (double) (info[5] ? 1e6f / info[5] : 0.0f));
            if (s_fps_set && (uint32_t) info[5] > 1000000u / s_fps_set) {
                mp_printf(&mp_plat_print, "  WARNING: %lu fps is not reachable with this height/binning/vblank; "
                          "use a smaller height, bin=1 or a lower vblank\n", (unsigned long) s_fps_set);
            }
        }
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_cashew_setup_obj, 0, py_cashew_setup);

// ---- cashew.param(**kw) -> dict of current parameters
static mp_obj_t py_cashew_param(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    (void) n_args; (void) pos_args;
    ensure_params(320, 200);
    for (size_t i = 0; kw_args && i < kw_args->alloc; i++) {
        if (!mp_map_slot_is_filled(kw_args, i)) continue;
        qstr k = mp_obj_str_get_qstr(kw_args->table[i].key);
        mp_obj_t v = kw_args->table[i].value;
        switch (k) {
            case MP_QSTR_threshold: s_p.threshold = (uint16_t) mp_obj_get_int(v); break;
            case MP_QSTR_invert:    s_p.invert = mp_obj_is_true(v); break;
            case MP_QSTR_filter:    s_p.filter = mp_obj_is_true(v); break;
            case MP_QSTR_edge_px:   s_p.edge_px = (uint8_t) mp_obj_get_int(v); break;
            case MP_QSTR_min_pix:   s_p.min_pix = (uint32_t) mp_obj_get_int(v); break;
            case MP_QSTR_max_pix:   s_p.max_pix = (uint32_t) mp_obj_get_int(v); break;
            case MP_QSTR_mm_px:     s_p.mm_px = mp_obj_get_float(v); break;
            case MP_QSTR_lmax_mm:   s_p.lmax_mm = mp_obj_get_float(v); break;
            case MP_QSTR_ar_max:    s_p.ar_max = mp_obj_get_float(v); break;
            case MP_QSTR_ar_min:    s_p.ar_min = mp_obj_get_float(v); break;
            case MP_QSTR_wmax_mm:   s_p.wmax_mm = mp_obj_get_float(v); break;
            case MP_QSTR_sol_min:   s_p.sol_min = mp_obj_get_float(v); break;
            case MP_QSTR_neck_px:   s_p.neck_px = (uint8_t) mp_obj_get_int(v); break;
            case MP_QSTR_gap_mm:    s_p.gap_mm = mp_obj_get_float(v); break;
            case MP_QSTR_crowd_min_pix: s_p.crowd_min_pix = (uint32_t) mp_obj_get_int(v); break;
            case MP_QSTR_cfa:       s_p.cfa = (uint8_t) mp_obj_get_int(v); break;
            case MP_QSTR_belt_mm_s: s_belt_mm_s = mp_obj_get_float(v); break;
            case MP_QSTR_dir:       s_dir = mp_obj_get_int(v) >= 0 ? 1 : -1; break;
            case MP_QSTR_dup_mm:    cz_dedup_init(&s_dd, mp_obj_get_float(v), s_dd.tol_y_mm); break;
            case MP_QSTR_dup_y_mm:  cz_dedup_init(&s_dd, s_dd.tol_mm, mp_obj_get_float(v)); break;
            case MP_QSTR_roi: {
                mp_obj_t *t; mp_obj_get_array_fixed_n(v, 4, &t);
                s_p.roi_x0 = (uint16_t) mp_obj_get_int(t[0]);
                s_p.roi_y0 = (uint16_t) mp_obj_get_int(t[1]);
                s_p.roi_x1 = (uint16_t) (mp_obj_get_int(t[0]) + mp_obj_get_int(t[2]) - 1);
                s_p.roi_y1 = (uint16_t) (mp_obj_get_int(t[1]) + mp_obj_get_int(t[3]) - 1);
                break;
            }
            default:
                mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("unknown param %q"), k);
        }
    }
    mp_obj_t d = mp_obj_new_dict(28);
    #define PUT(name, obj) mp_obj_dict_store(d, MP_OBJ_NEW_QSTR(MP_QSTR_ ## name), obj)
    PUT(threshold, mp_obj_new_int(s_p.threshold));
    PUT(invert, mp_obj_new_bool(s_p.invert));
    PUT(filter, mp_obj_new_bool(s_p.filter));
    PUT(edge_px, mp_obj_new_int(s_p.edge_px));
    PUT(min_pix, mp_obj_new_int(s_p.min_pix));
    PUT(max_pix, mp_obj_new_int(s_p.max_pix));
    PUT(mm_px, mp_obj_new_float(s_p.mm_px));
    PUT(lmax_mm, mp_obj_new_float(s_p.lmax_mm));
    PUT(ar_max, mp_obj_new_float(s_p.ar_max));
    PUT(ar_min, mp_obj_new_float(s_p.ar_min));
    PUT(wmax_mm, mp_obj_new_float(s_p.wmax_mm));
    PUT(sol_min, mp_obj_new_float(s_p.sol_min));
    PUT(neck_px, mp_obj_new_int(s_p.neck_px));
    PUT(gap_mm, mp_obj_new_float(s_p.gap_mm));
    PUT(crowd_min_pix, mp_obj_new_int(s_p.crowd_min_pix));
    PUT(cfa, mp_obj_new_int(s_p.cfa));
    PUT(belt_mm_s, mp_obj_new_float(s_belt_mm_s));
    PUT(dir, mp_obj_new_int(s_dir));
    PUT(dup_mm, mp_obj_new_float(s_dd.tol_mm));
    PUT(dup_y_mm, mp_obj_new_float(s_dd.tol_y_mm));
    mp_obj_t roi[4] = {
        mp_obj_new_int(s_p.roi_x0), mp_obj_new_int(s_p.roi_y0),
        mp_obj_new_int(s_p.roi_x1 - s_p.roi_x0 + 1), mp_obj_new_int(s_p.roi_y1 - s_p.roi_y0 + 1)
    };
    PUT(roi, mp_obj_new_tuple(4, roi));
    #undef PUT
    return d;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_cashew_param_obj, 0, py_cashew_param);

// ---- cashew.frame_us(us): write sensor frame-time register directly
static mp_obj_t py_cashew_frame_us(mp_obj_t us_in) {
    omv_csi_t *csi = get_csi();
    uint32_t us = (uint32_t) mp_obj_get_int(us_in);
    if (us < 500) {
        mp_raise_ValueError(MP_ERROR_TEXT("frame_us too small"));
    }
    set_frame_us(csi, us);
    s_fps_set = 1000000u / us;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_cashew_frame_us_obj, py_cashew_frame_us);

// Measure n frames: returns fps (from IRQ timestamps), min/max frame interval, errors.
static void measure(omv_csi_t *csi, int n, float *fps, uint32_t *dmin, uint32_t *dmax, int *errs) {
    image_t img;
    *errs = 0; *dmin = 0xFFFFFFFF; *dmax = 0;
    omv_csi_abort(csi, true, false);          // drop queued frames so timestamps pair 1:1
    s_ts_cons = s_ts_prod;
    uint32_t first = 0, last = 0, prev = 0;
    int got = 0;
    for (int i = 0; i < n; i++) {
        if (omv_csi_snapshot(csi, &img, 0) != 0) { (*errs)++; continue; }
        while (s_ts_cons < s_ts_prod) {
            uint32_t t = s_ts_ring[s_ts_cons % TS_RING];
            s_ts_cons++;
            if (got == 0) first = t;
            else {
                uint32_t d = t - prev;
                if (d < *dmin) *dmin = d;
                if (d > *dmax) *dmax = d;
            }
            prev = last = t;
            got++;
        }
        mp_handle_pending(true);
    }
    *fps = (got > 1) ? (float) (got - 1) * 1e6f / (float) (last - first) : 0.0f;
}

// ---- cashew.sweep(start_us, stop_us, step_us, frames=200) -> list of (frame_us, fps, dmin, dmax, errors)
static mp_obj_t py_cashew_sweep(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_start, ARG_stop, ARG_step, ARG_frames };
    static const mp_arg_t allowed[] = {
        { MP_QSTR_start,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 4166} },
        { MP_QSTR_stop,   MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 2000} },
        { MP_QSTR_step,   MP_ARG_INT, {.u_int = 250} },
        { MP_QSTR_frames, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = 200} },
    };
    mp_arg_val_t a[MP_ARRAY_SIZE(allowed)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed), allowed, a);
    omv_csi_t *csi = get_csi();
    int step = a[ARG_step].u_int > 0 ? a[ARG_step].u_int : 250;
    mp_obj_t list = mp_obj_new_list(0, NULL);
    mp_printf(&mp_plat_print, "frame_us   target_fps   measured_fps   dt_min..dt_max us   errors\n");
    for (int us = a[ARG_start].u_int; us >= a[ARG_stop].u_int; us -= step) {
        set_frame_us(csi, (uint32_t) us);
        mp_hal_delay_ms(50);
        float fps; uint32_t dmin, dmax; int errs;
        measure(csi, a[ARG_frames].u_int, &fps, &dmin, &dmax, &errs);
        mp_printf(&mp_plat_print, "%6d     %7.1f      %7.1f        %5lu..%-5lu        %d\n",
                  us, (double) (1e6f / us), (double) fps, (unsigned long) dmin, (unsigned long) dmax, errs);
        mp_obj_t t[5] = { mp_obj_new_int(us), mp_obj_new_float(fps), mp_obj_new_int(dmin),
                          mp_obj_new_int(dmax), mp_obj_new_int(errs) };
        mp_obj_list_append(list, mp_obj_new_tuple(5, t));
    }
    set_frame_us(csi, 1000000u / (s_fps_set ? s_fps_set : 240));   // restore
    return list;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_cashew_sweep_obj, 2, py_cashew_sweep);

// ---- blob -> tuple
static mp_obj_t blob_tuple(const cz_blob_t *b) {
    mp_obj_t t[12] = {
        MP_OBJ_NEW_QSTR(qstr_from_str(status_name[b->status])),
        mp_obj_new_int(b->area),
        mp_obj_new_float(b->cx), mp_obj_new_float(b->cy),
        mp_obj_new_float(b->len_mm), mp_obj_new_float(b->wid_mm),
        mp_obj_new_float(b->angle_deg), mp_obj_new_float(b->solidity),
        mp_obj_new_float(b->r), mp_obj_new_float(b->g), mp_obj_new_float(b->b),
        mp_obj_new_tuple(4, (mp_obj_t[4]) { mp_obj_new_int(b->x0), mp_obj_new_int(b->y0),
                                            mp_obj_new_int(b->x1 - b->x0 + 1), mp_obj_new_int(b->y1 - b->y0 + 1) }),
    };
    return mp_obj_new_tuple(12, t);
}

// ---- cashew.process(img) -> list of (status, area, cx, cy, len_mm, wid_mm, angle, solidity, r, g, b, rect)
// img must be a raw BAYER image (e.g. from csi.snapshot() with pixformat BAYER).
static mp_obj_t py_cashew_process(mp_obj_t img_obj) {
    image_t *img = (image_t *) py_image_cobj(img_obj);
    ensure_params(img->w, img->h);
    ensure_ws(img->w, img->h);
    if (s_p.roi_x1 >= img->w) s_p.roi_x1 = (uint16_t) (img->w - 1);
    if (s_p.roi_y1 >= img->h) s_p.roi_y1 = (uint16_t) (img->h - 1);
    static cz_blob_t out[CZ_MAX_OUT];
    cz_frame_t fi;
    uint32_t t0 = mp_hal_ticks_us();
    int n = cz_process(img->data, img->w, img->h, &s_p, out, CZ_MAX_OUT, &fi);
    uint32_t t1 = mp_hal_ticks_us();
    if (n < 0) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("image must be 8-bit, width multiple of 32"));
    }
    mp_obj_t list = mp_obj_new_list(0, NULL);
    for (int i = 0; i < n; i++) mp_obj_list_append(list, blob_tuple(&out[i]));
    mp_printf(&mp_plat_print, "process: %lu us, %lu blobs, %lu runs\n",
              (unsigned long) (t1 - t0), (unsigned long) fi.nblobs, (unsigned long) fi.nruns);
    return list;
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_cashew_process_obj, py_cashew_process);

// ---- cashew.run(frames=0, report=240, verbose=0) -> dict of totals
static mp_obj_t py_cashew_run(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_frames, ARG_report, ARG_verbose };
    static const mp_arg_t allowed[] = {
        { MP_QSTR_frames,  MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = 0} },
        { MP_QSTR_report,  MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = 240} },
        { MP_QSTR_verbose, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = 0} },
    };
    mp_arg_val_t a[MP_ARRAY_SIZE(allowed)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed), allowed, a);
    omv_csi_t *csi = get_csi();
    int w = csi->resolution[csi->framesize][0], h = csi->resolution[csi->framesize][1];
    ensure_params(w, h);
    ensure_ws(w, h);
    if (s_p.roi_x1 >= w) s_p.roi_x1 = (uint16_t) (w - 1);
    if (s_p.roi_y1 >= h) s_p.roi_y1 = (uint16_t) (h - 1);

    const int report = a[ARG_report].u_int > 0 ? a[ARG_report].u_int : 240;
    const int verbose = a[ARG_verbose].u_int;
    const uint32_t period = s_fps_set ? 1000000u / s_fps_set : 4166u;
    static cz_blob_t out[CZ_MAX_OUT];

    // totals
    uint32_t tot_frames = 0, tot_errs = 0, tot_lost = 0, tot_accept = 0, tot_dup = 0;
    uint32_t tot_status[CZ_NSTATUS] = {0};
    // window stats
    uint32_t n = 0, wait_sum = 0, proc_sum = 0, proc_max = 0, errs = 0, lost = 0, accept = 0, dup = 0;
    uint32_t st[CZ_NSTATUS] = {0};
    uint32_t t_first = 0, t_last = 0, prev_ts = 0;
    bool have_prev = false;

    image_t img;
    omv_csi_abort(csi, true, false);           // drop queued frames so timestamps pair 1:1
    s_ts_cons = s_ts_prod;
    mp_printf(&mp_plat_print, "cashew.run: %dx%d, report every %d frames. Stop from IDE to exit.\n", w, h, report);

    for (;;) {
        uint32_t t0 = mp_hal_ticks_us();
        int e = omv_csi_snapshot(csi, &img, 0);
        uint32_t t1 = mp_hal_ticks_us();
        if (e != 0) {
            errs++; tot_errs++;
            if (verbose) mp_printf(&mp_plat_print, "snapshot error: %s\n", omv_csi_strerror(e));
            mp_handle_pending(true);
            continue;
        }
        // frame timestamp from the capture-complete IRQ (FIFO order)
        uint32_t ts = t1;
        if (s_ts_cons < s_ts_prod) {
            if (s_ts_prod - s_ts_cons > TS_RING) s_ts_cons = s_ts_prod - 1;
            ts = s_ts_ring[s_ts_cons % TS_RING];
            s_ts_cons++;
        }
        if (have_prev) {
            uint32_t d = ts - prev_ts;
            if (d > period + period / 2) {
                uint32_t l = (d + period / 2) / period - 1;
                lost += l; tot_lost += l;
            }
        }
        prev_ts = ts; have_prev = true;
        if (n == 0) t_first = ts;
        t_last = ts;

        cz_frame_t fi;
        int nb = cz_process(img.data, img.w, img.h, &s_p, out, CZ_MAX_OUT, &fi);
        uint32_t t2 = mp_hal_ticks_us();

        for (int s = 0; s < CZ_NSTATUS; s++) { st[s] += fi.count[s]; tot_status[s] += fi.count[s]; }
        for (int i = 0; i < nb; i++) {
            cz_blob_t *b = &out[i];
            if (b->status != CZ_OK) {
                if (verbose >= 2) {
                    mp_printf(&mp_plat_print, "  f%lu %s A=%lu L=%.1f W=%.1f sol=%.2f\n", (unsigned long) tot_frames,
                              status_name[b->status], (unsigned long) b->area, (double) b->len_mm,
                              (double) b->wid_mm, (double) b->solidity);
                }
                continue;
            }
            // belt coordinate: constant for one kernel across frames
            float pos = (float) s_dir * b->cx * s_p.mm_px - s_belt_mm_s * (float) ts * 1e-6f;
            if (!cz_dedup_first(&s_dd, pos, b->cy * s_p.mm_px)) { dup++; tot_dup++; continue; }
            accept++; tot_accept++;
            kernel_rec_t *k = &s_kring[s_khead];
            k->ts_us = ts; k->frame = tot_frames; k->cx = b->cx; k->cy = b->cy;
            k->len_mm = b->len_mm; k->wid_mm = b->wid_mm; k->solidity = b->solidity;
            k->pos_mm = pos; k->r = b->r; k->g = b->g; k->b = b->b; k->area = b->area;
            s_khead = (s_khead + 1) % KRING;
            if (s_kcount < KRING) s_kcount++;
            if (verbose >= 1) {
                mp_printf(&mp_plat_print, "  K f%lu x=%.0f y=%.0f L=%.1f W=%.1f A=%.0fmm2 sol=%.2f rgb=%.0f/%.0f/%.0f\n",
                          (unsigned long) tot_frames, (double) b->cx, (double) b->cy, (double) b->len_mm,
                          (double) b->wid_mm, (double) (b->area * s_p.mm_px * s_p.mm_px), (double) b->solidity,
                          (double) b->r, (double) b->g, (double) b->b);
            }
        }

        uint32_t pt = t2 - t1;
        wait_sum += t1 - t0; proc_sum += pt; if (pt > proc_max) proc_max = pt;
        n++; tot_frames++;

        if (n >= (uint32_t) report) {
            float fps = (t_last != t_first) ? (float) (n - 1) * 1e6f / (float) (t_last - t_first) : 0.0f;
            mp_printf(&mp_plat_print,
                      "fps %.1f | proc avg %lu max %lu us | wait avg %lu us | lost %lu err %lu | "
                      "kernels %lu dup %lu | edge %lu long %lu aspect %lu concave %lu neck %lu big %lu crowded %lu\n",
                      (double) fps, (unsigned long) (proc_sum / n), (unsigned long) proc_max,
                      (unsigned long) (wait_sum / n), (unsigned long) lost, (unsigned long) errs,
                      (unsigned long) accept, (unsigned long) dup,
                      (unsigned long) st[CZ_EDGE], (unsigned long) st[CZ_LONG], (unsigned long) st[CZ_ASPECT],
                      (unsigned long) st[CZ_CONCAVE], (unsigned long) st[CZ_NECK], (unsigned long) st[CZ_BIG],
                      (unsigned long) st[CZ_CROWDED]);
            n = wait_sum = proc_sum = proc_max = errs = lost = accept = dup = 0;
            memset(st, 0, sizeof(st));
        }
        if (a[ARG_frames].u_int > 0 && tot_frames >= (uint32_t) a[ARG_frames].u_int) break;
        mp_handle_pending(true);
    }

    mp_obj_t d = mp_obj_new_dict(8);
    mp_obj_dict_store(d, MP_OBJ_NEW_QSTR(MP_QSTR_frames), mp_obj_new_int(tot_frames));
    mp_obj_dict_store(d, MP_OBJ_NEW_QSTR(MP_QSTR_errors), mp_obj_new_int(tot_errs));
    mp_obj_dict_store(d, MP_OBJ_NEW_QSTR(MP_QSTR_lost), mp_obj_new_int(tot_lost));
    mp_obj_dict_store(d, MP_OBJ_NEW_QSTR(MP_QSTR_kernels), mp_obj_new_int(tot_accept));
    mp_obj_dict_store(d, MP_OBJ_NEW_QSTR(MP_QSTR_dup), mp_obj_new_int(tot_dup));
    for (int s = 1; s < CZ_NSTATUS; s++) {
        mp_obj_dict_store(d, MP_OBJ_NEW_QSTR(qstr_from_str(status_name[s])), mp_obj_new_int(tot_status[s]));
    }
    return d;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_cashew_run_obj, 0, py_cashew_run);

// ---- cashew.kernels(clear=True) -> list of (ts_us, frame, cx, cy, len, wid, area_mm2, sol, r, g, b)
static mp_obj_t py_cashew_kernels(size_t n_args, const mp_obj_t *args) {
    bool clear = (n_args < 1) || mp_obj_is_true(args[0]);
    mp_obj_t list = mp_obj_new_list(0, NULL);
    uint32_t start = (s_khead + KRING - s_kcount) % KRING;
    for (uint32_t i = 0; i < s_kcount; i++) {
        kernel_rec_t *k = &s_kring[(start + i) % KRING];
        mp_obj_t t[11] = {
            mp_obj_new_int_from_uint(k->ts_us), mp_obj_new_int(k->frame),
            mp_obj_new_float(k->cx), mp_obj_new_float(k->cy),
            mp_obj_new_float(k->len_mm), mp_obj_new_float(k->wid_mm),
            mp_obj_new_float(k->area * s_p.mm_px * s_p.mm_px), mp_obj_new_float(k->solidity),
            mp_obj_new_float(k->r), mp_obj_new_float(k->g), mp_obj_new_float(k->b),
        };
        mp_obj_list_append(list, mp_obj_new_tuple(11, t));
    }
    if (clear) { s_kcount = 0; }
    return list;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_cashew_kernels_obj, 0, 1, py_cashew_kernels);

// ---- cashew.mask(y) -> bytes of the filtered mask row (bit-packed), for debugging
static mp_obj_t py_cashew_mask(mp_obj_t y_in) {
    int y = mp_obj_get_int(y_in);
    const uint32_t *row = cz_mask_row(y);
    if (!row) return mp_const_none;
    return mp_obj_new_bytes((const byte *) row, (CZ_MAX_W + 31) / 32 * 4);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_cashew_mask_obj, py_cashew_mask);

static const mp_rom_map_elem_t cashew_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),  MP_ROM_QSTR(MP_QSTR_cashew) },
    { MP_ROM_QSTR(MP_QSTR_setup),     MP_ROM_PTR(&py_cashew_setup_obj) },
    { MP_ROM_QSTR(MP_QSTR_param),     MP_ROM_PTR(&py_cashew_param_obj) },
    { MP_ROM_QSTR(MP_QSTR_frame_us),  MP_ROM_PTR(&py_cashew_frame_us_obj) },
    { MP_ROM_QSTR(MP_QSTR_sweep),     MP_ROM_PTR(&py_cashew_sweep_obj) },
    { MP_ROM_QSTR(MP_QSTR_process),   MP_ROM_PTR(&py_cashew_process_obj) },
    { MP_ROM_QSTR(MP_QSTR_run),       MP_ROM_PTR(&py_cashew_run_obj) },
    { MP_ROM_QSTR(MP_QSTR_kernels),   MP_ROM_PTR(&py_cashew_kernels_obj) },
    { MP_ROM_QSTR(MP_QSTR_mask),      MP_ROM_PTR(&py_cashew_mask_obj) },
};
static MP_DEFINE_CONST_DICT(cashew_globals, cashew_globals_table);

const mp_obj_module_t cashew_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *) &cashew_globals,
};
MP_REGISTER_MODULE(MP_QSTR_cashew, cashew_user_cmodule);

#endif // MICROPY_PY_CSI_NG
