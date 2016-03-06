/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * A super simple GIF encoder.
 *
 */
#include "mp.h"
#include "ff_wrapper.h"
#include "imlib.h"
#include "fb_alloc.h"

#define BLOCK_SIZE (126) // (2^(7)) - 2 (DO NOT CHANGE!)

void gif_open(FIL *fp, int width, int height, bool color, bool loop)
{
    write_data(fp, "GIF89a", 6);
    write_word(fp, width);
    write_word(fp, height);
    write_data(fp, (uint8_t []) {0xF6, 0x00, 0x00}, 3);

    if (color) {
        for (int i=0; i<128; i++) {
            int red =   ((((i & 0x60) >> 5) * 255) + 1.5) / 3;
            int green = ((((i & 0x1C) >> 2) * 255) + 3.5) / 7;
            int blue =   (((i & 0x3)        * 255) + 1.5) / 3;
            write_data(fp, (uint8_t []) {red, green, blue}, 3);
        }
    } else {
        for (int i=0; i<128; i++) {
            int gray = ((i * 255) + 63.5) / 127;
            write_data(fp, (uint8_t []) {gray, gray, gray}, 3);
        }
    }

    if (loop) {
        write_data(fp, (uint8_t []) {'!', 0xFF, 0x0B}, 3);
        write_data(fp, "NETSCAPE2.0", 11);
        write_data(fp, (uint8_t []) {0x03, 0x01, 0x00, 0x00, 0x00}, 5);
    }
}

void gif_add_frame(FIL *fp, image_t *img, uint16_t delay)
{
    if (delay) {
        write_data(fp, (uint8_t []) {'!', 0xF9, 0x04, 0x04}, 4);
        write_word(fp, delay);
        write_word(fp, 0); // end
    }

    write_byte(fp, 0x2C);
    write_long(fp, 0);
    write_word(fp, img->w);
    write_word(fp, img->h);
    write_data(fp, (uint8_t []) {0x00, 0x07}, 2); // 7-bits

    uint32_t size;
    uint8_t *buf = fb_alloc_all(&size);

    // This should never happen unless someone forgot to free.
    if (size < (sizeof(uint32_t) * 2)) { // We need atleast 8 bytes.
        nlr_raise(mp_obj_new_exception_msg(&mp_type_MemoryError,
                                           "Memory leak detected!!!"));
    }

    int buf_ofs = 0;
    int bytes   = img->h * img->w;

    // When a sector boundary is encountered while writing a file and there are
    // more than 512 bytes left to write FatFs will detect that it can bypass
    // its internal write buffer and pass the data buffer passed to it directly
    // to the disk write function. However, the disk write function needs the
    // buffer to be aligned to a 4-byte boundary. FatFs doesn't know this and
    // will pass an unaligned buffer if we don't fix the issue.

    int fat_ofs = f_tell(fp) % 4;
    buf += fat_ofs;
    size -= fat_ofs;

    // Since we're about to do a more than 512 byte write which will trigger
    // the above issue we can fix the problem by misaligning our buffer before
    // calling write so that the first partial write gets us back aligned.

    for (int x=0; x<bytes; x++) {
        if ((x%BLOCK_SIZE)==0) {
            buf[buf_ofs++] = 1 + IM_MIN(BLOCK_SIZE, bytes - x);
            buf[buf_ofs++] = 0x80; // clear code
        }

        if (IM_IS_GS(img)) {
            buf[buf_ofs++] = img->pixels[x]>>1;
        } else {
            int pixel = ((uint16_t *) img->pixels)[x];
            int red = IM_R565(pixel) >> 3;
            int green = IM_G565(pixel) >> 3;
            int blue = IM_B565(pixel) >> 3;
            buf[buf_ofs++] = (red << 5) | (green << 2) | blue;
        }

        if (buf_ofs==size) { // flush data
            buf_ofs = 0;
            write_data(fp, buf, size);
            // Undo alignment fix. fp will be aligned from now on.
            buf -= fat_ofs;
            size += fat_ofs;
            fat_ofs = 0;
        }
    }

    if (buf_ofs) { // flush remaining
        write_data(fp, buf, buf_ofs);
    }

    write_data(fp, (uint8_t []) {0x01, 0x81, 0x00}, 3); // end code
    fb_free();
}

void gif_close(FIL *fp)
{
    write_byte(fp, ';');
    file_close(fp);
}
