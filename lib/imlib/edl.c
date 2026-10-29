/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2026 OpenMV, LLC.
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
 * Edge Drawing Lines (EDL) - Line Segment Detector.
 *
 * A fast line segment detector based on the Edge Drawing algorithm.
 * Reference: Akinlar & Topal, "EDLines: A real-time line segment detector
 * with a false detection control", Pattern Recognition Letters, 2011.
 */
#include "imlib.h"
#include "umalloc.h"

#ifdef IMLIB_ENABLE_FIND_LINE_SEGMENTS

// Algorithm parameters
#define ED_ANCHOR_SCAN_INTERVAL     2       // Stride for anchor point scanning
#define ED_MIN_LINE_LENGTH          10      // Minimum edge chain length for line fitting (pixels)
#define ED_LINE_FIT_ERROR           1.4f    // Max perpendicular distance for line fit (pixels)

// 8 edge directions at 22.5 degree intervals.
// Edge angle in screen coords (y-down): 0=right, increases counterclockwise.
// Finer quantization than 4 bins prevents tracing failures at intermediate angles.
#define NUM_DIRS    8

typedef struct {
    int16_t x, y;
} ed_point_t;

// Forward tracing candidates for each direction (3 neighbors closest to edge angle).
// Backward candidates are the negation of these.
static const int8_t trace_fwd[NUM_DIRS][3][2] = {
    [0] = {{ 1, 0}, { 1, -1}, { 1, 1}},   // 0 deg (right)
    [1] = {{ 1, 0}, { 1, -1}, { 0, -1}},   // 22.5 deg
    [2] = {{ 1, -1}, { 1, 0}, { 0, -1}},   // 45 deg (up-right)
    [3] = {{ 0, -1}, { 1, -1}, { 1, 0}},   // 67.5 deg
    [4] = {{ 0, -1}, {-1, -1}, { 1, -1}},   // 90 deg (up)
    [5] = {{ 0, -1}, {-1, -1}, {-1, 0}},   // 112.5 deg
    [6] = {{-1, -1}, { 0, -1}, {-1, 0}},   // 135 deg (up-left)
    [7] = {{-1, 0}, {-1, -1}, {-1, 1}},   // 157.5 deg (left)
};

// NMS perpendicular neighbor pairs for each direction.
// Each pair checks pixels on opposite sides across the edge.
static const int8_t nms_perp[NUM_DIRS][2][2] = {
    [0] = {{ 0, -1}, { 0, 1}},   // perp to 0 -> above/below
    [1] = {{ 1, 1}, {-1, -1}},   // perp to 22.5 -> lower-right/upper-left
    [2] = {{ 1, 1}, {-1, -1}},   // perp to 45 -> lower-right/upper-left
    [3] = {{ 1, 0}, {-1, 0}},   // perp to 67.5 -> right/left
    [4] = {{ 1, 0}, {-1, 0}},   // perp to 90 -> right/left
    [5] = {{ 1, -1}, {-1, 1}},   // perp to 112.5 -> upper-right/lower-left
    [6] = {{ 1, -1}, {-1, 1}},   // perp to 135 -> upper-right/lower-left
    [7] = {{ 0, -1}, { 0, 1}},   // perp to 157.5 -> above/below
};

