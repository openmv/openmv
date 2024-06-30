/**
 * @file services_lib_linux.h
 *
 * @brief Services library public API header file
 * @defgroup host_services host_services
 * @par
 *
 * COPYRIGHT NOTICE: (c) 2022 Alif Group. All rights reserved.
 */

#ifndef __SERVICES_LIB_LINUX_H__
#define __SERVICES_LIB_LINUX_H__

/******************************************************************************
 *  I N C L U D E   F I L E S
 *****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include "services_lib_api.h"

/*******************************************************************************
 *  M A C R O   D E F I N E S
 ******************************************************************************/


/*******************************************************************************
 *  T Y P E D E F S
 ******************************************************************************/

// Initialization to be done by host
/**
 * @struct services_lib_t
 */
typedef struct {
	uint32_t             packet_buffer_address;
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

#define SERVICES_LIB_ERROR   0xFFFFFFFFul

#ifdef __cplusplus
}
#endif
#endif /* __SERVICES_LIB_LINUX_H__ */
