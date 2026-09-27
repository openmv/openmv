/*
 * cashew_app.c -- singulated-cashew detection loop and serial command interface
 * (standalone version of the OpenMV "cashew" module: same detector, same report lines).
 *
 * Memory: the detector's per-row work buffers (masks, runs, labels, union-find) live in
 * DTCM, its code in ITCM; the per-candidate buffers (hull, neck test) in AXISRAM2; frames
 * in AXISRAM1. This file runs from ITCM (see the linker script).
 *
 * Commands (115200..921600 baud, CR or LF terminated) -- type "help".
 */
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "cashew_app.h"
#include "cashew_core.h"
#include "camera_vd66gy.h"
#include "capture.h"
#include "timebase.h"
#include "uart_log.h"

#define FW_VERSION      "cashew-n6 standalone 1.0"

// ---- detector work buffers ----------------------------------------------------------------
// Sized for the largest frame this firmware captures (CAP_MAX_W x CAP_MAX_H).
#define WS_RUNS         4800
#define WS_LABELS       512
#define HOT_BYTES       (93u * 1024u)   // >= cz_ws_hot_size() for 320x240 (92,608 B)
#define WARM_BYTES      (64u * 1024u)   // >= cz_ws_warm_size() for 320x240 (60,296 B)

static uint8_t s_hot[HOT_BYTES] __attribute__((aligned(32)));                           // DTCM (.bss)
static uint8_t s_warm[WARM_BYTES] __attribute__((section(".sram_bss"), aligned(32)));   // AXISRAM2

// ---- state --------------------------------------------------------------------------------
static cz_params_t s_p;
static cz_dedup_t  s_dd;
static cz_blob_t   s_out[CZ_MAX_OUT];
static cam_cfg_t   s_cam;
static float       s_belt_mm_s = 3000.0f;
static int         s_dir = 1;
static bool        s_cam_ok;
static bool        s_running;
static uint32_t    s_run_limit;     // frames to process, 0 = forever
static int         s_verbose = 1;
static uint32_t    s_report = 240;

static const char *const status_name[CZ_NSTATUS] = {
    "ok", "edge", "small", "long", "aspect", "concave", "big", "neck", "crowded"
};

// Run statistics.
typedef struct {
    uint32_t n, wait_sum, proc_sum, proc_max, lost, accept, dup, timeouts;
    uint32_t st[CZ_NSTATUS];
    uint32_t t_first, t_last;
} win_t;

static win_t    s_win;
static uint32_t s_tot_frames, s_tot_lost, s_tot_accept, s_tot_dup;
static uint32_t s_prev_seq;
static bool     s_have_prev;
static bool     s_kernel_pin;
static uint32_t s_epoch_us;         // time origin of the belt coordinate (moves forward)

// Seconds since the belt-coordinate epoch. The epoch is moved forward every 10 s (and the
// remembered kernel positions shifted to match), so float precision stays at micrometres
// and the 71-minute wrap of the microsecond clock never reaches the coordinate.
static float belt_time_s(uint32_t ts) {
    uint32_t d = ts - s_epoch_us;
    if (d > 10000000u) {
        float shift = s_belt_mm_s * (float) d * 1e-6f;   // pos = x - v*t; t drops by d
        for (int i = 0; i < CZ_DEDUP_N; i++) {
            if (s_dd.pos[i] > -1e29f) {
                s_dd.pos[i] += shift;
            }
        }
        s_epoch_us = ts;
        d = 0;
    }
    return (float) d * 1e-6f;
}

// ---- small helpers -------------------------------------------------------------------------
static void led(GPIO_TypeDef *port, uint16_t pin, bool on) {
    HAL_GPIO_WritePin(port, pin, on ? GPIO_PIN_RESET : GPIO_PIN_SET);     // active low
}

static bool parse_float(const char *s, float *out) {
    if (!s || !*s) {
        return false;
    }
    float sign = 1.0f, v = 0.0f, scale = 1.0f;
    bool digits = false, frac = false;
    if (*s == '-' || *s == '+') {
        sign = (*s == '-') ? -1.0f : 1.0f;
        s++;
    }
    for (; *s; s++) {
        if (*s >= '0' && *s <= '9') {
            digits = true;
            if (frac) {
                scale *= 0.1f;
                v += (float) (*s - '0') * scale;
            } else {
                v = v * 10.0f + (float) (*s - '0');
            }
        } else if (*s == '.' && !frac) {
            frac = true;
        } else {
            return false;
        }
    }
    *out = sign * v;
    return digits;
}

static bool parse_int(const char *s, int32_t *out) {
    float f;
    if (!parse_float(s, &f)) {
        return false;
    }
    *out = (int32_t) (f < 0 ? f - 0.5f : f + 0.5f);
    return true;
}