// Compute Sobel gradient magnitude and quantized direction over grayscale image.
// Border pixels are left at zero (mag) / 0 (dir).
static void compute_gradient(const uint8_t *gray, int w, int h,
                             uint16_t *mag, uint8_t *dir) {
    memset(mag, 0, w * h * sizeof(uint16_t));
    memset(dir, 0, w * h);

    for (int y = 1; y < h - 1; y++) {
        imlib_poll_events();
        for (int x = 1; x < w - 1; x++) {
            const uint8_t *row_m1 = gray + (y - 1) * w;
            const uint8_t *row_0 = gray + y * w;
            const uint8_t *row_p1 = gray + (y + 1) * w;

            // 3x3 Sobel horizontal kernel (vx > 0 -> left brighter)
            int vx = row_m1[x - 1] - row_m1[x + 1]
                     + (row_0[x - 1] << 1) - (row_0[x + 1] << 1)
                     + row_p1[x - 1] - row_p1[x + 1];

            // 3x3 Sobel vertical kernel (vy > 0 -> top brighter)
            int vy = row_m1[x - 1] + (row_m1[x] << 1) + row_m1[x + 1]
                     - row_p1[x - 1] - (row_p1[x] << 1) - row_p1[x + 1];

            int g = (int) fast_sqrtf((float) (vx * vx + vy * vy));
            if (g > 65535) {
                g = 65535;
            }
            mag[y * w + x] = (uint16_t) g;

            // Edge direction perpendicular to gradient in screen coords.
            // Gradient in screen = (-vx, -vy), edge = perpendicular = angle of (vx, vy).
            // fast_atan2f returns [0, 2*PI]. Reduce to [0, PI) for undirected edges.
            float a = fast_atan2f((float) vx, (float) vy);
            if (a >= 3.14159265f) {
                a -= 3.14159265f;
            }
            int d = (int) (a * (8.0f / 3.14159265f) + 0.5f);
            // Guard against out-of-range angles, d indexes nms_perp/trace_fwd.
            if (d < 0 || d >= NUM_DIRS) {
                d = 0;
            }
            dir[y * w + x] = (uint8_t) d;
        }
    }
}

// Find anchor points: local gradient maxima perpendicular to edge direction.
// Returns number of anchors found.
static int find_anchors(const uint16_t *mag, const uint8_t *dir, int w, int h,
                        uint16_t threshold, ed_point_t *anchors, int max_anchors) {
    int count = 0;

    for (int y = 2; y < h - 2; y += ED_ANCHOR_SCAN_INTERVAL) {
        for (int x = 2; x < w - 2; x += ED_ANCHOR_SCAN_INTERVAL) {
            uint16_t m = mag[y * w + x];
            if (m < threshold) {
                continue;
            }

            // Check if local maximum perpendicular to edge direction
            uint8_t d = dir[y * w + x];
            int pa = nms_perp[d][0][0], pb = nms_perp[d][0][1];
            int qa = nms_perp[d][1][0], qb = nms_perp[d][1][1];

            if (m > mag[(y + pb) * w + (x + pa)] &&
                m > mag[(y + qb) * w + (x + qa)] &&
                count < max_anchors) {
                anchors[count].x = x;
                anchors[count].y = y;
                count++;
            }
        }
    }

    return count;
}

// Sort anchors by descending gradient magnitude (simple insertion sort, good for mostly-ordered data).
static void sort_anchors(ed_point_t *anchors, int count, const uint16_t *mag, int w) {
    for (int i = 1; i < count; i++) {
        ed_point_t key = anchors[i];
        uint16_t key_mag = mag[key.y * w + key.x];
        int j = i - 1;
        while (j >= 0 && mag[anchors[j].y * w + anchors[j].x] < key_mag) {
            anchors[j + 1] = anchors[j];
            j--;
        }
        anchors[j + 1] = key;
    }
}

// Walk along edge in one direction from a starting point.
// Returns number of points added to chain.
static int trace_one_direction(const uint16_t *mag, const uint8_t *dir, uint8_t *edge_map,
                               int w, int h, int sx, int sy, uint16_t threshold,
                               ed_point_t *chain, int max_len, bool forward) {
    int count = 0;
    int cx = sx, cy = sy;
    int sign = forward ? 1 : -1;

    while (count < max_len) {
        uint8_t d = dir[cy * w + cx];
        int best_x = -1, best_y = -1;
        uint16_t best_mag = 0;

        for (int i = 0; i < 3; i++) {
            int nx = cx + sign * trace_fwd[d][i][0];
            int ny = cy + sign * trace_fwd[d][i][1];

            if (nx < 1 || nx >= w - 1 || ny < 1 || ny >= h - 1) {
                continue;
            }
            if (edge_map[ny * w + nx]) {
                continue;
            }

            uint16_t nm = mag[ny * w + nx];
            if (nm < threshold || nm <= best_mag) {
                continue;
            }

            // Only accept if pixel is a local max perpendicular to its edge
            // direction (ridge pixel). This prevents tracing both sides of
            // thin lines.
            uint8_t nd = dir[ny * w + nx];
            int p1x = nx + nms_perp[nd][0][0], p1y = ny + nms_perp[nd][0][1];
            int p2x = nx + nms_perp[nd][1][0], p2y = ny + nms_perp[nd][1][1];
            if (nm <= mag[p1y * w + p1x] || nm <= mag[p2y * w + p2x]) {
                continue;
            }

            best_mag = nm;
            best_x = nx;
            best_y = ny;
        }

        if (best_x < 0) {
            break;
        }

        edge_map[best_y * w + best_x] = 1;
        chain[count].x = best_x;
        chain[count].y = best_y;
        count++;

        cx = best_x;
        cy = best_y;
    }

    return count;
}

