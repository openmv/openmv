/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013-2016 Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * File System Helper Functions
 *
 */
#ifndef __FF_WRAPPER_H__
#define __FF_WRAPPER_H__
#include <stdint.h>
#include <ff.h>

#define READ_BYTE(fp, value) \
    ({ FRESULT _res = read_byte((fp), (value)); \
       if (_res != FR_OK) { f_close((fp)); return _res; } })

#define READ_BYTE_EXPECT(fp, value) \
    ({ uint8_t comp; FRESULT _res = read_byte((fp), &comp); \
       if (_res != FR_OK) { f_close((fp)); return _res; } \
       if (comp != (value)) { f_close((fp)); return -1; } })

#define READ_BYTE_IGNORE(fp) \
    ({ uint8_t value; FRESULT _res = read_byte((fp), &value); \
       if (_res != FR_OK) { f_close((fp)); return _res; } })

#define READ_WORD(fp, value) \
    ({ FRESULT _res = read_word((fp), (value)); \
       if (_res != FR_OK) { f_close((fp)); return _res; } })

#define READ_WORD_EXPECT(fp, value) \
    ({ uint16_t comp; FRESULT _res = read_word((fp), &comp); \
       if (_res != FR_OK) { f_close((fp)); return _res; } \
       if (comp != (value)) { f_close((fp)); return -1; } })

#define READ_WORD_IGNORE(fp) \
    ({ uint16_t value; FRESULT _res = read_word((fp), &value); \
       if (_res != FR_OK) { f_close((fp)); return _res; } })

#define READ_LONG(fp, value) \
    ({ FRESULT _res = read_long((fp), (value)); \
       if (_res != FR_OK) { f_close((fp)); return _res; } })

#define READ_LONG_EXPECT(fp, value) \
    ({ uint32_t comp; FRESULT _res = read_long((fp), &comp); \
       if (_res != FR_OK) { f_close((fp)); return _res; } \
       if (comp != (value)) { f_close((fp)); return -1; } })

#define READ_LONG_IGNORE(fp) \
    ({ uint32_t value; FRESULT _res = read_long((fp), &value); \
       if (_res != FR_OK) { f_close((fp)); return _res; } })

#define READ_DATA(fp, data, size) \
    ({ FRESULT _res = read_data((fp), (data), (size)); \
       if (_res != FR_OK) { f_close((fp)); return _res; } })

#define WRITE_BYTE(fp, value) \
    ({ FRESULT _res = write_byte((fp), (value)); \
       if (_res != FR_OK) { f_close((fp)); return _res; } })

#define WRITE_WORD(fp, value) \
    ({ FRESULT _res = write_word((fp), (value)); \
       if (_res != FR_OK) { f_close((fp)); return _res; } })

#define WRITE_LONG(fp, value) \
    ({ FRESULT _res = write_long((fp), (value)); \
       if (_res != FR_OK) { f_close((fp)); return _res; } })

#define WRITE_DATA(fp, data, size) \
    ({ FRESULT _res = write_data((fp), (data), (size)); \
       if (_res != FR_OK) { f_close((fp)); return _res; } })

int read_byte(FIL *fp, uint8_t *value);
int read_word(FIL *fp, uint16_t *value);
int read_long(FIL *fp, uint32_t *value);
int read_data(FIL *fp, void *data, UINT size);
int write_byte(FIL *fp, uint8_t value);
int write_word(FIL *fp, uint16_t value);
int write_long(FIL *fp, uint32_t value);
int write_data(FIL *fp, const void *data, UINT size);
#endif /* __FF_WRAPPER_H__ */