static bool parse_bool(const char *s, bool *out) {
    if (!strcmp(s, "1") || !strcmp(s, "on") || !strcmp(s, "true") || !strcmp(s, "yes")) {
        *out = true;
        return true;
    }
    if (!strcmp(s, "0") || !strcmp(s, "off") || !strcmp(s, "false") || !strcmp(s, "no")) {
        *out = false;
        return true;
    }
    return false;
}

static void clamp_roi(void) {
    if (s_p.roi_x1 >= s_cam.width) {
        s_p.roi_x1 = (uint16_t) (s_cam.width - 1);
    }
    if (s_p.roi_y1 >= s_cam.height) {
        s_p.roi_y1 = (uint16_t) (s_cam.height - 1);
    }
}

static void reset_window(void) {
    memset(&s_win, 0, sizeof(s_win));
}

static void print_cam_info(void) {
    cam_info_t ci;
    cam_get_info(&ci);
    ulog_printf("VD66GY: model 0x%04lx %s, %ux%u bin %u (sensor rows %u), line %u clk (%lu ns), frame %u lines = %lu us,"
                " min frame %lu us (%.0f fps), expo %lu us, gain code %u\r\n",
                (unsigned long) ci.model, ci.mono ? "mono" : "colour", s_cam.width, s_cam.height, ci.bin, ci.rows,
                ci.line_len, (unsigned long) ci.line_ns, ci.frame_len, (unsigned long) ci.frame_us,
                (unsigned long) ci.min_frame_us, (double) (ci.min_frame_us ? 1e6f / (float) ci.min_frame_us : 0.0f),
                (unsigned long) ci.expo_us, ci.again);
    if (s_cam.frame_us < ci.min_frame_us) {
        ulog_printf("WARNING: frame time %lu us is below the sensor minimum; use a smaller height, bin 1 or a lower vblank\r\n",
                    (unsigned long) s_cam.frame_us);
    }
}

static void print_params(void) {
    ulog_printf("detector: threshold %u invert %u filter %u edge_px %u min_pix %lu max_pix %lu mm_px %.4f\r\n",
                s_p.threshold, s_p.invert, s_p.filter, s_p.edge_px, (unsigned long) s_p.min_pix,
                (unsigned long) s_p.max_pix, (double) s_p.mm_px);
    ulog_printf("  lmax_mm %.1f ar_max %.2f ar_min %.2f wmax_mm %.1f sol_min %.2f neck_px %u gap_mm %.1f crowd_min_pix %lu cfa %u\r\n",
                (double) s_p.lmax_mm, (double) s_p.ar_max, (double) s_p.ar_min, (double) s_p.wmax_mm,
                (double) s_p.sol_min, s_p.neck_px, (double) s_p.gap_mm, (unsigned long) s_p.crowd_min_pix, s_p.cfa);
    ulog_printf("  roi %u %u %u %u  belt_mm_s %.0f dir %d dup_mm %.1f dup_y_mm %.1f\r\n",
                s_p.roi_x0, s_p.roi_y0, s_p.roi_x1 - s_p.roi_x0 + 1, s_p.roi_y1 - s_p.roi_y0 + 1,
                (double) s_belt_mm_s, s_dir, (double) s_dd.tol_mm, (double) s_dd.tol_y_mm);
    ulog_printf("camera: height %u bin %u fps %.1f (frame_us %lu) expo %lu gain %.1f vblank %u strobe %d inv %u mirror %u flip %u colorbar %u\r\n",
                s_cam.height, s_cam.bin, (double) (1e6f / (float) s_cam.frame_us), (unsigned long) s_cam.frame_us,
                (unsigned long) s_cam.expo_us, (double) s_cam.gain_db, s_cam.vblank, s_cam.strobe_gpio,
                s_cam.strobe_inv, s_cam.hmirror, s_cam.vflip, s_cam.colorbar);
    ulog_printf("output: verbose %d report %lu\r\n", s_verbose, (unsigned long) s_report);
}

extern uint8_t _sitcm[], _eitcm[], _sdata[], _ebss[], _sstack[], _estack[];

static void print_memory(void) {
    ulog_printf("memory: ITCM code %lu B used of 64 KB | DTCM data+bss %lu B + stack %lu B of 128 KB\r\n",
                (unsigned long) (_eitcm - _sitcm), (unsigned long) (_ebss - _sdata),
                (unsigned long) (_estack - _sstack));
    ulog_printf("  detector hot buffers %lu B at 0x%08lx (DTCM), warm %lu B at 0x%08lx (AXISRAM2), cz_process at 0x%08lx\r\n",
                (unsigned long) HOT_BYTES, (unsigned long) (uintptr_t) s_hot, (unsigned long) WARM_BYTES,
                (unsigned long) (uintptr_t) s_warm, (unsigned long) (uintptr_t) &cz_process);
}

