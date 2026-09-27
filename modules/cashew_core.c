/*
 * cashew_core.c -- see cashew_core.h. Portable C99, no MicroPython dependencies.
 */
#include "cashew_core.h"
// Firmware: compile only for the core that owns the camera (HP). Host tests: always.
#if !defined(CORE_M55_HE)
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define MW_MAX      ((CZ_MAX_W + 31) / 32)

#define NOLABEL     0xFFFF

typedef struct {
    uint16_t y, x0, x1, lab;
} cz_run_t;

typedef struct {
    uint32_t n;
    uint64_t sx, sy, sxx, syy, sxy;
    uint16_t x0, x1, y0, y1;
    uint8_t  touch;
    int8_t   cand;              // candidate slot or -1
    int16_t  outi;              // index into out[] or -1
} cz_acc_t;

typedef struct {
    uint32_t mask[CZ_MAX_H][MW_MAX];
    uint32_t filt[CZ_MAX_H][MW_MAX];
    cz_run_t runs[CZ_MAX_RUNS];
    cz_acc_t acc[CZ_MAX_LABELS];
    uint16_t parent[CZ_MAX_LABELS];
    int16_t  left[CZ_MAX_CAND][CZ_MAX_H];
    int16_t  right[CZ_MAX_CAND][CZ_MAX_H];
    int16_t  pts[4 * CZ_MAX_H][2];
    int16_t  hull[4 * CZ_MAX_H + 1][2];
    uint32_t lm[2][CZ_MAX_H][MW_MAX];
    cz_run_t lruns[CZ_MAX_RUNS / 4];
    uint16_t lpar[CZ_MAX_LABELS / 4];
    uint32_t larea[CZ_MAX_LABELS / 4];
} cz_ws_t;

static cz_ws_t *g_ws;
#define s_mask (g_ws->mask)
#define s_filt (g_ws->filt)
#define s_runs (g_ws->runs)
#define s_acc (g_ws->acc)
#define s_parent (g_ws->parent)
#define s_left (g_ws->left)
#define s_right (g_ws->right)
#define s_pts (g_ws->pts)
#define s_hull (g_ws->hull)
#define s_lm (g_ws->lm)
#define s_lruns (g_ws->lruns)
#define s_lpar (g_ws->lpar)
#define s_larea (g_ws->larea)

size_t cz_ws_size(void) { return sizeof(cz_ws_t); }
void cz_ws_set(void *mem) { g_ws = (cz_ws_t *) mem; }
bool cz_ws_ready(void) { return g_ws != NULL; }

// ---- static workspace (single instance, not re-entrant) --------------------
static uint16_t s_luma[CZ_MAX_W];
static uint32_t s_csum[CZ_MAX_CAND][3];
static uint32_t s_ccnt[CZ_MAX_CAND][3];
static int      s_w, s_h, s_mw;

static const uint8_t cfa_map[4][4] = {  // [cfa][(y&1)*2 + (x&1)] -> 0=R 1=G 2=B
    { 2, 1, 1, 0 },     // BGGR
    { 1, 2, 0, 1 },     // GBRG
    { 1, 0, 2, 1 },     // GRBG
    { 0, 1, 1, 2 },     // RGGB
};

void cz_default_params(cz_params_t *p, int w, int h) {
    memset(p, 0, sizeof(*p));
    p->threshold = 240;
    p->invert = false;
    p->filter = true;
    p->edge_px = 1;
    p->roi_x0 = 0; p->roi_y0 = 0;
    p->roi_x1 = (uint16_t) (w - 1); p->roi_y1 = (uint16_t) (h - 1);
    p->min_pix = 500;
    p->max_pix = 40000;
    p->mm_px = 0.25f;
    p->lmax_mm = 38.0f;
    p->ar_max = 2.3f;
    p->ar_min = 1.2f;
    p->wmax_mm = 25.0f;
    p->sol_min = 0.80f;
    p->neck_px = 40;     // max erosion depth (px) for the split test
    p->gap_mm = 5.0f;    // ejector needs a clear stretch of belt around each kernel
    p->crowd_min_pix = 150;
    p->cfa = CZ_BGGR;
}

const uint32_t *cz_mask_row(int y) {
    return (y >= 0 && y < s_h) ? s_filt[y] : NULL;
}

