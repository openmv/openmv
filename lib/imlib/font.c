/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2026 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * TrueType font loader, rasterizer and text layout.
 */
#include <string.h>
#include "imlib.h"
#include "font.h"
#include "umalloc.h"

// Deepest nesting of composite glyphs followed; deeper components are not drawn.
#ifndef FONT_MAX_COMPOSITE_DEPTH
#define FONT_MAX_COMPOSITE_DEPTH    (4)
#endif

// Most line segments a curve is flattened into.
#ifndef FONT_MAX_CURVE_SEGMENTS
#define FONT_MAX_CURVE_SEGMENTS     (16)
#endif

#define FONT_TAG(a, b, c, d)        (((uint32_t) (a) << 24) | ((uint32_t) (b) << 16) | ((uint32_t) (c) << 8) | (d))

// Simple glyph flags.
#define GLYF_ON_CURVE               (0x01)
#define GLYF_X_SHORT                (0x02)
#define GLYF_Y_SHORT                (0x04)
#define GLYF_REPEAT                 (0x08)
#define GLYF_X_SAME                 (0x10)
#define GLYF_Y_SAME                 (0x20)

// Composite glyph flags.
#define COMP_ARGS_ARE_WORDS         (0x0001)
#define COMP_ARGS_ARE_XY_VALUES     (0x0002)
#define COMP_HAVE_SCALE             (0x0008)
#define COMP_MORE_COMPONENTS        (0x0020)
#define COMP_HAVE_XY_SCALE          (0x0040)
#define COMP_HAVE_2X2               (0x0080)

// Glyph cache entry key bits.
#define CACHE_KEY_MASK              (0x3FFF)    // Pixel size and x offset.
#define CACHE_USED                  (0x8000)    // Drawn since eviction last passed over it.

// Pixel sizes are multiples of 1 / FONT_PX_STEPS, so that a cache key says exactly how a
// glyph was rasterized.
#define FONT_PX_STEPS               (16)

// A cache smaller than this is not worth having.
#define CACHE_MIN_SIZE              (1024)

// Arabic joining classes.
#define AR_NONE     (0)     // Not Arabic.
#define AR_U        (1)     // Arabic but non-joining (hamza, punctuation, digits).
#define AR_R        (2)     // Right-joining: joins to the preceding letter only.
#define AR_D        (3)     // Dual-joining.
#define AR_C        (4)     // Join-causing (tatweel).
#define AR_T        (5)     // Transparent (marks).

static inline uint16_t rd16(const uint8_t *p) {
    return (p[0] << 8) | p[1];
}

static inline int16_t rds16(const uint8_t *p) {
    return (int16_t) rd16(p);
}

// fast_floorf() and fast_ceilf() are a single instruction, but on the Cortex-M4 that
// instruction truncates toward zero, which is only floor/ceil for values >= 0. Text
// coordinates can be negative (ink left of or above the origin), so correct the result
// where the sign is not known. On other cores the correction is never taken.
static inline int font_floorf(float x) {
    int i = fast_floorf(x);
    return i - (i > x);
}

static inline int font_ceilf(float x) {
    int i = fast_ceilf(x);
    return i - ((i - 1) >= x);
}

static inline uint32_t rd32(const uint8_t *p) {
    return ((uint32_t) p[0] << 24) | ((uint32_t) p[1] << 16) | ((uint32_t) p[2] << 8) | p[3];
}

// Find a table in the directory, returning its offset and length (0 if missing).
static font_err_t font_table(const font_t *f, uint32_t tag, uint32_t *offset, uint32_t *len) {
    uint16_t n = rd16(f->data + 4);
    for (uint32_t i = 0, rec = 12; i < n; i++, rec += 16) {
        if (rec + 16 > f->size) {
            break;
        }
        if (rd32(f->data + rec) == tag) {
            uint32_t off = rd32(f->data + rec + 8);
            uint32_t l = rd32(f->data + rec + 12);
            if ((off > f->size) || (l > (f->size - off))) {
                return FONT_ERR_TABLE_BOUNDS;
            }
            *offset = off;
            *len = l;
            return FONT_OK;
        }
    }
    *offset = 0;
    *len = 0;
    return FONT_OK;
}

static font_err_t font_require(const font_t *f, uint32_t tag, uint32_t min_len, uint32_t *offset, uint32_t *len) {
    font_err_t err = font_table(f, tag, offset, len);
    if (err != FONT_OK) {
        return err;
    }
    if ((!*len) || (*len < min_len)) {
        return FONT_ERR_TABLE_MISSING;
    }
    return FONT_OK;
}

font_err_t font_load(font_t *f, const void *data, size_t size, float px) {
    memset(f, 0, sizeof(*f));
    f->data = data;
    f->size = size;
    f->px = px;

    if (size < 12) {
        return FONT_ERR_NOT_TRUETYPE;
    }
    uint32_t ver = rd32(f->data);
    if (ver == FONT_TAG('t', 't', 'c', 'f')) {
        return FONT_ERR_COLLECTION;
    }
    if (ver == FONT_TAG('O', 'T', 'T', 'O')) {
        return FONT_ERR_CFF;
    }
    if ((ver != 0x00010000) && (ver != FONT_TAG('t', 'r', 'u', 'e'))) {
        return FONT_ERR_NOT_TRUETYPE;
    }

    font_err_t err;
    uint32_t off, len;
    if ((err = font_require(f, FONT_TAG('h', 'e', 'a', 'd'), 54, &off, &len)) != FONT_OK) {
        return err;
    }
    const uint8_t *head = f->data + off;
    f->units_per_em = rd16(head + 18);
    f->loca_long = rds16(head + 50) != 0;
    if (!f->units_per_em) {
        return FONT_ERR_UNITS;
    }

    if ((err = font_require(f, FONT_TAG('m', 'a', 'x', 'p'), 6, &off, &len)) != FONT_OK) {
        return err;
    }
    const uint8_t *maxp = f->data + off;
    f->num_glyphs = rd16(maxp + 4);
    if (len >= 32) {
        f->max_points = rd16(maxp + 6);
        f->max_contours = rd16(maxp + 8);
    }
    if (!f->num_glyphs) {
        return FONT_ERR_NO_GLYPHS;
    }

    if ((err = font_require(f, FONT_TAG('h', 'h', 'e', 'a'), 36, &off, &len)) != FONT_OK) {
        return err;
    }
    const uint8_t *hhea = f->data + off;
    f->ascender = rds16(hhea + 4);
    f->descender = rds16(hhea + 6);
    f->line_gap = rds16(hhea + 8);
    f->num_hmetrics = rd16(hhea + 34);
    if ((!f->num_hmetrics) || (f->num_hmetrics > f->num_glyphs)) {
        return FONT_ERR_METRICS;
    }

    if ((err = font_require(f, FONT_TAG('h', 'm', 't', 'x'), f->num_hmetrics * 4, &f->hmtx, &len)) != FONT_OK) {
        return err;
    }
    if ((err = font_require(f, FONT_TAG('l', 'o', 'c', 'a'), (f->num_glyphs + 1) * (f->loca_long ? 4 : 2),
                            &f->loca, &len)) != FONT_OK) {
        return err;
    }
    if ((err = font_table(f, FONT_TAG('g', 'l', 'y', 'f'), &f->glyf, &f->glyf_len)) != FONT_OK) {
        return err;
    }
    if (!f->glyf) {
        return FONT_ERR_TABLE_MISSING;
    }

    // Pick the best Unicode cmap subtable: full repertoire (3,10)/(0,4+) first, then BMP.
    uint32_t cmap;
    if ((err = font_require(f, FONT_TAG('c', 'm', 'a', 'p'), 4, &cmap, &len)) != FONT_OK) {
        return err;
    }
    uint16_t n = rd16(f->data + cmap + 2);
    int best = -1;
    for (uint32_t i = 0, rec = cmap + 4; i < n; i++, rec += 8) {
        if ((rec + 8) > (cmap + len)) {
            break;
        }
        uint16_t pid = rd16(f->data + rec);
        uint16_t eid = rd16(f->data + rec + 2);
        uint32_t off = rd32(f->data + rec + 4);
        if (off >= len) {
            continue;
        }
        uint32_t sub = cmap + off;
        uint16_t fmt = rd16(f->data + sub);
        int rank;
        if ((pid == 3) && (eid == 10)) {
            rank = 4;
        } else if ((pid == 0) && (eid >= 4)) {
            rank = 3;
        } else if ((pid == 3) && (eid == 1)) {
            rank = 2;
        } else if (pid == 0) {
            rank = 1;
        } else {
            continue;
        }
        if (fmt == 4) {
            uint16_t sub_len = rd16(f->data + sub + 2);
            uint16_t segx2 = rd16(f->data + sub + 6);
            if ((sub_len < 16) || (segx2 < 2) || ((sub + 16 + segx2 * 4) > f->size)) {
                continue;
            }
        } else if (fmt == 12) {
            if ((sub + 16) > f->size) {
                continue;
            }
            uint32_t groups = rd32(f->data + sub + 12);
            if (((uint64_t) sub + 16 + ((uint64_t) groups * 12)) > f->size) {
                continue;
            }
        } else {
            continue;
        }
        if (rank > best) {
            best = rank;
            f->cmap = sub;
            f->cmap_format = fmt;
        }
    }
    if (best < 0) {
        return FONT_ERR_NO_CMAP;
    }
    return FONT_OK;
}

