/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Image library configuration.
 *
 */
#ifndef __IMLIB_CONFIG_H__
#define __IMLIB_CONFIG_H__

// Enable remove_shadows()
//#define OMV_ENABLE_REMOVE_SHADOWS

// Enable linpolar()
//#define OMV_ENABLE_LINPOLAR

// Enable logpolar()
//#define OMV_ENABLE_LOGPOLAR

// Enable chrominvar()
//#define OMV_ENABLE_CHROMINVAR

// Enable illuminvar()
//#define OMV_ENABLE_ILLUMINVAR

// Enable rotation_corr()
//#define OMV_ENABLE_ROTATION_CORR

// Enable get_similarity()
//#define OMV_ENABLE_GET_SIMILARITY

// Enable find_lines()
//#define OMV_ENABLE_FIND_LINES

// Enable find_line_segments()
//#define OMV_ENABLE_FIND_LINE_SEGMENTS

// Enable find_circles()
//#define OMV_ENABLE_FIND_CIRCLES

// Enable find_rects()
//#define OMV_ENABLE_FIND_RECTS

// Enable find_qrcodes() (14 KB)
//#define OMV_ENABLE_QRCODES

// Enable find_apriltags() (64 KB)
//#define OMV_ENABLE_APRILTAGS

// Enable find_datamatrices() (26 KB)
//#define OMV_ENABLE_DATAMATRICES

// Enable find_barcodes() (42 KB)
//#define OMV_ENABLE_BARCODES

// Enable find_displacement()
//#ifdef OMV_ENABLE_ROTATION_CORR
//#define OMV_ENABLE_FIND_DISPLACEMENT
//#endif

// Enable LENET (200+ KB).
//#define OMV_ENABLE_LENET

#endif //__IMLIB_CONFIG_H__
