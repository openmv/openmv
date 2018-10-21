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
#include <py/obj.h>
#include <extmod/vfs.h>
#include <py/stream.h>
#include <lib/oofatfs/ff.h>

typedef struct _FILE {
    mp_obj_t file;
    uint8_t flag;
} file_t;

extern const char *ffs_strerror(FRESULT res);

//OOFATFS wrappers
FRESULT f_open_helper(file_t *fp, const TCHAR *path, BYTE mode);
FRESULT f_opendir_helper(FF_DIR *dp, const TCHAR *path);
FRESULT f_stat_helper(const TCHAR *path, FILINFO *fno);
FRESULT f_mkdir_helper(const TCHAR *path);
FRESULT f_unlink_helper(const TCHAR *path);
FRESULT f_rename_helper(const TCHAR *path_old, const TCHAR *path_new);

void ff_unsupported_format(file_t *fp);
void ff_file_corrupted(file_t *fp);
void ff_not_equal(file_t *fp);
void ff_no_intersection(file_t *fp);
void file_read_open(file_t *fp, const char *path);
void file_write_open(file_t *fp, const char *path);
void file_close(file_t *fp);
ssize_t file_write(file_t *fp, const void *buf, size_t len);
ssize_t file_read(file_t *fp, void *buf, size_t len);
off_t file_lseek(file_t *fp, off_t offset, int whence);
void file_seek(file_t *fp, UINT offset);
#if 0
void file_truncate(file_t *fp);
#endif
void file_sync(file_t *fp);
bool file_eof(file_t *fp);
int file_size(file_t *fp);
int file_tell(file_t *fp);

// File buffer functions.
void file_buffer_init0();
void file_buffer_on(file_t *fp); // does fb_alloc_all
uint32_t file_tell_w_buf(file_t *fp); // valid between on and off
uint32_t file_size_w_buf(file_t *fp); // valid between on and off
void file_buffer_off(file_t *fp); // does fb_free
void file_read_byte(file_t *fp, uint8_t *value);
void file_read_byte_expect(file_t *fp, uint8_t value);
void file_read_byte_ignore(file_t *fp);
void file_read_word(file_t *fp, uint16_t *value);
void file_read_word_expect(file_t *fp, uint16_t value);
void file_read_word_ignore(file_t *fp);
void file_read_long(file_t *fp, uint32_t *value);
void file_read_long_expect(file_t *fp, uint32_t value);
void file_read_long_ignore(file_t *fp);
void file_read_data(file_t *fp, void *data, UINT size);
void file_write_byte(file_t *fp, uint8_t value);
void file_write_word(file_t *fp, uint16_t value);
void file_write_long(file_t *fp, uint32_t value);
void file_write_data(file_t *fp, const void *data, UINT size);

#endif /* __FF_WRAPPER_H__ */
