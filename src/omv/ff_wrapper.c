/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013-2016 Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * File System Helper Functions
 *
 */
#include <py/builtin.h>
#include "mp.h"
#include "common.h"
#include "fb_alloc.h"
#include "ff_wrapper.h"

#define FF_MIN(x,y) (((x)<(y))?(x):(y))

NORETURN static void ff_fail(file_t *fp, FRESULT res)
{
    if (fp && fp->file) mp_stream_close(fp->file);
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
}

NORETURN static void ffile_read_fail(file_t *fp)
{
    if (fp && fp->file) mp_stream_close(fp->file);
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Failed to read requested bytes!"));
}

NORETURN static void ffile_write_fail(file_t *fp)
{
    if (fp && fp->file) mp_stream_close(fp->file);
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Failed to write requested bytes!"));
}

NORETURN static void ff_expect_fail(file_t *fp)
{
    if (fp && fp->file) mp_stream_close(fp->file);
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Unexpected value read!"));
}

NORETURN void ff_unsupported_format(file_t *fp)
{
    if (fp && fp->file) mp_stream_close(fp->file);
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Unsupported format!"));
}

NORETURN void ff_file_corrupted(file_t *fp)
{
    if (fp && fp->file) mp_stream_close(fp->file);
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "File corrupted!"));
}

NORETURN void ff_not_equal(file_t *fp)
{
    if (fp && fp->file) mp_stream_close(fp->file);
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Images not equal!"));
}

NORETURN void ff_no_intersection(file_t *fp)
{
    if (fp && fp->file) mp_stream_close(fp->file);
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "No intersection!"));
}

void file_read_open(file_t *fp, const char *path)
{
    mp_obj_t mp_path = mp_obj_new_str(path, strlen(path));
    fp->file = mp_builtin_open(1, &mp_path, (mp_map_t*)&mp_const_empty_map);
    fp->flag = FA_READ|FA_OPEN_EXISTING;
}

void file_write_open(file_t *fp, const char *path)
{
    mp_obj_t args[]  = {
        mp_obj_new_str(path, strlen(path)),
        MP_ROM_QSTR(MP_QSTR_w)
    };
    fp->file = mp_builtin_open(2, args, (mp_map_t*)&mp_const_empty_map);
    fp->flag = FA_WRITE|FA_CREATE_ALWAYS;
}

void file_close(file_t *fp)
{
    mp_stream_close(fp->file);
}

off_t file_lseek(file_t *fp, off_t offset, int whence)
{
    int errno;
    const mp_stream_p_t *stream_p = mp_get_stream_raise(fp->file, MP_STREAM_OP_IOCTL);
    struct mp_stream_seek_t seek_s;
    seek_s.offset = offset;
    seek_s.whence = whence;
    mp_uint_t res = stream_p->ioctl(MP_OBJ_FROM_PTR(fp->file), MP_STREAM_SEEK, (mp_uint_t)(uintptr_t)&seek_s, &errno);
    if (res == MP_STREAM_ERROR) {
        ff_fail(fp, errno);
    }
    return seek_s.offset;
}

void file_seek(file_t *fp, UINT offset)
{
    file_lseek(fp, offset, SEEK_SET);
}

#if 0
void file_truncate(file_t *fp)
{
    FRESULT res = f_truncate(fp->file);
    if (res != FR_OK) ff_fail(fp, res);
}
#endif

void file_sync(file_t *fp)
{
    int errno;
    const mp_stream_p_t *stream_p = mp_get_stream_raise(fp->file, MP_STREAM_OP_WRITE);
    mp_uint_t res = stream_p->ioctl(MP_OBJ_FROM_PTR(fp->file), MP_STREAM_FLUSH, 0, &errno);
    if (res == MP_STREAM_ERROR) {
        ff_fail(fp, errno);
    }
}

int file_size(file_t *fp) 
{
    int current = file_lseek(fp, 0, SEEK_CUR);
    int end = file_lseek(fp, 0, MP_SEEK_END);
    file_lseek(fp, current, MP_SEEK_SET);
    return end;
}

bool file_eof(file_t *fp)
{
    int current = file_lseek(fp, 0, SEEK_CUR);
    int end = file_lseek(fp, 0, MP_SEEK_END);
    if (current == end) {
        return true;
    }
    file_lseek(fp, current, MP_SEEK_SET);
    return false;
}