// ---- stage 1: 2x2 luma threshold into a bit mask ---------------------------
static void mask_row(const uint8_t *r0, const uint8_t *r1, int w, uint16_t th, bool inv, uint32_t *m) {
    // luma(x) = r0[x] + r0[x+1] + r1[x] + r1[x+1], last column replicated.
    for (int x = 0; x < w - 1; x++) {
        s_luma[x] = (uint16_t) (r0[x] + r0[x + 1] + r1[x] + r1[x + 1]);
    }
    s_luma[w - 1] = (uint16_t) (2 * (r0[w - 1] + r1[w - 1]));

    for (int wi = 0; wi < w / 32; wi++) {
        const uint16_t *l = &s_luma[wi * 32];
        uint32_t bits = 0;
        if (!inv) {
            for (int b = 0; b < 32; b++) bits |= (uint32_t) (l[b] > th) << b;
        } else {
            for (int b = 0; b < 32; b++) bits |= (uint32_t) (l[b] < th) << b;
        }
        m[wi] = bits;
    }
}

// ---- stage 2: true 3x3 majority (>= 5 of 9) with replicate padding ---------
// Bit-sliced 9-input population count, 32 pixels per word.
#define FA(a, b, c, s, co) do { uint32_t _t = (a) ^ (b); s = _t ^ (c); co = ((a) & (b)) | ((c) & _t); } while (0)

static inline void shift_lr(const uint32_t *v, int i, int mw, uint32_t *lo, uint32_t *hi) {
    // lo: neighbour at x-1, hi: neighbour at x+1 (replicate at frame edges)
    *lo = (v[i] << 1) | (i > 0 ? v[i - 1] >> 31 : (v[0] & 1u));
    *hi = (v[i] >> 1) | (i < mw - 1 ? v[i + 1] << 31 : (v[mw - 1] & 0x80000000u));
}

static void majority_row(const uint32_t *a, const uint32_t *b, const uint32_t *c, int mw, uint32_t *out) {
    for (int i = 0; i < mw; i++) {
        uint32_t a0, a2, b0, b2, c0, c2;
        shift_lr(a, i, mw, &a0, &a2);
        shift_lr(b, i, mw, &b0, &b2);
        shift_lr(c, i, mw, &c0, &c2);
        uint32_t s1, c1, s2, c2_, s3, c3, bit1, c4, t, d;
        FA(a0, a[i], a2, s1, c1);
        FA(b0, b[i], b2, s2, c2_);
        FA(c0, c[i], c2, s3, c3);
        FA(s1, s2, s3, bit1, c4);          // bit1: weight 1, c4: weight 2
        FA(c1, c2_, c3, t, d);             // t: weight 2, d: weight 4
        uint32_t bit2 = t ^ c4, e = t & c4;// e: weight 4
        uint32_t bit4 = d ^ e, bit8 = d & e;
        out[i] = bit8 | (bit4 & (bit2 | bit1));   // count >= 5
    }
}

// ---- run helpers -------------------------------------------------------------
static inline int next_set(const uint32_t *m, int x, int w) {
    while (x < w) {
        uint32_t word = m[x >> 5] >> (x & 31);
        if (word) return x + __builtin_ctz(word);
        x = (x | 31) + 1;
    }
    return w;
}

static inline int next_clr(const uint32_t *m, int x, int w) {
    while (x < w) {
        uint32_t word = ~m[x >> 5] >> (x & 31);
        if (word) {
            int r = x + __builtin_ctz(word);
            return r < w ? r : w;
        }
        x = (x | 31) + 1;
    }
    return w;
}

// ---- union-find --------------------------------------------------------------
static inline uint16_t uf_find(uint16_t a) {
    while (s_parent[a] != a) {
        s_parent[a] = s_parent[s_parent[a]];
        a = s_parent[a];
    }
    return a;
}

static void acc_merge(cz_acc_t *d, const cz_acc_t *s) {
    d->n += s->n;
    d->sx += s->sx; d->sy += s->sy;
    d->sxx += s->sxx; d->syy += s->syy; d->sxy += s->sxy;
    if (s->x0 < d->x0) d->x0 = s->x0;
    if (s->x1 > d->x1) d->x1 = s->x1;
    if (s->y0 < d->y0) d->y0 = s->y0;
    if (s->y1 > d->y1) d->y1 = s->y1;
    d->touch |= s->touch;
}

