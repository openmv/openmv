/*
 * Copyright (C) 2026 OpenMV, LLC.
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
 * PyTorch (ExecuTorch) ML backend.
 */
#if MICROPY_PY_ML_TORCH
#include <string.h>
#include <stdint.h>
#include <new>

#include "imlib_config.h"
#include "omv_common.h"

#include <executorch/extension/data_loader/buffer_data_loader.h>
#include <executorch/runtime/core/memory_allocator.h>
#include <executorch/runtime/executor/memory_manager.h>
#include <executorch/runtime/executor/method.h>
#include <executorch/runtime/executor/program.h>
#include <executorch/runtime/platform/platform.h>

using executorch::aten::ScalarType;
using executorch::aten::Tensor;
using executorch::extension::BufferDataLoader;
using executorch::runtime::Error;
using executorch::runtime::EValue;
using executorch::runtime::HierarchicalAllocator;
using executorch::runtime::MemoryAllocator;
using executorch::runtime::MemoryManager;
using executorch::runtime::Method;
using executorch::runtime::MethodMeta;
using executorch::runtime::Program;
using executorch::runtime::Result;
using executorch::runtime::Span;
using executorch::runtime::TensorInfo;

// Required by the deleting destructors of the runtime's polymorphic classes.
// Nothing is ever allocated with operator new.
__attribute__((weak)) void operator delete(void *, unsigned int) {

}

extern "C" {
#include "py/runtime.h"
#include "py/obj.h"
#include "py/objtuple.h"
#include "py_ml.h"
#include "umalloc.h"
#include "common/omv_profiler.h"
}

#ifdef ETHOS_U
// Largest Ethos-U scratch buffer required by the delegates initialized since the
// last reset (libtorch patch 0004).
namespace executorch {
namespace backends {
namespace arm {
size_t get_ethos_u_scratch_size();
void reset_ethos_u_scratch_size();
} // namespace arm
} // namespace backends
} // namespace executorch
#endif

#define TORCH_METHOD_NAME       "forward"
#define TORCH_BUFFER_ALIGN      (16)
#define TORCH_CHUNK_SIZE        (4096U)

// Method allocator that allocates UMA chunks on demand. The method allocator holds
// the runtime's own structures (values, tensor metadata, delegate handles), which
// are allocated while loading the method and whose total size can't be queried.
class ChunkMemoryAllocator : public MemoryAllocator {
    public:
        ChunkMemoryAllocator() : MemoryAllocator(0, nullptr), head_(nullptr), cur_(0), end_(0), size_(0) {
        }

        void *allocate(size_t size, size_t alignment = kDefaultAlignment) override {
            uintptr_t ptr = OMV_ALIGN_TO(cur_, alignment);
            if (!head_ || ptr + size > end_) {
                // Allocate a new chunk, prefixed with a link to the previous one.
                size_t header = OMV_ALIGN_TO(sizeof(void *), TORCH_BUFFER_ALIGN);
                size_t chunk_size = OMV_MAX(TORCH_CHUNK_SIZE, header + size + alignment);
                void **chunk = (void **) uma_malign(chunk_size, TORCH_BUFFER_ALIGN, UMA_PERSIST | UMA_MAYBE);
                if (!chunk) {
                    return nullptr;
                }
                chunk[0] = head_;
                head_ = chunk;
                size_ += chunk_size;
                cur_ = (uintptr_t) chunk + header;
                end_ = (uintptr_t) chunk + chunk_size;
                ptr = OMV_ALIGN_TO(cur_, alignment);
            }
            cur_ = ptr + size;
            return (void *) ptr;
        }

        size_t used_size() const override {
            return size_;
        }

        void reset() override {
            // Allocations live until the method is unloaded.
        }

        void free_all() {
            while (head_) {
                void **next = (void **) head_[0];
                uma_free(head_);
                head_ = next;
            }
            cur_ = end_ = size_ = 0;
        }

    private:
        void **head_;
        uintptr_t cur_;
        uintptr_t end_;
        size_t size_;
};