// ---- camera (re)configuration -------------------------------------------------------------
static int apply_camera(void) {
    cap_stop();
    int e = cam_configure(&s_cam);
    if (e != 0) {
        ulog_printf("camera configure failed (%d)\r\n", e);
        led(LED_RED_PORT, LED_RED_PIN, true);
        return e;
    }
    cam_info_t ci;
    cam_get_info(&ci);
    s_p.cfa = ci.cfa;
    clamp_roi();
    e = cap_start(s_cam.width, s_cam.height);
    if (e != 0) {
        ulog_printf("capture start failed (%d)\r\n", e);
        led(LED_RED_PORT, LED_RED_PIN, true);
        return e;
    }
    s_have_prev = false;
    reset_window();
    return 0;
}

// ---- one frame of the detection loop -------------------------------------------------------
static void run_frame(void) {
    cap_frame_t f;
    uint32_t t0 = tb_us();
    if (!cap_get(&f, 50)) {
        if (++s_win.timeouts % 20 == 1) {
            cap_stats_t cs;
            cap_get_stats(&cs);
            ulog_printf("no frames from the camera (frames %lu, errors %lu, last error 0x%lx)\r\n",
                        (unsigned long) cs.frames, (unsigned long) cs.errors, (unsigned long) cs.last_error);
            led(LED_RED_PORT, LED_RED_PIN, true);
        }
        return;
    }
    uint32_t t1 = tb_us();

    if (s_kernel_pin) {                 // end of the previous kernel pulse
        HAL_GPIO_WritePin(KERNEL_OUT_PORT, KERNEL_OUT_PIN, GPIO_PIN_RESET);
        led(LED_BLUE_PORT, LED_BLUE_PIN, false);
        s_kernel_pin = false;
    }

    if (s_have_prev) {
        uint32_t gap = f.seq - s_prev_seq - 1u;
        s_win.lost += gap;
        s_tot_lost += gap;
    }
    s_prev_seq = f.seq;
    s_have_prev = true;
    if (s_win.n == 0) {
        s_win.t_first = f.ts_us;
    }
    s_win.t_last = f.ts_us;

    cz_frame_t fi;
    HAL_GPIO_WritePin(BUSY_OUT_PORT, BUSY_OUT_PIN, GPIO_PIN_SET);
    int nb = cz_process(f.data, f.w, f.h, &s_p, s_out, CZ_MAX_OUT, &fi);
    HAL_GPIO_WritePin(BUSY_OUT_PORT, BUSY_OUT_PIN, GPIO_PIN_RESET);
    uint32_t t2 = tb_us();
    const uint32_t ts = f.ts_us;
    const uint32_t frame_no = s_tot_frames;
    cap_release(&f);

    if (nb < 0) {
        ulog_printf("cz_process: bad frame size %ux%u\r\n", f.w, f.h);
        s_running = false;
        return;
    }
    for (int s = 0; s < CZ_NSTATUS; s++) {
        s_win.st[s] += fi.count[s];
    }
    bool accepted = false;
    for (int i = 0; i < nb; i++) {
        const cz_blob_t *b = &s_out[i];
        if (b->status != CZ_OK) {
            if (s_verbose >= 2) {
                ulog_printf("  f%lu %s A=%lu L=%.1f W=%.1f sol=%.2f\r\n", (unsigned long) frame_no,
                            status_name[b->status], (unsigned long) b->area, (double) b->len_mm,
                            (double) b->wid_mm, (double) b->solidity);
            }
            continue;
        }
        // Belt coordinate: constant for one kernel across frames.
        float pos = (float) s_dir * b->cx * s_p.mm_px - s_belt_mm_s * belt_time_s(ts);
        if (!cz_dedup_first(&s_dd, pos, b->cy * s_p.mm_px)) {
            s_win.dup++;
            s_tot_dup++;
            continue;
        }
        s_win.accept++;
        s_tot_accept++;
        accepted = true;
        if (s_verbose >= 1) {
            ulog_printf("  K f%lu t=%lu x=%.0f y=%.0f L=%.1f W=%.1f A=%.0fmm2 sol=%.2f rgb=%.0f/%.0f/%.0f\r\n",
                        (unsigned long) frame_no, (unsigned long) ts, (double) b->cx, (double) b->cy,
                        (double) b->len_mm, (double) b->wid_mm, (double) ((float) b->area * s_p.mm_px * s_p.mm_px),
                        (double) b->solidity, (double) b->r, (double) b->g, (double) b->b);
        }
    }
    if (accepted) {
        HAL_GPIO_WritePin(KERNEL_OUT_PORT, KERNEL_OUT_PIN, GPIO_PIN_SET);
        led(LED_BLUE_PORT, LED_BLUE_PIN, true);
        s_kernel_pin = true;
    }

    uint32_t pt = t2 - t1;
    s_win.wait_sum += t1 - t0;
    s_win.proc_sum += pt;
    if (pt > s_win.proc_max) {
        s_win.proc_max = pt;
    }
    s_win.n++;
    s_tot_frames++;
    if ((s_tot_frames % 120) == 0) {
        HAL_GPIO_TogglePin(LED_GREEN_PORT, LED_GREEN_PIN);
    }

    if (s_win.n >= s_report) {
        float fps = (s_win.t_last != s_win.t_first) ?
                    (float) (s_win.n - 1) * 1e6f / (float) (s_win.t_last - s_win.t_first) : 0.0f;
        cap_stats_t cs;
        cap_get_stats(&cs);
        ulog_printf("fps %.1f | proc avg %lu max %lu us | wait avg %lu us | lost %lu err %lu | kernels %lu dup %lu | "
                    "edge %lu long %lu aspect %lu concave %lu neck %lu big %lu crowded %lu\r\n",
                    (double) fps, (unsigned long) (s_win.proc_sum / s_win.n), (unsigned long) s_win.proc_max,
                    (unsigned long) (s_win.wait_sum / s_win.n), (unsigned long) s_win.lost, (unsigned long) cs.errors,
                    (unsigned long) s_win.accept, (unsigned long) s_win.dup,
                    (unsigned long) s_win.st[CZ_EDGE], (unsigned long) s_win.st[CZ_LONG], (unsigned long) s_win.st[CZ_ASPECT],
                    (unsigned long) s_win.st[CZ_CONCAVE], (unsigned long) s_win.st[CZ_NECK], (unsigned long) s_win.st[CZ_BIG],
                    (unsigned long) s_win.st[CZ_CROWDED]);
        reset_window();
    }
    if (s_run_limit && s_tot_frames >= s_run_limit) {
        s_running = false;
        ulog_printf("run done: frames %lu lost %lu kernels %lu dup %lu\r\n", (unsigned long) s_tot_frames,
                    (unsigned long) s_tot_lost, (unsigned long) s_tot_accept, (unsigned long) s_tot_dup);
    }
}

