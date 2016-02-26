/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013-2016 Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * File System Helper Functions
 *
 */
#include <mp.h>
#include "ff_wrapper.h"

extern const char *ffs_strerror(FRESULT res);

NORETURN static void ff_fail(FIL *fp, FRESULT res)
{
    if (fp) f_close(fp);
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
}

NORETURN static void ff_read_fail(FIL *fp)
{
    if (fp) f_close(fp);
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Failed to read requested bytes!"));
}

NORETURN static void ff_write_fail(FIL *fp)
{
    if (fp) f_close(fp);
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Failed to write requested bytes!"));
}

NORETURN static void ff_expect_fail(FIL *fp)
{
    if (fp) f_close(fp);
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Unexpected value read!"));
}

NORETURN void ff_unsupported_format(FIL *fp)
{
    if (fp) f_close(fp);
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Unsupported format!"));
}

NORETURN void ff_file_corrupted(FIL *fp)
{
    if (fp) f_close(fp);
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "File corrupted!"));
}

NORETURN void ff_not_equal()
{
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Images not equal!"));
}

NORETURN void ff_no_intersection()
{
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "No intersection!"));
}

void file_read_open(FIL *fp, const char *path)
{
    FRESULT res = f_open(fp, path, FA_READ|FA_OPEN_EXISTING);
    if (res != FR_OK) ff_fail(fp, res);
}

void file_write_open(FIL *fp, const char *path)
{
    FRESULT res = f_open(fp, path, FA_WRITE|FA_CREATE_ALWAYS);
    if (res != FR_OK) ff_fail(fp, res);
}

void file_close(FIL *fp)
{
    FRESULT res = f_close(fp);
    if (res != FR_OK) ff_fail(fp, res);
}

void file_seek(FIL *fp, UINT offset)
{
    FRESULT res = f_lseek(fp, offset);
    if (res != FR_OK) ff_fail(fp, res);
}

void read_byte(FIL *fp, uint8_t *value)
{
    UINT bytes;
    FRESULT res = f_read(fp, value, sizeof(*value), &bytes);
    if (res != FR_OK) ff_fail(fp, res);
    if (bytes != sizeof(*value)) ff_read_fail(fp);
}

void read_byte_expect(FIL *fp, uint8_t value)
{
    uint8_t compare;
    read_byte(fp, &compare);
    if (value != compare) ff_expect_fail(fp);
}

void read_byte_ignore(FIL *fp)
{
    uint8_t trash;
    read_byte(fp, &trash);
}

void read_word(FIL *fp, uint16_t *value)
{
    UINT bytes;
    FRESULT res = f_read(fp, value, sizeof(*value), &bytes);
    if (res != FR_OK) ff_fail(fp, res);
    if (bytes != sizeof(*value)) ff_read_fail(fp);
}

void read_word_expect(FIL *fp, uint16_t value)
{
    uint16_t compare;
    read_word(fp, &compare);
    if (value != compare) ff_expect_fail(fp);
}

void read_word_ignore(FIL *fp)
{
    uint16_t trash;
    read_word(fp, &trash);
}

void read_long(FIL *fp, uint32_t *value)
{
    UINT bytes;
    FRESULT res = f_read(fp, value, sizeof(*value), &bytes);
    if (res != FR_OK) ff_fail(fp, res);
    if (bytes != sizeof(*value)) ff_read_fail(fp);
}

void read_long_expect(FIL *fp, uint32_t value)
{
    uint32_t compare;
    read_long(fp, &compare);
    if (value != compare) ff_expect_fail(fp);
}

void read_long_ignore(FIL *fp)
{
    uint32_t trash;
    read_long(fp, &trash);
}

void read_data(FIL *fp, void *data, UINT size)
{
    UINT bytes;
    FRESULT res = f_read(fp, data, size, &bytes);
    if (res != FR_OK) ff_fail(fp, res);
    if (bytes != size) ff_read_fail(fp);
}

void write_byte(FIL *fp, uint8_t value)
{
    UINT bytes;
    FRESULT res = f_write(fp, &value, sizeof(value), &bytes);
    if (res != FR_OK) ff_fail(fp, res);
    if (bytes != sizeof(value)) ff_write_fail(fp);
}

void write_word(FIL *fp, uint16_t value)
{
    UINT bytes;
    FRESULT res = f_write(fp, &value, sizeof(value), &bytes);
    if (res != FR_OK) ff_fail(fp, res);
    if (bytes != sizeof(value)) ff_write_fail(fp);
}

void write_long(FIL *fp, uint32_t value)
{
    UINT bytes;
    FRESULT res = f_write(fp, &value, sizeof(value), &bytes);
    if (res != FR_OK) ff_fail(fp, res);
    if (bytes != sizeof(value)) ff_write_fail(fp);
}

void write_data(FIL *fp, const void *data, UINT size)
{
    UINT bytes;
    FRESULT res = f_write(fp, data, size, &bytes);
    if (res != FR_OK) ff_fail(fp, res);
    if (bytes != size) ff_write_fail(fp);
}
