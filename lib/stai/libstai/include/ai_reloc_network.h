/**
 ******************************************************************************
 * @file    ai_reloc_network.h
 * @author  MCD/AIS Team
 * @brief   Relocatable  network support
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019,2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#ifndef __AI_RELOC_NETWORK_H__
#define __AI_RELOC_NETWORK_H__

#include <ai_platform_interface.h>

/* -----------------------------------------------------------------------------
 * AI RELOC definition
 * -----------------------------------------------------------------------------
 */

/*
 * v1.0 : initial version until v7.0 tools
 * v2.0 : update the init fct to support fragmented activations/weights buffer
 */

/* version of the AI RELOC runtime (bootstrap) */
#define AI_RELOC_RT_VERSION_MAJOR 2
#define AI_RELOC_RT_VERSION_MINOR 0

/* AI RT executing mode definitions */
#define AI_RELOC_RT_LOAD_MODE_XIP	  (1 << 0)  /* (default) only the data/bss section are
                                                 copied in RAM, code is executed in-place */
#define AI_RELOC_RT_LOAD_MODE_COPY  (1 << 1)  /* code and data sections are copied in RAM */

/* AI RT error definitions */
#define AI_RELOC_RT_ERR_NONE            (0)
#define AI_RELOC_RT_ERR_INVALID_BIN     (-1)		  /* Invalid binary object */
#define AI_RELOC_RT_ERR_MEMORY          (-2)		  /* RAM size is insufficient */
#define AI_RELOC_RT_ERR_NOT_SUPPORTED   (-3)		  /* feature/option not supported */
#define AI_RELOC_RT_ERR_PARAM           (-4)		  /* param not valid */

/*
 * AI RELOC flags (32b) - part of the binary header
 *
 * 	 b31..b24 : 8b  - RT version major.minor (4b+4b)
 *
 *   b23..b20 : 4b  - fields reserved for post-process script
 *
 *  Variant fields
 *
 * 	 b19..b12 : 8b  - compilation options: ARM tool-chain, FPU, FLOAT-ABI
 * 	                  b19..b16: 4b - ARM tool-chain - 1000b: GNU Arm Embedded Tool-chain
 * 	                  b15:      1b - reserved
 * 	                  b14.b13:  2b - Floating-point ABI used - '00b':soft, '01b':softfp, '10b':hard
 * 	                  b12:      1b - FPU is used
 * 	 b11..b0  : 12b - CPUID (Part Number fields of the @0xE000ED00 CPUID register)
 */

/* CPUID/Cortex-mM definition */
#define AI_RELOC_ARM_CORTEX_M0P  (0xC60UL)
#define AI_RELOC_ARM_CORTEX_M3   (0xC23UL)
#define AI_RELOC_ARM_CORTEX_M4   (0xC24UL)
#define AI_RELOC_ARM_CORTEX_M7   (0xC27UL)
#define AI_RELOC_ARM_CORTEX_M33  (0xD21UL)
#define AI_RELOC_ARM_CORTEX_M55  (0xD22UL)

/* Tool-chain definition (ONLY this tool-chain is currently supported)*/
#define AI_RELOC_TOOLCHAIN_ARM_EMBEDDED    (0x8UL)

/* Floating-point ABI definition (in relation with the tool-chain) */
#define AI_RELOC_TOOLCHAIN_FP_ABI_SOFT    (0x0UL)
#define AI_RELOC_TOOLCHAIN_FP_ABI_SOFTFP  (0x1UL)
#define AI_RELOC_TOOLCHAIN_FP_ABI_HARD    (0x2UL)

/* Getter/setter macros to read */
#define AI_RELOC_RT_SET_FLAGS(_var)  (((AI_RELOC_RT_VERSION_MAJOR << 4 |\
                                        AI_RELOC_RT_VERSION_MINOR << 0) << 24) |\
                                        ((_var) & 0xFFFFF) )

#define AI_RELOC_RT_GET_MAJOR(_flags)           (int)(((_flags) >> 28) & 0xF)
#define AI_RELOC_RT_GET_MINOR(_flags)           (int)(((_flags) >> 24) & 0xF)

#define AI_RELOC_RT_GET_VARIANT(_flags)         ((_flags) & 0xFFFFF)
#define AI_RELOC_RT_GET_POST_OPTIONS(_flags)    ((_flags >> 20) & 0xF)
#define AI_RELOC_RT_GET_CPUID(_flags)           ((_flags >> 0) & 0xFFF)
#define AI_RELOC_RT_GET_COPTS(_flags)           ((_flags >> 12) & 0xF)
#define AI_RELOC_RT_FPU_USED(_flags)            (((_flags) >> 12) & 1)


