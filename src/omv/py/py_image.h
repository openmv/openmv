#ifndef __PY_IMAGE_H__
#define __PY_IMAGE_H__
#include "imlib.h"
mp_obj_t py_image_load_image(mp_obj_t path_obj);
mp_obj_t py_image_load_cascade(mp_obj_t path_obj);
mp_obj_t py_image_load_descriptor(mp_obj_t path_obj);
mp_obj_t py_image_save_descriptor(mp_obj_t path_obj, mp_obj_t kpts_obj);
mp_obj_t py_image(int width, int height, int bpp, void *pixels);
mp_obj_t py_image_from_struct(image_t *image);
void *py_image_cobj(mp_obj_t image);
int py_image_descriptor_from_roi(image_t *image, const char *path, rectangle_t *roi);
#endif /* __PY_IMAGE_H__ */

