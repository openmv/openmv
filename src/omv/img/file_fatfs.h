/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#ifndef __FILE_FATFS_H__
#define __FILE_FATFS_H__
#include <stddef.h>
#include <stdbool.h>
#include <ff.h>

typedef struct file_fatfs_file
{
    FIL file;
    size_t file_buffer_index, file_buffer_offset, file_buffer_size;
    char *file_buffer_pointer;
}
file_fatfs_file_t;

void file_fatfs_file_corrupted(file_fatfs_file_t *ptr);
void file_fatfs_unsupported_format(file_fatfs_file_t *ptr);
void file_fatfs_buffer_on(file_fatfs_file_t *ptr); // does fb_alloc_all
void file_fatfs_buffer_off(file_fatfs_file_t *ptr); // does fb_free
void file_fatfs_read_open(file_fatfs_file_t *ptr, const char *path, bool file_buffer_on);
void file_fatfs_write_open(file_fatfs_file_t *ptr, const char *path, bool file_buffer_on);
void file_fatfs_close(file_fatfs_file_t *ptr);
void file_fatfs_seek(file_fatfs_file_t *ptr, size_t offset);
size_t file_fatfs_tell(file_fatfs_file_t *ptr);
size_t file_fatfs_size(file_fatfs_file_t *ptr);
size_t file_fatfs_read_byte(file_fatfs_file_t *ptr);
void file_fatfs_read_byte_expect(file_fatfs_file_t *ptr, size_t value);
void file_fatfs_read_byte_ignore(file_fatfs_file_t *ptr);
size_t file_fatfs_read_word(file_fatfs_file_t *ptr);
void file_fatfs_read_word_expect(file_fatfs_file_t *ptr, size_t value);
void file_fatfs_read_word_ignore(file_fatfs_file_t *ptr);
size_t file_fatfs_read_long(file_fatfs_file_t *ptr);
void file_fatfs_read_long_expect(file_fatfs_file_t *ptr, size_t value);
void file_fatfs_read_long_ignore(file_fatfs_file_t *ptr);
void file_fatfs_read_data(file_fatfs_file_t *ptr, char *data, size_t size);
void file_fatfs_write_byte(file_fatfs_file_t *ptr, size_t value);
void file_fatfs_write_word(file_fatfs_file_t *ptr, size_t value);
void file_fatfs_write_long(file_fatfs_file_t *ptr, size_t value);
void file_fatfs_write_data(file_fatfs_file_t *ptr, const char *data, size_t size);

#endif /* __FILE_FATFS_H__ */
