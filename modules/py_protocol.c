/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2025 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Protocol Python module - Provides bindings for OpenMV protocol system.
 * Allows creating custom channels in Python that integrate with the C protocol.
 */
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#if MICROPY_PY_PROTOCOL

#include "py/obj.h"
#include "py/runtime.h"
#include "py/objstr.h"
#include "py/objint.h"

#include "../protocol/omv_protocol.h"
#include "shared/runtime/softtimer.h"

/***************************************************************************
* Python Channel Delegates
***************************************************************************/

// Helper function to check if method exists on Python object
static bool py_channel_has_method(mp_obj_t obj, qstr method_name) {
    mp_obj_t dest[2];
    mp_load_method_maybe(obj, method_name, dest);
    return dest[0] != MP_OBJ_NULL;
}

// Helper function to safely call Python method with exception handling
static mp_obj_t py_channel_call(mp_obj_t obj, qstr method_name, size_t n_args, const mp_obj_t *args) {
    mp_obj_t dest[2 + n_args];
    mp_load_method_maybe(obj, method_name, dest);

    if (dest[0] == MP_OBJ_NULL) {
        return MP_OBJ_NULL; // Method not found
    }

    // Copy arguments to dest array after method and self
    for (size_t i = 0; i < n_args; i++) {
        dest[i + 2] = args[i];
    }

    // Call the method with exception handling
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_obj_t result = mp_call_method_n_kw(n_args, 0, dest);
        nlr_pop();
        return result;
    } else {
        // Exception occurred
        return MP_OBJ_NULL;
    }
}

// Delegate function implementations
static int py_channel_init(const omv_protocol_channel_t *channel) {
    mp_obj_t obj = MP_OBJ_FROM_PTR(channel->priv);
    mp_obj_t result = py_channel_call(obj, MP_QSTR_init, 0, NULL);
    return result == MP_OBJ_NULL ? -1 : 0;
}

static bool py_channel_poll(const omv_protocol_channel_t *channel) {
    mp_obj_t obj = MP_OBJ_FROM_PTR(channel->priv);
    mp_obj_t result = py_channel_call(obj, MP_QSTR_poll, 0, NULL);
    return result == MP_OBJ_NULL ? false : mp_obj_is_true(result);
}

static int py_channel_lock(const omv_protocol_channel_t *channel) {
    mp_obj_t obj = MP_OBJ_FROM_PTR(channel->priv);
    mp_obj_t result = py_channel_call(obj, MP_QSTR_lock, 0, NULL);
    return result == MP_OBJ_NULL ? -1 : (mp_obj_is_true(result) ? 0 : -1);
}

static int py_channel_unlock(const omv_protocol_channel_t *channel) {
    mp_obj_t obj = MP_OBJ_FROM_PTR(channel->priv);
    mp_obj_t result = py_channel_call(obj, MP_QSTR_unlock, 0, NULL);
    return result == MP_OBJ_NULL ? -1 : (mp_obj_is_true(result) ? 0 : -1);
}

static size_t py_channel_size(const omv_protocol_channel_t *channel) {
    mp_obj_t obj = MP_OBJ_FROM_PTR(channel->priv);
    mp_obj_t result = py_channel_call(obj, MP_QSTR_size, 0, NULL);
    return result == MP_OBJ_NULL ? 0 : mp_obj_get_int(result);
}

static size_t py_channel_shape(const omv_protocol_channel_t *channel, size_t shape[4]) {
    mp_obj_t obj = MP_OBJ_FROM_PTR(channel->priv);
    mp_obj_t result = py_channel_call(obj, MP_QSTR_shape, 0, NULL);

    // Expect a tuple/list of 4 elements
    if (result != MP_OBJ_NULL && mp_obj_is_type(result, &mp_type_tuple)) {
        size_t len;
        mp_obj_t *items;
        mp_obj_get_array(result, &len, &items);

        for (size_t i = 0; i < 4 && i < len; i++) {
            shape[i] = mp_obj_get_int(items[i]);
        }
        return len;
    }

    return 0;
}