static void start_run(uint32_t frames) {
    if (!s_cam_ok || !cap_running()) {
        ulog_puts("camera not running\r\n");
        return;
    }
    s_run_limit = frames;
    s_tot_frames = s_tot_lost = s_tot_accept = s_tot_dup = 0;
    s_have_prev = false;
    reset_window();
    cap_flush();
    s_epoch_us = tb_us();
    cz_dedup_init(&s_dd, s_dd.tol_mm, s_dd.tol_y_mm);
    s_running = true;
    ulog_printf("run: %ux%u, report every %lu frames, verbose %d (\"stop\" or the blue button to stop)\r\n",
                s_cam.width, s_cam.height, (unsigned long) s_report, s_verbose);
}

static void stop_run(void) {
    if (s_running) {
        s_running = false;
        ulog_printf("stopped: frames %lu lost %lu kernels %lu dup %lu\r\n", (unsigned long) s_tot_frames,
                    (unsigned long) s_tot_lost, (unsigned long) s_tot_accept, (unsigned long) s_tot_dup);
    }
}

void app_toggle_run(void) {
    if (s_running) {
        stop_run();
    } else {
        start_run(0);
    }
}

// Get a fresh frame (older queued frames are dropped first).
static bool fresh_frame(cap_frame_t *f) {
    cap_flush();
    s_have_prev = false;                // flushed frames are not "lost"
    if (!cap_get(f, 200)) {
        ulog_puts("no frame from the camera\r\n");
        return false;
    }
    return true;
}

// ---- commands ------------------------------------------------------------------------------
static void cmd_mean(void) {
    cap_frame_t f;
    if (!fresh_frame(&f)) {
        return;
    }
    static const uint8_t cmap[4][4] = {     // [cfa][(y&1)*2+(x&1)] -> 0 R 1 G 2 B
        { 2, 1, 1, 0 }, { 1, 2, 0, 1 }, { 1, 0, 2, 1 }, { 0, 1, 1, 2 }
    };
    uint32_t sum[3] = { 0 }, cnt[3] = { 0 }, all = 0, mn = 255, mx = 0;
    for (int y = 0; y < f.h; y++) {
        const uint8_t *row = f.data + (size_t) y * f.w;
        for (int x = 0; x < f.w; x++) {
            uint32_t v = row[x];
            int c = cmap[s_p.cfa & 3][((y & 1) << 1) | (x & 1)];
            sum[c] += v;
            cnt[c]++;
            all += v;
            mn = v < mn ? v : mn;
            mx = v > mx ? v : mx;
        }
    }
    uint32_t n = (uint32_t) f.w * f.h;
    ulog_printf("frame %ux%u #%lu: mean %lu min %lu max %lu | R %lu G %lu B %lu\r\n", f.w, f.h,
                (unsigned long) f.seq, (unsigned long) (all / n), (unsigned long) mn, (unsigned long) mx,
                (unsigned long) (sum[0] / (cnt[0] ? cnt[0] : 1)), (unsigned long) (sum[1] / (cnt[1] ? cnt[1] : 1)),
                (unsigned long) (sum[2] / (cnt[2] ? cnt[2] : 1)));
    if (all / n < 5 || all / n > 250) {
        ulog_puts("mean near 0 or 255: change expo / gain or the lighting\r\n");
    }
    cap_release(&f);
}