/* AI RELOC RT context definition */
struct ai_reloc_rt_ctx {
  volatile uint32_t state;              /* current state */
  ai_handle network;                    /* real handle of the network instance */
  uint32_t ram_addr;                    /* loaded base address for the RAM sections */
  uint32_t rom_addr;                    /* loaded base address for the ROM sections */
  uint32_t ram_alloc_addr;              /* base address of the allocated buffer (optional) */
  const char *c_name;                   /* c-name of model */
  const uint32_t act_size;              /* requested RAM size for the activations */
  const uint32_t weights_size;          /* size for the weights */
  ai_observer_exec_ctx obs_ctx;         /* RT low-level context for the observer */
};

#define AI_RELOC_RT_STATE_NOT_INITIALIZED     (0)
#define AI_RELOC_RT_STATE_INITIALIZED         (1 << 0)
#define AI_RELOC_RT_STATE_XIP_MODE            (1 << 1)


#if defined(AI_NETWORK_RELOC)

/* -----------------------------------------------------------------------------
 * This part is only used during the compilation of the network.c to
 * generate the entry points (see linker and relocatable_pp.py files).
 * -----------------------------------------------------------------------------
 */

#if !defined(C_NAME)

#include <network_data.h>

#define C_NAME    network
#define C_NAME_UP NETWORK

#else

#if !defined(C_INC_DATA_FILE)
#error C_INC_DATA_FILE should be defined
#endif

#if !defined(C_NAME_UP)
#error C_NAME_UP should be defined (=str.upper(C_NAME))
#endif

#include C_INC_DATA_FILE

#endif

#if !defined(VARIANT)
/* Default variant definition */
#define VARIANT  ( (AI_RELOC_TOOLCHAIN_ARM_EMBEDDED << 16) | (AI_RELOC_TOOLCHAIN_FP_ABI_HARD << 13) |\
                   (1UL << 12) | AI_RELOC_ARM_CORTEX_M4)
#endif


#if !defined(__GNUC__)
#error "AI_NETWORK_RELOC code generation is only supported with a GCC-based tool-chain"
#endif

#define MAKE_FN_(_x, _e)     ai_ ## _x ## _e
#define MAKE_DEF_(_x, _e)    AI_ ## _x ## _e

#define _DATA_WEIGHTS(name)     MAKE_DEF_(name, _DATA_WEIGHTS)
#define _DATA_ACTIVATIONS(name) MAKE_DEF_(name, _DATA_ACTIVATIONS)

#define _MODEL_NAME(name)       MAKE_DEF_(name, _MODEL_NAME)
#define _ACT_SIZE(name)         MAKE_DEF_(name, _DATA_ACTIVATIONS_SIZE)
#define _WEIGHTS_SIZE(name)     MAKE_DEF_(name, _DATA_WEIGHTS_SIZE)

#define _CREATE(name)   MAKE_FN_(name, _create)
#define _INIT(name)     MAKE_FN_(name, _init)
#define _RUN(name)      MAKE_FN_(name, _run)
#define _REPORT(name)   MAKE_FN_(name, _get_report)
#define _ERROR(name)    MAKE_FN_(name, _get_error)
#define _DESTROY(name)  MAKE_FN_(name, _destroy)
#define _FORWARD(name)  MAKE_FN_(name, _forward)
#define _DATA_PARAMS_GET(name)   MAKE_FN_(name, _data_params_get)

static ai_bool ai_network_init_v2(ai_handle hdl, const ai_handle *weights, const ai_handle *activations)
{
  ai_network_params params;
  ai_bool (*fct_)(ai_network_params* params) = _DATA_PARAMS_GET(C_NAME);
  fct_(&params);

  for (int idx=0; idx < params.map_activations.size; idx++)
    AI_BUFFER_ARRAY_ITEM_SET_ADDRESS(&params.map_activations, idx, activations[idx]);

  for (int idx=0; idx < params.map_weights.size; idx++)
    AI_BUFFER_ARRAY_ITEM_SET_ADDRESS(&params.map_weights, idx, weights[idx]);

  return _INIT(C_NAME)(hdl, &params);
}

/*
 *  Entry table to handle the offset of network entry point and
 *  the RT context.
 */