static int py_channel_flush(const omv_protocol_channel_t *channel) {
    mp_obj_t obj = MP_OBJ_FROM_PTR(channel->priv);
    mp_obj_t result = py_channel_call(obj, MP_QSTR_flush, 0, NULL);
    return result == MP_OBJ_NULL ? -1 : 0;
}

// Read delegate - calls Python read method and copies data to C buffer
static int py_channel_read(const omv_protocol_channel_t *channel, uint32_t offset, size_t size, void *data) {
    mp_obj_t obj = MP_OBJ_FROM_PTR(channel->priv);

    // Prepare arguments: offset, size
    mp_obj_t args[2] = {
        mp_obj_new_int(offset),
        mp_obj_new_int(size)
    };

    // Call the Python read method
    mp_obj_t result = py_channel_call(obj, MP_QSTR_read, 2, args);

    // Get buffer from Python result (should be bytes/bytearray)
    mp_buffer_info_t bufinfo;
    if (result != MP_OBJ_NULL && mp_get_buffer(result, &bufinfo, MP_BUFFER_READ)) {
        size_t copy_size = (bufinfo.len < size) ? bufinfo.len : size;
        memcpy(data, bufinfo.buf, copy_size);
        return copy_size; // Return number of bytes read
    }

    return -1; // Failed to get buffer
}

// Write delegate - passes C buffer to Python as bytearray
static int py_channel_write(const omv_protocol_channel_t *channel, uint32_t offset, size_t size, const void *data) {
    mp_obj_t obj = MP_OBJ_FROM_PTR(channel->priv);

    // Prepare arguments: offset, data (as bytearray)
    mp_obj_t args[2] = {
        mp_obj_new_int(offset),
        mp_obj_new_bytearray_by_ref(size, (void *) data)
    };

    // Call the Python write method
    mp_obj_t result = py_channel_call(obj, MP_QSTR_write, 2, args);

    if (result == MP_OBJ_NULL) {
        return -1; // Method not found or exception
    } else if (mp_obj_is_int(result)) {
        return mp_obj_get_int(result);
    }

    return 0; // Default success
}

// Readp delegate - calls Python readp method and returns pointer (dangerous but needed)
static const void *py_channel_readp(const omv_protocol_channel_t *channel, uint32_t offset, size_t size) {
    mp_obj_t obj = MP_OBJ_FROM_PTR(channel->priv);

    // Prepare arguments: offset, size
    mp_obj_t args[2] = {
        mp_obj_new_int(offset),
        mp_obj_new_int(size)
    };

    // Call the Python readp method
    mp_obj_t result = py_channel_call(obj, MP_QSTR_readp, 2, args);

    // Get buffer pointer directly from Python object
    mp_buffer_info_t bufinfo;
    if (result != MP_OBJ_NULL && mp_get_buffer(result, &bufinfo, MP_BUFFER_READ)) {
        return bufinfo.buf; // Return direct pointer - DANGEROUS but needed for readp
    }

    return NULL;
}

// Ioctl delegate - passes command and data to Python
static int py_channel_ioctl(const omv_protocol_channel_t *channel, uint32_t cmd, size_t len, void *arg) {
    mp_obj_t obj = MP_OBJ_FROM_PTR(channel->priv);

    // Prepare arguments: command, length, arg (as bytearray if data present)
    mp_obj_t args[3] = {
        mp_obj_new_int(cmd),
        mp_obj_new_int(len),
        (arg == NULL || len == 0) ? mp_const_none : mp_obj_new_bytearray_by_ref(len, arg)
    };

    // Call the Python ioctl method
    mp_obj_t result = py_channel_call(obj, MP_QSTR_ioctl, 3, args);

    if (result == MP_OBJ_NULL) {
        return -1; // Method not found or exception
    } else if (result == mp_const_none) {
        return 0; // Success
    } else if (mp_obj_is_int(result)) {
        return mp_obj_get_int(result);
    }

    return 0; // Default success
}