static uint16_t uf_union(uint16_t a, uint16_t b) {
    a = uf_find(a); b = uf_find(b);
    if (a == b) return a;
    if (b < a) { uint16_t t = a; a = b; b = t; }
    s_parent[b] = a;
    acc_merge(&s_acc[a], &s_acc[b]);
    return a;
}

static void acc_add_run(cz_acc_t *a, int y, int x0, int x1, const cz_params_t *p) {
    uint64_t n = (uint64_t) (x1 - x0 + 1);
    uint64_t sx = n * (uint64_t) (x0 + x1) / 2;
    // sum_{x=x0..x1} x^2 = S(x1) - S(x0-1), S(k) = k(k+1)(2k+1)/6
    uint64_t k1 = (uint64_t) x1, k0 = (uint64_t) (x0 > 0 ? x0 - 1 : 0);
    uint64_t sxx = k1 * (k1 + 1) * (2 * k1 + 1) / 6 - (x0 > 0 ? k0 * (k0 + 1) * (2 * k0 + 1) / 6 : 0);
    a->n += (uint32_t) n;
    a->sx += sx;
    a->sxx += sxx;
    a->sy += n * (uint64_t) y;
    a->syy += n * (uint64_t) y * (uint64_t) y;
    a->sxy += sx * (uint64_t) y;
    if (x0 < a->x0) a->x0 = (uint16_t) x0;
    if (x1 > a->x1) a->x1 = (uint16_t) x1;
    if (y < a->y0) a->y0 = (uint16_t) y;
    if (y > a->y1) a->y1 = (uint16_t) y;
    int e = p->edge_px;
    if (y < (int) p->roi_y0 + e || y > (int) p->roi_y1 - e ||
        x0 < (int) p->roi_x0 + e || x1 > (int) p->roi_x1 - e) {
        a->touch = 1;
    }
}

// ---- convex hull (Andrew monotone chain) -------------------------------------
static int cmp_pt(const void *pa, const void *pb) {
    const int16_t *a = pa, *b = pb;
    return (a[0] != b[0]) ? (a[0] - b[0]) : (a[1] - b[1]);
}

static inline int32_t cross3(const int16_t *o, const int16_t *a, const int16_t *b) {
    return (int32_t) (a[0] - o[0]) * (b[1] - o[1]) - (int32_t) (a[1] - o[1]) * (b[0] - o[0]);
}

static float hull_area(int n) {
    if (n < 3) return 0.0f;
    qsort(s_pts, (size_t) n, sizeof(s_pts[0]), cmp_pt);
    int k = 0;
    for (int i = 0; i < n; i++) {
        while (k >= 2 && cross3(s_hull[k - 2], s_hull[k - 1], s_pts[i]) <= 0) k--;
        s_hull[k][0] = s_pts[i][0]; s_hull[k][1] = s_pts[i][1]; k++;
    }
    for (int i = n - 2, lo = k + 1; i >= 0; i--) {
        while (k >= lo && cross3(s_hull[k - 2], s_hull[k - 1], s_pts[i]) <= 0) k--;
        s_hull[k][0] = s_pts[i][0]; s_hull[k][1] = s_pts[i][1]; k++;
    }
    int64_t a2 = 0;
    for (int i = 0; i < k - 1; i++) {
        a2 += (int64_t) s_hull[i][0] * s_hull[i + 1][1] - (int64_t) s_hull[i + 1][0] * s_hull[i][1];
    }
    return (float) (a2 < 0 ? -a2 : a2) * 0.5f;
}

// ---- neck test: erode the blob, count significant pieces ----------------------

static inline uint16_t lf(uint16_t a) {
    while (s_lpar[a] != a) { s_lpar[a] = s_lpar[s_lpar[a]]; a = s_lpar[a]; }
    return a;
}

