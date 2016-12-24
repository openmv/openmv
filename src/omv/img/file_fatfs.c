/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include <string.h>
#include <mp.h>
#include "fb_alloc.h"
#include "other_maxmin.h"
#include "file_fatfs.h"

#define SECTOR_SIZE 512
#define SECTOR_SIZE_MASK (SECTOR_SIZE - 1)

extern const char *ffs_strerror(FRESULT res);

static void file_fatfs_ffs_strerror(file_fatfs_file_t *ptr, FRESULT res)
{
    f_close(&(ptr->file));
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
}

static void file_fatfs_read_fail(file_fatfs_file_t *ptr)
{
    f_close(&(ptr->file));
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Failed to read requested bytes!"));
}

static void file_fatfs_write_fail(file_fatfs_file_t *ptr)
{
    f_close(&(ptr->file));
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Failed to write requested bytes!"));
}

static void file_fatfs_expect_fail(file_fatfs_file_t *ptr)
{
    f_close(&(ptr->file));
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Unexpected value read!"));
}

void file_fatfs_file_corrupted(file_fatfs_file_t *ptr)
{
    f_close(&(ptr->file));
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "File corrupted!"));
}

void file_fatfs_unsupported_format(file_fatfs_file_t *ptr)
{
    f_close(&(ptr->file));
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Unsupported format!"));
}

void file_fatfs_buffer_on(file_fatfs_file_t *ptr)
{
    ptr->file_buffer_index = 0;
    ptr->file_buffer_offset = f_tell(&(ptr->file)) & SECTOR_SIZE_MASK;
    ptr->file_buffer_pointer = fb_alloc_all((uint32_t *) &(ptr->file_buffer_size)) + ptr->file_buffer_offset;

    if (ptr->file_buffer_size > (SECTOR_SIZE_MASK + ptr->file_buffer_offset)) {
        f_close(&(ptr->file));
        nlr_raise(mp_obj_new_exception_msg(&mp_type_MemoryError, "Out-of-Memory!"));
    }

    ptr->file_buffer_size -= (ptr->file_buffer_size & SECTOR_SIZE_MASK) + ptr->file_buffer_offset;

    if (ptr->file.flag & FA_READ) {
        uint32_t can_do = OTHER_MIN(ptr->file_buffer_size, f_size(&(ptr->file)) - f_tell(&(ptr->file)));
        UINT bytes;
        FRESULT res = f_read(&(ptr->file), ptr->file_buffer_pointer, can_do, &bytes);
        if (res != FR_OK) file_fatfs_ffs_strerror(ptr, res);
        if (bytes != can_do) file_fatfs_read_fail(ptr);
    }
}

void file_fatfs_buffer_off(file_fatfs_file_t *ptr)
{
    if ((ptr->file.flag & FA_WRITE) && ptr->file_buffer_index) {
        UINT bytes;
        FRESULT res = f_write(&(ptr->file), ptr->file_buffer_pointer, ptr->file_buffer_index, &bytes);
        if (res != FR_OK) file_fatfs_ffs_strerror(ptr, res);
        if (bytes != ptr->file_buffer_index) file_fatfs_write_fail(ptr);
    }

    fb_free();

    ptr->file_buffer_index = 0;
    ptr->file_buffer_offset = 0;
    ptr->file_buffer_size = 0;
    ptr->file_buffer_pointer = NULL;
}

void file_fatfs_read_open(file_fatfs_file_t *ptr, const char *path, bool file_buffer_on)
{
    FRESULT res = f_open(&(ptr->file), path, FA_READ | FA_OPEN_EXISTING);
    if (res != FR_OK) file_fatfs_ffs_strerror(ptr, res);
    if (file_buffer_on) file_fatfs_buffer_on(ptr);
}

void file_fatfs_write_open(file_fatfs_file_t *ptr, const char *path, bool file_buffer_on)
{
    FRESULT res = f_open(&(ptr->file), path, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) file_fatfs_ffs_strerror(ptr, res);
    if (file_buffer_on) file_fatfs_buffer_on(ptr);
}