// Compute perpendicular distance from point (px, py) to line through (x1, y1)-(x2, y2).
static inline float point_line_distance(float px, float py, float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float len_sq = dx * dx + dy * dy;
    if (len_sq < 1e-6f) {
        dx = px - x1;
        dy = py - y1;
        return fast_sqrtf(dx * dx + dy * dy);
    }
    float dist = fast_fabsf(dy * px - dx * py + x2 * y1 - y2 * x1);
    return dist / fast_sqrtf(len_sq);
}

// Recursive line fitting using Douglas-Peucker style splitting.
// Emits accepted line segments into the output list.
static void fit_line_recursive(const ed_point_t *chain, int start, int end,
                               const uint16_t *mag, int img_w,
                               list_t *out, rectangle_t *roi, float max_error) {
    int len = end - start + 1;
    if (len < ED_MIN_LINE_LENGTH) {
        return;
    }

    float x1 = (float) chain[start].x;
    float y1 = (float) chain[start].y;
    float x2 = (float) chain[end].x;
    float y2 = (float) chain[end].y;

    // Find point with maximum perpendicular distance
    float max_dist = 0.0f;
    int split_idx = -1;

    for (int i = start + 1; i < end; i++) {
        float d = point_line_distance((float) chain[i].x, (float) chain[i].y, x1, y1, x2, y2);
        if (d > max_dist) {
            max_dist = d;
            split_idx = i;
        }
    }

    if (max_dist > max_error && split_idx >= 0) {
        // Split and recurse
        fit_line_recursive(chain, start, split_idx, mag, img_w, out, roi, max_error);
        fit_line_recursive(chain, split_idx, end, mag, img_w, out, roi, max_error);
        return;
    }

    // Accept this segment. Compute mean gradient magnitude.
    uint32_t mag_sum = 0;
    for (int i = start; i <= end; i++) {
        mag_sum += mag[chain[i].y * img_w + chain[i].x];
    }

    find_lines_list_lnk_data_t lnk_line;
    lnk_line.line.x1 = chain[start].x;
    lnk_line.line.y1 = chain[start].y;
    lnk_line.line.x2 = chain[end].x;
    lnk_line.line.y2 = chain[end].y;

    if (!lb_clip_line(&lnk_line.line, 0, 0, roi->w, roi->h)) {
        return;
    }

    lnk_line.line.x1 += roi->x;
    lnk_line.line.y1 += roi->y;
    lnk_line.line.x2 += roi->x;
    lnk_line.line.y2 += roi->y;

    int dx = lnk_line.line.x2 - lnk_line.line.x1;
    int dy = lnk_line.line.y2 - lnk_line.line.y1;
    int mdx = lnk_line.line.x1 + (dx / 2);
    int mdy = lnk_line.line.y1 + (dy / 2);
    float rotation = (dx ? fast_atan2f(dy, dx) : 1.570796f) + 1.570796f; // + PI/2

    lnk_line.theta = (int) fast_roundf(rotation * 57.295780f) % 180; // * (180 / PI)
    if (lnk_line.theta < 0) {
        lnk_line.theta += 180;
    }
    lnk_line.rho = fast_roundf((mdx * cos_table[lnk_line.theta]) + (mdy * sin_table[lnk_line.theta]));
    lnk_line.magnitude = mag_sum / (unsigned) len;

    list_push_back(out, &lnk_line);
}