extern "C" {
// Registers the kernels and backends linked into libtorch.
int et_register_all(void);

typedef struct ml_backend_state {
    // Objects constructed in place; the loader and program must outlive the method.
    alignas(BufferDataLoader) uint8_t loader[sizeof(BufferDataLoader)];
    alignas(Program) uint8_t program[sizeof(Program)];
    alignas(Method) uint8_t method[sizeof(Method)];
    alignas(ChunkMemoryAllocator) uint8_t method_allocator[sizeof(ChunkMemoryAllocator)];
    alignas(MemoryAllocator) uint8_t temp_allocator[sizeof(MemoryAllocator)];
    alignas(HierarchicalAllocator) uint8_t planned_allocator[sizeof(HierarchicalAllocator)];
    alignas(MemoryManager) uint8_t memory_manager[sizeof(MemoryManager)];
    size_t planned_count;
    Span<uint8_t> *planned_spans;
    uint8_t *temp_pool;
    bool loaded;
} ml_backend_state_t;

#define STATE_OBJ(state, type, field) (reinterpret_cast<type *>((state)->field))

// ExecuTorch aborts on unrecoverable kernel errors. Raise instead of halting.
void et_pal_abort(void) {
    mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("torch_backend: abort"));
}

// Allow ports that provide their own abort() to take precedence.
__attribute__((weak)) void abort(void) {
    et_pal_abort();
    while (1) {
        ;
    }
}

static void ml_backend_check(Error err, const char *what) {
    if (err != Error::Ok) {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("torch_backend: %s failed (error 0x%x)"),
                          what, (unsigned int) err);
    }
}

// Buffers used by the kernels and the NPU (planned memory and the temp pool) are
// allocated from fast memory if available.
static uint8_t *ml_backend_alloc(size_t size) {
    return (uint8_t *) uma_malign(size, TORCH_BUFFER_ALIGN, UMA_FAST | UMA_PERSIST);
}

static char ml_backend_map_dtype(ScalarType type) {
    switch (type) {
        case ScalarType::Byte:
            return 'B';
        case ScalarType::Char:
            return 'b';
        case ScalarType::UInt16:
            return 'H';
        case ScalarType::Short:
            return 'h';
        case ScalarType::Int:
            return 'i';
        case ScalarType::Long:
            return 'q';
        case ScalarType::Bool:
            return '?';
        case ScalarType::Float:
            return 'f';
        default:
            return 0;
    }
}

// Returns the tensor shape as the Python ML libraries expect it. 4D channels-last
// tensors (dim order 0, 2, 3, 1) are reported as NHWC, which matches their layout.
static mp_obj_t ml_backend_shape(const TensorInfo &info) {
    Span<const int32_t> sizes = info.sizes();
    Span<const uint8_t> order = info.dim_order();
    mp_obj_tuple_t *o = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(sizes.size(), NULL));
    for (size_t i = 0; i < sizes.size(); i++) {
        size_t dim = (order.size() == sizes.size()) ? order[i] : i;
        o->items[i] = mp_obj_new_int(sizes[dim]);
    }
    return MP_OBJ_FROM_PTR(o);
}

// Returns the layout of 4D tensors, or NULL for other tensors. Channels-last tensors
// (dim order 0, 2, 3, 1) are NHWC. The dim order of contiguous tensors doesn't tell
// NCHW and NHWC apart, so they're NCHW if dim 1 is a channel count (1 or 3) and dim
// 3 isn't, else NHWC.
static const char *ml_backend_layout(const TensorInfo &info) {
    Span<const int32_t> sizes = info.sizes();
    Span<const uint8_t> order = info.dim_order();
    if (sizes.size() != 4) {
        return NULL;
    }
    if (order.size() == 4 && order[1] == 2 && order[2] == 3 && order[3] == 1) {
        return "NHWC";
    }
    bool c1 = sizes[1] == 1 || sizes[1] == 3;
    bool c3 = sizes[3] == 1 || sizes[3] == 3;
    return (c1 && !c3) ? "NCHW" : "NHWC";
}