void file_fatfs_close(file_fatfs_file_t *ptr)
{
    if (ptr->file_buffer_pointer) file_fatfs_buffer_off(ptr);
    FRESULT res = f_close(&(ptr->file));
    if (res != FR_OK) file_fatfs_ffs_strerror(ptr, res);
}

void file_fatfs_seek(file_fatfs_file_t *ptr, size_t offset)
{
    bool file_buf = ptr->file_buffer_pointer;
    if(file_buf) file_fatfs_buffer_off(ptr);
    FRESULT res = f_lseek(&(ptr->file), offset);
    if (res != FR_OK) file_fatfs_ffs_strerror(ptr, res);
    if(file_buf) file_fatfs_buffer_on(ptr);
}

size_t file_fatfs_tell(file_fatfs_file_t *ptr)
{
    if (!ptr->file_buffer_pointer) return f_tell(&(ptr->file)); // normal
    if (ptr->file.flag & FA_READ) return f_tell(&(ptr->file)) - ptr->file_buffer_size + ptr->file_buffer_index; // read
    return f_tell(&(ptr->file)) + ptr->file_buffer_index; // write
}

size_t file_fatfs_size(file_fatfs_file_t *ptr)
{
    if (!ptr->file_buffer_pointer) return f_size(&(ptr->file)); // normal
    if (ptr->file.flag & FA_READ) return f_size(&(ptr->file)); // read
    return f_size(&(ptr->file)) + ptr->file_buffer_index; // write
}

size_t file_fatfs_read_byte(file_fatfs_file_t *ptr)
{
    size_t value = 0;
    file_fatfs_read_data(ptr, (char *) &value, sizeof(uint8_t));
    return value;
}

void file_fatfs_read_byte_expect(file_fatfs_file_t *ptr, size_t value)
{
    if (file_fatfs_read_byte(ptr) != value) file_fatfs_expect_fail(ptr);
}

void file_fatfs_read_byte_ignore(file_fatfs_file_t *ptr)
{
    file_fatfs_read_byte(ptr);
}

size_t file_fatfs_read_word(file_fatfs_file_t *ptr)
{
    size_t value = 0;
    file_fatfs_read_data(ptr, (char *) &value, sizeof(uint16_t));
    return value;
}

void file_fatfs_read_word_expect(file_fatfs_file_t *ptr, size_t value)
{
    if (file_fatfs_read_word(ptr) != value) file_fatfs_expect_fail(ptr);
}

void file_fatfs_read_word_ignore(file_fatfs_file_t *ptr)
{
    file_fatfs_read_word(ptr);
}

size_t file_fatfs_read_long(file_fatfs_file_t *ptr)
{
    size_t value = 0;
    file_fatfs_read_data(ptr, (char *) &value, sizeof(uint32_t));
    return value;
}

void file_fatfs_read_long_expect(file_fatfs_file_t *ptr, size_t value)
{
    if (file_fatfs_read_long(ptr) != value) file_fatfs_expect_fail(ptr);
}

void file_fatfs_read_long_ignore(file_fatfs_file_t *ptr)
{
    file_fatfs_read_long(ptr);
}