int file_tell(file_t *fp) 
{
    return file_lseek(fp, 0, SEEK_CUR);
}

ssize_t file_write(file_t *fp, const void *buf, size_t len) 
{
    int errno;
    const mp_stream_p_t *stream_p = mp_get_stream_raise(fp->file, MP_STREAM_OP_WRITE);
    mp_uint_t out_sz = stream_p->write(MP_OBJ_FROM_PTR(fp->file), buf, len, &errno);
    if (out_sz == MP_STREAM_ERROR) {
        ff_fail(fp, errno);
    } 
    return out_sz;
}

ssize_t file_read(file_t *fp, void *buf, size_t len) 
{
    int errno;
    const mp_stream_p_t *stream_p = mp_get_stream_raise(fp->file, MP_STREAM_OP_READ);
    mp_uint_t out_sz = stream_p->read(MP_OBJ_FROM_PTR(fp->file), buf, len, &errno);
    if (out_sz == MP_STREAM_ERROR) {
        ff_fail(fp, errno);
    }
    return out_sz;
}

// These wrapper functions are used for backward compatibility with
// OpenMV code using vanilla FatFS. Note: Extracted from cc3200 ftp.c
#if 0
STATIC FATFS *lookup_path(const TCHAR **path) {
    mp_vfs_mount_t *fs = mp_vfs_lookup_path(*path, path);
    if (fs == MP_VFS_NONE || fs == MP_VFS_ROOT) {
        return NULL;
    }
    // here we assume that the mounted device is FATFS
    return &((fs_user_mount_t*)MP_OBJ_TO_PTR(fs->obj))->fatfs;
}
#endif

FRESULT f_open_helper(file_t *fp, const TCHAR *path, BYTE mode) {
    if (mode & FA_WRITE) {
        file_write_open(fp, path);
        return FR_OK;
    } else if (mode & FA_READ) {
        file_read_open(fp, path);
        return FR_OK;
    }
    return FR_INT_ERR;
}

#if 0
FRESULT f_opendir_helper(FF_DIR *dp, const TCHAR *path) {
    FATFS *fs = lookup_path(&path);
    if (fs == NULL) {
        return FR_NO_PATH;
    }
    return f_opendir(fs, dp, path);
}

FRESULT f_stat_helper(const TCHAR *path, FILINFO *fno) {
    FATFS *fs = lookup_path(&path);
    if (fs == NULL) {
        return FR_NO_PATH;
    }
    return f_stat(fs, path, fno);
}

FRESULT f_mkdir_helper(const TCHAR *path) {
    FATFS *fs = lookup_path(&path);
    if (fs == NULL) {
        return FR_NO_PATH;
    }
    return f_mkdir(fs, path);
}

FRESULT f_unlink_helper(const TCHAR *path) {
    FATFS *fs = lookup_path(&path);
    if (fs == NULL) {
        return FR_NO_PATH;
    }
    return f_unlink(fs, path);
}

FRESULT f_rename_helper(const TCHAR *path_old, const TCHAR *path_new) {
    FATFS *fs_old = lookup_path(&path_old);
    if (fs_old == NULL) {
        return FR_NO_PATH;
    }
    FATFS *fs_new = lookup_path(&path_new);
    if (fs_new == NULL) {
        return FR_NO_PATH;
    }
    if (fs_old != fs_new) {
        return FR_NO_PATH;
    }
    return f_rename(fs_new, path_old, path_new);
}
#endif

// When a sector boundary is encountered while writing a file and there are
// more than 512 bytes left to write FatFs will detect that it can bypass
// its internal write buffer and pass the data buffer passed to it directly
// to the disk write function. However, the disk write function needs the
// buffer to be aligned to a 4-byte boundary. FatFs doesn't know this and
// will pass an unaligned buffer if we don't fix the issue. To fix this problem
// we use a temporary buffer to fix the alignment and to speed everything up.

// We use this temporary buffer for both reads and writes. The buffer allows us
// to do multi-block reads and writes which signifcantly speed things up.

static uint32_t file_buffer_offset = 0;
static uint8_t *file_buffer_pointer = 0;
static uint32_t file_buffer_size = 0;
static uint32_t file_buffer_index = 0;

void file_buffer_init0(void)
{
    file_buffer_offset = 0;
    file_buffer_pointer = 0;
    file_buffer_size = 0;
    file_buffer_index = 0;
}