// Active check delegate for transport channels
static bool py_channel_is_active(const omv_protocol_channel_t *channel) {
    mp_obj_t obj = MP_OBJ_FROM_PTR(channel->priv);

    // Call the Python is_active method
    mp_obj_t result = py_channel_call(obj, MP_QSTR_is_active, 0, NULL);

    return result != MP_OBJ_NULL && mp_obj_is_true(result);
}


/***************************************************************************
* Protocol Channel Object
***************************************************************************/

// Protocol Channel handle object type
typedef struct _py_channel_obj_t {
    mp_obj_base_t base;
    omv_protocol_channel_t *channel;
} py_channel_obj_t;

static mp_obj_t py_channel_deinit(mp_obj_t self_in) {
    py_channel_obj_t *self = self_in;
    (void) self;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_channel_deinit_obj, py_channel_deinit);

static void py_channel_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    py_channel_obj_t *self = self_in;
    mp_printf(print, "<ProtocolChannel: %s>", self->channel->name);
}

static mp_obj_t py_channel_send_event(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_event, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_wait_ack, MP_ARG_BOOL, {.u_bool = false} },
    };

    py_channel_obj_t *self = pos_args[0];
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    uint32_t event = args[0].u_int;
    bool wait_ack = args[1].u_bool;

    if (omv_protocol_send_event(self->channel->id, event, wait_ack)) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to send channel event"));
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_channel_send_event_obj, 2, py_channel_send_event);

static const mp_rom_map_elem_t py_channel_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_ProtocolChannel) },
    { MP_ROM_QSTR(MP_QSTR___del__),  MP_ROM_PTR(&py_channel_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_send_event), MP_ROM_PTR(&py_channel_send_event_obj) },
};
static MP_DEFINE_CONST_DICT(py_channel_locals_dict, py_channel_locals_dict_table);

// Protocol Channel type definition
MP_DEFINE_CONST_OBJ_TYPE(
    py_channel_type,
    MP_QSTR_ProtocolChannel,
    MP_TYPE_FLAG_NONE,
    print, py_channel_print,
    locals_dict, &py_channel_locals_dict
    );

/***************************************************************************
* Protocol Module
***************************************************************************/
static void py_protocol_task(mp_sched_node_t *node) {
    omv_protocol_task();
}

static void py_protocol_schedule(void) {
    static mp_sched_node_t protocol_task_node;
    mp_sched_schedule_node(&protocol_task_node, py_protocol_task);
}

// Soft timer callback to call py_protocol_poll
static void py_protocol_soft_timer_callback(soft_timer_entry_t *self) {
    return py_protocol_schedule();
}

// Protocol active check
static mp_obj_t py_protocol_is_active(void) {
    return mp_obj_new_bool(omv_protocol_is_active());
}
static MP_DEFINE_CONST_FUN_OBJ_0(py_protocol_is_active_obj, py_protocol_is_active);