static void cmd_snap(void) {
    cap_frame_t f;
    if (!fresh_frame(&f)) {
        return;
    }
    cz_frame_t fi;
    uint32_t t0 = tb_us();
    int n = cz_process(f.data, f.w, f.h, &s_p, s_out, CZ_MAX_OUT, &fi);
    uint32_t t1 = tb_us();
    cap_release(&f);
    ulog_printf("process: %lu us, %lu blobs, %lu runs, overflow %lu\r\n", (unsigned long) (t1 - t0),
                (unsigned long) fi.nblobs, (unsigned long) fi.nruns, (unsigned long) fi.overflow);
    for (int i = 0; i < n; i++) {
        const cz_blob_t *b = &s_out[i];
        ulog_printf("%-8s A=%5lu px  L=%5.1f W=%5.1f mm  ang=%4.0f  sol=%.2f  rgb=%3.0f/%3.0f/%3.0f  rect=(%u,%u,%u,%u)\r\n",
                    status_name[b->status], (unsigned long) b->area, (double) b->len_mm, (double) b->wid_mm,
                    (double) b->angle_deg, (double) b->solidity, (double) b->r, (double) b->g, (double) b->b,
                    b->x0, b->y0, b->x1 - b->x0 + 1, b->y1 - b->y0 + 1);
    }
}

// Raw frame as hex text, read by tools/grab_frame.py.
static void cmd_dump(void) {
    cap_frame_t f;
    if (!fresh_frame(&f)) {
        return;
    }
    static const char hex[] = "0123456789abcdef";
    char line[2 * CAP_MAX_W + 2];
    ulog_set_blocking(true);
    ulog_printf("FRAME %u %u cfa %u seq %lu\r\n", f.w, f.h, s_p.cfa, (unsigned long) f.seq);
    for (int y = 0; y < f.h; y++) {
        const uint8_t *row = f.data + (size_t) y * f.w;
        for (int x = 0; x < f.w; x++) {
            line[2 * x] = hex[row[x] >> 4];
            line[2 * x + 1] = hex[row[x] & 15];
        }
        line[2 * f.w] = '\r';
        line[2 * f.w + 1] = '\n';
        ulog_write(line, 2u * f.w + 2u);
    }
    ulog_puts("END\r\n");
    ulog_flush();
    ulog_set_blocking(false);
    cap_release(&f);
}

static void cmd_sweep(int32_t start, int32_t stop, int32_t step, int32_t frames) {
    if (step <= 0) {
        step = 250;
    }
    if (frames <= 1) {
        frames = 240;
    }
    bool was_running = s_running;
    s_running = false;
    ulog_puts("frame_us   target_fps   measured_fps   dt_min..dt_max us   lost   (any key stops)\r\n");
    HAL_Delay(5);
    (void) ulog_rx_activity();          // forget the keys of the sweep command itself
    for (int32_t us = start; us >= stop && us >= 500; us -= step) {
        cam_set_frame_us((uint32_t) us);
        HAL_Delay(50);
        cap_flush();
        uint32_t first = 0, last = 0, prev = 0, dmin = 0xFFFFFFFFu, dmax = 0, lost = 0, prev_seq = 0;
        int got = 0;
        for (int i = 0; i < frames; i++) {
            cap_frame_t f;
            if (!cap_get(&f, 100)) {
                break;
            }
            if (got == 0) {
                first = f.ts_us;
            } else {
                uint32_t d = f.ts_us - prev;
                dmin = d < dmin ? d : dmin;
                dmax = d > dmax ? d : dmax;
                lost += f.seq - prev_seq - 1u;
            }
            prev = last = f.ts_us;
            prev_seq = f.seq;
            got++;
            cap_release(&f);
        }
        float fps = (got > 1 && last != first) ? (float) (got - 1 + (int) lost) * 1e6f / (float) (last - first) : 0.0f;
        ulog_printf("%6ld     %7.1f      %7.1f        %5lu..%-5lu        %lu\r\n", (long) us,
                    (double) (1e6f / (float) us), (double) fps, (unsigned long) (got > 1 ? dmin : 0),
                    (unsigned long) dmax, (unsigned long) lost);
        ulog_flush();
        if (ulog_rx_activity()) {
            ulog_puts("sweep interrupted\r\n");
            break;
        }
    }
    cam_set_frame_us(s_cam.frame_us);   // restore
    if (was_running) {
        start_run(0);
    }
}

