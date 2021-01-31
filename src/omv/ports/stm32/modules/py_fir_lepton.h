/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2020 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2020 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * FIR Python module.
 */
#ifndef __PY_FIR_LEPTON_H__
#define __PY_FIR_LEPTON_H__
void fir_lepton_deinit();
int fir_lepton_init(cambus_t *bus, int *w, int *h, int *refresh, int *resolution);
void fir_lepton_register_vsync_cb(mp_obj_t cb);
mp_obj_t fir_lepton_get_radiometry();
void fir_lepton_register_frame_cb(mp_obj_t cb);
mp_obj_t fir_lepton_get_frame_available();
mp_obj_t fir_lepton_read_ta();
mp_obj_t fir_lepton_read_ir(int w, int h, bool mirror, bool flip, bool transpose, int timeout);
void fir_lepton_fill_image(image_t *img, int w, int h, bool auto_range, float min, float max,
                           bool mirror, bool flip, bool transpose, int timeout);
void fir_lepton_trigger_ffc(uint n_args, const mp_obj_t *args, mp_map_t *kw_args);
#endif // __PY_FIR_LEPTON_H__
