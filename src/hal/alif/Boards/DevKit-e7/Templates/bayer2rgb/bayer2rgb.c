/**
 * bayer2rgb: Comandline converter for bayer grid to rgb images.
 * This file is part of bayer2rgb.
 *
 * Copyright (c) 2009 Jeff Thomas
 *
 * bayer2rgb is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * bayer2rgb is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 **/

/**************************************************************************//**
 * @file     bayer2rgb.c
 * @author   Tanay Rami
 * @email    tanay@alifsemi.com
 * @version  V1.0.0
 * @date     10-Nov-2021
 * @brief    Original file is modified only in order to port "Open-Source"
 *            Linux Command-line Script (https://github.com/jdthomas/bayer2rgb)
 *            to run into a bare metal environment.
 *             - In order to make RTOS compatible,
 *                Removed all the references to standard unix/linux
 *                library and system calls.
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bayer.h"

#define ENABLE_8_BIT_VERSION      1   /* Enable 8-bit Version. */
#define ENABLE_16_BIT_VERSION     0   /* Enable 16-bit Version.(Not Tested.) */

// tiff types: short = 3, int = 4
// Tags: ( 2-byte tag ) ( 2-byte type ) ( 4-byte count ) ( 4-byte data )
//    0100 0003 0000 0001 0064 0000
//       |        |    |         |
// tag --+        |    |         |
// short int -----+    |         |
// one value ----------+         |
// value of 100 -----------------+
//
#define TIFF_HDR_NUM_ENTRY 8
#define TIFF_HDR_SIZE 10+TIFF_HDR_NUM_ENTRY*12
uint8_t tiff_header[TIFF_HDR_SIZE] = {
    // I     I     42
      0x49, 0x49, 0x2a, 0x00,
    // ( offset to tags, 0 )
      0x08, 0x00, 0x00, 0x00,
    // ( num tags )
      0x08, 0x00,
    // ( newsubfiletype, 0 full-image )
      0xfe, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ( image width )
      0x00, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ( image height )
      0x01, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ( bits per sample )
      0x02, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ( Photometric Interpretation, 2 = RGB )
      0x06, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
    // ( Strip offsets, 8 )
      0x11, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
    // ( samples per pixel, 3 - RGB)
      0x15, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    // ( Strip byte count )
      0x17, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
uint8_t *
put_tiff(uint8_t * rgb, uint32_t width, uint32_t height, uint16_t bpp)
{
    uint32_t ulTemp=0;
    uint16_t sTemp=0;
    memcpy(rgb, tiff_header, TIFF_HDR_SIZE);

    sTemp = TIFF_HDR_NUM_ENTRY;
    memcpy(rgb + 8, &sTemp, 2);

    memcpy(rgb + 10 + 1*12 + 8, &width, 4);
    memcpy(rgb + 10 + 2*12 + 8, &height, 4);
    memcpy(rgb + 10 + 3*12 + 8, &bpp, 2);

    // strip byte count
    ulTemp = width * height * (bpp / 8) * 3;
    memcpy(rgb + 10 + 7*12 + 8, &ulTemp, 4);

    //strip offset
    sTemp = TIFF_HDR_SIZE;
    memcpy(rgb + 10 + 5*12 + 8, &sTemp, 2);

    return rgb + TIFF_HDR_SIZE;
};



/*
 * Below Modifications are done by Alif Semiconductor:
 * - Added function "bayer_to_RGB" for Bayer to RGB Conversion
 *    which is nothing but a re-used code of "main" function.
 *
 * - Selected Bayer to RGB Configurations:
 *   - bpp bit per pixel : 8-bit per pixel
 *   - Color Filter      : DC1394_COLOR_FILTER_GBRG
 *   - Bayer Method      : DC1394_BAYER_METHOD_HQLINEAR
 */

/* DC1394_COLOR_FILTER:
 *  \ref dc1394color_filter_t
 *
 *  Valid Color Filters are:
 *   - DC1394_COLOR_FILTER_RGGB
 *   - DC1394_COLOR_FILTER_GBRG
 *   - DC1394_COLOR_FILTER_GRBG
 *   - DC1394_COLOR_FILTER_BGGR
 *
 *  @Observation: Output image color will be Reddish or Greenish
 *                 depending on selected color filter.
 */
#define DC1394_COLOR_FILTER       DC1394_COLOR_FILTER_GBRG

/* DC1394_BAYER_METHOD:
 *  \ref dc1394bayer_method_t
 *
 *  Valid Bayer Methods are:
 *   - DC1394_BAYER_METHOD_NEAREST
 *   - DC1394_BAYER_METHOD_SIMPLE
 *   - DC1394_BAYER_METHOD_BILINEAR
 *   - DC1394_BAYER_METHOD_HQLINEAR
 *   - DC1394_BAYER_METHOD_DOWNSAMPLE
 *   - DC1394_BAYER_METHOD_EDGESENSE
 *   - DC1394_BAYER_METHOD_VNG
 *   - DC1394_BAYER_METHOD_AHD
 *
 *  @Observation: There is not much difference between various
 *                 Bayer to RGB interpolation methods.
 *                 (all output images looks essentially the same.)
 */
#define DC1394_BAYER_METHOD       DC1394_BAYER_METHOD_HQLINEAR

/* bpp bit per pixel
 *  Valid parameters are:
 *   -  8-bit
 *   - 16-bit
 */
#define BITS_PER_PIXEL_8_BIT      8

/* Bayer to RGB Conversion. */
extern int32_t bayer_to_RGB(uint8_t  *src,   uint8_t  *dest,   \
                            uint32_t  width, uint32_t  height);

/**
  \fn          int32_t bayer_to_RGB(uint8_t  *src,   uint8_t  *dest,
                                    uint32_t  width, uint32_t  height)
  \brief       Bayer to RGB Conversion.
                - Selected Bayer to RGB Configurations:
                  - bpp bit per pixel : 8-bit per pixel
                  - Color Filter      : DC1394_COLOR_FILTER_GBRG
                  - Bayer Method      : DC1394_BAYER_METHOD_HQLINEAR
  \param[in]   src    : Source address, Pointer to already available
                         Bayer image data Address
  \param[in]   dest   : Destination address,Pointer to Address,
                         where Bayer to RGB converted data will be stored.
  \param[in]   width  : width  of the Bayer image
  \param[in]   height : height of the Bayer image
  \return      Success: 0;
               Error  : 1
*/
int32_t bayer_to_RGB(uint8_t  *src,   uint8_t  *dest,   \
                     uint32_t  width, uint32_t  height)
{
    uint32_t in_size=0, out_size=0;
    uint32_t bpp = BITS_PER_PIXEL_8_BIT;
    int first_color = DC1394_COLOR_FILTER;
    int tiff = TIFF_HDR_SIZE;
    int method = DC1394_BAYER_METHOD;
    uint8_t *bayer = NULL;
    uint8_t *rgb = NULL;
    uint8_t *rgb_start = NULL;
    int swap = 0;

    // arguments: src dest width height bpp first_color
    if( src == NULL || dest == NULL || bpp == 0 || width == 0 || height == 0 )
    {
        printf("Bad parameter\n");
        return 1;
    }

    out_size = width * height * (bpp / 8) * 3 + tiff;

    bayer = (uint8_t *) src;     //source      address
    rgb   = (uint8_t *) dest;    //destination address
    rgb_start = rgb;

    if(tiff)
    {
        rgb_start = put_tiff(rgb, width, height, bpp);
    }

    switch(bpp)
    {
#if ENABLE_8_BIT_VERSION
        case 8:
            dc1394_bayer_decoding_8bit((const uint8_t*)bayer, (uint8_t*)rgb_start, width, height, first_color, method);
            break;
#endif /* end of ENABLE_8_BIT_VERSION */

#if ENABLE_16_BIT_VERSION
        case 16:
        default:
            if(swap){
                uint8_t tmp=0;
                uint32_t i=0;
                for(i=0;i<in_size;i+=2){
                    tmp = *(((uint8_t*)bayer)+i);
                    *(((uint8_t*)bayer)+i) = *(((uint8_t*)bayer)+i+1);
                    *(((uint8_t*)bayer)+i+1) = tmp;
                }
            }
            dc1394_bayer_decoding_16bit((const uint16_t*)bayer, (uint16_t*)rgb_start, width, height, first_color, method, bpp);
            break;
#endif /* end of ENABLE_16_BIT_VERSION */
    }

    printf("\n Bayer to RGB Converted Tiff Color Image Memory Address is :");
    printf("\n \t starting_addr: 0x%X \r\n \t ending_addr  : 0x%X\n", \
                          (uint32_t) dest, (uint32_t) (dest+out_size-1));

    return 0;
}