static void cmd_status(void) {
    cap_stats_t cs;
    cap_get_stats(&cs);
    ulog_printf("%s, CPU %lu MHz, uptime %lu s\r\n", FW_VERSION, (unsigned long) (SystemCoreClock / 1000000u),
                (unsigned long) (HAL_GetTick() / 1000u));
    ulog_printf("capture: %s, frames %lu dropped %lu errors %lu (last 0x%lx) | run: %s, frames %lu kernels %lu | log dropped %lu chars\r\n",
                cap_running() ? "on" : "off", (unsigned long) cs.frames, (unsigned long) cs.dropped,
                (unsigned long) cs.errors, (unsigned long) cs.last_error, s_running ? "on" : "off",
                (unsigned long) s_tot_frames, (unsigned long) s_tot_accept, (unsigned long) ulog_dropped());
    if (s_cam_ok) {
        print_cam_info();
    }
    print_memory();
}

static void cmd_help(void) {
    ulog_puts(
        "commands:\r\n"
        "  run [frames]         detection loop (report line every 'report' frames, K line per new kernel)\r\n"
        "  stop                 stop the loop (the blue USER button toggles run/stop too)\r\n"
        "  snap                 process one frame and list every blob with its status\r\n"
        "  mean                 frame brightness (mean/min/max, per colour)\r\n"
        "  sweep A B [step] [n] frame-time sweep from A down to B us: real maximum frame rate\r\n"
        "  dump                 one raw frame as hex text (tools/grab_frame.py saves it as .pgm)\r\n"
        "  param                show all settings\r\n"
        "  status               camera, capture and memory information\r\n"
        "  set NAME VALUE       change a setting, e.g. 'set threshold 260', 'set fps 300', 'set expo 180'\r\n"
        "     detector: threshold invert filter edge_px min_pix max_pix mm_px lmax_mm ar_max ar_min\r\n"
        "               wmax_mm sol_min neck_px gap_mm crowd_min_pix belt_mm_s dir dup_mm dup_y_mm cfa\r\n"
        "     camera:   fps frame_us expo gain vblank height bin strobe strobe_inv mirror flip colorbar\r\n"
        "     output:   verbose (0 summary, 1 +kernels, 2 +rejects) report\r\n"
        "  roi X Y W H          detector region of interest (px)\r\n");
}

typedef enum { T_U8, T_U16, T_U32, T_BOOL, T_FLOAT } ptype_t;
typedef struct {
    const char *name;
    ptype_t type;
    void *ptr;
} param_t;

static const param_t s_params[] = {
    { "threshold",     T_U16,   &s_p.threshold },
    { "invert",        T_BOOL,  &s_p.invert },
    { "filter",        T_BOOL,  &s_p.filter },
    { "edge_px",       T_U8,    &s_p.edge_px },
    { "min_pix",       T_U32,   &s_p.min_pix },
    { "max_pix",       T_U32,   &s_p.max_pix },
    { "mm_px",         T_FLOAT, &s_p.mm_px },
    { "lmax_mm",       T_FLOAT, &s_p.lmax_mm },
    { "ar_max",        T_FLOAT, &s_p.ar_max },
    { "ar_min",        T_FLOAT, &s_p.ar_min },
    { "wmax_mm",       T_FLOAT, &s_p.wmax_mm },
    { "sol_min",       T_FLOAT, &s_p.sol_min },
    { "neck_px",       T_U8,    &s_p.neck_px },
    { "gap_mm",        T_FLOAT, &s_p.gap_mm },
    { "crowd_min_pix", T_U32,   &s_p.crowd_min_pix },
    { "cfa",           T_U8,    &s_p.cfa },
    { "belt_mm_s",     T_FLOAT, &s_belt_mm_s },
};

