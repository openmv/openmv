/**
 * @file services_lib_bare_metal.h
 *
 * @brief Services library public API header file
 * @defgroup host_services host_services
 * @par
 *
 * Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
#ifndef __SERVICES_LIB_BARE_METAL_H__
#define __SERVICES_LIB_BARE_METAL_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *  I N C L U D E   F I L E S
 *****************************************************************************/
#include "mhu.h"
#include "services_lib_api.h"

/*******************************************************************************
 *  M A C R O   D E F I N E S
 ******************************************************************************/

/*******************************************************************************
 *  T Y P E D E F S
 ******************************************************************************/

/**
 * @struct services_lib_t
 */
typedef struct {
	uint32_t packet_buffer_address;
	MHU_send_message_t   fn_send_mhu_message;
	wait_ms_t            fn_wait_ms;
	uint32_t             wait_timeout;
	print_msg_t          fn_print_msg;
} services_lib_t;

/*******************************************************************************
 *  G L O B A L   D E F I N E S
 ******************************************************************************/

/*******************************************************************************
 *  F U N C T I O N   P R O T O T Y P E S
 ******************************************************************************/

// Services infrastructure APIs
void SERVICES_initialize(services_lib_t *init_params);
int  SERVICES_synchronize_with_se(uint32_t services_handle);
void SERVICES_send_msg_acked_callback(uint32_t sender_id, uint32_t channel_number);
void SERVICES_rx_msg_callback(uint32_t receiver_id, uint32_t channel_number,
			      uint32_t data);
#ifdef __cplusplus
}
#endif
#endif /* __SERVICES_LIB_BARE_METAL_H__ */
