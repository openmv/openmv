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
#include "ll_aton_reloc_network.h"

typedef struct ml_backend_state {
    NN_Interface_TypeDef nn_interface;
    NN_Instance_TypeDef nn_instance;
    uintptr_t nn_reloc_inst;
} ml_backend_state_t;

static void _assert_func(const char *filename, int line,
                         const char *assert_func, const char *expr ) {
    printf("-> assert_func called from relo code : %d %s : %s %s\n", line, filename, assert_func, expr);
    assert(1 != 1);
}

static const struct ai_reloc_callback __network_reloc_callback = {
    .assert_func = &_assert_func,

    .mcu_cache_invalidate_range = &mcu_cache_invalidate_range,
    .mcu_cache_clean_range = &mcu_cache_clean_range,

    .npu_cache_clean_invalidate_range = &npu_cache_clean_invalidate_range,
    .npu_cache_clean_range = &npu_cache_clean_range,

    .ll_aton_lib_concat = &LL_ATON_LIB_Concat,
    .ll_aton_lib_cast = &LL_ATON_LIB_Cast,
    .ll_aton_lib_softmax = &LL_ATON_LIB_Softmax,
    .ll_aton_lib_dma_imagetorow = &LL_ATON_LIB_DMA_ImageToRow,
    .ll_aton_lib_dma_spacetodepth = &LL_ATON_LIB_DMA_SpaceToDepth,
    .ll_aton_lib_dma_rowtoimage = &LL_ATON_LIB_DMA_RowToImage,
    .ll_aton_lib_dma_depthtospace = &LL_ATON_LIB_DMA_DepthToSpace,
    .ll_aton_lib_dma_outputs_flat_copy = &LL_ATON_LIB_DMA_Outputs_Flat_Copy,
    .ll_aton_lib_dma_outputs_slice_splitlike = &LL_ATON_LIB_DMA_Outputs_Slice_SplitLike,
    .ll_aton_lib_dma_outputs_channel_split_aton = &LL_ATON_LIB_DMA_Outputs_Channel_Split_Aton,
    .ll_aton_lib_dma_outputs_channel_split_batched = &LL_ATON_LIB_DMA_Outputs_Channel_Split_Batched,
    .ll_aton_lib_dma_pad_memset = &LL_ATON_LIB_DMA_Pad_Memset,
    .ll_aton_lib_dma_pad_filling = &LL_ATON_LIB_DMA_Pad_Filling,
    .ll_aton_lib_dma_transpose = &LL_ATON_LIB_DMA_Transpose,
};

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

        __HAL_RCC_NPU_FORCE_RESET();
        __HAL_RCC_NPU_RELEASE_RESET();

        // Enable NPU cache.
        __HAL_RCC_CACHEAXIRAM_MEM_CLK_ENABLE();
        __HAL_RCC_CACHEAXIRAM_MEM_CLK_SLEEP_ENABLE();

        __HAL_RCC_CACHEAXI_CLK_ENABLE();
        __HAL_RCC_CACHEAXI_CLK_SLEEP_ENABLE();

        __HAL_RCC_CACHEAXI_FORCE_RESET();
        __HAL_RCC_CACHEAXI_RELEASE_RESET();

        static CACHEAXI_HandleTypeDef cacheaxi = {
            .Instance = CACHEAXI
        };
        if (HAL_CACHEAXI_Init(&cacheaxi) != HAL_OK ||
            HAL_CACHEAXI_Enable(&cacheaxi) != HAL_OK) {
            return -1;
        }

        npu_initialized = true;
    }
    return 0;
}

int ml_backend_load_model(py_ml_model_obj_t *model, const char *path) {
    if (ml_backend_npu_init() != 0) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to initialize NPU"));
        return -1;
    }

    // Allocate the persistent model state.
    ml_backend_state_t *state = m_new0(ml_backend_state_t, 1);
    state->nn_interface.network_name = "Default";
    state->nn_instance.network = &state->nn_interface;

    // Retrieve the @ of the binary object.
    uintptr_t rom_addr = 0x71000000;

    // Retrieve the info from the binary objects.
    ai_rel_network_info rt;
    if (ai_rel_network_rt_get_info(rom_addr, &rt)) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to load network"));
        return -1;
    }

    // Create and install an instance of the relocatable model
    uint32_t ext_ram_addr = 0x92000000;
    uint32_t ext_ram_size = 1024 *1024;

    // NN executable memory region
    // For COPY mode - XIP region is expected, else only RW region is requested.
    // In the case where the HW epoch blob is embedded in the binary image, this
    // memory region should be also memory-mapped and accessible by the NPU (ATON IP).
    uintptr_t exec_ram_addr = 0x24350000; // SRAM6_ORIGIN
    uint32_t  exec_ram_size = 448U * 1024U;

    // or AI_RELOC_RT_LOAD_MODE_XIP
    if (ai_rel_network_install(rom_addr, exec_ram_addr, exec_ram_size, ext_ram_addr, ext_ram_size,
                               (uint32_t) NULL, AI_RELOC_RT_LOAD_MODE_XIP, &state->nn_reloc_inst)) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to load network"));
        return -1;
    }

    // Register the callbacks
    ai_rel_network_set_callbacks(state->nn_reloc_inst, &__network_reloc_callback);
    state->nn_instance.exec_state.inst_reloc = state->nn_reloc_inst;

    // Initialize the model's state.
    model->state = state;
    model->data = (void *) rom_addr;
    model->memory_addr = exec_ram_addr;
    model->memory_size = exec_ram_size;
    return 0;
}

