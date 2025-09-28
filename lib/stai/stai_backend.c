/*
 * Copyright (C) 2023-2024 OpenMV, LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Any redistribution, use, or modification in source or binary form
 *    is done solely for personal benefit and not for any commercial
 *    purpose or for monetary gain. For commercial licensing options,
 *    please contact openmv@openmv.io
 *
 * THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
 * OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * STAI ML backend.
 */
#include <string.h>
#include <stdint.h>
#include "imlib_config.h"
#include "omv_common.h"
#include STM32_HAL_H

#include "py/runtime.h"
#include "py/obj.h"
#include "py/objlist.h"
#include "py/objtuple.h"
#include "py/binary.h"
#include "py/gc.h"
#include "py_ml.h"

#include "ll_aton_runtime.h"
#include "ll_aton_platform.h"
#include "ll_aton_caches_interface.h"
#include "ll_aton_reloc_network.h"

#define AI_RELOC_ALIGNMENT      (32)

typedef struct ml_backend_state {
    void *exec_ram_addr;
    uint32_t exec_ram_size;

    void *ext_ram_addr;
    uintptr_t ext_ram_size;

    NN_Instance_TypeDef nn_inst;
    NN_Interface_TypeDef nn_iface;
} ml_backend_state_t;

static bool ml_backend_valid_dataype(Buffer_DataType_TypeDef type) {
    return (type == DataType_UINT8 ||
            type == DataType_INT8 ||
            type == DataType_UINT16 ||
            type == DataType_INT16 ||
            type == DataType_FLOAT);
}

static char ml_backend_map_dtype(Buffer_DataType_TypeDef type) {
    if (type == DataType_UINT8) {
        return 'B';
    } else if (type == DataType_INT8) {
        return 'b';
    } else if (type == DataType_UINT16) {
        return 'H';
    } else if (type == DataType_INT16) {
        return 'h';
    } else {
        return 'f';
    }
}

static int ml_backend_npu_init() {
    static int npu_initialized = false;

    if (!npu_initialized) {
        // Enable NPU clocks.
        __HAL_RCC_NPU_CLK_ENABLE();
        __HAL_RCC_NPU_CLK_SLEEP_ENABLE();

        // Reset NPU.
        __HAL_RCC_NPU_FORCE_RESET();
        __HAL_RCC_NPU_RELEASE_RESET();

        // Enable NPU cache clocks.
        __HAL_RCC_CACHEAXI_CLK_ENABLE();
        __HAL_RCC_CACHEAXI_CLK_SLEEP_ENABLE();

        // Reset NPU cache.
        __HAL_RCC_CACHEAXI_FORCE_RESET();
        __HAL_RCC_CACHEAXI_RELEASE_RESET();

        // Initialize NPU cache.
        npu_cache_init();
        npu_cache_enable();

        npu_initialized = true;
    }

    return 0;
}

