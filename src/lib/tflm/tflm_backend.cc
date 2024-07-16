/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * TensorFlow Lite Micro ML backend.
 */
#include <string.h>
#include <stdint.h>
#include "imlib_config.h"
#ifdef IMLIB_ENABLE_TFLM

#include "tensorflow/lite/micro/micro_op_resolver.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/cortex_m_generic/debug_log_callback.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

extern "C" {
#include "py/runtime.h"
#include "py/obj.h"
#include "py/objlist.h"
#include "py/objtuple.h"
#include "py/binary.h"
#include "py_ml.h"
#include "fb_alloc.h"

using namespace tflite;
#define TF_ARENA_ALIGNMENT  (16 - 1)
typedef MicroMutableOpResolver<113> MicroOpsResolver;

typedef struct ml_backend_state {
    void *arena;
    const Model *model;
    MicroOpsResolver *resolver;
    MicroInterpreter *interpreter;
} ml_backend_state_t;

void abort(void) {
    while (1);
}

void ml_backend_log_handler(const char *s) {
    if (strcmp(s, "\r\n")) {
        mp_printf(MP_PYTHON_PRINTER, "tflm_backend: %s\n", s);
    }
}

static bool ml_backend_valid_dataype(TfLiteType type) {
    return (type == kTfLiteUInt8 ||
            type == kTfLiteInt8 ||
            type == kTfLiteInt16 ||
            type == kTfLiteFloat32);
}

static char ml_backend_map_dtype(TfLiteType type) {
    if (type == kTfLiteUInt8) {
        return 'B';
    } else if (type == kTfLiteInt8) {
        return 'b';
    } else if (type == kTfLiteInt16) {
        return 'h';
    } else {
        return 'f';
    }
}

static void ml_backend_init_ops_resolver(MicroOpsResolver *resolver) {
    resolver->AddAbs();
    resolver->AddAdd();
    resolver->AddAddN();
    resolver->AddArgMax();
    resolver->AddArgMin();
    resolver->AddAssignVariable();
    resolver->AddAveragePool2D();
    resolver->AddBatchMatMul();
    resolver->AddBatchToSpaceNd();
    resolver->AddBroadcastArgs();
    resolver->AddBroadcastTo();
    resolver->AddCallOnce();
    resolver->AddCast();
    resolver->AddCeil();
    resolver->AddCircularBuffer();
    resolver->AddConcatenation();
    resolver->AddConv2D();
    resolver->AddCos();
    resolver->AddCumSum();
    resolver->AddDelay();
    resolver->AddDepthToSpace();
    resolver->AddDepthwiseConv2D();
    resolver->AddDequantize();
    //resolver->AddDetectionPostprocess();
    resolver->AddDiv();
    resolver->AddElu();
    resolver->AddEmbeddingLookup();
    resolver->AddEnergy();
    resolver->AddEqual();
    #ifdef ETHOS_U
    resolver->AddEthosU();
    #endif
    resolver->AddExp();
    resolver->AddExpandDims();
    resolver->AddFftAutoScale();
    resolver->AddFill();
    resolver->AddFilterBank();
    resolver->AddFilterBankLog();
    resolver->AddFilterBankSpectralSubtraction();
    resolver->AddFilterBankSquareRoot();
    resolver->AddFloor();
    resolver->AddFloorDiv();
    resolver->AddFloorMod();
    resolver->AddFramer();
    resolver->AddFullyConnected();
    resolver->AddGather();
    resolver->AddGatherNd();
    resolver->AddGreater();
    resolver->AddGreaterEqual();
    resolver->AddHardSwish();
    resolver->AddIf();
    resolver->AddIrfft();
    resolver->AddL2Normalization();
    resolver->AddL2Pool2D();
    resolver->AddLeakyRelu();
    resolver->AddLess();
    resolver->AddLessEqual();
    resolver->AddLog();
    resolver->AddLogSoftmax();
    resolver->AddLogicalAnd();
    resolver->AddLogicalNot();
    resolver->AddLogicalOr();
    resolver->AddLogistic();
    resolver->AddMaxPool2D();
    resolver->AddMaximum();
    resolver->AddMean();
    resolver->AddMinimum();
    resolver->AddMirrorPad();
    resolver->AddMul();
    resolver->AddNeg();
    resolver->AddNotEqual();
    resolver->AddOverlapAdd();
    resolver->AddPCAN();
    resolver->AddPack();
    resolver->AddPad();
    resolver->AddPadV2();
    resolver->AddPrelu();
    resolver->AddQuantize();
    resolver->AddReadVariable();
    resolver->AddReduceMax();
    resolver->AddRelu();
    resolver->AddRelu6();
    resolver->AddReshape();
    resolver->AddResizeBilinear();
    resolver->AddResizeNearestNeighbor();
    resolver->AddRfft();
    resolver->AddRound();
    resolver->AddRsqrt();
    resolver->AddSelectV2();
    resolver->AddShape();
    resolver->AddSin();
    resolver->AddSlice();
    resolver->AddSoftmax();
    resolver->AddSpaceToBatchNd();
    resolver->AddSpaceToDepth();
    resolver->AddSplit();
    resolver->AddSplitV();
    resolver->AddSqrt();
    resolver->AddSquare();
    resolver->AddSquaredDifference();
    resolver->AddSqueeze();
    resolver->AddStacker();
    resolver->AddStridedSlice();
    resolver->AddSub();
    resolver->AddSum();
    resolver->AddSvdf();
    resolver->AddTanh();
    resolver->AddTranspose();
    resolver->AddTransposeConv();
    resolver->AddUnidirectionalSequenceLSTM();
    resolver->AddUnpack();
    resolver->AddVarHandle();
    resolver->AddWhile();
    resolver->AddWindow();
    resolver->AddZerosLike();
}

int ml_backend_init_model(py_ml_model_obj_t *model) {
    RegisterDebugLogCallback(ml_backend_log_handler);

    // Parse model's data.
    const Model *tflite_model = GetModel(model->data);
    if (tflite_model->version() != TFLITE_SCHEMA_VERSION) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Unsupported model schema"));
    }

    // Initialize the op resolver.
    MicroOpsResolver resolver;
    ml_backend_init_ops_resolver(&resolver);

    // Allocate the interpreter and tensors once to initialize the model, check input
    // and output data types and to get the optimal tensor arena size.
    fb_alloc_mark();
    uint32_t tensor_arena_size;
    uint8_t *tensor_arena = (uint8_t *) fb_alloc_all(&tensor_arena_size, FB_ALLOC_PREFER_SIZE | FB_ALLOC_CACHE_ALIGN);

    MicroInterpreter interpreter(tflite_model,
                                 resolver,
                                 tensor_arena,
                                 tensor_arena_size);
    if (interpreter.AllocateTensors() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Failed to allocate tensors"));
    }

    model->inputs_size = interpreter.inputs_size();
    model->input_shape = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->inputs_size, NULL));
    model->input_scale = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->inputs_size, NULL));
    model->input_zero_point = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->inputs_size, NULL));
    model->input_dtype = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->inputs_size, NULL));

    for (size_t i=0; i<model->inputs_size; i++) {
        TfLiteTensor *input = interpreter.input(i);

        // Check input data type.
        if (!ml_backend_valid_dataype(input->type)) {
            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Unsupported input data type %d"), input->type);
        }

        mp_obj_tuple_t *o = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(input->dims->size, NULL));
        for (int j=0; j<input->dims->size; j++) {
            o->items[j] = mp_obj_new_int(input->dims->data[j]);
        }

        float input_scale = input->params.scale;
        model->input_shape->items[i] = MP_OBJ_FROM_PTR(o);
        model->input_scale->items[i] = mp_obj_new_float((input_scale == 0.0f) ? 1.0f : input_scale);
        model->input_zero_point->items[i] = mp_obj_new_int(input->params.zero_point);
        model->input_dtype->items[i] = mp_obj_new_int(ml_backend_map_dtype(input->type));
    }

    model->outputs_size = interpreter.outputs_size();
    model->output_shape = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->outputs_size, NULL));
    model->output_scale = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->outputs_size, NULL));
    model->output_zero_point = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->outputs_size, NULL));
    model->output_dtype = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->outputs_size, NULL));

    for (size_t i=0; i<model->outputs_size; i++) {
        TfLiteTensor *output = interpreter.output(i);

        // Check output data type.
        if (!ml_backend_valid_dataype(output->type)) {
            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Unsupported output data type %d"), output->type);
        }

        mp_obj_tuple_t *o = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(output->dims->size, NULL));
        for (int j=0; j<output->dims->size; j++) {
            o->items[j] = mp_obj_new_int(output->dims->data[j]);
        }

        float output_scale = output->params.scale;
        model->output_shape->items[i] = MP_OBJ_FROM_PTR(o);
        model->output_scale->items[i] = mp_obj_new_float((output_scale == 0.0f) ? 1.0f : output_scale);
        model->output_zero_point->items[i] = mp_obj_new_int(output->params.zero_point);
        model->output_dtype->items[i] = mp_obj_new_int(ml_backend_map_dtype(output->type));
    }

    model->memory_size = interpreter.arena_used_bytes() + 1024;

    // Free the temporary arena.
    fb_alloc_free_till_mark();

    // Allocate the persistent state.
    ml_backend_state_t *state = m_new0(ml_backend_state_t, 1);
    state->model = GetModel(model->data);
    state->arena = m_new(char, model->memory_size + TF_ARENA_ALIGNMENT);
    state->resolver = new(m_new0(MicroOpsResolver, 1)) MicroOpsResolver();
    ml_backend_init_ops_resolver(state->resolver);
    uint8_t *aligned_arena = (uint8_t *) (((uintptr_t) state->arena + TF_ARENA_ALIGNMENT) & ~(TF_ARENA_ALIGNMENT));
    state->interpreter = new(m_new0(MicroInterpreter, 1)) MicroInterpreter(state->model,
                                                                           *state->resolver,
                                                                           aligned_arena,
                                                                           model->memory_size);
    if (state->interpreter->AllocateTensors() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Failed to allocate tensors"));
    }

    model->state = state;
    model->memory_addr = (uint32_t) state->arena;
    return 0;
}

int ml_backend_run_inference(py_ml_model_obj_t *model) {
    RegisterDebugLogCallback(ml_backend_log_handler);
    ml_backend_state_t *state = (ml_backend_state_t *) model->state;

    if (state->interpreter->Invoke() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invoke failed"));
    }

    return 0;
}

void *ml_backend_get_input(py_ml_model_obj_t *model, size_t index) {
    ml_backend_state_t *state = (ml_backend_state_t *) model->state;
    if (index < state->interpreter->inputs_size()) {
        return state->interpreter->input(index)->data.data;
    }
    mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid input tensor index"));
}

void *ml_backend_get_output(py_ml_model_obj_t *model, size_t index) {
    ml_backend_state_t *state = (ml_backend_state_t *) model->state;
    if (index < state->interpreter->outputs_size()) {
        return state->interpreter->output(index)->data.data;
    }
    mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid output tensor index"));
}
} // extern "C"
#endif // IMLIB_ENABLE_TFLM