// Schedule protocol task function
static mp_obj_t py_protocol_poll(void) {
    py_protocol_schedule();
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(py_protocol_poll_obj, py_protocol_poll);

// Protocol init function
static mp_obj_t py_protocol_init(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_crc, MP_ARG_BOOL, {.u_bool = true} },
        { MP_QSTR_seq, MP_ARG_BOOL, {.u_bool = true} },
        { MP_QSTR_ack, MP_ARG_BOOL, {.u_bool = false} },
        { MP_QSTR_events, MP_ARG_BOOL, {.u_bool = true} },
        { MP_QSTR_max_payload, MP_ARG_INT, {.u_int = OMV_PROTOCOL_MAX_PAYLOAD_SIZE} },
        { MP_QSTR_soft_reboot, MP_ARG_BOOL, {.u_bool = false} },
        { MP_QSTR_rtx_retries, MP_ARG_INT, {.u_int = OMV_PROTOCOL_DEF_RTX_RETRIES} },
        { MP_QSTR_rtx_timeout_ms, MP_ARG_INT, {.u_int = OMV_PROTOCOL_DEF_RTX_TIMEOUT_MS} },
        { MP_QSTR_lock_interval_ms, MP_ARG_INT, {.u_int = OMV_PROTOCOL_MIN_LOCK_INTERVAL_MS} },
        { MP_QSTR_timer_ms, MP_ARG_INT, {.u_int = 0} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // Build configuration structure from arguments
    omv_protocol_config_t config = {
        .crc_enabled = args[0].u_bool,
        .seq_enabled = args[1].u_bool,
        .ack_enabled = args[2].u_bool,
        .event_enabled = args[3].u_bool,
        .max_payload = args[4].u_int,
        .soft_reboot = args[5].u_bool,
        .rtx_retries = args[6].u_int,
        .rtx_timeout_ms = args[7].u_int,
        .lock_intval_ms = args[8].u_int,
    };

    uint32_t timer_ms = args[9].u_int;

    if (omv_protocol_init(&config)) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to initialize protocol"));
    }

    // Register the default logical data channels
    omv_protocol_register_channel(&omv_stdin_channel);
    omv_protocol_register_channel(&omv_stdout_channel);
    omv_protocol_register_channel(&omv_stream_channel);

    // Register the profiler channel (if enabled)
    #if OMV_PROFILER_ENABLE
    omv_protocol_register_channel(&omv_profile_channel);
    #endif // OMV_PROFILER_ENABLE

    // Start periodic timer if timer_ms > 0
    if (timer_ms > 0) {
        // Soft timer for periodic protocol task scheduling
        static soft_timer_entry_t protocol_soft_timer;

        soft_timer_static_init(&protocol_soft_timer, SOFT_TIMER_MODE_PERIODIC,
                               timer_ms, py_protocol_soft_timer_callback);
        soft_timer_insert(&protocol_soft_timer, timer_ms);
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_protocol_init_obj, 0, py_protocol_init);

// Register channel function - dynamically allocates C channel for Python object
static mp_obj_t py_protocol_register(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_name, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_flags, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_backend, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_obj_t backend = args[2].u_obj;
    const char *channel_name = mp_obj_str_get_str(args[0].u_obj);

    // Allocate C channel structure
    omv_protocol_channel_t *channel = m_malloc0(sizeof(omv_protocol_channel_t));

    // Initialize C channel
    channel->priv = MP_OBJ_TO_PTR(backend);
    channel->flags = args[1].u_int | OMV_PROTOCOL_CHANNEL_FLAG_DYNAMIC;
    strncpy(channel->name, channel_name, OMV_PROTOCOL_CHANNEL_NAME_SIZE);
    channel->name[OMV_PROTOCOL_CHANNEL_NAME_SIZE - 1] = '\0';

    // Set delegate functions only if methods exist on the Python object
    channel->init = py_channel_has_method(backend, MP_QSTR_init) ? py_channel_init : NULL;
    channel->poll = py_channel_has_method(backend, MP_QSTR_poll) ? py_channel_poll : NULL;
    channel->lock = py_channel_has_method(backend, MP_QSTR_lock) ? py_channel_lock : NULL;
    channel->unlock = py_channel_has_method(backend, MP_QSTR_unlock) ? py_channel_unlock : NULL;
    channel->size = py_channel_has_method(backend, MP_QSTR_size) ? py_channel_size : NULL;
    channel->shape = py_channel_has_method(backend, MP_QSTR_shape) ? py_channel_shape : NULL;
    channel->read = py_channel_has_method(backend, MP_QSTR_read) ? py_channel_read : NULL;
    channel->write = py_channel_has_method(backend, MP_QSTR_write) ? py_channel_write : NULL;
    channel->readp = py_channel_has_method(backend, MP_QSTR_readp) ? py_channel_readp : NULL;
    channel->flush = py_channel_has_method(backend, MP_QSTR_flush) ? py_channel_flush : NULL;
    channel->ioctl = py_channel_has_method(backend, MP_QSTR_ioctl) ? py_channel_ioctl : NULL;
    channel->is_active = py_channel_has_method(backend, MP_QSTR_is_active) ? py_channel_is_active : NULL;

    // Automatically set flags based on available methods
    if (channel->read || channel->readp) {
        channel->flags |= OMV_PROTOCOL_CHANNEL_FLAG_READ;
    }
    if (channel->write) {
        channel->flags |= OMV_PROTOCOL_CHANNEL_FLAG_WRITE;
    }
    if (channel->lock && channel->unlock) {
        channel->flags |= OMV_PROTOCOL_CHANNEL_FLAG_LOCK;
    }

    // Register with protocol system
    if (omv_protocol_register_channel(channel) == -1) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to register protocol channel"));
    }

    // Create Python channel handle object
    py_channel_obj_t *channel_obj = mp_obj_malloc_with_finaliser(py_channel_obj_t, &py_channel_type);
    channel_obj->channel = channel;

    // Store dynamic channels in root pointers to prevent GC
    if (OMV_PROTOCOL_CHANNEL_FLAG_GET(channel, DYNAMIC)) {
        MP_STATE_PORT(protocol_channels)[channel->id] = MP_OBJ_FROM_PTR(channel_obj);
    }

    return MP_OBJ_FROM_PTR(channel_obj);
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_protocol_register_obj, 0, py_protocol_register);