// One 3x3 erosion step s_lm[src] -> s_lm[src^1] over rows y0..y1. Returns remaining pixel count.
static uint32_t erode_step(int src, int y0, int y1, int wi0, int wi1) {
    int dst = src ^ 1;
    uint32_t cnt = 0;
    for (int y = y0; y <= y1; y++) {
        const uint32_t *a = (y > y0) ? s_lm[src][y - 1] : NULL;
        const uint32_t *b = s_lm[src][y];
        const uint32_t *c = (y < y1) ? s_lm[src][y + 1] : NULL;
        for (int i = wi0; i <= wi1; i++) {
            uint32_t v  = (a && c) ? (a[i] & b[i] & c[i]) : 0;
            uint32_t vl = (a && c && i > wi0) ? (a[i - 1] & b[i - 1] & c[i - 1]) : 0;
            uint32_t vr = (a && c && i < wi1) ? (a[i + 1] & b[i + 1] & c[i + 1]) : 0;
            uint32_t lo = (v << 1) | (vl >> 31);
            uint32_t hi = (v >> 1) | (vr << 31);
            uint32_t r = v & lo & hi;
            s_lm[dst][y][i] = r;
            cnt += (uint32_t) __builtin_popcount(r);
        }
    }
    return cnt;
}

// Count pieces in s_lm[buf] whose area >= frac * total.
static int count_pieces(int buf, int y0, int y1, int xs, int w, float frac) {
    int nr = 0, nl = 0, ps = 0, pe = 0;
    uint32_t total = 0;
    for (int y = y0; y <= y1; y++) {
        const uint32_t *f = s_lm[buf][y];
        int cs = nr, pi = ps;
        for (int x = next_set(f, xs, w); x < w; ) {
            int e = next_clr(f, x, w);
            int x0 = x, x1 = e - 1;
            if (nr >= (int) (sizeof(s_lruns) / sizeof(s_lruns[0]))) return 1;
            uint16_t lab = NOLABEL;
            while (pi < pe && s_lruns[pi].x1 + 1 < x0) pi++;
            for (int j = pi; j < pe && s_lruns[j].x0 <= x1 + 1; j++) {
                uint16_t r = lf(s_lruns[j].lab);
                if (lab == NOLABEL) lab = r;
                else if (r != lab) {
                    uint16_t lo = r < lab ? r : lab, hi = r < lab ? lab : r;
                    s_lpar[hi] = lo; s_larea[lo] += s_larea[hi]; s_larea[hi] = 0; lab = lo;
                }
            }
            if (lab == NOLABEL) {
                if (nl >= (int) (sizeof(s_lpar) / sizeof(s_lpar[0]))) return 1;
                lab = (uint16_t) nl++; s_lpar[lab] = lab; s_larea[lab] = 0;
            }
            lab = lf(lab);
            s_larea[lab] += (uint32_t) (x1 - x0 + 1);
            total += (uint32_t) (x1 - x0 + 1);
            s_lruns[nr].y = (uint16_t) y; s_lruns[nr].x0 = (uint16_t) x0;
            s_lruns[nr].x1 = (uint16_t) x1; s_lruns[nr].lab = lab; nr++;
            x = next_set(f, e, w);
        }
        ps = cs; pe = nr;
    }
    int pieces = 0;
    for (int l = 0; l < nl; l++) {
        if (s_lpar[l] == l && s_larea[l] > 0 && s_larea[l] >= frac * (float) total) pieces++;
    }
    return pieces;
}

// Iterative erosion: a touching pair splits at its contact neck before either body
// vanishes; a single kernel (including a crescent) erodes down to one piece.
// Blob must already be painted into s_lm[0]. Returns erosion depth of split, or 0.
static int neck_split(uint32_t area0, int y0, int y1, int x0, int x1, int max_k, float frac, float stop_frac) {
    int buf = 0, wi0 = x0 >> 5, wi1 = x1 >> 5;
    int xs = wi0 * 32, xe = (wi1 + 1) * 32;
    for (int k = 1; k <= max_k; k++) {
        uint32_t left = erode_step(buf, y0, y1, wi0, wi1);
        buf ^= 1;
        if (left < stop_frac * (float) area0) break;
        // a real split persists over several erosion steps: check every 2nd step
        if (k >= 2 && !(k & 1) && count_pieces(buf, y0, y1, xs, xe, frac) >= 2) return k;
    }
    return 0;
}