static void cmd_set(const char *name, const char *val) {
    if (!name || !val) {
        ulog_puts("usage: set NAME VALUE\r\n");
        return;
    }
    float fv;
    int32_t iv;
    bool bv;
    for (size_t i = 0; i < sizeof(s_params) / sizeof(s_params[0]); i++) {
        const param_t *p = &s_params[i];
        if (strcmp(name, p->name)) {
            continue;
        }
        switch (p->type) {
            case T_BOOL:
                if (!parse_bool(val, &bv)) {
                    goto bad;
                }
                *(bool *) p->ptr = bv;
                break;
            case T_FLOAT:
                if (!parse_float(val, &fv)) {
                    goto bad;
                }
                *(float *) p->ptr = fv;
                break;
            default:
                if (!parse_int(val, &iv) || iv < 0) {
                    goto bad;
                }
                if (p->type == T_U8) {
                    *(uint8_t *) p->ptr = (uint8_t) (iv > 255 ? 255 : iv);
                } else if (p->type == T_U16) {
                    *(uint16_t *) p->ptr = (uint16_t) (iv > 65535 ? 65535 : iv);
                } else {
                    *(uint32_t *) p->ptr = (uint32_t) iv;
                }
                break;
        }
        ulog_printf("%s = %s\r\n", name, val);
        return;
    }

    // Settings with side effects.
    if (!strcmp(name, "dir")) {
        if (!parse_int(val, &iv)) {
            goto bad;
        }
        s_dir = iv >= 0 ? 1 : -1;
    } else if (!strcmp(name, "dup_mm") || !strcmp(name, "dup_y_mm")) {
        if (!parse_float(val, &fv) || fv < 0) {
            goto bad;
        }
        if (name[4] == 'y') {
            cz_dedup_init(&s_dd, s_dd.tol_mm, fv);
        } else {
            cz_dedup_init(&s_dd, fv, s_dd.tol_y_mm);
        }
    } else if (!strcmp(name, "verbose")) {
        if (!parse_int(val, &iv)) {
            goto bad;
        }
        s_verbose = iv;
    } else if (!strcmp(name, "report")) {
        if (!parse_int(val, &iv) || iv < 1) {
            goto bad;
        }
        s_report = (uint32_t) iv;
        reset_window();
    } else if (!strcmp(name, "fps") || !strcmp(name, "frame_us")) {
        if (!parse_float(val, &fv) || fv <= 0) {
            goto bad;
        }
        uint32_t us = (name[0] == 'f' && name[1] == 'p') ? (uint32_t) (1e6f / fv + 0.5f) : (uint32_t) fv;
        if (us < 500) {
            goto bad;
        }
        s_cam.frame_us = us;
        cam_set_frame_us(us);
        print_cam_info();
    } else if (!strcmp(name, "expo")) {
        if (!parse_int(val, &iv) || iv <= 0) {
            goto bad;
        }
        s_cam.expo_us = (uint32_t) iv;
        cam_set_exposure_us(s_cam.expo_us);
        print_cam_info();
    } else if (!strcmp(name, "gain")) {
        if (!parse_float(val, &fv) || fv < 0 || fv > 18.1f) {
            goto bad;
        }
        s_cam.gain_db = fv;
        cam_set_gain_db(fv);
        print_cam_info();
    } else if (!strcmp(name, "vblank")) {
        if (!parse_int(val, &iv) || iv < 1 || iv > 4000) {
            goto bad;
        }
        s_cam.vblank = (uint16_t) iv;
        cam_set_vblank(s_cam.vblank);
        print_cam_info();
    } else if (!strcmp(name, "height") || !strcmp(name, "bin") || !strcmp(name, "strobe") ||
               !strcmp(name, "strobe_inv") || !strcmp(name, "mirror") || !strcmp(name, "flip") ||
               !strcmp(name, "colorbar")) {
        cam_cfg_t old = s_cam;
        if (!strcmp(name, "height")) {
            if (!parse_int(val, &iv) || iv < 16 || iv > CAP_MAX_H || (iv & 1)) {
                goto bad;
            }
            s_cam.height = (uint16_t) iv;
            s_p.roi_y0 = 0;
            s_p.roi_y1 = (uint16_t) (iv - 1);
        } else if (!strcmp(name, "bin")) {
            if (!parse_int(val, &iv) || (iv != 1 && iv != 2 && iv != 4)) {
                goto bad;
            }
            s_cam.bin = (uint8_t) iv;
        } else if (!strcmp(name, "strobe")) {
            if (!parse_int(val, &iv) || iv < -1 || iv > 7) {
                goto bad;
            }
            s_cam.strobe_gpio = (int8_t) iv;
        } else {
            if (!parse_bool(val, &bv)) {
                goto bad;
            }
            if (!strcmp(name, "strobe_inv")) {
                s_cam.strobe_inv = bv;
            } else if (!strcmp(name, "mirror")) {
                s_cam.hmirror = bv;
            } else if (!strcmp(name, "flip")) {
                s_cam.vflip = bv;
            } else {
                s_cam.colorbar = bv;
            }
        }
        bool was_running = s_running;
        s_running = false;
        if (apply_camera() != 0) {
            ulog_puts("restoring the previous camera setting\r\n");
            s_cam = old;
            s_p.roi_y1 = (uint16_t) (s_cam.height - 1);
            apply_camera();
        }
        print_cam_info();
        if (was_running) {
            start_run(0);
        }
    } else {
        ulog_printf("unknown setting '%s' (type help)\r\n", name);
        return;
    }
    ulog_printf("%s = %s\r\n", name, val);
    return;
bad:
    ulog_printf("bad value for %s: '%s'\r\n", name, val);
}

