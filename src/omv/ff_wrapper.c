/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013-2016 Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * File System Helper Functions
 *
 */
#include "ff_wrapper.h"

int read_byte(FIL *fp, uint8_t *value)
{
    UINT bytes; FRESULT res = f_read(fp, value, sizeof(*value), &bytes);
    return (bytes==sizeof(*value)) ? res : -1;
}

int read_word(FIL *fp, uint16_t *value)
{
    UINT bytes; FRESULT res = f_read(fp, value, sizeof(*value), &bytes);
    return (bytes==sizeof(*value)) ? res : -1;
}

int read_long(FIL *fp, uint32_t *value)
{
    UINT bytes; FRESULT res = f_read(fp, value, sizeof(*value), &bytes);
    return (bytes==sizeof(*value)) ? res : -1;
}

int read_data(FIL *fp, void *data, UINT size)
{
    UINT bytes; FRESULT res = f_read(fp, data, size, &bytes);
    return (bytes==size) ? res : -1;
}

int write_byte(FIL *fp, uint8_t value)
{
    UINT bytes; FRESULT res = f_write(fp, &value, sizeof(value), &bytes);
    return (bytes==sizeof(value)) ? res : -1;
}

int write_word(FIL *fp, uint16_t value)
{
    UINT bytes; FRESULT res = f_write(fp, &value, sizeof(value), &bytes);
    return (bytes==sizeof(value)) ? res : -1;
}

int write_long(FIL *fp, uint32_t value)
{
    UINT bytes; FRESULT res = f_write(fp, &value, sizeof(value), &bytes);
    return (bytes==sizeof(value)) ? res : -1;
}

int write_data(FIL *fp, const void *data, UINT size)
{
    UINT bytes; FRESULT res = f_write(fp, data, size, &bytes);
    return (bytes==size) ? res : -1;
}