ALWAYS_INLINE static void file_fill(file_t *fp)
{
    if (file_buffer_index == file_buffer_size) {
        file_buffer_pointer -= file_buffer_offset;
        file_buffer_size += file_buffer_offset;
        file_buffer_offset = 0;
        file_buffer_index = 0;
        uint32_t file_remaining = file_size(fp) - file_tell(fp);
        uint32_t can_do = FF_MIN(file_buffer_size, file_remaining);
        UINT bytes = file_read(fp, file_buffer_pointer, can_do);
        if (bytes != can_do) ffile_read_fail(fp);
    }
}

ALWAYS_INLINE static void file_flush(file_t *fp)
{
    if (file_buffer_index == file_buffer_size) {
        UINT bytes = file_write(fp, file_buffer_pointer, file_buffer_index);
        if (bytes != file_buffer_index) ffile_write_fail(fp);
        file_buffer_pointer -= file_buffer_offset;
        file_buffer_size += file_buffer_offset;
        file_buffer_offset = 0;
        file_buffer_index = 0;
    }
}

uint32_t file_tell_w_buf(file_t *fp)
{
    if (fp->flag & FA_READ) {
        return file_tell(fp) - file_buffer_size + file_buffer_index;
    } else {
        return file_tell(fp) + file_buffer_index;
    }
}

uint32_t file_size_w_buf(file_t *fp)
{
    if (fp->flag & FA_READ) {
        return file_size(fp);
    } else {
        return file_size(fp) + file_buffer_index;
    }
}

void file_buffer_on(file_t *fp)
{
    file_buffer_offset = file_tell(fp) % 4;
    file_buffer_pointer = (uint8_t *)fb_alloc_all(&file_buffer_size) + file_buffer_offset;
    if (!file_buffer_size) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_MemoryError, "No memory!"));
    }
    file_buffer_size -= file_buffer_offset;
    file_buffer_index = 0;
    if (fp->flag & FA_READ) {
        uint32_t file_remaining = file_size(fp) - file_tell(fp);
        uint32_t can_do = FF_MIN(file_buffer_size, file_remaining);
        UINT bytes = file_read(fp, file_buffer_pointer, can_do);
        if (bytes != can_do) ffile_read_fail(fp);
    }
}

void file_buffer_off(file_t *fp)
{
    if ((fp->flag & FA_WRITE) && file_buffer_index) {
        UINT bytes = file_write(fp, file_buffer_pointer, file_buffer_index);
        if (bytes != file_buffer_index) ffile_write_fail(fp);
    }
    file_buffer_pointer = 0;
    fb_free();
}

void file_read_byte(file_t *fp, uint8_t *value)
{
    if (file_buffer_pointer) {
        // We get a massive speed boost by buffering up as much data as possible
        // via massive reads. So much so that the time wasted by
        // all these operations does not cost us.
        for (size_t i = 0; i < sizeof(*value); i++) {
            file_fill(fp);
            ((uint8_t *) value)[i] = file_buffer_pointer[file_buffer_index++];
        }
    } else {
        UINT bytes = file_read(fp, value, sizeof(*value));
        if (bytes != sizeof(*value)) ffile_read_fail(fp);
    }
}

void file_read_byte_expect(file_t *fp, uint8_t value)
{
    uint8_t compare;
    file_read_byte(fp, &compare);
    if (value != compare) ff_expect_fail(fp);
}

void file_read_byte_ignore(file_t *fp)
{
    uint8_t trash;
    file_read_byte(fp, &trash);
}

void file_read_word(file_t *fp, uint16_t *value)
{
    if (file_buffer_pointer) {
        // We get a massive speed boost by buffering up as much data as possible
        // via massive reads. So much so that the time wasted by
        // all these operations does not cost us.
        for (size_t i = 0; i < sizeof(*value); i++) {
            file_fill(fp);
            ((uint8_t *) value)[i] = file_buffer_pointer[file_buffer_index++];
        }
    } else {
        UINT bytes = file_read(fp, value, sizeof(*value));
        if (bytes != sizeof(*value)) ffile_read_fail(fp);
    }
}

void file_read_word_expect(file_t *fp, uint16_t value)
{
    uint16_t compare;
    file_read_word(fp, &compare);
    if (value != compare) ff_expect_fail(fp);
}