struct ai_reloc_network_entries {
    ai_error (*create)(ai_handle* network, const ai_buffer* network_config);
    ai_bool (*init)(ai_handle network, const ai_network_params* params);
    ai_bool (*init_v2)(ai_handle network, const ai_handle *weights, const ai_handle *act);
    ai_i32 (*run)(ai_handle network, const ai_buffer* input, ai_buffer* output);
    ai_bool (*report)(ai_handle network, ai_network_report* report);
    ai_error (*error)(ai_handle network);
    ai_handle (*destroy)(ai_handle network);
    ai_i32 (*forward)(ai_handle network, const ai_buffer* input);
    ai_bool (*plt_obs_register_s)(ai_handle network, ai_observer_exec_ctx *ctx);
    ai_bool (*plt_obs_unregister_s)(ai_handle network,  ai_observer_exec_ctx *ctx);
    ai_bool (*plt_obs_node_info)(ai_handle network,  ai_observer_node *node_info);
    struct ai_reloc_rt_ctx *rt_ctx;
};

#define AI_RELOC_NETWORK()\
    static struct ai_reloc_rt_ctx __attribute__((used, section (".network_rt_ctx"), )) _network_rt_ctx = { 0, 0, 0, 0, 0, _MODEL_NAME(C_NAME_UP), _ACT_SIZE(C_NAME_UP), _WEIGHTS_SIZE(C_NAME_UP) }; \
    const struct ai_reloc_network_entries __attribute__((used, section (".network_rt_init"), visibility("default"))) _network_entries = { \
        .create = _CREATE(C_NAME), \
        .init = _INIT(C_NAME), \
        .init_v2 = ai_network_init_v2, \
        .run = _RUN(C_NAME), \
        .report = _REPORT(C_NAME), \
        .error = _ERROR(C_NAME), \
        .destroy = _DESTROY(C_NAME), \
        .forward = _FORWARD(C_NAME), \
        .plt_obs_register_s = ai_platform_observer_register_s, \
        .plt_obs_unregister_s = ai_platform_observer_unregister_s, \
        .plt_obs_node_info = ai_platform_observer_node_info, \
        .rt_ctx = &_network_rt_ctx,\
    }; \
    const uint32_t __attribute__((used, section (".network_rt_flags"), visibility("default"))) _network_flags = AI_RELOC_RT_SET_FLAGS(VARIANT);\


#else

#define AI_RELOC_NETWORK()

#endif


AI_API_DECLARE_BEGIN

typedef struct _ai_rel_network_info {
  const char*   c_name;       /* c-name of the model */
  ai_u32        variant;      /* 32-b word to handle the reloc rt version,
                                 the used ARM Embedded compiler,
                                 Cortex-Mx (CPUID) and if the FPU is requested */
  ai_size       code_sz;      /* size of the code (header + txt + rodata + data + got + rel sections) */
  ai_handle     weights;      /* address of the weights (= @ of the object + offset) */
  ai_size       weights_sz;   /* size (in bytes) of the weights */
  ai_size       acts_sz;      /* minimum requested RAM size (in bytes) for the activations buffer */
  ai_size       rt_ram_xip;   /* minimum requested RAM size to install it, XIP mode */
  ai_size       rt_ram_copy;  /* minimum requested RAM size to install it, COPY mode */
} ai_rel_network_info;


/* -----------------------------------------------------------------------------
 * Public API declaration
 * -----------------------------------------------------------------------------
 */

/*!
 * @brief   utility function to retrieve the dimensioning information
 *          to install the relocatable binary network
 *
 * @param   obj   address of the binary object
 * @rt      rt    a pointer to the ai_rel_network_info struct where to
 *                store info.
 *
 * @return an error type/code pair indicating both the error type and code
 *         see @ref ai_error for struct definition
 */
AI_API_ENTRY
ai_error ai_rel_network_rt_get_info(const void* obj, ai_rel_network_info* rt);

/*!
 * @brief   install and create an instance of the network
 *
 * @param   obj             address of the binary object
 * @param   ram_addr        indicate the address of the RAM to install
 *                          the different sections according the requested mode.
 * @param   ram_size        indicate the size of the provided RAM
 * @param   mode            indicate the expected executing mode
 *                           - AI_RELOC_RT_LOAD_MODE_XIP: code is executed in place
 *                           - AI_RELOC_RT_LOAD_MODE_COPY: code is copied in ram before.
 * @param   hdl             a pointer to a ai_handle object to store the reference
 *                          (opaque object) of the instance.
 *
 * @return an error type/code pair indicating both the error type and code
 *         see @ref ai_error for struct definition
 *
 * Note: If ram_size or ram_addr parameters are null, requested memory buffer
 *       is allocated with the AI_RELOC_MALLOC/AI_RELOC_FREE functions.
 *       _crc_cb can be the NULL pointers to have a default behavior.
 */