static bool ml_backend_probe(const uint8_t *data, size_t size) {
    // ExecuTorch programs carry the "ET12" flatbuffer identifier at offset 4.
    return size >= 8 && memcmp(data + 4, "ET12", 4) == 0;
}

// Frees all the model's memory. Called by the model's finaliser, including when
// initialization fails part way.
static void ml_backend_deinit_model(py_ml_model_obj_t *model) {
    ml_backend_state_t *state = (ml_backend_state_t *) model->state;
    if (state == NULL) {
        return;
    }

    if (state->loaded) {
        STATE_OBJ(state, Method, method)->~Method();
        state->loaded = false;
    }

    STATE_OBJ(state, ChunkMemoryAllocator, method_allocator)->free_all();

    for (size_t i = 0; i < state->planned_count; i++) {
        if (state->planned_spans[i].data()) {
            uma_free(state->planned_spans[i].data());
        }
    }
    state->planned_count = 0;

    if (state->temp_pool) {
        uma_free(state->temp_pool);
        state->temp_pool = NULL;
    }
}

static int ml_backend_init_model(py_ml_model_obj_t *model) {
    if (et_register_all() != 0) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("torch_backend: kernel registration failed"));
    }

    // Allocate the persistent model state.
    ml_backend_state_t *state = m_new0(ml_backend_state_t, 1);
    model->state = state;

    BufferDataLoader *loader = new(state->loader) BufferDataLoader(model->data, model->size);
    // Verify the whole program, so a corrupted file is rejected instead of read out of bounds.
    Result<Program> program = Program::load(loader, Program::Verification::InternalConsistency);
    ml_backend_check(program.error(), "Program::load");
    new(state->program) Program(std::move(program.get()));

    Result<MethodMeta> meta = STATE_OBJ(state, Program, program)->method_meta(TORCH_METHOD_NAME);
    ml_backend_check(meta.error(), "method_meta");

    // Allocate the memory-planned buffers (activations, planned inputs and outputs).
    // Their sizes are fixed by the program's memory plan.
    size_t planned_count = meta->num_memory_planned_buffers();
    size_t planned_size = 0;
    state->planned_spans = m_new0(Span<uint8_t>, planned_count);
    for (size_t i = 0; i < planned_count; i++) {
        size_t size = meta->memory_planned_buffer_size(i).get();
        state->planned_spans[i] = Span<uint8_t>(ml_backend_alloc(size), size);
        state->planned_count = i + 1;
        planned_size += size;
    }

    // Load the method. The method allocator grows as needed, and the temp allocator
    // is empty until the method's needs are known.
    ChunkMemoryAllocator *method_allocator = new(state->method_allocator) ChunkMemoryAllocator();
    MemoryAllocator *temp_allocator = new(state->temp_allocator) MemoryAllocator(0, nullptr);
    HierarchicalAllocator *planned = new(state->planned_allocator)
                                     HierarchicalAllocator({state->planned_spans, state->planned_count});
    MemoryManager *memory_manager = new(state->memory_manager)
                                    MemoryManager(method_allocator, planned, temp_allocator);

    #ifdef ETHOS_U
    executorch::backends::arm::reset_ethos_u_scratch_size();
    #endif

    Result<Method> method = STATE_OBJ(state, Program, program)->load_method(TORCH_METHOD_NAME, memory_manager);
    ml_backend_check(method.error(), "load_method");
    new(state->method) Method(std::move(method.get()));
    state->loaded = true;

    // The temp allocator is reset after every kernel or delegate call. Of the
    // registered kernels and backends, only the Ethos-U backend uses it, for its
    // scratch buffer; the Cortex-M kernels' scratch buffers are memory-planned.
    size_t temp_size = 0;
    #ifdef ETHOS_U
    temp_size = executorch::backends::arm::get_ethos_u_scratch_size();
    #endif
    if (temp_size) {
        state->temp_pool = ml_backend_alloc(temp_size);
        new(state->temp_allocator) MemoryAllocator(temp_size, state->temp_pool);
    }

    model->memory_addr = state->planned_count ? (uint32_t) state->planned_spans[0].data() : 0;
    model->memory_size = planned_size + temp_size + method_allocator->used_size();

    // Initialize the model's inputs. Inputs are written in place, so they must be
    // memory-planned (the default for exported programs).
    model->inputs_size = meta->num_inputs();
    model->input_shape = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->inputs_size, NULL));
    model->input_scale = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->inputs_size, NULL));
    model->input_zero_point = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->inputs_size, NULL));
    model->input_dtype = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->inputs_size, NULL));
    model->input_layout = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->inputs_size, NULL));

    for (size_t i = 0; i < model->inputs_size; i++) {
        Result<TensorInfo> input = meta->input_tensor_meta(i);
        ml_backend_check(input.error(), "input_tensor_meta");

        if (!input->is_memory_planned()) {
            mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("torch_backend: inputs must be memory-planned"));
        }

        // Quantization parameters are not stored in the program; IO is float or
        // raw integer data.
        model->input_shape->items[i] = ml_backend_shape(input.get());
        model->input_scale->items[i] = mp_obj_new_float(1.0f);
        model->input_zero_point->items[i] = mp_obj_new_int(0);
        char dtype = ml_backend_map_dtype(input->scalar_type());
        if (dtype == 0) {
            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Unsupported input tensor data type %d"),
                              (int) input->scalar_type());
        }
        model->input_dtype->items[i] = mp_obj_new_int(dtype);
        model->input_layout->items[i] = py_ml_layout_obj(ml_backend_layout(input.get()));
    }

    // Initialize the model's outputs.
    model->outputs_size = meta->num_outputs();
    model->output_shape = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->outputs_size, NULL));
    model->output_scale = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->outputs_size, NULL));
    model->output_zero_point = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->outputs_size, NULL));
    model->output_dtype = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->outputs_size, NULL));

    for (size_t i = 0; i < model->outputs_size; i++) {
        Result<TensorInfo> output = meta->output_tensor_meta(i);
        ml_backend_check(output.error(), "output_tensor_meta");

        model->output_shape->items[i] = ml_backend_shape(output.get());
        model->output_scale->items[i] = mp_obj_new_float(1.0f);
        model->output_zero_point->items[i] = mp_obj_new_int(0);
        // Outputs of unsupported data types are returned as None, without blocking the others.
        model->output_dtype->items[i] = mp_obj_new_int(ml_backend_map_dtype(output->scalar_type()));
    }

    return 0;
}

