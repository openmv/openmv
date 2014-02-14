#ifndef __PY_FILE_H__
#define __PY_FILE_H__
#include <ff.h>
const char *ffs_strerror(FRESULT res);
mp_obj_t py_file_open(mp_obj_t path, mp_obj_t mode);
void *py_file_cobj(mp_obj_t file);
#endif /* __PY_FILE_H__ */