AI_API_ENTRY
ai_error ai_rel_network_load_and_create(const void* obj, ai_handle ram_addr,
    ai_size ram_size, uint32_t mode,
    ai_handle* hdl);

/*!
 * @brief   initialize the instance of the network
 *
 * @param   hdl       network handle (@ref ai_rel_network_load_and_create())
 * @param   weights   array of weights buffers
 * @param   act       array of activations buffers
 *
 * @return  false if the handle is invalid, weights and act addresses are
 *          invalid, otherwise network instance is fully initialized.
 *          Note that ai_rel_network_get_error() can be used to have more details
 *          about the error.
 */
AI_API_ENTRY
ai_bool ai_rel_network_init(ai_handle hdl, const ai_handle *weights,
    const ai_handle *act);

/*!
 * @brief   retrieve the network information
 *
 * @param   hdl       network handle (@ref ai_rel_network_load_and_create())
 * @param   report    reference report object to store the informations
 *
 * @return false if the handle is invalid, report is NULL,
 *         otherwise network instance is fully initialized.
 *
 * Note: in case of error the error type could be queried by
 *       using @ref ai_rel_network_get_error
 */
AI_API_ENTRY
ai_bool ai_rel_network_get_report(ai_handle hdl, ai_network_report* report);

/*!
 * @brief   return the last error
 *
 * @param   hdl       network handle (@ref ai_rel_network_load_and_create())
 *
 * @return an error type/code pair indicating both the error type and code
 *         see @ref ai_error for struct definition
 */
AI_API_ENTRY
ai_error ai_rel_network_get_error(ai_handle hdl);

/*!
 * @brief   run the network and return the predicted output
 *
 * @param   hdl       network handle (@ref ai_rel_network_load_and_create())
 * @param
 *
 * @return number of input batches processed (default 1) or <= 0 if it fails
 *
 * Note: in case of error the error type could be queried by
 *       using @ref ai_rel_network_get_error
 */
AI_API_ENTRY
ai_i32 ai_rel_network_run(ai_handle hdl, const ai_buffer* input, ai_buffer* output);


/*!
 * @brief   un-install and destroy the instantiated network
 *
 * @param   hdl       network handle (@ref ai_rel_network_load_and_create())
 *
 * @return AI_HANDLE_NULL if network was destroyed correctly
 */
AI_API_ENTRY
ai_handle ai_rel_network_destroy(ai_handle hdl);

/*!
 * @brief   register an observer context. Allows to register a client CB which
 *          will be called before or/and after the execution of a c-node with
 *          the references of the used tensors (see @ref ai_observer_node).
 *
 * @param   hdl       network handle (@ref ai_rel_network_load_and_create())
 * @param   cb        reference of the user callback function
 * @param   cookie    reference of a user object/ctx
 * @param   flags     indicate expected events (see AI_OBSERVER_XX_EVT flag definition)
 *
 * @return false if the registration has failed (network error is updated)
 *         else true (network error is updated).
 */
AI_API_ENTRY
ai_bool ai_rel_platform_observer_register(ai_handle hdl,
    ai_observer_node_cb cb, ai_handle cookie, ai_u32 flags);

/*!
 * @brief   un-register the observer context.
 *
 * @param   hdl       network handle (@ref ai_rel_network_load_and_create())
 * @param   cb        reference of the user callback function
 * @param   cookie    reference of a user object/ctx
 *
 * @return false if the un-registration has failed (network error is updated)
 *         else true
 *
 * Note: in case of error the error type could be queried by
 *       using @ref ai_rel_network_get_error
 */
AI_API_ENTRY
ai_bool ai_rel_platform_observer_unregister(ai_handle hdl,
    ai_observer_node_cb cb, ai_handle cookie);


/*!
 * @brief   return the info of a requested c-node (defined by the
 *          c_idx field). Should be called after the initialization phase.
 *
 * @param   hdl       network handle (@ref ai_rel_network_load_and_create())
 * @param   node_info a pointer to a reference of the node description
 *
 * @return true if the node_info->c_idx designates a valid index else
 *         false
 *
 * Note: in case of error the error type could be queried by
 *       using @ref ai_rel_network_get_error
 */
AI_API_ENTRY
ai_bool ai_rel_platform_observer_node_info(ai_handle hdl,
    ai_observer_node *node_info);

AI_API_DECLARE_END

#endif