int ml_backend_init_model(py_ml_model_obj_t *model) {
    if (ml_backend_npu_init() != 0) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to initialize NPU"));
        return -1;
    }

    ml_backend_state_t *state = model->state;
    const LL_Buffer_InfoTypeDef *model_inputs = ai_rel_network_get_input_buffers_info(state->nn_reloc_inst);
    const LL_Buffer_InfoTypeDef *model_outputs = ai_rel_network_get_output_buffers_info(state->nn_reloc_inst);

    // Initialize the model's inputs.
    model->inputs_size = 1;
    model->input_shape = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->inputs_size, NULL));
    model->input_scale = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->inputs_size, NULL));
    model->input_zero_point = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->inputs_size, NULL));
    model->input_dtype = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->inputs_size, NULL));

    for (size_t i=0; i<model->inputs_size; i++) {
        const LL_Buffer_InfoTypeDef *input = &model_inputs[i];

        // Check input data type.
        if (!ml_backend_valid_dataype(input->type)) {
            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Unsupported input data type %d"), input->type);
        }

        mp_obj_tuple_t *o = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(input->mem_ndims, NULL));
        for (int j=0; j<input->mem_ndims; j++) {
            o->items[j] = mp_obj_new_int(input->mem_shape[j]);
        }

        float input_scale = input->scale[0];
        model->input_shape->items[i] = MP_OBJ_FROM_PTR(o);
        model->input_scale->items[i] = mp_obj_new_float((input_scale == 0.0f) ? 1.0f : input_scale);
        model->input_zero_point->items[i] = mp_obj_new_int(input->offset[0]);
        model->input_dtype->items[i] = mp_obj_new_int(ml_backend_map_dtype(input->type));
    }

    // Initialize the model's outputs.
    model->outputs_size = 1;
    model->output_shape = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->outputs_size, NULL));
    model->output_scale = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->outputs_size, NULL));
    model->output_zero_point = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->outputs_size, NULL));
    model->output_dtype = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->outputs_size, NULL));

    for (size_t i=0; i<model->outputs_size; i++) {
        const LL_Buffer_InfoTypeDef *output = &model_outputs[i];

        // Check output data type.
        if (!ml_backend_valid_dataype(output->type)) {
            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Unsupported output data type %d"), output->type);
        }

        mp_obj_tuple_t *o = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(output->mem_ndims, NULL));
        for (int j=0; j<output->mem_ndims; j++) {
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
    for (size_t i=0; i< model->inputs_size; i++) {
        const LL_Buffer_InfoTypeDef *buf = &ai_rel_network_get_input_buffers_info(state->nn_reloc_inst)[i]; 
        SCB_CleanDCache_by_Addr(LL_Buffer_addr_start(buf), LL_Buffer_len(buf));
    }

    // TODO there might be a way to save some steps. See ll_aton/ll_aton_rt_main.c
    LL_ATON_RT_Main(&state->nn_instance);
    // mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invoke failed"));
    return 0;
}

void *ml_backend_get_input(py_ml_model_obj_t *model, size_t index) {
    ml_backend_state_t *state = (ml_backend_state_t *) model->state;

    if (index < model->inputs_size) {
        const LL_Buffer_InfoTypeDef *buf = &ai_rel_network_get_input_buffers_info(state->nn_reloc_inst)[index]; 
        return LL_Buffer_addr_start(buf);
    }
    mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("invalid input tensor index"));
}

void *ml_backend_get_output(py_ml_model_obj_t *model, size_t index) {
    ml_backend_state_t *state = (ml_backend_state_t *) model->state;
    if (index < model->outputs_size) {
        const LL_Buffer_InfoTypeDef *buf = &ai_rel_network_get_output_buffers_info(state->nn_reloc_inst)[index]; 
        SCB_InvalidateDCache_by_Addr(LL_Buffer_addr_start(buf), LL_Buffer_len(buf) & ~(0x1F));
        return LL_Buffer_addr_start(buf);
    }
    mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid output tensor index"));
}