// ---- main entry ----------------------------------------------------------------
int cz_process(const uint8_t *raw, int w, int h, const cz_params_t *p,
               cz_blob_t *out, int max_out, cz_frame_t *fi) {
    memset(fi, 0, sizeof(*fi));
    if (!g_ws) {
        return -2;
    }
    if (w <= 0 || h <= 0 || (w & 31) || w > CZ_MAX_W || h > CZ_MAX_H) {
        return -1;
    }
    s_w = w; s_h = h; s_mw = w / 32;
    const int mw = s_mw;

    // ROI column mask words.
    uint32_t roi_cols[MW_MAX];
    for (int i = 0; i < mw; i++) {
        uint32_t m = 0;
        for (int b = 0; b < 32; b++) {
            int x = i * 32 + b;
            if (x >= p->roi_x0 && x <= p->roi_x1) m |= 1u << b;
        }
        roi_cols[i] = m;
    }

    // Stage 1: threshold every row.
    for (int y = 0; y < h; y++) {
        const uint8_t *r0 = raw + (size_t) y * w;
        const uint8_t *r1 = (y + 1 < h) ? r0 + w : r0;
        mask_row(r0, r1, w, p->threshold, p->invert, s_mask[y]);
    }

    // Stage 2 + 3: filter, clip to ROI, extract runs, label.
    int nruns = 0, nlab = 0;
    int prev_s = 0, prev_e = 0;
    for (int y = 0; y < h; y++) {
        uint32_t *f = s_filt[y];
        if (y < p->roi_y0 || y > p->roi_y1) {
            memset(f, 0, (size_t) mw * 4);
            prev_s = prev_e = nruns;
            continue;
        }
        if (p->filter) {
            const uint32_t *a = s_mask[y > 0 ? y - 1 : 0];
            const uint32_t *c = s_mask[y < h - 1 ? y + 1 : h - 1];
            majority_row(a, s_mask[y], c, mw, f);
        } else {
            memcpy(f, s_mask[y], (size_t) mw * 4);
        }
        for (int i = 0; i < mw; i++) f[i] &= roi_cols[i];

        int cur_s = nruns;
        int pi = prev_s;
        for (int x = next_set(f, 0, w); x < w; ) {
            int e = next_clr(f, x, w);      // run is [x, e-1]
            int x0 = x, x1 = e - 1;
            if (nruns >= CZ_MAX_RUNS) { fi->overflow++; break; }

            uint16_t lab = NOLABEL;
            // advance prev pointer past runs that end before x0-1
            while (pi < prev_e && s_runs[pi].x1 + 1 < x0) pi++;
            for (int j = pi; j < prev_e && s_runs[j].x0 <= x1 + 1; j++) {
                uint16_t r = uf_find(s_runs[j].lab);
                lab = (lab == NOLABEL) ? r : uf_union(lab, r);
            }
            if (lab == NOLABEL) {
                if (nlab >= CZ_MAX_LABELS) { fi->overflow++; break; }
                lab = (uint16_t) nlab++;
                s_parent[lab] = lab;
                cz_acc_t *a = &s_acc[lab];
                memset(a, 0, sizeof(*a));
                a->x0 = 0xFFFF; a->y0 = 0xFFFF; a->cand = -1;
            }
            lab = uf_find(lab);
            acc_add_run(&s_acc[lab], y, x0, x1, p);
            s_runs[nruns].y = (uint16_t) y;
            s_runs[nruns].x0 = (uint16_t) x0;
            s_runs[nruns].x1 = (uint16_t) x1;
            s_runs[nruns].lab = lab;
            nruns++;
            x = next_set(f, e, w);
        }
        prev_s = cur_s;
        prev_e = nruns;
    }
    fi->nruns = (uint32_t) nruns;

    // Stage 4: classify roots.
    int ncand = 0, nout = 0;
    for (int l = 0; l < nlab; l++) {
        if (s_parent[l] != l) continue;
        cz_acc_t *a = &s_acc[l];
        a->cand = -1;
        a->outi = -1;
        fi->nblobs++;
        uint8_t st;
        float len = 0, wid = 0, ang = 0, cx = 0, cy = 0;
        if (a->n < p->min_pix) {
            st = CZ_SMALL;
        } else if (a->touch) {
            st = CZ_EDGE;
        } else if (a->n > p->max_pix) {
            st = CZ_BIG;
        } else {
            st = CZ_OK;
        }
        if (a->n) {
            // float is sufficient: E[x^2] ~ 1e5, float eps ~ 1e-7 -> variance error << 1 px^2
            float n = (float) a->n;
            float mx = (float) a->sx / n, my = (float) a->sy / n;
            float cxx = (float) a->sxx / n - mx * mx + (1.0f / 12.0f);   // + pixel extent
            float cyy = (float) a->syy / n - my * my + (1.0f / 12.0f);
            float cxy = (float) a->sxy / n - mx * my;
            float d = sqrtf((cxx - cyy) * (cxx - cyy) + 4.0f * cxy * cxy);
            float l1 = 0.5f * (cxx + cyy + d), l2 = 0.5f * (cxx + cyy - d);
            if (l2 < 0.0f) l2 = 0.0f;
            len = 4.0f * sqrtf(l1) * p->mm_px;
            wid = 4.0f * sqrtf(l2) * p->mm_px;
            ang = 0.5f * atan2f(2.0f * cxy, cxx - cyy) * 57.2957795f;
            cx = (float) mx; cy = (float) my;
        }
        if (st == CZ_OK) {
            if (len > p->lmax_mm * 1.1f) st = CZ_LONG;
            else if (wid > 0 && (len / wid > p->ar_max || len / wid < p->ar_min)) st = CZ_ASPECT;
            else if (wid > p->wmax_mm) st = CZ_ASPECT;
        }
        if (st == CZ_OK) {
            if (ncand < CZ_MAX_CAND) {
                a->cand = (int8_t) ncand;
                for (int yy = a->y0; yy <= a->y1; yy++) {
                    s_left[ncand][yy] = INT16_MAX; s_right[ncand][yy] = -1;
                }
                memset(s_csum[ncand], 0, sizeof(s_csum[0]));
                memset(s_ccnt[ncand], 0, sizeof(s_ccnt[0]));
                ncand++;
            } else {
                fi->overflow++;
            }
        }
        if (st != CZ_SMALL && nout < max_out) {
            cz_blob_t *o = &out[nout++];
            o->status = st;
            o->x0 = a->x0; o->y0 = a->y0; o->x1 = a->x1; o->y1 = a->y1;
            o->area = a->n;
            o->cx = cx; o->cy = cy;
            o->len_mm = len; o->wid_mm = wid; o->angle_deg = ang;
            o->solidity = 0; o->r = o->g = o->b = 0;
            a->outi = (int16_t) (nout - 1);
        }
        if (st != CZ_OK || a->cand < 0) fi->count[st]++;
    }

    // Stage 5: second pass for candidates -- hull extents + colour sums.
    if (ncand) {
        const uint8_t *map = cfa_map[p->cfa & 3];
        for (int i = 0; i < nruns; i++) {
            cz_acc_t *a = &s_acc[uf_find(s_runs[i].lab)];
            int c = a->cand;
            if (c < 0) continue;
            int y = s_runs[i].y, x0 = s_runs[i].x0, x1 = s_runs[i].x1;
            if (x0 < s_left[c][y]) s_left[c][y] = (int16_t) x0;
            if (x1 > s_right[c][y]) s_right[c][y] = (int16_t) x1;
            const uint8_t *row = raw + (size_t) y * w;
            const uint8_t *m = &map[(y & 1) * 2];
            for (int x = x0; x <= x1; x++) {
                int ch = m[x & 1];
                s_csum[c][ch] += row[x];
                s_ccnt[c][ch]++;
            }
        }
        for (int l = 0; l < nlab; l++) {
            if (s_parent[l] != l) continue;
            cz_acc_t *a = &s_acc[l];
            int c = a->cand;
            if (c < 0) continue;
            int np = 0;
            for (int y = a->y0; y <= a->y1; y++) {
                if (s_right[c][y] < 0) continue;
                int16_t L = s_left[c][y], R = (int16_t) (s_right[c][y] + 1);
                s_pts[np][0] = L; s_pts[np][1] = (int16_t) y; np++;
                s_pts[np][0] = L; s_pts[np][1] = (int16_t) (y + 1); np++;
                s_pts[np][0] = R; s_pts[np][1] = (int16_t) y; np++;
                s_pts[np][0] = R; s_pts[np][1] = (int16_t) (y + 1); np++;
            }
            float ha = hull_area(np);
            float sol = ha > 0 ? (float) a->n / ha : 0.0f;
            uint8_t st = (sol < p->sol_min) ? CZ_CONCAVE : CZ_OK;
            if (st == CZ_OK && p->neck_px > 0) {
                for (int y = a->y0; y <= a->y1; y++) {
                    memset(s_lm[0][y], 0, (size_t) mw * 4);
                    memset(s_lm[1][y], 0, (size_t) mw * 4);
                }
                for (int i = 0; i < nruns; i++) {
                    if (uf_find(s_runs[i].lab) != (uint16_t) l) continue;
                    int y = s_runs[i].y, x0 = s_runs[i].x0, x1 = s_runs[i].x1;
                    for (int x = x0; x <= x1; ) {           // set bits x0..x1
                        int wi = x >> 5, b0 = x & 31;
                        int b1 = (x1 >> 5) == wi ? (x1 & 31) : 31;
                        uint32_t m = (b1 - b0 == 31) ? 0xFFFFFFFFu : (((1u << (b1 - b0 + 1)) - 1) << b0);
                        s_lm[0][y][wi] |= m;
                        x = (wi << 5) + b1 + 1;
                    }
                }
                if (neck_split(a->n, a->y0, a->y1, a->x0, a->x1, p->neck_px, 0.20f, 0.10f)) st = CZ_NECK;
            }
            fi->count[st]++;
            int oi = a->outi;
            if (oi >= 0 && oi < nout) {
                cz_blob_t *o = &out[oi];
                o->status = st;
                o->solidity = sol;
                o->r = s_ccnt[c][0] ? (float) s_csum[c][0] / s_ccnt[c][0] : 0;
                o->g = s_ccnt[c][1] ? (float) s_csum[c][1] / s_ccnt[c][1] : 0;
                o->b = s_ccnt[c][2] ? (float) s_csum[c][2] / s_ccnt[c][2] : 0;
            }
        }
    }
    // Stage 6: exclusivity along travel. A valid kernel must own its stretch of belt:
    // no other object (side by side, staggered, overlapping without touching) may have an
    // x-extent within gap_mm of its own, or one ejector blast would hit both.
    if (p->gap_mm >= 0.0f) {
        int gap_px = (int) (p->gap_mm / p->mm_px + 0.5f);
        for (int l = 0; l < nlab; l++) {
            if (s_parent[l] != l) continue;
            cz_acc_t *a = &s_acc[l];
            if (a->outi < 0 || out[a->outi].status != CZ_OK) continue;
            for (int m = 0; m < nlab; m++) {
                if (m == l || s_parent[m] != m) continue;
                cz_acc_t *o = &s_acc[m];
                if (o->n < p->crowd_min_pix) continue;
                if ((int) o->x1 + gap_px >= (int) a->x0 && (int) o->x0 <= (int) a->x1 + gap_px) {
                    out[a->outi].status = CZ_CROWDED;
                    fi->count[CZ_OK]--;
                    fi->count[CZ_CROWDED]++;
                    break;
                }
            }
        }
    }
    fi->nout = (uint32_t) nout;
    return nout;
}

// ---- de-duplication --------------------------------------------------------------
void cz_dedup_init(cz_dedup_t *d, float tol_mm, float tol_y_mm) {
    for (int i = 0; i < CZ_DEDUP_N; i++) { d->pos[i] = -1e30f; d->y[i] = -1e30f; }
    d->head = 0;
    d->tol_mm = tol_mm;
    d->tol_y_mm = tol_y_mm;
}

bool cz_dedup_first(cz_dedup_t *d, float pos_mm, float y_mm) {
    for (int i = 0; i < CZ_DEDUP_N; i++) {
        if (fabsf(pos_mm - d->pos[i]) < d->tol_mm && fabsf(y_mm - d->y[i]) < d->tol_y_mm) return false;
    }
    d->pos[d->head] = pos_mm;
    d->y[d->head] = y_mm;
    d->head = (d->head + 1) % CZ_DEDUP_N;
    return true;
}

#endif // !CORE_M55_HE