void file_read_word_ignore(file_t *fp)
{
    uint16_t trash;
    file_read_word(fp, &trash);
}

void file_read_long(file_t *fp, uint32_t *value)
{
    if (file_buffer_pointer) {
        // We get a massive speed boost by buffering up as much data as possible
        // via massive reads. So much so that the time wasted by
        // all these operations does not cost us.
        for (size_t i = 0; i < sizeof(*value); i++) {
            file_fill(fp);
            ((uint8_t *) value)[i] = file_buffer_pointer[file_buffer_index++];
        }
    } else {
        UINT bytes = file_read(fp, value, sizeof(*value));
        if (bytes != sizeof(*value)) ffile_read_fail(fp);
    }
}

void file_read_long_expect(file_t *fp, uint32_t value)
{
    uint32_t compare;
    file_read_long(fp, &compare);
    if (value != compare) ff_expect_fail(fp);
}

void file_read_long_ignore(file_t *fp)
{
    uint32_t trash;
    file_read_long(fp, &trash);
}

void file_read_data(file_t *fp, void *data, UINT size)
{
    if (file_buffer_pointer) {
        // We get a massive speed boost by buffering up as much data as possible
        // via massive reads. So much so that the time wasted by
        // all these operations does not cost us.
        while (size) {
            file_fill(fp);
            uint32_t file_buffer_space_left = file_buffer_size - file_buffer_index;
            uint32_t can_do = FF_MIN(size, file_buffer_space_left);
            memcpy(data, file_buffer_pointer+file_buffer_index, can_do);
            file_buffer_index += can_do;
            data = (uint32_t *)data + can_do;
            size -= can_do;
        }
    } else {
        UINT bytes = file_read(fp, data, size);
        if (bytes != size) ffile_read_fail(fp);
    }
}

void file_write_byte(file_t *fp, uint8_t value)
{
    if (file_buffer_pointer) {
        // We get a massive speed boost by buffering up as much data as possible
        // before a write to the SD card. So much so that the time wasted by
        // all these operations does not cost us.
        for (size_t i = 0; i < sizeof(value); i++) {
            file_buffer_pointer[file_buffer_index++] = ((uint8_t *) &value)[i];
            file_flush(fp);
        }
    } else {
        UINT bytes = file_write(fp, &value, sizeof(value));
        if (bytes != sizeof(value)) ffile_write_fail(fp);
    }
}

void file_write_word(file_t *fp, uint16_t value)
{
    if (file_buffer_pointer) {
        // We get a massive speed boost by buffering up as much data as possible
        // before a write to the SD card. So much so that the time wasted by
        // all these operations does not cost us.
        for (size_t i = 0; i < sizeof(value); i++) {
            file_buffer_pointer[file_buffer_index++] = ((uint8_t *) &value)[i];
            file_flush(fp);
        }
    } else {
        UINT bytes = file_write(fp, &value, sizeof(value));
        if (bytes != sizeof(value)) ffile_write_fail(fp);
    }
}

void file_write_long(file_t *fp, uint32_t value)
{
    if (file_buffer_pointer) {
        // We get a massive speed boost by buffering up as much data as possible
        // before a write to the SD card. So much so that the time wasted by
        // all these operations does not cost us.
        for (size_t i = 0; i < sizeof(value); i++) {
            file_buffer_pointer[file_buffer_index++] = ((uint8_t *) &value)[i];
            file_flush(fp);
        }
    } else {
        UINT bytes = file_write(fp, &value, sizeof(value));
        if (bytes != sizeof(value)) ffile_write_fail(fp);
    }
}

void file_write_data(file_t *fp, const void *data, UINT size)
{
    if (file_buffer_pointer) {
        // We get a massive speed boost by buffering up as much data as possible
        // before a write to the SD card. So much so that the time wasted by
        // all these operations does not cost us.
        while (size) {
            uint32_t file_buffer_space_left = file_buffer_size - file_buffer_index;
            uint32_t can_do = FF_MIN(size, file_buffer_space_left);
            memcpy(file_buffer_pointer+file_buffer_index, data, can_do);
            file_buffer_index += can_do;
            data = (uint32_t *)data + can_do;
            size -= can_do;
            file_flush(fp);
        }
    } else {
        UINT bytes = file_write(fp, data, size);
        if (bytes != size) ffile_write_fail(fp);
    }
}
