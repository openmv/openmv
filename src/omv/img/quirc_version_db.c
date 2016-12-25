/* quirc -- QR-code recognition library
 * Copyright (C) 2010-2012 Daniel Beer <dlbeer@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Modified by Kwabena W. Agyeman <kwagyeman@openmv.io> for the OpenMV project.
 */

#include "quirc_quirc_internal.h"

const struct quirc_version_info quirc_version_db[QUIRC_MAX_VERSION + 1] = {
    {0},
    { /* Version 1 */
        .data_bytes = 26,
        .apat = {0},
        .ecc = {
            {.bs = 26, .dw = 16, .ce = 4},
            {.bs = 26, .dw = 19, .ce = 2},
            {.bs = 26, .dw = 9, .ce = 8},
            {.bs = 26, .dw = 13, .ce = 6}
        }
    },
    { /* Version 2 */
        .data_bytes = 44,
        .apat = {6, 18, 0},
        .ecc = {
            {.bs = 44, .dw = 28, .ce = 8},
            {.bs = 44, .dw = 34, .ce = 4},
            {.bs = 44, .dw = 16, .ce = 14},
            {.bs = 44, .dw = 22, .ce = 11}
        }
    },
    { /* Version 3 */
        .data_bytes = 70,
        .apat = {6, 22, 0},
        .ecc = {
            {.bs = 70, .dw = 44, .ce = 13},
            {.bs = 70, .dw = 55, .ce = 7},
            {.bs = 35, .dw = 13, .ce = 11},
            {.bs = 35, .dw = 17, .ce = 9}
        }
    },
    { /* Version 4 */
        .data_bytes = 100,
        .apat = {6, 26, 0},
        .ecc = {
            {.bs = 50, .dw = 32, .ce = 9},
            {.bs = 100, .dw = 80, .ce = 10},
            {.bs = 25, .dw = 9, .ce = 8},
            {.bs = 50, .dw = 24, .ce = 13}
        }
    },
    { /* Version 5 */
        .data_bytes = 134,
        .apat = {6, 30, 0},
        .ecc = {
            {.bs = 67, .dw = 43, .ce = 12},
            {.bs = 134, .dw = 108, .ce = 13},
            {.bs = 33, .dw = 11, .ce = 11},
            {.bs = 33, .dw = 15, .ce = 9}
        }
    },
    { /* Version 6 */
        .data_bytes = 172,
        .apat = {6, 34, 0},
        .ecc = {
            {.bs = 43, .dw = 27, .ce = 8},
            {.bs = 86, .dw = 68, .ce = 9},
            {.bs = 43, .dw = 15, .ce = 14},
            {.bs = 43, .dw = 19, .ce = 12}
        }
    },
    { /* Version 7 */
        .data_bytes = 196,
        .apat = {6, 22, 38, 0},
        .ecc = {
            {.bs = 49, .dw = 31, .ce = 9},
            {.bs = 98, .dw = 78, .ce = 10},
            {.bs = 39, .dw = 13, .ce = 13},
            {.bs = 32, .dw = 14, .ce = 9}
        }
    },
    { /* Version 8 */
        .data_bytes = 242,
        .apat = {6, 24, 42, 0},
        .ecc = {
            {.bs = 60, .dw = 38, .ce = 11},
            {.bs = 121, .dw = 97, .ce = 12},
            {.bs = 40, .dw = 14, .ce = 13},
            {.bs = 40, .dw = 18, .ce = 11}
        }
    },
    { /* Version 9 */
        .data_bytes = 292,
        .apat = {6, 26, 46, 0},
        .ecc = {
            {.bs = 58, .dw = 36, .ce = 11},
            {.bs = 146, .dw = 116, .ce = 15},
            {.bs = 36, .dw = 12, .ce = 12},
            {.bs = 36, .dw = 16, .ce = 10}
        }
    },
    { /* Version 10 */
        .data_bytes = 346,
        .apat = {6, 28, 50, 0},
        .ecc = {
            {.bs = 69, .dw = 43, .ce = 13},
            {.bs = 86, .dw = 68, .ce = 9},
            {.bs = 43, .dw = 15, .ce = 14},
            {.bs = 43, .dw = 19, .ce = 12}
        }
    },
    { /* Version 11 */
        .data_bytes = 404,
        .apat = {6, 30, 54, 0},
        .ecc = {
            {.bs = 80, .dw = 50, .ce = 15},
            {.bs = 101, .dw = 81, .ce = 10},
            {.bs = 36, .dw = 12, .ce = 12},
            {.bs = 50, .dw = 22, .ce = 14}
        }
    },
    { /* Version 12 */
        .data_bytes = 466,
        .apat = {6, 32, 58, 0},
        .ecc = {
            {.bs = 58, .dw = 36, .ce = 11},
            {.bs = 116, .dw = 92, .ce = 12},
            {.bs = 42, .dw = 14, .ce = 14},
            {.bs = 46, .dw = 20, .ce = 14}
        }
    },
    { /* Version 13 */
        .data_bytes = 532,
        .apat = {6, 34, 62, 0},
        .ecc = {
            {.bs = 59, .dw = 37, .ce = 11},
            {.bs = 133, .dw = 107, .ce = 13},
            {.bs = 33, .dw = 11, .ce = 11},
            {.bs = 44, .dw = 20, .ce = 12}
        }
    },
    { /* Version 14 */
        .data_bytes = 581,
        .apat = {6, 26, 46, 66, 0},
        .ecc = {
            {.bs = 65, .dw = 41, .ce = 12},
            {.bs = 109, .dw = 87, .ce = 11},
            {.bs = 36, .dw = 12, .ce = 12},
            {.bs = 54, .dw = 24, .ce = 15}
        }
    },
    { /* Version 15 */
        .data_bytes = 655,
        .apat = {6, 26, 48, 70, 0},
        .ecc = {
            {.bs = 65, .dw = 41, .ce = 12},
            {.bs = 109, .dw = 87, .ce = 11},
            {.bs = 36, .dw = 12, .ce = 12},
            {.bs = 54, .dw = 24, .ce = 15}
        }
    },
    { /* Version 16 */
        .data_bytes = 733,
        .apat = {6, 26, 50, 74, 0},
        .ecc = {
            {.bs = 73, .dw = 45, .ce = 14},
            {.bs = 122, .dw = 98, .ce = 12},
            {.bs = 45, .dw = 15, .ce = 15},
            {.bs = 43, .dw = 19, .ce = 12}
        }
    },
    { /* Version 17 */
        .data_bytes = 815,
        .apat = {6, 30, 54, 78, 0},
        .ecc = {
            {.bs = 74, .dw = 46, .ce = 14},
            {.bs = 135, .dw = 107, .ce = 14},
            {.bs = 42, .dw = 14, .ce = 14},
            {.bs = 50, .dw = 22, .ce = 14}
        }
    },
    { /* Version 18 */
        .data_bytes = 901,
        .apat = {6, 30, 56, 82, 0},
        .ecc = {
            {.bs = 69, .dw = 43, .ce = 13},
            {.bs = 150, .dw = 120, .ce = 15},
            {.bs = 42, .dw = 14, .ce = 14},
            {.bs = 50, .dw = 22, .ce = 14}
        }
    },
    { /* Version 19 */
        .data_bytes = 991,
        .apat = {6, 30, 58, 86, 0},
        .ecc = {
            {.bs = 70, .dw = 44, .ce = 13},
            {.bs = 141, .dw = 113, .ce = 14},
            {.bs = 39, .dw = 13, .ce = 13},
            {.bs = 47, .dw = 21, .ce = 13}
        }
    },
    { /* Version 20 */
        .data_bytes = 1085,
        .apat = {6, 34, 62, 90, 0},
        .ecc = {
            {.bs = 67, .dw = 41, .ce = 13},
            {.bs = 135, .dw = 107, .ce = 14},
            {.bs = 43, .dw = 15, .ce = 14},
            {.bs = 54, .dw = 24, .ce = 15}
        }
    },
    { /* Version 21 */
        .data_bytes = 1156,
        .apat = {6, 28, 50, 72, 92, 0},
        .ecc = {
            {.bs = 68, .dw = 42, .ce = 13},
            {.bs = 144, .dw = 116, .ce = 14},
            {.bs = 46, .dw = 16, .ce = 15},
            {.bs = 50, .dw = 22, .ce = 14}
        }
    },
    { /* Version 22 */
        .data_bytes = 1258,
        .apat = {6, 26, 50, 74, 98, 0},
        .ecc = {
            {.bs = 74, .dw = 46, .ce = 14},
            {.bs = 139, .dw = 111, .ce = 14},
            {.bs = 37, .dw = 13, .ce = 12},
            {.bs = 54, .dw = 24, .ce = 15}
        }
    },
    { /* Version 23 */
        .data_bytes = 1364,
        .apat = {6, 30, 54, 78, 102, 0},
        .ecc = {
            {.bs = 75, .dw = 47, .ce = 14},
            {.bs = 151, .dw = 121, .ce = 15},
            {.bs = 45, .dw = 15, .ce = 15},
            {.bs = 54, .dw = 24, .ce = 15}
        }
    },
    { /* Version 24 */
        .data_bytes = 1474,
        .apat = {6, 28, 54, 80, 106, 0},
        .ecc = {
            {.bs = 73, .dw = 45, .ce = 14},
            {.bs = 147, .dw = 117, .ce = 15},
            {.bs = 46, .dw = 16, .ce = 15},
            {.bs = 54, .dw = 24, .ce = 15}
        }
    },
    { /* Version 25 */
        .data_bytes = 1588,
        .apat = {6, 32, 58, 84, 110, 0},
        .ecc = {
            {.bs = 75, .dw = 47, .ce = 14},
            {.bs = 132, .dw = 106, .ce = 13},
            {.bs = 45, .dw = 15, .ce = 15},
            {.bs = 54, .dw = 24, .ce = 15}
        }
    },
    { /* Version 26 */
        .data_bytes = 1706,
        .apat = {6, 30, 58, 86, 114, 0},
        .ecc = {
            {.bs = 74, .dw = 46, .ce = 14},
            {.bs = 142, .dw = 114, .ce = 14},
            {.bs = 46, .dw = 16, .ce = 15},
            {.bs = 50, .dw = 22, .ce = 14}
        }
    },
    { /* Version 27 */
        .data_bytes = 1828,
        .apat = {6, 34, 62, 90, 118, 0},
        .ecc = {
            {.bs = 73, .dw = 45, .ce = 14},
            {.bs = 152, .dw = 122, .ce = 15},
            {.bs = 45, .dw = 15, .ce = 15},
            {.bs = 53, .dw = 23, .ce = 15}
        }
    },
    { /* Version 28 */
        .data_bytes = 1921,
        .apat = {6, 26, 50, 74, 98, 122, 0},
        .ecc = {
            {.bs = 73, .dw = 45, .ce = 14},
            {.bs = 147, .dw = 117, .ce = 15},
            {.bs = 45, .dw = 15, .ce = 15},
            {.bs = 54, .dw = 24, .ce = 15}
        }
    },
    { /* Version 29 */
        .data_bytes = 2051,
        .apat = {6, 30, 54, 78, 102, 126, 0},
        .ecc = {
            {.bs = 73, .dw = 45, .ce = 14},
            {.bs = 146, .dw = 116, .ce = 15},
            {.bs = 45, .dw = 15, .ce = 15},
            {.bs = 73, .dw = 45, .ce = 14}
        }
    },
    { /* Version 30 */
        .data_bytes = 2185,
        .apat = {6, 26, 52, 78, 104, 130, 0},
        .ecc = {
            {.bs = 75, .dw = 47, .ce = 14},
            {.bs = 145, .dw = 115, .ce = 15},
            {.bs = 45, .dw = 15, .ce = 15},
            {.bs = 54, .dw = 24, .ce = 15}
        }
    },
    { /* Version 31 */
        .data_bytes = 2323,
        .apat = {6, 30, 56, 82, 108, 134, 0},
        .ecc = {
            {.bs = 74, .dw = 46, .ce = 14},
            {.bs = 145, .dw = 115, .ce = 15},
            {.bs = 45, .dw = 15, .ce = 15},
            {.bs = 54, .dw = 24, .ce = 15}
        }
    },
    { /* Version 32 */
        .data_bytes = 2465,
        .apat = {6, 34, 60, 86, 112, 138, 0},
        .ecc = {
            {.bs = 74, .dw = 46, .ce = 14},
            {.bs = 145, .dw = 115, .ce = 15},
            {.bs = 45, .dw = 15, .ce = 15},
            {.bs = 54, .dw = 24, .ce = 15}
        }
    },
    { /* Version 33 */
        .data_bytes = 2611,
        .apat = {6, 30, 58, 96, 114, 142, 0},
        .ecc = {
            {.bs = 74, .dw = 46, .ce = 14},
            {.bs = 145, .dw = 115, .ce = 15},
            {.bs = 45, .dw = 15, .ce = 15},
            {.bs = 54, .dw = 24, .ce = 15}
        }
    },
    { /* Version 34 */
        .data_bytes = 2761,
        .apat = {6, 34, 62, 90, 118, 146, 0},
        .ecc = {
            {.bs = 74, .dw = 46, .ce = 14},
            {.bs = 145, .dw = 115, .ce = 15},
            {.bs = 46, .dw = 16, .ce = 15},
            {.bs = 54, .dw = 24, .ce = 15}
        }
    },
    { /* Version 35 */
        .data_bytes = 2876,
        .apat = {6, 30, 54, 78, 102, 126, 150},
        .ecc = {
            {.bs = 75, .dw = 47, .ce = 14},
            {.bs = 151, .dw = 121, .ce = 15},
            {.bs = 45, .dw = 15, .ce = 15},
            {.bs = 54, .dw = 24, .ce = 15}
        }
    },
    { /* Version 36 */
        .data_bytes = 3034,
        .apat = {6, 24, 50, 76, 102, 128, 154},
        .ecc = {
            {.bs = 75, .dw = 47, .ce = 14},
            {.bs = 151, .dw = 121, .ce = 15},
            {.bs = 45, .dw = 15, .ce = 15},
            {.bs = 54, .dw = 24, .ce = 15}
        }
    },
    { /* Version 37 */
        .data_bytes = 3196,
        .apat = {6, 28, 54, 80, 106, 132, 158},
        .ecc = {
            {.bs = 74, .dw = 46, .ce = 14},
            {.bs = 152, .dw = 122, .ce = 15},
            {.bs = 45, .dw = 15, .ce = 15},
            {.bs = 54, .dw = 24, .ce = 15}
        }
    },
    { /* Version 38 */
        .data_bytes = 3362,
        .apat = {6, 32, 58, 84, 110, 136, 162},
        .ecc = {
            {.bs = 74, .dw = 46, .ce = 14},
            {.bs = 152, .dw = 122, .ce = 15},
            {.bs = 45, .dw = 15, .ce = 15},
            {.bs = 54, .dw = 24, .ce = 15}
        }
    },
    { /* Version 39 */
        .data_bytes = 3532,
        .apat = {6, 26, 54, 82, 110, 138, 166},
        .ecc = {
            {.bs = 75, .dw = 47, .ce = 14},
            {.bs = 147, .dw = 117, .ce = 15},
            {.bs = 45, .dw = 15, .ce = 15},
            {.bs = 54, .dw = 24, .ce = 15}
        }
    },
    { /* Version 40 */
        .data_bytes = 3706,
        .apat = {6, 30, 58, 86, 114, 142, 170},
        .ecc = {
            {.bs = 75, .dw = 47, .ce = 14},
            {.bs = 148, .dw = 118, .ce = 15},
            {.bs = 45, .dw = 15, .ce = 15},
            {.bs = 54, .dw = 24, .ce = 15}
        }
    }
};