static void handle_line(char *line) {
    char *argv[6] = { 0 };
    int argc = 0;
    for (char *t = line; *t && argc < 6;) {        // split on blanks (no strtok: it needs malloc)
        while (*t == ' ' || *t == '\t') {
            *t++ = 0;
        }
        if (!*t) {
            break;
        }
        argv[argc++] = t;
        while (*t && *t != ' ' && *t != '\t') {
            t++;
        }
    }
    if (argc == 0) {
        return;
    }
    const char *c = argv[0];
    int32_t a = 0, b = 0, st = 250, n = 240;
    if (!strcmp(c, "help") || !strcmp(c, "?")) {
        cmd_help();
    } else if (!strcmp(c, "run")) {
        if (argc > 1 && !parse_int(argv[1], &a)) {
            a = 0;
        }
        start_run(a > 0 ? (uint32_t) a : 0);
    } else if (!strcmp(c, "stop")) {
        stop_run();
    } else if (!strcmp(c, "snap")) {
        cmd_snap();
    } else if (!strcmp(c, "mean")) {
        cmd_mean();
    } else if (!strcmp(c, "dump")) {
        bool was_running = s_running;
        s_running = false;
        cmd_dump();
        if (was_running) {
            start_run(0);
        }
    } else if (!strcmp(c, "sweep")) {
        if (argc < 3 || !parse_int(argv[1], &a) || !parse_int(argv[2], &b)) {
            ulog_puts("usage: sweep START_US STOP_US [STEP_US] [FRAMES]\r\n");
            return;
        }
        if (argc > 3) {
            parse_int(argv[3], &st);
        }
        if (argc > 4) {
            parse_int(argv[4], &n);
        }
        cmd_sweep(a, b, st, n);
    } else if (!strcmp(c, "param") || !strcmp(c, "params")) {
        print_params();
    } else if (!strcmp(c, "status") || !strcmp(c, "info")) {
        cmd_status();
    } else if (!strcmp(c, "set")) {
        cmd_set(argv[1], argv[2]);
    } else if (!strcmp(c, "roi")) {
        int32_t x, y, w, h;
        if (argc < 5 || !parse_int(argv[1], &x) || !parse_int(argv[2], &y) || !parse_int(argv[3], &w) ||
            !parse_int(argv[4], &h) || x < 0 || y < 0 || w < 8 || h < 8 ||
            x + w > s_cam.width || y + h > s_cam.height) {
            ulog_puts("usage: roi X Y W H (inside the frame)\r\n");
            return;
        }
        s_p.roi_x0 = (uint16_t) x;
        s_p.roi_y0 = (uint16_t) y;
        s_p.roi_x1 = (uint16_t) (x + w - 1);
        s_p.roi_y1 = (uint16_t) (y + h - 1);
        ulog_printf("roi = %ld %ld %ld %ld\r\n", (long) x, (long) y, (long) w, (long) h);
    } else {
        ulog_printf("unknown command '%s' (type help)\r\n", c);
    }
}

// ---- init / poll ----------------------------------------------------------------------------
int app_init(bool camera_ok) {
    s_cam_ok = camera_ok;
    cam_default_cfg(&s_cam);

    // Detector parameters: same defaults as cashew_bench_n6.py.
    cz_default_params(&s_p, s_cam.width, s_cam.height);
    s_p.threshold = 240;
    s_p.invert = false;
    s_p.filter = true;
    s_p.edge_px = 1;
    s_p.min_pix = 500;
    s_p.max_pix = 40000;
    s_p.mm_px = 0.25f;
    s_p.lmax_mm = 38.0f;
    s_p.ar_max = 2.3f;
    s_p.ar_min = 1.2f;
    s_p.wmax_mm = 25.0f;
    s_p.sol_min = 0.80f;
    s_p.neck_px = 40;
    s_p.gap_mm = 5.0f;
    s_p.crowd_min_pix = 150;
    s_belt_mm_s = 3000.0f;
    s_dir = 1;
    cz_dedup_init(&s_dd, 10.0f, 6.0f);

    cz_dims_t d = { CAP_MAX_W, CAP_MAX_H, WS_RUNS, WS_LABELS };
    size_t hot = cz_ws_hot_size(&d), warm = cz_ws_warm_size(&d);
    if (hot > sizeof(s_hot) || warm > sizeof(s_warm) || !cz_ws_init(&d, s_hot, s_warm)) {
        ulog_printf("detector work buffers too small (need hot %lu warm %lu)\r\n",
                    (unsigned long) hot, (unsigned long) warm);
        return -1;
    }
    ulog_printf("detector: hot buffers %lu B in DTCM, warm %lu B in AXISRAM2, max frame %ux%u\r\n",
                (unsigned long) hot, (unsigned long) warm, CAP_MAX_W, CAP_MAX_H);
    print_memory();

    if (!s_cam_ok) {
        ulog_puts("camera not available: commands work, capture does not\r\n");
        return -1;
    }
    if (apply_camera() != 0) {
        s_cam_ok = false;
        return -1;
    }
    HAL_Delay(100);
    print_cam_info();
    print_params();
    ulog_puts("type 'help' for commands\r\n");
    if (APP_AUTORUN) {
        start_run(0);
    }
    return 0;
}

void app_poll(void) {
    static char line[128];
    if (ulog_getline(line, sizeof(line))) {
        handle_line(line);
    }
    if (s_running) {
        run_frame();
    } else if (cap_running()) {
        cap_flush();        // idle: keep the buffers free so 'dropped' only counts real drops
    }
}