// Glyph id for a code point (0 = .notdef).
static uint16_t font_lookup(const font_t *f, uint32_t cp) {
    const uint8_t *sub = f->data + f->cmap;
    if (f->cmap_format == 4) {
        if (cp > 0xFFFF) {
            return 0;
        }
        uint32_t segs = rd16(sub + 6) / 2;
        const uint8_t *ends = sub + 14;
        const uint8_t *starts = ends + segs * 2 + 2;
        const uint8_t *deltas = starts + segs * 2;
        const uint8_t *ros = deltas + segs * 2;
        uint32_t lo = 0;
        uint32_t hi = segs;
        while (lo < hi) {
            uint32_t mid = (lo + hi) / 2;
            if (rd16(ends + mid * 2) < cp) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        if (lo >= segs) {
            return 0;
        }
        uint32_t start = rd16(starts + lo * 2);
        if (cp < start) {
            return 0;
        }
        uint32_t ro = rd16(ros + lo * 2);
        uint32_t g;
        if (!ro) {
            g = cp;
        } else {
            const uint8_t *p = ros + lo * 2 + ro + (cp - start) * 2;
            if ((p + 2) > (f->data + f->size)) {
                return 0;
            }
            g = rd16(p);
            if (!g) {
                return 0;
            }
        }
        g = (g + rd16(deltas + lo * 2)) & 0xFFFF;
        return (g < f->num_glyphs) ? g : 0;
    } else {
        uint32_t lo = 0;
        uint32_t hi = rd32(sub + 12);
        const uint8_t *groups = sub + 16;
        while (lo < hi) {
            uint32_t mid = (lo + hi) / 2;
            const uint8_t *grp = groups + mid * 12;
            if (rd32(grp + 4) < cp) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        if (lo >= rd32(sub + 12)) {
            return 0;
        }
        const uint8_t *grp = groups + lo * 12;
        uint32_t start = rd32(grp);
        if (cp < start) {
            return 0;
        }
        uint32_t g = rd32(grp + 8) + (cp - start);
        return (g < f->num_glyphs) ? g : 0;
    }
}

static uint16_t font_advance(const font_t *f, uint16_t gid) {
    if (gid >= f->num_hmetrics) {
        gid = f->num_hmetrics - 1;
    }
    return rd16(f->data + f->hmtx + gid * 4);
}

// Locate a glyph's outline. Returns false for an empty glyph, and false with *err set
// for a glyph record that points outside the glyf table.
static bool font_glyph(const font_t *f, uint16_t gid, const uint8_t **glyph, uint32_t *len, font_err_t *err) {
    uint32_t off, next;
    if (gid >= f->num_glyphs) {
        return false;
    }
    if (f->loca_long) {
        off = rd32(f->data + f->loca + gid * 4);
        next = rd32(f->data + f->loca + gid * 4 + 4);
    } else {
        off = rd16(f->data + f->loca + gid * 2) * 2;
        next = rd16(f->data + f->loca + gid * 2 + 2) * 2;
    }
    if ((next < off) || (next > f->glyf_len)) {
        *err = FONT_ERR_GLYPH;
        return false;
    }
    if ((next - off) < 10) {
        return false;
    }
    *glyph = f->data + f->glyf + off;
    *len = next - off;
    return true;
}

// UTF-8 decoder. Invalid sequences decode to U+FFFD and consume one byte.
static uint32_t utf8_next(const char **s) {
    const uint8_t *p = (const uint8_t *) *s;
    uint32_t c = *p++;
    int n = 0;
    if (c >= 0xF0) {
        if (c > 0xF4) {
            *s = (const char *) p;
            return 0xFFFD;
        }
        c &= 0x07;
        n = 3;
    } else if (c >= 0xE0) {
        c &= 0x0F;
        n = 2;
    } else if (c >= 0xC0) {
        c &= 0x1F;
        n = 1;
    } else if (c >= 0x80) {
        *s = (const char *) p;
        return 0xFFFD;
    }
    for (int i = 0; i < n; i++) {
        if ((p[i] & 0xC0) != 0x80) {
            *s = (const char *) p;
            return 0xFFFD;
        }
        c = (c << 6) | (p[i] & 0x3F);
    }
    *s = (const char *) (p + n);
    return c;
}

///////////////////////////////////////////////////////////////////////////////
// Rasterizer: signed-area accumulation (one float cell per pixel, w + 2 stride).
///////////////////////////////////////////////////////////////////////////////

typedef struct raster {
    float *acc;
    int w;
    int h;
    int stride;
} raster_t;

// Accumulate the signed area and cover of the line (x0,y0)-(x1,y1) in pixel coordinates.
static void raster_line(raster_t *r, float x0, float y0, float x1, float y1) {
    if (y0 == y1) {
        return;
    }
    float dir = 1.0f;
    if (y0 > y1) {
        float t;
        t = x0; x0 = x1; x1 = t;
        t = y0; y0 = y1; y1 = t;
        dir = -1.0f;
    }
    // Clamp x into the buffer so a glyph whose bbox lies cannot write out of bounds.
    float xmax = (float) r->w;
    x0 = IM_CLAMP(x0, 0.0f, xmax);
    x1 = IM_CLAMP(x1, 0.0f, xmax);
    float dxdy = (x1 - x0) / (y1 - y0);
    float x = x0;
    int ya = fast_floorf(IM_MAX(y0, 0.0f));
    int yb = IM_MIN(r->h, fast_ceilf(y1));
    if (y0 < 0.0f) {
        x -= y0 * dxdy;
    }
    for (int y = ya; y < yb; y++) {
        float *line = r->acc + (y * r->stride);
        float dy = IM_MIN((float) (y + 1), y1) - IM_MAX((float) y, y0);
        float xnext = x + (dxdy * dy);
        float d = dy * dir;
        float xa, xb;
        if (x < xnext) {
            xa = x; xb = xnext;
        } else {
            xa = xnext; xb = x;
        }
        xa = IM_CLAMP(xa, 0.0f, xmax);
        xb = IM_CLAMP(xb, 0.0f, xmax);
        int xai = fast_floorf(xa);
        int xbi = fast_ceilf(xb);
        float xa_floor = xai;
        float xb_ceil = xbi;
        if (xbi <= xai + 1) {
            float xmf = (0.5f * (xa + xb)) - xa_floor;
            line[xai] += d - (d * xmf);
            line[xai + 1] += d * xmf;
        } else {
            float s = 1.0f / (xb - xa);
            float xaf = xa - xa_floor;
            float a0 = 0.5f * s * (1.0f - xaf) * (1.0f - xaf);
            float xbf = xb - xb_ceil + 1.0f;
            float am = 0.5f * s * xbf * xbf;
            line[xai] += d * a0;
            if (xbi == xai + 2) {
                line[xai + 1] += d * (1.0f - a0 - am);
            } else {
                float a1 = s * (1.5f - xaf);
                line[xai + 1] += d * (a1 - a0);
                for (int xi = xai + 2; xi < xbi - 1; xi++) {
                    line[xi] += d * s;
                }
                float a2 = a1 + ((xbi - xai - 3) * s);
                line[xbi - 1] += d * (1.0f - a2 - am);
            }
            line[xbi] += d * am;
        }
        x = xnext;
    }
}

// Flatten a quadratic Bezier into lines.
static void raster_quad(raster_t *r, float x0, float y0, float cx, float cy, float x1, float y1) {
    float ddx = x0 - (2.0f * cx) + x1;
    float ddy = y0 - (2.0f * cy) + y1;
    float dd = fast_sqrtf((ddx * ddx) + (ddy * ddy));
    int n = IM_CLAMP(fast_ceilf(fast_sqrtf(dd * 2.0f)), 1, FONT_MAX_CURVE_SEGMENTS);
    float px = x0;
    float py = y0;
    for (int i = 1; i <= n; i++) {
        float t = ((float) i) / n;
        float u = 1.0f - t;
        float nx = (u * u * x0) + (2.0f * u * t * cx) + (t * t * x1);
        float ny = (u * u * y0) + (2.0f * u * t * cy) + (t * t * y1);
        raster_line(r, px, py, nx, ny);
        px = nx;
        py = ny;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Outline decoding
///////////////////////////////////////////////////////////////////////////////

typedef struct glyph_scratch {
    int16_t *xs;
    int16_t *ys;
    uint8_t *on;
    uint16_t *ends;
} glyph_scratch_t;

// Affine map from font units to pixels: px = ox + (a*x + c*y + e) * s, py = oy - (b*x + d*y + f) * s.
typedef struct glyph_xform {
    float a, b, c, d, e, f;
    float s;
    float ox, oy;
} glyph_xform_t;

static inline void xform_apply(const glyph_xform_t *t, int x, int y, float *px, float *py) {
    *px = t->ox + (((t->a * x) + (t->c * y) + t->e) * t->s);
    *py = t->oy - (((t->b * x) + (t->d * y) + t->f) * t->s);
}

static void glyph_contour(raster_t *r, const glyph_xform_t *t, const glyph_scratch_t *g, int first, int last) {
    int n = last - first + 1;
    if (n < 2) {
        return;
    }
    // Find a starting on-curve point, or synthesize one between two off-curve points.
    int start = -1;
    for (int i = 0; i < n; i++) {
        if (g->on[first + i]) {
            start = i;
            break;
        }
    }
    float sx, sy;
    if (start < 0) {
        float ax, ay, bx, by;
        xform_apply(t, g->xs[first], g->ys[first], &ax, &ay);
        xform_apply(t, g->xs[first + 1], g->ys[first + 1], &bx, &by);
        sx = 0.5f * (ax + bx);
        sy = 0.5f * (ay + by);
        start = 0;
    } else {
        xform_apply(t, g->xs[first + start], g->ys[first + start], &sx, &sy);
    }
    // Current on-curve point.
    float cx = sx;
    float cy = sy;
    // Pending control point.
    float qx = 0;
    float qy = 0;
    bool have_q = false;
    // Walk every point after the start (wrapping back onto it), then close to the start.
    for (int k = 1; k <= n + 1; k++) {
        float px, py;
        bool on;
        if (k <= n) {
            int i = first + ((start + k) % n);
            xform_apply(t, g->xs[i], g->ys[i], &px, &py);
            on = g->on[i];
        } else {
            px = sx;
            py = sy;
            on = true;
        }
        if (on) {
            if (have_q) {
                raster_quad(r, cx, cy, qx, qy, px, py);
                have_q = false;
            } else {
                raster_line(r, cx, cy, px, py);
            }
            cx = px;
            cy = py;
        } else if (have_q) {
            // Two off-curve points in a row imply an on-curve midpoint.
            float mx = 0.5f * (qx + px);
            float my = 0.5f * (qy + py);
            raster_quad(r, cx, cy, qx, qy, mx, my);
            cx = mx;
            cy = my;
            qx = px;
            qy = py;
        } else {
            qx = px;
            qy = py;
            have_q = true;
        }
    }
}

static font_err_t glyph_raster(const font_t *f, uint16_t gid, raster_t *r, const glyph_xform_t *t,
                               const glyph_scratch_t *g, int depth);

// Decodes one axis of a simple glyph's points. A coordinate is a delta from the one before:
// a byte whose sign is the "same" flag, or a word, or (the "same" flag alone) no change.
// Returns the position after the data, or NULL if it runs past the end of the glyph.
static const uint8_t *glyph_coords(const uint8_t *p, const uint8_t *end, const uint8_t *flags, int npts,
                                   int16_t *out, uint8_t is_short, uint8_t is_same) {
    int v = 0;
    for (int i = 0; i < npts; i++) {
        uint8_t fl = flags[i];
        if (fl & is_short) {
            if (p >= end) {
                return NULL;
            }
            v += (fl & is_same) ? *p : -(*p);
            p++;
        } else if (!(fl & is_same)) {
            if ((p + 2) > end) {
                return NULL;
            }
            v += rds16(p);
            p += 2;
        }
        out[i] = v;
    }
    return p;
}

static font_err_t glyph_simple(const font_t *f, const uint8_t *p, uint32_t len, int nc, raster_t *r,
                               const glyph_xform_t *t, const glyph_scratch_t *g) {
    const uint8_t *end = p + len;
    p += 10;
    if ((nc > f->max_contours) || ((p + nc * 2 + 2) > end)) {
        return FONT_ERR_GLYPH;
    }
    int npts = 0;
    for (int i = 0; i < nc; i++) {
        g->ends[i] = rd16(p);
        p += 2;
        if (g->ends[i] + 1 < npts) {
            return FONT_ERR_GLYPH;
        }
        npts = g->ends[i] + 1;
    }
    if (npts > f->max_points) {
        return FONT_ERR_GLYPH;
    }
    p += 2 + rd16(p); // Skip instructions.
    // Flags.
    for (int i = 0; i < npts;) {
        if (p >= end) {
            return FONT_ERR_GLYPH;
        }
        uint8_t fl = *p++;
        int rep = 1;
        if (fl & GLYF_REPEAT) {
            if (p >= end) {
                return FONT_ERR_GLYPH;
            }
            rep += *p++;
        }
        for (; rep && (i < npts); rep--, i++) {
            g->on[i] = fl;
        }
    }
    // Coordinates (deltas): all the x, then all the y.
    p = glyph_coords(p, end, g->on, npts, g->xs, GLYF_X_SHORT, GLYF_X_SAME);
    p = p ? glyph_coords(p, end, g->on, npts, g->ys, GLYF_Y_SHORT, GLYF_Y_SAME) : NULL;
    if (!p) {
        return FONT_ERR_GLYPH;
    }
    for (int i = 0; i < npts; i++) {
        g->on[i] &= GLYF_ON_CURVE;
    }
    for (int i = 0, first = 0; i < nc; i++) {
        glyph_contour(r, t, g, first, g->ends[i]);
        first = g->ends[i] + 1;
    }
    return FONT_OK;
}

static font_err_t glyph_composite(const font_t *f, const uint8_t *p, uint32_t len, raster_t *r,
                                  const glyph_xform_t *t, const glyph_scratch_t *g, int depth) {
    const uint8_t *end = p + len;
    p += 10;
    for (;;) {
        if ((p + 4) > end) {
            return FONT_ERR_GLYPH;
        }
        uint16_t flags = rd16(p);
        uint16_t gid = rd16(p + 2);
        p += 4;
        int dx = 0;
        int dy = 0;
        if (flags & COMP_ARGS_ARE_WORDS) {
            if ((p + 4) > end) {
                return FONT_ERR_GLYPH;
            }
            if (flags & COMP_ARGS_ARE_XY_VALUES) {
                dx = rds16(p);
                dy = rds16(p + 2);
            }
            p += 4;
        } else {
            if ((p + 2) > end) {
                return FONT_ERR_GLYPH;
            }
            if (flags & COMP_ARGS_ARE_XY_VALUES) {
                dx = (int8_t) p[0];
                dy = (int8_t) p[1];
            }
            p += 2;
        }
        // Point-matching placement (ARGS_ARE_XY_VALUES clear) is treated as a zero offset.
        float a = 1.0f;
        float b = 0.0f;
        float c = 0.0f;
        float d = 1.0f;
        if (flags & COMP_HAVE_SCALE) {
            if ((p + 2) > end) {
                return FONT_ERR_GLYPH;
            }
            a = d = rds16(p) / 16384.0f;
            p += 2;
        } else if (flags & COMP_HAVE_XY_SCALE) {
            if ((p + 4) > end) {
                return FONT_ERR_GLYPH;
            }
            a = rds16(p) / 16384.0f;
            d = rds16(p + 2) / 16384.0f;
            p += 4;
        } else if (flags & COMP_HAVE_2X2) {
            if ((p + 8) > end) {
                return FONT_ERR_GLYPH;
            }
            a = rds16(p) / 16384.0f;
            b = rds16(p + 2) / 16384.0f;
            c = rds16(p + 4) / 16384.0f;
            d = rds16(p + 6) / 16384.0f;
            p += 8;
        }
        // Compose: parent(child(x)) where child(x) = [a b; c d] x + (dx, dy).
        glyph_xform_t ct;
        ct.a = (t->a * a) + (t->c * b);
        ct.b = (t->b * a) + (t->d * b);
        ct.c = (t->a * c) + (t->c * d);
        ct.d = (t->b * c) + (t->d * d);
        ct.e = (t->a * dx) + (t->c * dy) + t->e;
        ct.f = (t->b * dx) + (t->d * dy) + t->f;
        ct.s = t->s;
        ct.ox = t->ox;
        ct.oy = t->oy;
        font_err_t err = glyph_raster(f, gid, r, &ct, g, depth + 1);
        if (err != FONT_OK) {
            return err;
        }
        if (!(flags & COMP_MORE_COMPONENTS)) {
            break;
        }
    }
    return FONT_OK;
}

static font_err_t glyph_raster(const font_t *f, uint16_t gid, raster_t *r, const glyph_xform_t *t,
                               const glyph_scratch_t *g, int depth) {
    const uint8_t *p;
    uint32_t len;
    font_err_t err = FONT_OK;
    if ((depth > FONT_MAX_COMPOSITE_DEPTH) || (!font_glyph(f, gid, &p, &len, &err))) {
        return err;
    }
    int nc = rds16(p);
    if (nc >= 0) {
        return glyph_simple(f, p, len, nc, r, t, g);
    } else {
        return glyph_composite(f, p, len, r, t, g, depth);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Arabic shaping
///////////////////////////////////////////////////////////////////////////////
// Arabic letters change shape with their neighbours and read right to left. Instead
// of OpenType layout tables this maps each letter to the Unicode presentation form
// the font carries (isolated, final, initial and medial forms are consecutive code
// points in the FBxx/FExx blocks), joins lam and alef into their ligature, keeps
// marks on their letter, and reverses each Arabic run for display with numbers left
// to right inside it. Callers pass text in logical order, as typed.

typedef struct arabic_form {
    uint16_t base;      // The letter.
    uint16_t iso;       // Isolated form; final = iso + 1, initial = iso + 2, medial = iso + 3.
    uint8_t dual;       // 1: joins on both sides (4 forms), 0: joins to the right only (2 forms).
} arabic_form_t;

// Generated from Unicode decomposition data, sorted by base code point.
static const arabic_form_t arabic_forms[] = {
    {0x0622, 0xFE81, 0}, // ARABIC LETTER ALEF WITH MADDA ABOVE
    {0x0623, 0xFE83, 0}, // ARABIC LETTER ALEF WITH HAMZA ABOVE
    {0x0624, 0xFE85, 0}, // ARABIC LETTER WAW WITH HAMZA ABOVE
    {0x0625, 0xFE87, 0}, // ARABIC LETTER ALEF WITH HAMZA BELOW
    {0x0626, 0xFE89, 1}, // ARABIC LETTER YEH WITH HAMZA ABOVE
    {0x0627, 0xFE8D, 0}, // ARABIC LETTER ALEF
    {0x0628, 0xFE8F, 1}, // ARABIC LETTER BEH
    {0x0629, 0xFE93, 0}, // ARABIC LETTER TEH MARBUTA
    {0x062A, 0xFE95, 1}, // ARABIC LETTER TEH
    {0x062B, 0xFE99, 1}, // ARABIC LETTER THEH
    {0x062C, 0xFE9D, 1}, // ARABIC LETTER JEEM
    {0x062D, 0xFEA1, 1}, // ARABIC LETTER HAH
    {0x062E, 0xFEA5, 1}, // ARABIC LETTER KHAH
    {0x062F, 0xFEA9, 0}, // ARABIC LETTER DAL
    {0x0630, 0xFEAB, 0}, // ARABIC LETTER THAL
    {0x0631, 0xFEAD, 0}, // ARABIC LETTER REH
    {0x0632, 0xFEAF, 0}, // ARABIC LETTER ZAIN
    {0x0633, 0xFEB1, 1}, // ARABIC LETTER SEEN
    {0x0634, 0xFEB5, 1}, // ARABIC LETTER SHEEN
    {0x0635, 0xFEB9, 1}, // ARABIC LETTER SAD
    {0x0636, 0xFEBD, 1}, // ARABIC LETTER DAD
    {0x0637, 0xFEC1, 1}, // ARABIC LETTER TAH
    {0x0638, 0xFEC5, 1}, // ARABIC LETTER ZAH
    {0x0639, 0xFEC9, 1}, // ARABIC LETTER AIN
    {0x063A, 0xFECD, 1}, // ARABIC LETTER GHAIN
    {0x0641, 0xFED1, 1}, // ARABIC LETTER FEH
    {0x0642, 0xFED5, 1}, // ARABIC LETTER QAF
    {0x0643, 0xFED9, 1}, // ARABIC LETTER KAF
    {0x0644, 0xFEDD, 1}, // ARABIC LETTER LAM
    {0x0645, 0xFEE1, 1}, // ARABIC LETTER MEEM
    {0x0646, 0xFEE5, 1}, // ARABIC LETTER NOON
    {0x0647, 0xFEE9, 1}, // ARABIC LETTER HEH
    {0x0648, 0xFEED, 0}, // ARABIC LETTER WAW
    {0x064A, 0xFEF1, 1}, // ARABIC LETTER YEH
    {0x0671, 0xFB50, 0}, // ARABIC LETTER ALEF WASLA
    {0x0679, 0xFB66, 1}, // ARABIC LETTER TTEH
    {0x067A, 0xFB5E, 1}, // ARABIC LETTER TTEHEH
    {0x067B, 0xFB52, 1}, // ARABIC LETTER BEEH
    {0x067E, 0xFB56, 1}, // ARABIC LETTER PEH
    {0x067F, 0xFB62, 1}, // ARABIC LETTER TEHEH
    {0x0680, 0xFB5A, 1}, // ARABIC LETTER BEHEH
    {0x0683, 0xFB76, 1}, // ARABIC LETTER NYEH
    {0x0684, 0xFB72, 1}, // ARABIC LETTER DYEH
    {0x0686, 0xFB7A, 1}, // ARABIC LETTER TCHEH
    {0x0687, 0xFB7E, 1}, // ARABIC LETTER TCHEHEH
    {0x0688, 0xFB88, 0}, // ARABIC LETTER DDAL
    {0x068C, 0xFB84, 0}, // ARABIC LETTER DAHAL
    {0x068D, 0xFB82, 0}, // ARABIC LETTER DDAHAL
    {0x068E, 0xFB86, 0}, // ARABIC LETTER DUL
    {0x0691, 0xFB8C, 0}, // ARABIC LETTER RREH
    {0x0698, 0xFB8A, 0}, // ARABIC LETTER JEH
    {0x06A4, 0xFB6A, 1}, // ARABIC LETTER VEH
    {0x06A6, 0xFB6E, 1}, // ARABIC LETTER PEHEH
    {0x06A9, 0xFB8E, 1}, // ARABIC LETTER KEHEH
    {0x06AD, 0xFBD3, 1}, // ARABIC LETTER NG
    {0x06AF, 0xFB92, 1}, // ARABIC LETTER GAF
    {0x06B1, 0xFB9A, 1}, // ARABIC LETTER NGOEH
    {0x06B3, 0xFB96, 1}, // ARABIC LETTER GUEH
    {0x06BA, 0xFB9E, 0}, // ARABIC LETTER NOON GHUNNA
    {0x06BB, 0xFBA0, 1}, // ARABIC LETTER RNOON
    {0x06BE, 0xFBAA, 1}, // ARABIC LETTER HEH DOACHASHMEE
    {0x06C0, 0xFBA4, 0}, // ARABIC LETTER HEH WITH YEH ABOVE
    {0x06C1, 0xFBA6, 1}, // ARABIC LETTER HEH GOAL
    {0x06C5, 0xFBE0, 0}, // ARABIC LETTER KIRGHIZ OE
    {0x06C6, 0xFBD9, 0}, // ARABIC LETTER OE
    {0x06C7, 0xFBD7, 0}, // ARABIC LETTER U
    {0x06C8, 0xFBDB, 0}, // ARABIC LETTER YU
    {0x06C9, 0xFBE2, 0}, // ARABIC LETTER KIRGHIZ YU
    {0x06CB, 0xFBDE, 0}, // ARABIC LETTER VE
    {0x06CC, 0xFBFC, 1}, // ARABIC LETTER FARSI YEH
    {0x06D0, 0xFBE4, 1}, // ARABIC LETTER E
    {0x06D2, 0xFBAE, 0}, // ARABIC LETTER YEH BARREE
    {0x06D3, 0xFBB0, 0}, // ARABIC LETTER YEH BARREE WITH HAMZA ABOVE
};

static bool arabic_cp(uint32_t cp) {
    return ((cp >= 0x0600) && (cp <= 0x06FF)) || ((cp >= 0x0750) && (cp <= 0x077F)) ||
           ((cp >= 0xFB50) && (cp <= 0xFDFF)) || ((cp >= 0xFE70) && (cp <= 0xFEFF));
}

static bool arabic_mark(uint32_t cp) {
    return ((cp >= 0x0610) && (cp <= 0x061A)) || ((cp >= 0x064B) && (cp <= 0x065F)) || (cp == 0x0670) ||
           ((cp >= 0x06D6) && (cp <= 0x06DC)) || ((cp >= 0x06DF) && (cp <= 0x06E4)) ||
           (cp == 0x06E7) || (cp == 0x06E8) || ((cp >= 0x06EA) && (cp <= 0x06ED));
}

static const arabic_form_t *arabic_form(uint32_t cp) {
    int lo = 0;
    int hi = (sizeof(arabic_forms) / sizeof(arabic_forms[0])) - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (arabic_forms[mid].base < cp) {
            lo = mid + 1;
        } else if (arabic_forms[mid].base > cp) {
            hi = mid - 1;
        } else {
            return &arabic_forms[mid];
        }
    }
    return NULL;
}

// Alef maksura (U+0649) is the one letter whose forms are not consecutive.
static const uint16_t arabic_alef_maksura[4] = { 0xFEEF, 0xFEF0, 0xFBE8, 0xFBE9 };

static uint8_t arabic_class(uint32_t cp) {
    if (!arabic_cp(cp)) {
        return AR_NONE;
    }
    if (arabic_mark(cp)) {
        return AR_T;
    }
    if (cp == 0x0640) {
        return AR_C;
    }
    if (cp == 0x0649) {
        return AR_D;
    }
    const arabic_form_t *f = arabic_form(cp);
    return f ? (f->dual ? AR_D : AR_R) : AR_U;
}

// Lam + alef ligature code point for an alef variant, or 0.
static uint32_t arabic_lam_alef(uint32_t alef) {
    switch (alef) {
        case 0x0622: return 0xFEF5;
        case 0x0623: return 0xFEF7;
        case 0x0625: return 0xFEF9;
        case 0x0627: return 0xFEFB;
        default: return 0;
    }
}

// The letter a presentation form came from, or 0 (used when the font lacks the form).
static uint32_t arabic_unshape(uint32_t cp) {
    if ((cp >= 0xFEF5) && (cp <= 0xFEFC)) {
        return 0x0644;
    }
    for (size_t i = 0; i < 4; i++) {
        if (arabic_alef_maksura[i] == cp) {
            return 0x0649;
        }
    }
    for (size_t i = 0; i < (sizeof(arabic_forms) / sizeof(arabic_forms[0])); i++) {
        const arabic_form_t *f = &arabic_forms[i];
        if ((cp >= f->iso) && (cp < (f->iso + (f->dual ? 4 : 2)))) {
            return f->base;
        }
    }
    return 0;
}

static bool arabic_neutral(uint32_t cp) {
    return (cp == ' ') || ((cp >= '0') && (cp <= '9')) || (cp == '.') || (cp == ',') || (cp == ':') ||
           (cp == '%') || (cp == '-') || (cp == '+') || (cp == '/') || (cp == 0x00B0);
}

static inline bool arabic_digit(uint32_t cp) {
    return ((cp >= '0') && (cp <= '9')) || ((cp >= 0x0660) && (cp <= 0x0669)) || ((cp >= 0x06F0) && (cp <= 0x06F9));
}

static void arabic_reverse(uint32_t *cps, int lo, int hi) {
    for (hi--; lo < hi; lo++, hi--) {
        uint32_t t = cps[lo];
        cps[lo] = cps[hi];
        cps[hi] = t;
    }
}

// Gives the Arabic letters of a paragraph their contextual forms, in place and still in
// reading order. Returns the new length.
static int arabic_shape(uint32_t *cps, uint8_t *cls, int n) {
    for (int i = 0; i < n; i++) {
        cls[i] = arabic_class(cps[i]);
    }
    // Contextual forms.
    for (int i = 0; i < n; i++) {
        if ((cls[i] != AR_D) && (cls[i] != AR_R)) {
            continue;
        }
        int j = i - 1;
        int k = i + 1;
        while ((j >= 0) && (cls[j] == AR_T)) {
            j--;
        }
        while ((k < n) && (cls[k] == AR_T)) {
            k++;
        }
        bool prev = (j >= 0) && ((cls[j] == AR_D) || (cls[j] == AR_C));
        bool next = (cls[i] == AR_D) && (k < n) && ((cls[k] == AR_D) || (cls[k] == AR_R) || (cls[k] == AR_C));
        uint32_t lig;
        if ((cps[i] == 0x0644) && (k < n) && ((lig = arabic_lam_alef(cps[k])) != 0)) {
            cps[i] = lig + (prev ? 1 : 0);
            cls[i] = AR_R;      // The ligature does not join to the next letter.
            cps[k] = 0;         // The alef is consumed.
            cls[k] = AR_T;
            continue;
        }
        int form = (prev ? 1 : 0) + (next ? 2 : 0);
        if (cps[i] == 0x0649) {
            cps[i] = arabic_alef_maksura[form];
        } else {
            cps[i] = arabic_form(cps[i])->iso + form;
        }
    }
    // Drop consumed code points.
    int m = 0;
    for (int i = 0; i < n; i++) {
        if (cps[i]) {
            cps[m++] = cps[i];
        }
    }
    return m;
}

// Puts one line of shaped code points in the order they are drawn, left to right: each
// Arabic run (letters plus the spaces, digits and punctuation between them) is reversed,
// then marks are put back after their letter and numbers back in reading order.
static void arabic_reorder(uint32_t *cps, int n) {
    for (int i = 0; i < n;) {
        if (!arabic_cp(cps[i])) {
            i++;
            continue;
        }
        int j = i;
        while ((j < n) && (arabic_cp(cps[j]) || arabic_neutral(cps[j]))) {
            j++;
        }
        while ((j > i) && arabic_neutral(cps[j - 1])) {
            j--;
        }
        arabic_reverse(cps, i, j);
        for (int k = i; k < j;) {
            if (arabic_mark(cps[k])) {
                int l = k;
                while ((l < j) && arabic_mark(cps[l])) {
                    l++;
                }
                if (l < j) {
                    arabic_reverse(cps, k, l + 1);
                }
                k = l + 1;
            } else if (arabic_digit(cps[k])) {
                int l = k;
                while ((l < j) && (arabic_digit(cps[l]) ||
                                   (((cps[l] == '.') || (cps[l] == ',')) && ((l + 1) < j) && arabic_digit(cps[l + 1])))) {
                    l++;
                }
                arabic_reverse(cps, k, l);
                k = l;
            } else {
                k++;
            }
        }
        i = j;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Glyph cache
///////////////////////////////////////////////////////////////////////////////
// One block of caller provided memory: this module keeps only a pointer to it. The block
// starts with the cache's own state, then a hash table of 16-bit slots, then the character
// table (below), then a ring of entries packed end to end. An entry is this header followed by w * h coverage bytes,
// padded to 4 bytes. Offsets (in the hash table and the chains) are in units of 4 bytes
// from the start of the block; 0 means none, which no entry can have since the state is
// there.
typedef struct cache_entry {
    uint32_t font;      // Complement of the font file's address, 0 if the entry is dead. The
                        // complement keeps a garbage collector that scans this block from
                        // taking the key for a reference to the font.
    uint16_t gid;
    uint16_t key;       // Pixel size in 1/16ths (bits 0-11), quarter pixel x offset (12-13),
                        // and CACHE_USED.
    uint16_t next;      // Next entry with the same hash.
    uint8_t w;          // Of the bitmap. Where it goes follows from the glyph and the key.
    uint8_t h;
    uint8_t stroke;     // Pixels of outline in the bitmap.
} cache_entry_t;

// What laying out a character needs: which font of the fallback chain draws it, the glyph,
// its advance and its bounding box, in that font's units, so one entry serves every size.
// Looking these up means searching the font's character map and reading three more of its
// tables, for each font of the chain that lacks the character, twice per draw. This table
// is direct mapped: a character overwrites whatever shared its slot.
typedef struct cache_char {
    uint32_t chain;             // Identifies the font and its fallbacks; 0 if the slot is empty.
    uint32_t cp;                // Code point (bits 0-23) and the font's place in the chain (24-31).
    uint16_t gid;
    uint16_t advance;
    int16_t x0, y0, x1, y1;     // All zero for a glyph with no outline.
} cache_char_t;

typedef struct font_cache {
    uint32_t mask;              // Hash slots - 1.
    uint32_t char_mask;         // Character table entries - 1.
    uint32_t chars;             // Offset of the character table in the block.
    uint32_t size;              // Of the block.
    uint32_t start, end;        // The ring is [start, end) in the block.
    uint32_t head, tail;        // Oldest entry and write position.
    uint32_t wrap;              // Where the entries before the wrap point stop.
    uint32_t wrapped;           // The newest entries are at the start, before head.
    uint32_t glyphs, hits, misses, evictions;
    uint16_t slots[];           // Hash table.
} font_cache_t;

// The block; NULL when there is no cache.
static font_cache_t *cache;

static inline cache_entry_t *cache_at(uint32_t off) {
    return (cache_entry_t *) (((uint8_t *) cache) + off);
}

static inline uint32_t cache_entry_size(int w, int h) {
    return (sizeof(cache_entry_t) + (w * h) + 3) & ~3;
}

static inline uint32_t cache_hash(uint32_t font, uint32_t gid, uint32_t key, uint32_t stroke) {
    uint32_t h = (font * 2654435761u) ^ (gid * 40503u) ^ (((key & CACHE_KEY_MASK) | (stroke << 16)) * 2246822519u);
    return (h ^ (h >> 15)) & cache->mask;
}

static void cache_link(uint32_t off) {
    cache_entry_t *e = cache_at(off);
    uint16_t *slot = &cache->slots[cache_hash(e->font, e->gid, e->key, e->stroke)];
    e->next = *slot;
    *slot = off >> 2;
}

static void cache_unlink(uint32_t off) {
    cache_entry_t *e = cache_at(off);
    uint16_t *link = &cache->slots[cache_hash(e->font, e->gid, e->key, e->stroke)];
    while (*link && (*link != (off >> 2))) {
        link = &cache_at(*link << 2)->next;
    }
    if (*link) {
        *link = e->next;
    }
}

static cache_entry_t *cache_find(uint32_t font, uint32_t gid, uint32_t key, uint32_t stroke) {
    for (uint32_t i = cache->slots[cache_hash(font, gid, key, stroke)]; i; ) {
        cache_entry_t *e = cache_at(i << 2);
        if ((e->font == font) && (e->gid == gid) && ((e->key & CACHE_KEY_MASK) == key) && (e->stroke == stroke)) {
            return e;
        }
        i = e->next;
    }
    return NULL;
}

// Space for an entry of n bytes at the write position, dropping the oldest entries as
// needed. An entry that was used since it was last looked at here is not dropped: it is
// moved to the write position and gets another lap. Returns the offset.
static uint32_t cache_alloc(uint32_t n) {
    for (;;) {
        if (!cache->wrapped) {
            if (cache->head == cache->tail) {
                // Empty.
                cache->head = cache->tail = cache->start;
            }
            if (n <= (cache->end - cache->tail)) {
                break;
            }
            // No room before the end: carry on from the start, behind the oldest entries.
            cache->wrap = cache->tail;
            cache->tail = cache->start;
            cache->wrapped = true;
        }
        if (n <= (cache->head - cache->tail)) {
            break;
        }
        // Make room by passing over the oldest entry.
        cache_entry_t *e = cache_at(cache->head);
        uint32_t size = cache_entry_size(e->w, e->h);
        if (e->font && (e->key & CACHE_USED)) {
            cache_unlink(cache->head);
            e->key &= ~CACHE_USED;
            if (cache->tail != cache->head) {
                memmove(((uint8_t *) cache) + cache->tail, e, size);
            }
            cache_link(cache->tail);
            cache->tail += size;
        } else if (e->font) {
            cache_unlink(cache->head);
            cache->glyphs--;
            cache->evictions++;
        }
        cache->head += size;
        if (cache->head == cache->wrap) {
            cache->head = cache->start;
            cache->wrapped = false;
        }
    }
    uint32_t off = cache->tail;
    cache->tail += n;
    return off;
}

void font_cache_init(void *buf, size_t size) {
    cache = NULL;
    // Offsets are 16 bits of 4 byte units.
    size = IM_MIN(size, (size_t) 0x3FFFC) & ~3;
    if ((!buf) || (size < CACHE_MIN_SIZE)) {
        return;
    }
    // About one slot per 64 bytes and one character per 128, both powers of two.
    uint32_t slots = 16;
    while ((slots * 2 * 64) <= size) {
        slots *= 2;
    }
    uint32_t chars = slots / 2;
    cache = buf;
    cache->chars = sizeof(font_cache_t) + (slots * sizeof(uint16_t));
    cache->start = cache->chars + (chars * sizeof(cache_char_t));
    memset(cache, 0, cache->start);
    cache->chars = sizeof(font_cache_t) + (slots * sizeof(uint16_t));
    cache->start = cache->head = cache->tail = cache->chars + (chars * sizeof(cache_char_t));
    cache->mask = slots - 1;
    cache->char_mask = chars - 1;
    cache->size = size;
    cache->end = size;
}

void font_cache_purge(const font_t *f) {
    if (!cache) {
        return;
    }
    // A character entry does not say which fonts its chain had, and this is rare: drop them all.
    memset(((uint8_t *) cache) + cache->chars, 0, (cache->char_mask + 1) * sizeof(cache_char_t));
    uint32_t font = ~(uint32_t) (uintptr_t) f->data;
    // Walk the ring: [head, wrap) then [start, tail) when wrapped, else [head, tail).
    uint32_t off = cache->head;
    uint32_t stop = cache->wrapped ? cache->wrap : cache->tail;
    for (int pass = cache->wrapped ? 2 : 1; pass; pass--) {
        while (off < stop) {
            cache_entry_t *e = cache_at(off);
            if (e->font == font) {
                cache_unlink(off);
                e->font = 0;
                cache->glyphs--;
            }
            off += cache_entry_size(e->w, e->h);
        }
        off = cache->start;
        stop = cache->tail;
    }
}

void font_cache_info(font_cache_info_t *info) {
    memset(info, 0, sizeof(*info));
    if (!cache) {
        return;
    }
    info->size = cache->size;
    info->used = cache->wrapped ? ((cache->wrap - cache->head) + (cache->tail - cache->start)) : (cache->tail - cache->head);
    info->glyphs = cache->glyphs;
    info->hits = cache->hits;
    info->misses = cache->misses;
    info->evictions = cache->evictions;
}

///////////////////////////////////////////////////////////////////////////////
// Layout
///////////////////////////////////////////////////////////////////////////////

typedef struct font_pass {
    const font_t *f;
    float px;               // Pixel em size of this pass (font size times scale).
    float s;                // Pixels per font unit of f.
    float line_h;           // Line advance in whole pixels.
    float baseline;         // First baseline below the origin, in whole pixels.
    float ink_x0, ink_y0, ink_x1, ink_y1;
    float layout_x0, layout_x1; // Extent of the lines, outline included.
    float wrap;             // Widest a line may be before it is broken (0 = any width).
    font_align_t align;
    int stroke;             // Pixels of outline.
    int lines;
    font_err_t err;         // First error hit while walking the string.
    uint32_t chain;         // Identifies f and its fallbacks in the character cache.
    // Render only.
    const font_box_t *box;
    image_t *out;
    glyph_scratch_t g;
    float *acc;
    size_t acc_size;
} font_pass_t;

// A character as the layout sees it (see cache_char_t).
typedef struct font_char {
    const font_t *f;
    uint16_t gid;
    uint16_t advance;
    int16_t x0, y0, x1, y1;
} font_char_t;

static const font_t *font_select(const font_t *f, uint32_t cp, uint16_t *gid);

// Looks a character up, in the cache if there is one. Returns false (with ps->err set) if
// the glyph record is corrupt.
static bool font_char(font_pass_t *ps, uint32_t cp, font_char_t *ch) {
    cache_char_t *c = NULL;
    if (cache) {
        uint32_t h = (ps->chain * 2654435761u) ^ (cp * 40503u);
        c = ((cache_char_t *) (((uint8_t *) cache) + cache->chars)) + ((h ^ (h >> 15)) & cache->char_mask);
        if ((c->chain == ps->chain) && ((c->cp & 0xFFFFFF) == cp)) {
            ch->f = ps->f;
            for (uint32_t depth = c->cp >> 24; depth; depth--) {
                ch->f = ch->f->fallback;
            }
            ch->gid = c->gid;
            ch->advance = c->advance;
            ch->x0 = c->x0;
            ch->y0 = c->y0;
            ch->x1 = c->x1;
            ch->y1 = c->y1;
            return true;
        }
    }
    const uint8_t *p;
    uint32_t len;
    ch->f = font_select(ps->f, cp, &ch->gid);
    ch->advance = font_advance(ch->f, ch->gid);
    ch->x0 = ch->y0 = ch->x1 = ch->y1 = 0;
    if (font_glyph(ch->f, ch->gid, &p, &len, &ps->err)) {
        ch->x0 = rds16(p + 2);
        ch->y0 = rds16(p + 4);
        ch->x1 = rds16(p + 6);
        ch->y1 = rds16(p + 8);
    } else if (ps->err != FONT_OK) {
        return false;
    }
    if (c) {
        uint32_t depth = 0;
        for (const font_t *f = ps->f; f != ch->f; f = f->fallback) {
            depth++;
        }
        c->chain = ps->chain;
        c->cp = cp | (depth << 24);
        c->gid = ch->gid;
        c->advance = ch->advance;
        c->x0 = ch->x0;
        c->y0 = ch->y0;
        c->x1 = ch->x1;
        c->y1 = ch->y1;
    }
    return true;
}

// The font in the fallback chain that has a glyph for cp (the first font if none has).
static const font_t *font_select(const font_t *f, uint32_t cp, uint16_t *gid) {
    for (const font_t *c = f; c; c = c->fallback) {
        uint16_t g = font_lookup(c, cp);
        if (g) {
            *gid = g;
            return c;
        }
    }
    *gid = 0;
    return f;
}

bool font_has(const font_t *f, const char *str) {
    for (uint32_t cp; (cp = utf8_next(&str));) {
        uint16_t gid;
        if (cp >= ' ') {
            font_select(f, cp, &gid);
            if (!gid) {
                return false;
            }
        }
    }
    return true;
}

// Max-blend a glyph's coverage bitmap into the output at (x, y) in layout coordinates.
static void font_blit(font_pass_t *ps, const uint8_t *bitmap, int w, int h, int x, int y) {
    image_t *out = ps->out;
    x -= ps->box->x;
    y -= ps->box->y;
    for (int yy = 0; yy < h; yy++, bitmap += w) {
        if (((y + yy) < 0) || ((y + yy) >= out->h)) {
            continue;
        }
        uint8_t *row = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(out, y + yy);
        for (int xx = 0; xx < w; xx++) {
            int c = bitmap[xx];
            if (((x + xx) >= 0) && ((x + xx) < out->w) && (c > row[x + xx])) {
                row[x + xx] = c;
            }
        }
    }
}

// Turns a glyph's coverage into the outlined form font_render() describes: out is the glyph
// grown by r pixels on every side, each pixel of the glyph spread over the disc of pixels
// within r of it (those the disc only partly covers get part of it, which rounds the
// corners and keeps the edge smooth), and then marked with where the glyph itself is.
static void font_stroke(const uint8_t *cov, int w, int h, uint8_t *out, int r) {
    int ow = w + (2 * r);
    memset(out, 0, ow * (h + (2 * r)));
    for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
            float reach = (r + 1) - fast_sqrtf((dx * dx) + (dy * dy));
            if (reach <= 0.0f) {
                continue;
            }
            int k = (reach >= 1.0f) ? 256 : fast_roundf(reach * 256.0f);
            const uint8_t *src = cov;
            uint8_t *dst = out + ((r + dy) * ow) + (r + dx);
            for (int y = 0; y < h; y++, src += w, dst += ow) {
                for (int x = 0; x < w; x++) {
                    int v = (src[x] * k) >> 8;
                    if (v > dst[x]) {
                        dst[x] = v;
                    }
                }
            }
        }
    }
    // The outline alone takes the lower half of the range, the glyph the upper half.
    for (int i = 0, n = ow * (h + (2 * r)); i < n; i++) {
        out[i] >>= 1;
    }
    uint8_t *dst = out + (r * ow) + r;
    for (int y = 0; y < h; y++, cov += w, dst += ow) {
        for (int x = 0; x < w; x++) {
            if (cov[x]) {
                dst[x] = 128 + (cov[x] >> 1);
            }
        }
    }
}

static void font_place(font_pass_t *ps, const font_char_t *ch, float s, float pen_x, float baseline) {
    const font_t *f = ch->f;
    uint16_t gid = ch->gid;
    int stroke = ps->stroke;
    // The pen is placed on a quarter pixel, so a glyph has at most four renderings at a size
    // and each can be cached. px and py are the pen's whole pixel and the (whole) baseline.
    int px = font_floorf(pen_x);
    int phase = fast_roundf((pen_x - px) * 4.0f);
    if (phase == 4) {
        px += 1;
        phase = 0;
    }
    int py = fast_roundf(baseline);
    // Glyph bbox in pixels, relative to (px, py).
    float x0 = (phase * 0.25f) + (ch->x0 * s);
    float y1 = -(ch->y0 * s);
    float x1 = (phase * 0.25f) + (ch->x1 * s);
    float y0 = -(ch->y1 * s);
    if ((x1 <= x0) || (y1 <= y0)) {
        return;
    }
    ps->ink_x0 = IM_MIN(ps->ink_x0, px + x0 - stroke);
    ps->ink_y0 = IM_MIN(ps->ink_y0, py + y0 - stroke);
    ps->ink_x1 = IM_MAX(ps->ink_x1, px + x1 + stroke);
    ps->ink_y1 = IM_MAX(ps->ink_y1, py + y1 + stroke);
    if (!ps->out) {
        return;
    }

    // The glyph's bitmap, and the same with the outline around it.
    int bx = font_floorf(x0);
    int by = font_floorf(y0);
    int bw = font_ceilf(x1) - bx;
    int bh = font_ceilf(y1) - by;
    int ox = px + bx - stroke;
    int oy = py + by - stroke;
    int ow = bw + (2 * stroke);
    int oh = bh + (2 * stroke);

    // In the cache? The key is the font, the glyph, the pixel size, the x offset and the
    // outline, which together fix the bitmap and where it goes.
    uint32_t font = ~(uint32_t) (uintptr_t) f->data;
    uint32_t key = fast_roundf(ps->px * FONT_PX_STEPS);
    uint32_t entry_size = cache_entry_size(ow, oh);
    bool cacheable = cache && (ow <= 255) && (oh <= 255) && (key <= 0xFFF) &&
                     (entry_size <= ((cache->end - cache->start) / 4));
    key |= phase << 12;
    if (cacheable) {
        cache_entry_t *e = cache_find(font, gid, key, stroke);
        if (e) {
            e->key |= CACHE_USED;
            cache->hits++;
            font_blit(ps, (const uint8_t *) (e + 1), e->w, e->h, ox, oy);
            return;
        }
        cache->misses++;
    }

    // Room to accumulate the glyph, and for its outlined form after its coverage.
    raster_t r = { .w = bw, .h = bh, .stride = bw + 2 };
    size_t acc_size = (size_t) r.stride * bh * sizeof(float);
    size_t size = IM_MAX(acc_size, stroke ? ((size_t) (bw * bh) + (ow * oh)) : 0);
    if (size > ps->acc_size) {
        if (ps->acc) {
            uma_free(ps->acc);
        }
        ps->acc = uma_malloc(size, UMA_DTCM);
        ps->acc_size = size;
    }
    r.acc = ps->acc;
    memset(r.acc, 0, acc_size);

    glyph_xform_t t = {
        .a = 1.0f, .b = 0.0f, .c = 0.0f, .d = 1.0f, .e = 0.0f, .f = 0.0f,
        .s = s, .ox = (phase * 0.25f) - bx, .oy = -by
    };
    ps->err = glyph_raster(f, gid, &r, &t, &ps->g, 0);
    if (ps->err != FONT_OK) {
        return;
    }

    // Prefix-sum the accumulation buffer into coverage bytes. This is done in place: byte
    // (y * bw) + x is written after the float at ((y * stride) + x) * 4 has been read, and
    // never lies beyond it.
    uint8_t *bitmap = (uint8_t *) r.acc;
    uint8_t *b = bitmap;
    const float *a = r.acc;
    float sum = 0.0f;
    for (int y = 0; y < bh; y++) {
        for (int x = 0; x < r.stride; x++) {
            sum += *a++;
            if (x < bw) {
                *b++ = IM_MIN(fast_roundf(fast_fabsf(sum) * 255.0f), 255);
            }
        }
    }
    if (stroke) {
        font_stroke(bitmap, bw, bh, b, stroke);
        bitmap = b;
    }

    if (cacheable) {
        cache_entry_t *e = cache_at(cache_alloc(entry_size));
        e->font = font;
        e->gid = gid;
        e->key = key;
        e->w = ow;
        e->h = oh;
        e->stroke = stroke;
        memcpy(e + 1, bitmap, ow * oh);
        cache_link((uint8_t *) e - (uint8_t *) cache);
        cache->glyphs++;
    }
    font_blit(ps, bitmap, ow, oh, ox, oy);
}

// Looks a character of shaped text up. Returns false (with ps->err set) if the glyph record
// is corrupt.
static bool font_shaped_char(font_pass_t *ps, uint32_t cp, bool arabic, font_char_t *ch) {
    if (!font_char(ps, cp, ch)) {
        return false;
    }
    if (!ch->gid && arabic) {
        // No font has this presentation form: draw the plain letter instead.
        uint32_t base = arabic_unshape(cp);
        if (base && (!font_char(ps, base, ch))) {
            return false;
        }
    }
    return true;
}

// Finds where the line starting at cps[first] of a shaped paragraph of len code points
// ends when lines are wrapped: at its last run of spaces (which is dropped), or, when it has
// none, before the first character that does not fit. Returns the line's width, the end of
// the line in *last and the start of the next one in *next.
static float font_line_fit(font_pass_t *ps, const uint32_t *cps, int first, int len, bool arabic, int *last, int *next) {
    float width = 0.0f;
    float space_width = 0.0f;
    int space = -1;
    *last = *next = len;
    for (int i = first; i < len; i++) {
        font_char_t ch;
        if (!font_shaped_char(ps, cps[i], arabic, &ch)) {
            break;
        }
        // A combining mark sits over the letter before it and does not move the pen.
        float advance = (arabic && arabic_mark(cps[i]) && (i > first)) ? 0.0f :
                        (ch.advance * ((ch.f == ps->f) ? ps->s : (ps->px / ch.f->units_per_em)));
        if ((ps->wrap > 0.0f) && ((width + advance) > ps->wrap) && (i > first) && (cps[i] != ' ')) {
            if (space > first) {
                *last = space;
                *next = space + 1;
                width = space_width;
            } else {
                *last = *next = i;
            }
            while ((*next < len) && (cps[*next] == ' ')) {
                (*next)++;
            }
            break;
        }
        if ((cps[i] == ' ') && ((i == first) || (cps[i - 1] != ' '))) {
            space = i;
            space_width = width;
        }
        width += advance;
    }
    return width;
}

static void font_walk(font_pass_t *ps, const char *str) {
    // One paragraph of code points at a time, so Arabic can be shaped with its neighbours.
    size_t n = strlen(str) + 1;
    uint32_t *cps = uma_malloc((n * sizeof(uint32_t)) + n, UMA_DTCM);
    uint8_t *cls = (uint8_t *) (cps + n);
    float baseline = ps->baseline;
    ps->lines = 0;
    for (bool end = false; (!end) && (ps->err == FONT_OK);) {
        int len = 0;
        bool arabic = false;
        for (uint32_t cp;;) {
            cp = utf8_next(&str);
            if (!cp) {
                end = true;
                break;
            }
            if (cp == '\r') {
                if (*str == '\n') {
                    str++;
                }
                break;
            }
            if (cp == '\n') {
                break;
            }
            if (cp < ' ') {
                continue;
            }
            cps[len++] = cp;
            arabic |= arabic_cp(cp);
        }
        if (arabic) {
            len = arabic_shape(cps, cls, len);
        }
        // The paragraph is one line, or as many as it takes to fit the wrap width.
        for (int first = 0, next = len; ps->err == FONT_OK; first = next, next = len) {
            font_char_t ch;
            float width = 0.0f;
            int last = len;
            if ((ps->wrap > 0.0f) || (ps->align != FONT_ALIGN_LEFT)) {
                // Where the line ends and how wide it is have to be known before it is drawn.
                int fit_last, fit_next;
                width = font_line_fit(ps, cps, first, len, arabic, &fit_last, &fit_next);
                last = fit_last;
                next = fit_next;
            }
            if (arabic) {
                arabic_reorder(cps + first, last - first);
            }
            // Lines start on a whole pixel whatever the alignment, so a glyph is rasterized
            // (and cached) at the same offsets as in left aligned text.
            float pen_x = ps->stroke;
            float prev_x = 0.0f;
            float prev_adv = 0.0f;
            if (ps->align == FONT_ALIGN_CENTER) {
                pen_x = -fast_roundf(width * 0.5f);
            } else if (ps->align == FONT_ALIGN_RIGHT) {
                pen_x = -fast_ceilf(width) - ps->stroke;
            }
            ps->layout_x0 = IM_MIN(ps->layout_x0, pen_x - ps->stroke);
            for (int i = first; (i < last) && (ps->err == FONT_OK); i++) {
                if (!font_shaped_char(ps, cps[i], arabic, &ch)) {
                    break;
                }
                float s = (ch.f == ps->f) ? ps->s : (ps->px / ch.f->units_per_em);
                if (arabic && arabic_mark(cps[i]) && (i > first)) {
                    // Centre the mark over its letter. Fonts position marks with OpenType
                    // tables we do not read, so do it here.
                    float mark_cx = (ch.x0 + ch.x1) * 0.5f * s;
                    font_place(ps, &ch, s, prev_x + (prev_adv * 0.5f) - mark_cx, baseline);
                    continue;
                }
                font_place(ps, &ch, s, pen_x, baseline);
                prev_x = pen_x;
                prev_adv = ch.advance * s;
                pen_x += prev_adv;
            }
            ps->layout_x1 = IM_MAX(ps->layout_x1, pen_x + ps->stroke);
            ps->lines++;
            baseline += ps->line_h;
            if (next >= len) {
                break;
            }
        }
    }
    uma_free(cps);
}

static void font_pass_init(font_pass_t *ps, const font_t *f, const font_layout_t *layout) {
    memset(ps, 0, sizeof(*ps));
    ps->f = f;
    ps->px = IM_MAX(fast_roundf(f->px * layout->scale * FONT_PX_STEPS), 1) * (1.0f / FONT_PX_STEPS);
    ps->s = ps->px / f->units_per_em;
    ps->align = layout->align;
    ps->stroke = IM_CLAMP(layout->stroke, 0, FONT_STROKE_MAX);
    // The outline is part of the text's width.
    ps->wrap = (layout->wrap_width > 0) ? IM_MAX(layout->wrap_width - (2 * ps->stroke), 1) : 0.0f;
    // Nonzero, and different for the same font with different fallbacks.
    for (const font_t *c = f; c; c = c->fallback) {
        ps->chain = (ps->chain * 31) + (uint32_t) (uintptr_t) c->data;
    }
    ps->chain |= 1;
    // Baselines sit on whole pixels, so every line is rasterized at the same vertical phase
    // and a font's pixel metrics (ascender, line height) are exact. Advances stay fractional.
    ps->baseline = fast_roundf(f->ascender * ps->s) + ps->stroke;
    ps->line_h = IM_MAX(fast_roundf((f->ascender - f->descender + f->line_gap) * ps->s), 1);
    ps->ink_x0 = ps->ink_y0 = 1e30f;
    ps->ink_x1 = ps->ink_y1 = -1e30f;
}

font_err_t font_measure(const font_t *f, const char *str, const font_layout_t *layout, font_box_t *box) {
    font_pass_t ps;
    font_pass_init(&ps, f, layout);
    font_walk(&ps, str);
    if (ps.err != FONT_OK) {
        memset(box, 0, sizeof(*box));
        return ps.err;
    }
    float layout_h = (ps.lines * ps.line_h) + (2 * ps.stroke);
    float x0 = IM_MIN(ps.layout_x0, ps.ink_x0);
    float y0 = IM_MIN(0.0f, ps.ink_y0);
    float x1 = IM_MAX(ps.layout_x1, ps.ink_x1);
    float y1 = IM_MAX(layout_h, ps.ink_y1);
    box->x = font_floorf(x0);
    box->y = font_floorf(y0);
    box->w = font_ceilf(x1) - box->x;
    box->h = font_ceilf(y1) - box->y;
    box->baseline = ps.baseline;
    return FONT_OK;
}

font_err_t font_render(const font_t *f, const char *str, const font_layout_t *layout, const font_box_t *box,
                       image_t *out) {
    font_pass_t ps;
    font_pass_init(&ps, f, layout);
    ps.box = box;
    ps.out = out;
    memset(out->data, 0, image_size(out));
    // Outline scratch sized for the largest glyph in the fallback chain.
    size_t n = 0;
    size_t nc = 0;
    for (const font_t *c = f; c; c = c->fallback) {
        n = IM_MAX(n, c->max_points);
        nc = IM_MAX(nc, c->max_contours);
    }
    ps.g.xs = uma_malloc((n * 5) + (nc * 2), UMA_DTCM);
    ps.g.ys = ps.g.xs + n;
    ps.g.ends = (uint16_t *) (ps.g.ys + n);
    ps.g.on = (uint8_t *) (ps.g.ends + nc);
    font_walk(&ps, str);
    if (ps.acc) {
        uma_free(ps.acc);
    }
    uma_free(ps.g.xs);
    return ps.err;
}
