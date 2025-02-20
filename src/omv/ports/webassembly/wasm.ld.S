#include "omv_boardconfig.h"

.global _fb_memory_start
.global _fb_memory_end
.global _fb_alloc_end
.global _jpeg_memory_start
.global _jpeg_memory_end

.section .data, "", @
_fb_memory_start:
    .space OMV_FB_SIZE
    .size _fb_memory_start, OMV_FB_SIZE

_fb_memory_end = _fb_memory_start + OMV_FB_SIZE
    .size _fb_memory_end, 0

_fb_alloc_start:
    .space OMV_FB_ALLOC_SIZE
    .size _fb_alloc_start, OMV_FB_ALLOC_SIZE

_fb_alloc_end = _fb_alloc_start + OMV_FB_ALLOC_SIZE
    .size _fb_alloc_end, 0

_jpeg_memory_start:
    .space OMV_JPEG_SIZE
    .size _jpeg_memory_start, OMV_JPEG_SIZE

_jpeg_memory_end = _jpeg_memory_start + OMV_JPEG_SIZE
    .size _jpeg_memory_end, 0

.section .text, "", @