void file_fatfs_read_data(file_fatfs_file_t *ptr, char *data, size_t size)
{
    if (ptr->file_buffer_pointer) {
        // We get a massive speed boost by buffering up as much data as possible
        // via massive reads. So much so that the time wasted by
        // all these operations does not cost us.
        while (size) {
            // When a sector boundary is encountered while writing a file and there are
            // more than 512 bytes left to write FatFs will detect that it can bypass
            // its internal write buffer and pass the data buffer passed to it directly
            // to the disk write function. However, the disk write function needs the
            // buffer to be aligned to a 4-byte boundary. FatFs doesn't know this and
            // will pass an unaligned buffer if we don't fix the issue. To fix this problem
            // we use a temporary buffer to fix the alignment and to speed everything up.
            // We use this temporary buffer for both reads and writes. The buffer allows us
            // to do multi-block reads and writes which signifcantly speed things up.
            if (ptr->file_buffer_index == ptr->file_buffer_size) {
                ptr->file_buffer_pointer -= ptr->file_buffer_offset;
                ptr->file_buffer_size += ptr->file_buffer_offset;
                ptr->file_buffer_offset = 0;
                ptr->file_buffer_index = 0;
                size_t can_do = OTHER_MIN(ptr->file_buffer_size, f_size(&(ptr->file)) - f_tell(&(ptr->file)));
                UINT bytes;
                FRESULT res = f_read(&(ptr->file), ptr->file_buffer_pointer, can_do, &bytes);
                if (res != FR_OK) file_fatfs_ffs_strerror(ptr, res);
                if (bytes != can_do) file_fatfs_read_fail(ptr);
            }
            size_t can_do = OTHER_MIN(size, ptr->file_buffer_size - ptr->file_buffer_index);
            memcpy(data, ptr->file_buffer_pointer + ptr->file_buffer_index, can_do);
            ptr->file_buffer_index += can_do;
            data += can_do;
            size -= can_do;
        }
    } else {
        UINT bytes;
        FRESULT res = f_read(&(ptr->file), data, size, &bytes);
        if (res != FR_OK) file_fatfs_ffs_strerror(ptr, res);
        if (bytes != size) file_fatfs_read_fail(ptr);
    }
}

void file_fatfs_write_byte(file_fatfs_file_t *ptr, size_t value)
{
    file_fatfs_write_data(ptr, (const char *) &value, sizeof(uint8_t));
}

void file_fatfs_write_word(file_fatfs_file_t *ptr, size_t value)
{
    file_fatfs_write_data(ptr, (const char *) &value, sizeof(uint16_t));
}

void file_fatfs_write_long(file_fatfs_file_t *ptr, size_t value)
{
    file_fatfs_write_data(ptr, (const char *) &value, sizeof(uint32_t));
}

void file_fatfs_write_data(file_fatfs_file_t *ptr, const char *data, size_t size)
{
    if (ptr->file_buffer_pointer) {
        // We get a massive speed boost by buffering up as much data as possible
        // before a write to the SD card. So much so that the time wasted by
        // all these operations does not cost us.
        while (size) {
            size_t can_do = OTHER_MIN(size, ptr->file_buffer_size - ptr->file_buffer_index);
            memcpy(ptr->file_buffer_pointer + ptr->file_buffer_index, data, can_do);
            ptr->file_buffer_index += can_do;
            data += can_do;
            size -= can_do;
            // When a sector boundary is encountered while writing a file and there are
            // more than 512 bytes left to write FatFs will detect that it can bypass
            // its internal write buffer and pass the data buffer passed to it directly
            // to the disk write function. However, the disk write function needs the
            // buffer to be aligned to a 4-byte boundary. FatFs doesn't know this and
            // will pass an unaligned buffer if we don't fix the issue. To fix this problem
            // we use a temporary buffer to fix the alignment and to speed everything up.
            // We use this temporary buffer for both reads and writes. The buffer allows us
            // to do multi-block reads and writes which signifcantly speed things up.
            if (ptr->file_buffer_index == ptr->file_buffer_size) {
                UINT bytes;
                FRESULT res = f_write(&(ptr->file), ptr->file_buffer_pointer, ptr->file_buffer_index, &bytes);
                if (res != FR_OK) file_fatfs_ffs_strerror(ptr, res);
                if (bytes != ptr->file_buffer_index) file_fatfs_write_fail(ptr);
                ptr->file_buffer_pointer -= ptr->file_buffer_offset;
                ptr->file_buffer_size += ptr->file_buffer_offset;
                ptr->file_buffer_offset = 0;
                ptr->file_buffer_index = 0;
            }
        }
    } else {
        UINT bytes;
        FRESULT res = f_write(&(ptr->file), data, size, &bytes);
        if (res != FR_OK) file_fatfs_ffs_strerror(ptr, res);
        if (bytes != size) file_fatfs_write_fail(ptr);
    }
}