static int ml_backend_run_inference(py_ml_model_obj_t *model) {
    OMV_PROFILER_ENTER(ml_backend_run_inference);

    ml_backend_state_t *state = (ml_backend_state_t *) model->state;
    ml_backend_check(STATE_OBJ(state, Method, method)->execute(), "execute");

    OMV_PROFILER_EXIT(ml_backend_run_inference);
    return 0;
}

static void *ml_backend_get_input(py_ml_model_obj_t *model, size_t index) {
    ml_backend_state_t *state = (ml_backend_state_t *) model->state;
    Method *method = STATE_OBJ(state, Method, method);
    if (index < method->inputs_size()) {
        // mutable_input() is deprecated upstream in favour of set_input(), which
        // copies. The planned input tensor is written in place instead.
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        EValue &input = method->mutable_input(index);
        #pragma GCC diagnostic pop
        return input.toTensor().mutable_data_ptr();
    }
    mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid input tensor index"));
}

static void *ml_backend_get_output(py_ml_model_obj_t *model, size_t index) {
    ml_backend_state_t *state = (ml_backend_state_t *) model->state;
    Method *method = STATE_OBJ(state, Method, method);
    if (index < method->outputs_size()) {
        return const_cast<void *>(method->get_output(index).toTensor().const_data_ptr());
    }
    mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid output tensor index"));
}

const ml_backend_t ml_backend_torch = {
    "torch",
    ml_backend_probe,
    ml_backend_init_model,
    ml_backend_deinit_model,
    ml_backend_run_inference,
    ml_backend_get_input,
    ml_backend_get_output,
};
} // extern "C"
#endif // MICROPY_PY_ML_TORCH