int ml_backend_init_model(py_ml_model_obj_t *model) {
    if (ml_backend_npu_init() != 0) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to initialize NPU"));
        return -1;
    }

    // Allocate the persistent model state.
    ml_backend_state_t *state = m_new0(ml_backend_state_t, 1);
    state->nn_iface.network_name = "Default";
    state->nn_inst.network = &state->nn_iface;

    // Retrieve the info from the relocatable model.
    ll_aton_reloc_info rt;
    if (ll_aton_reloc_get_info((uintptr_t) model->data, &rt)) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to load network"));
        return -1;
    }

    // Allocate executable memory.
    state->exec_ram_size = OMV_ALIGN_TO(rt.rt_ram_xip, AI_RELOC_ALIGNMENT);
    state->exec_ram_addr = m_new(uint8_t, state->exec_ram_size + AI_RELOC_ALIGNMENT);

    // Allocate external memory.
    state->ext_ram_size = OMV_ALIGN_TO(rt.ext_ram_sz, AI_RELOC_ALIGNMENT);
    state->ext_ram_addr = m_new(uint8_t, state->ext_ram_size + AI_RELOC_ALIGNMENT);

    // Create and install the relocatable model.
    ll_aton_reloc_config config = {
        .ext_ram_size = state->ext_ram_size,
        .ext_ram_addr = OMV_ALIGN_TO(state->ext_ram_addr, AI_RELOC_ALIGNMENT),
        .exec_ram_size = state->exec_ram_size,
        .exec_ram_addr = OMV_ALIGN_TO(state->exec_ram_addr, AI_RELOC_ALIGNMENT),
        .ext_param_addr = (uintptr_t) NULL,
        // For COPY mode - XIP region is expected, else only RW region is requested.
        // In the case where the HW epoch blob is embedded in the binary image, this
        // memory region should be also memory-mapped and accessible by the NPU (ATON IP).
        .mode = AI_RELOC_RT_LOAD_MODE_XIP,
    };

    // Invalidate DCache before installing the model's data.
    SCB_InvalidateDCache_by_Addr((void *) config.exec_ram_addr, config.exec_ram_size);

    if (ll_aton_reloc_install((uintptr_t) model->data, &config, &state->nn_inst)) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to load network"));
        return -1;
    }

    // Clean DCache after installing the model's data.
    SCB_CleanDCache_by_Addr((void *) config.exec_ram_addr, config.exec_ram_size);

    // Invalidate ICache in copy mode (executing code from ram).
    if (config.mode == AI_RELOC_RT_LOAD_MODE_COPY) {
        SCB_InvalidateICache_by_Addr((void *) config.exec_ram_addr, config.exec_ram_size);
    }

    // Initialize the model's state.
    model->state = state;
    model->memory_addr = config.exec_ram_addr;
    model->memory_size = config.exec_ram_size + config.ext_ram_size;

    const LL_Buffer_InfoTypeDef *model_inputs = ll_aton_reloc_get_input_buffers_info(&state->nn_inst, -1);
    const LL_Buffer_InfoTypeDef *model_outputs = ll_aton_reloc_get_output_buffers_info(&state->nn_inst, -1);

    // Initialize the model's inputs.
    for (model->inputs_size = 0; model_inputs[model->inputs_size].name != NULL; model->inputs_size++) {
        ;
    }
    model->input_shape = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->inputs_size, NULL));
    model->input_scale = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->inputs_size, NULL));
    model->input_zero_point = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->inputs_size, NULL));
    model->input_dtype = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->inputs_size, NULL));

    for (size_t i = 0; i < model->inputs_size; i++) {
        const LL_Buffer_InfoTypeDef *input = &model_inputs[i];

        // Check input data type.
        if (!ml_backend_valid_dataype(input->type)) {
            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Unsupported input data type %d"), input->type);
        }

        mp_obj_tuple_t *o = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(input->mem_ndims, NULL));
        for (int j = 0; j < input->mem_ndims; j++) {
            o->items[j] = mp_obj_new_int(input->mem_shape[j]);
        }

        float input_scale = input->scale[0];
        model->input_shape->items[i] = MP_OBJ_FROM_PTR(o);
        model->input_scale->items[i] = mp_obj_new_float((input_scale == 0.0f) ? 1.0f : input_scale);
        model->input_zero_point->items[i] = mp_obj_new_int(input->offset[0]);
        model->input_dtype->items[i] = mp_obj_new_int(ml_backend_map_dtype(input->type));
    }

    // Initialize the model's outputs.
    for (model->outputs_size = 0; model_outputs[model->outputs_size].name != NULL; model->outputs_size++) {
        ;
    }
    model->output_shape = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->outputs_size, NULL));
    model->output_scale = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->outputs_size, NULL));
    model->output_zero_point = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->outputs_size, NULL));
    model->output_dtype = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->outputs_size, NULL));

    for (size_t i = 0; i < model->outputs_size; i++) {
        const LL_Buffer_InfoTypeDef *output = &model_outputs[i];

        // Check output data type.
        if (!ml_backend_valid_dataype(output->type)) {
            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Unsupported output data type %d"), output->type);
        }

        mp_obj_tuple_t *o = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(output->mem_ndims, NULL));
        for (int j = 0; j < output->mem_ndims; j++) {
            o->items[j] = mp_obj_new_int(output->mem_shape[j]);
        }

        model->output_shape->items[i] = MP_OBJ_FROM_PTR(o);
        model->output_scale->items[i] = mp_obj_new_float((output->type == DataType_FLOAT) ? 1.0f : output->scale[0]);
        model->output_zero_point->items[i] = mp_obj_new_int((output->type == DataType_FLOAT) ? 0 : output->offset[0]);
        model->output_dtype->items[i] = mp_obj_new_int(ml_backend_map_dtype(output->type));
    }

    return 0;
}

int ml_backend_run_inference(py_ml_model_obj_t *model) {
    ml_backend_state_t *state = (ml_backend_state_t *) model->state;

    // Flush input buffers.
    for (size_t i = 0; i < model->inputs_size; i++) {
        const LL_Buffer_InfoTypeDef *buf = ll_aton_reloc_get_input_buffers_info(&state->nn_inst, i);
        SCB_CleanDCache_by_Addr(LL_Buffer_addr_start(buf), LL_Buffer_len(buf));
    }

    LL_ATON_RT_Main(&state->nn_inst);
    return 0;
}

void *ml_backend_get_input(py_ml_model_obj_t *model, size_t index) {
    ml_backend_state_t *state = (ml_backend_state_t *) model->state;

    if (index < model->inputs_size) {
        const LL_Buffer_InfoTypeDef *buf = ll_aton_reloc_get_input_buffers_info(&state->nn_inst, index);
        return LL_Buffer_addr_start(buf);
    }

    mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("invalid input tensor index"));
}

void *ml_backend_get_output(py_ml_model_obj_t *model, size_t index) {
    ml_backend_state_t *state = (ml_backend_state_t *) model->state;

    if (index < model->outputs_size) {
        const LL_Buffer_InfoTypeDef *buf = ll_aton_reloc_get_output_buffers_info(&state->nn_inst, index);
        SCB_InvalidateDCache_by_Addr(LL_Buffer_addr_start(buf), LL_Buffer_len(buf));
        return LL_Buffer_addr_start(buf);
    }

    mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid output tensor index"));
}
