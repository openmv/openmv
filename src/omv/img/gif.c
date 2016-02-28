/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * A super simple GIF encoder.
 * Only support Grayscale/128 colors/No compression.
 *
 */
#include "imlib.h"
#include "xalloc.h"
#include "fb_alloc.h"
#include "ff_wrapper.h"

#define BLOCK_SIZE  (100)
#define BUFF_SIZE   ((BLOCK_SIZE+2)*500)

void gif_open(FIL *fp, int width, int height, bool loop)
{
    write_data(fp, "GIF89a", 6);
    write_word(fp, width);
    write_word(fp, height);
    write_data(fp, (uint8_t []) {0xF6, 0x00, 0x00}, 3);

    // Grayscale
    for (int i=0; i<256; i+=2) {
        write_data(fp, (uint8_t []) {i, i, i}, 3);
    }

    if (loop == true) {
        write_data(fp, (uint8_t []) {'!', 0xFF, 0x0B}, 3);
        write_data(fp, "NETSCAPE2.0", 11);
        write_data(fp, (uint8_t []) {0x03, 0x01}, 2);
        write_word(fp, 0); // 0 == loop forever
        write_byte(fp, 0);
    }
}

void gif_add_frame(FIL *fp, image_t *img, uint16_t delay)
{
    if (delay) {
        write_data(fp, (uint8_t []) {'!', 0xF9, 0x04, 0x04}, 4);
        write_word(fp, delay);
        write_word(fp, 0);
    }

    write_byte(fp, 0x2C);
    write_long(fp, 0);
    write_word(fp, img->w);
    write_word(fp, img->h);
    write_data(fp, (uint8_t []) {0x00, 0x07}, 2); // DEPTH

    int bytes  = (img->h * img->w);
    int blocks =  bytes / BLOCK_SIZE;

    int buf_ofs=0;
    uint8_t *buf = fb_alloc(BUFF_SIZE * sizeof(*buf));

    for (int y=0; y<blocks; y++) {
        buf[buf_ofs++] = BLOCK_SIZE+1;
        buf[buf_ofs++] = 0x80;

        for (int x=0; x<BLOCK_SIZE; x++) {
            buf[buf_ofs++] = img->pixels[y*BLOCK_SIZE+x]>>1;
        }

        if (buf_ofs==BUFF_SIZE) {
            buf_ofs = 0;
            write_data(fp, buf, BUFF_SIZE);
        }
    }

    if (buf_ofs) {
        write_data(fp, buf, buf_ofs);
    }

    write_data(fp, (uint8_t []) {0x01, 0x81}, 2); // STOP code 
    write_byte(fp, 0x00);

    fb_free();
}

void gif_close(FIL *fp)
{
    write_byte(fp, ';');
    file_close(fp);
}