// Module globals table
static const mp_rom_map_elem_t protocol_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_protocol) },

    // Protocol management functions
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&py_protocol_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_is_active), MP_ROM_PTR(&py_protocol_is_active_obj) },
    { MP_ROM_QSTR(MP_QSTR_poll), MP_ROM_PTR(&py_protocol_poll_obj) },
    { MP_ROM_QSTR(MP_QSTR_register), MP_ROM_PTR(&py_protocol_register_obj) },

    // Channel flags constants
    { MP_ROM_QSTR(MP_QSTR_CHANNEL_FLAG_READ), MP_ROM_INT(OMV_PROTOCOL_CHANNEL_FLAG_READ) },
    { MP_ROM_QSTR(MP_QSTR_CHANNEL_FLAG_WRITE), MP_ROM_INT(OMV_PROTOCOL_CHANNEL_FLAG_WRITE) },
    { MP_ROM_QSTR(MP_QSTR_CHANNEL_FLAG_LOCK), MP_ROM_INT(OMV_PROTOCOL_CHANNEL_FLAG_LOCK) },
    { MP_ROM_QSTR(MP_QSTR_CHANNEL_FLAG_PHYSICAL), MP_ROM_INT(OMV_PROTOCOL_CHANNEL_FLAG_PHYSICAL) },

    // Protocol channel IDs
    { MP_ROM_QSTR(MP_QSTR_CHANNEL_ID_TRANSPORT), MP_ROM_INT(OMV_PROTOCOL_CHANNEL_ID_TRANSPORT) },
    { MP_ROM_QSTR(MP_QSTR_CHANNEL_ID_STDIN), MP_ROM_INT(OMV_PROTOCOL_CHANNEL_ID_STDIN) },
    { MP_ROM_QSTR(MP_QSTR_CHANNEL_ID_STDOUT), MP_ROM_INT(OMV_PROTOCOL_CHANNEL_ID_STDOUT) },
    { MP_ROM_QSTR(MP_QSTR_CHANNEL_ID_STREAM), MP_ROM_INT(OMV_PROTOCOL_CHANNEL_ID_STREAM) },
    { MP_ROM_QSTR(MP_QSTR_CHANNEL_ID_PROFILE), MP_ROM_INT(OMV_PROTOCOL_CHANNEL_ID_PROFILE) },
};

static MP_DEFINE_CONST_DICT(protocol_globals, protocol_globals_table);

// Module object
const mp_obj_module_t protocol_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *) &protocol_globals,
};

// Register the module
MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_protocol, protocol_module);

// Register root pointer for dynamic protocol channels
MP_REGISTER_ROOT_POINTER(mp_obj_t protocol_channels[OMV_PROTOCOL_MAX_CHANNELS]);

#endif // MICROPY_PY_PROTOCOL