void imlib_edl_find_line_segments(list_t *out, image_t *ptr, rectangle_t *roi,
                                  unsigned int merge_distance, unsigned int max_theta_diff,
                                  unsigned int threshold) {
    int w = roi->w;
    int h = roi->h;

    // Extract grayscale ROI
    uint8_t *grayscale_image = uma_malloc(w * h, UMA_CACHE);

    image_t img;
    img.w = w;
    img.h = h;
    img.pixfmt = PIXFORMAT_GRAYSCALE;
    img.data = grayscale_image;
    imlib_draw_image(&img, ptr, 0, 0, 1.f, 1.f, roi, -1, 255, NULL, NULL, 0, NULL, NULL, NULL, NULL);

    // Gaussian smoothing to suppress noise and merge thin-line double edges
    imlib_sepconv3(&img, kernel_gauss_3, 1.0f / 16.0f, 0.0f);

    // Compute gradient
    uint16_t *grad_mag = uma_malloc(w * h * sizeof(uint16_t), UMA_FAST);
    uint8_t *grad_dir = uma_malloc(w * h, UMA_FAST);
    compute_gradient(grayscale_image, w, h, grad_mag, grad_dir);

    // Find anchors
    int max_anchors = (w * h) / (ED_ANCHOR_SCAN_INTERVAL * ED_ANCHOR_SCAN_INTERVAL * 4);
    if (max_anchors < 256) {
        max_anchors = 256;
    }
    ed_point_t *anchors = uma_malloc(max_anchors * sizeof(ed_point_t), UMA_DTCM);
    int n_anchors = find_anchors(grad_mag, grad_dir, w, h, (uint16_t) threshold, anchors, max_anchors);

    // Sort anchors by descending magnitude for better chain quality
    if (n_anchors > 1) {
        sort_anchors(anchors, n_anchors, grad_mag, w);
    }

    // Reuse grayscale buffer as edge map (no longer needed after gradient computation)
    uint8_t *edge_map = grayscale_image;
    memset(edge_map, 0, w * h);

    // Chain buffer for tracing
    int max_chain_len = w + h;
    ed_point_t *chain = uma_malloc(max_chain_len * sizeof(ed_point_t), UMA_DTCM);

    list_init(out, sizeof(find_lines_list_lnk_data_t));

    // Process each anchor: trace edge chain, then fit line segments
    for (int a = 0; a < n_anchors; a++) {
        int ax = anchors[a].x;
        int ay = anchors[a].y;

        if (edge_map[ay * w + ax]) {
            continue;
        }

        edge_map[ay * w + ax] = 1;

        // Trace forward from anchor
        int half = max_chain_len / 2;
        int fwd_count = trace_one_direction(grad_mag, grad_dir, edge_map, w, h,
                                            ax, ay, (uint16_t) threshold,
                                            chain + half, half, true);

        // Trace backward from anchor (into lower half of chain buffer)
        int bwd_count = trace_one_direction(grad_mag, grad_dir, edge_map, w, h,
                                            ax, ay, (uint16_t) threshold,
                                            chain, half, false);

        // Build contiguous chain: reverse backward part, then anchor, then forward part.
        // Backward trace is in chain[0..bwd_count-1] but in reverse order.
        // We need: chain[bwd_count-1], ..., chain[0], anchor, chain[half], ..., chain[half+fwd_count-1]

        // Reverse the backward chain in-place
        for (int i = 0; i < bwd_count / 2; i++) {
            ed_point_t tmp = chain[i];
            chain[i] = chain[bwd_count - 1 - i];
            chain[bwd_count - 1 - i] = tmp;
        }

        // Insert anchor point
        chain[bwd_count].x = ax;
        chain[bwd_count].y = ay;

        // Move forward chain to be contiguous
        int total_len = bwd_count + 1 + fwd_count;
        if (fwd_count > 0) {
            memmove(&chain[bwd_count + 1], &chain[half], fwd_count * sizeof(ed_point_t));
        }

        if (total_len >= ED_MIN_LINE_LENGTH) {
            fit_line_recursive(chain, 0, total_len - 1, grad_mag, w, out, roi, ED_LINE_FIT_ERROR);
        }
    }

    // Cleanup
    uma_free(chain);
    uma_free(anchors);
    uma_free(grad_dir);
    uma_free(grad_mag);
    // edge_map == grayscale_image, already freed via grad_mag? No, edge_map is grayscale_image.
    uma_free(edge_map);

    if (merge_distance > 0) {
        merge_alot(out, merge_distance, max_theta_diff);
    }
}

#endif // IMLIB_ENABLE_FIND_LINE_SEGMENTS
