/**
 ******************************************************************************
 * @file    ll_aton_platform.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Header file for defining platform dependencies
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#ifndef __LL_ATON_PLATFORM_H
#define __LL_ATON_PLATFORM_H

#include <stdint.h>

#include "ll_aton_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Default timeout is 10s, but can be overridden */
#ifndef ATON_EPOCH_TIMEOUT_MS
#define ATON_EPOCH_TIMEOUT_MS 10000
#endif

/*** Platform dependent definitions & includes ***/

/* Bare metal Cortex-M NCSIM simulator platform*/
#if (LL_ATON_PLATFORM == LL_ATON_PLAT_NCSIM)
#define ATON_PLAT_HAS_FFLUSH  (0)
#define ATON_BASE             0xA0000000
#define SYSMEM1_BASE          0xA0080000
#define SYSMEM2_BASE          0xA00A0000
#define SYSMEM3_BASE          0xA00C0000
#define SYSMEM4_BASE          0xA00E0000
#define SYSMEM_LENGTH         0x00020000
#define DUMMY_MEM_BASE        0x40000000
#define DUMP_CONFIG_BASE_ADDR 0xA0108000
#define CDMA_BASE_ADDR        0xA0100000
#define OSPI2_BASE_ADDR       0x3D710000
#define OSPI1_BASE_ADDR       0x3D700000
#define OSPIIOM_BASE_ADDR     0x3D720000
#define OSPI_HSEL_SEL         0x3D83100C
#define OSPI1_MEM             0x60000000
#define OSPI2_MEM             0x80000000
#define ATON_EPOCH_TIMEOUT    (ATON_EPOCH_TIMEOUT_MS * 1000)
#include "cm4ikmcu.h"

/* Neuromem simulation environment */
#elif (LL_ATON_PLATFORM == LL_ATON_PLAT_NEUROMEM_SIM)
#define ATON_PLAT_HAS_FFLUSH  (0)
#define ATON_BASE             0xA0000000
#define SYSMEM1_BASE          0xA0080000
#define SYSMEM2_BASE          0xA00A0000
#define SYSMEM3_BASE          0xA00C0000
#define SYSMEM4_BASE          0xA00E0000
#define SYSMEM_LENGTH         0x00020000
#define DUMP_CONFIG_BASE_ADDR 0x3D807c00
#define CDMA_BASE_ADDR        0xA0100000
#define OSPI2_BASE_ADDR       0xA0150000
#define OSPI1_BASE_ADDR       0xA0160000
#define OSPI1_MEM             0x00000000
#define OSPI2_MEM             0x80000000
#define ATON_EPOCH_TIMEOUT    (ATON_EPOCH_TIMEOUT_MS * 1000)

#define ATON_INT_NR                       (2 * 6 + 2 + 2) // 6 streng, 6 streng err, 2 convacc, 2 busif

/* These macros are not available in Neuromem ATON.h so define them here */
#define ATON_STRENG_INT_MASK(A, B, C)     (0x003f)
#define ATON_STRENG_ERR_INT_MASK(A, B, C) (0x0fc0)
#define ATON_CONVACC_INT_MASK(A, B, C)    (0x3000)
#define ATON_BUSIF_INT_MASK(A, B, C)      (0xc000)

#ifndef ATON_STRENG_NUM
#define ATON_STRENG_NUM  6
#define ATON_CONVACC_NUM 2
#define ATON_DECUN_NUM   1
#define ATON_ACTIV_NUM   1
#define ATON_ARITH_NUM   1
#define ATON_POOL_NUM    1
#define ATON_BUSIF_NUM   2
#endif

#ifndef ATON_STRENG_CTRL_DT
#define ATON_STRENG_CTRL_DT ATON_STRENG_CTRL_RESET
#endif
#ifndef ATON_DECUN_CTRL_DT
#define ATON_DECUN_CTRL_DT ATON_DECUN_CTRL_RESET
#endif
#ifndef ATON_CONVACC_CTRL_DT
#define ATON_CONVACC_CTRL_DT ATON_CONVACC_CTRL_RESET
#endif
#ifndef ATON_ACTIV_CTRL_DT
#define ATON_ACTIV_CTRL_DT ATON_ACTIV_CTRL_RESET
#endif
#ifndef ATON_ARITH_CTRL_DT
#define ATON_ARITH_CTRL_DT ATON_ARITH_CTRL_RESET
#endif
#ifndef ATON_POOL_CTRL_DT
#define ATON_POOL_CTRL_DT ATON_POOL_CTRL_RESET
#endif
#ifndef ATON_ACTIV_FUNC_DT
#define ATON_ACTIV_FUNC_DT ATON_ACTIV_FUNC_RESET
#endif
#ifndef ATON_ACTIV_ACTIVPARAM_DT
#define ATON_ACTIV_ACTIVPARAM_DT ATON_ACTIV_ACTIVPARAM_RESET
#endif
#ifndef ATON_ACTIV_ACTIVPARAM2_DT
#define ATON_ACTIV_ACTIVPARAM2_DT ATON_ACTIV_ACTIVPARAM2_RESET
#endif

#include "cm4ikmcu.h"

/* Imaging simulation environment */
#elif (LL_ATON_PLATFORM == LL_ATON_PLAT_IMAGING_SIM)
#define ATON_PLAT_HAS_FFLUSH  (0)
#define ATON_BASE             0xA0000000
#define SYSMEM1_BASE          0xB0000000
#define SYSMEM2_BASE          0xB0040000
#define SYSMEM3_BASE          0xB0080000
#define SYSMEM_LENGTH         0x00040000
#define DUMMY_MEM_BASE        0x40000000
#define DUMP_CONFIG_BASE_ADDR 0xA0108000
#define CDMA_BASE_ADDR        0xA0100000
#define OSPI2_BASE_ADDR       0x3D710000
#define OSPI1_BASE_ADDR       0x3D700000
#define OSPIIOM_BASE_ADDR     0x3D720000
#define OSPI_HSEL_SEL         0x3D83100C
#define OSPI1_MEM             0x60000000
#define OSPI2_MEM             0x80000000
#define ATON_EPOCH_TIMEOUT    (ATON_EPOCH_TIMEOUT_MS * 1000)
#include "cm4ikmcu.h"

#define ATON_STRENG_CID_CACHE_SET_ALLOC(a, b)     0
#define ATON_STRENG_CID_CACHE_SET_CACHEABLE(a, b) 0
#define ATON_STRENG_CID_CACHE_SET_CID(a, b)       0
#define ATON_STRENG_CID_CACHE_SET_LINESIZE(a, b)  0
#define ATON_STRENG_CID_CACHE_SET_PFETCH(a, b)    0

/* Bare metal Cortex-M STICE4 FPGA platform */
#elif (LL_ATON_PLATFORM == LL_ATON_PLAT_STICE4)
#define ATON_PLAT_HAS_FFLUSH (1)
#define ATON_BASE            0x51100000

typedef int32_t IRQn_Type;

#include "stm32h7_map.h"

#include <core_cm7.h>

#define CDNN0_IRQn         (123)
#define ATON_EPOCH_TIMEOUT (ATON_EPOCH_TIMEOUT_MS * 1000)

/* Linux based Xilinx ZC706 FPGA platform */
#elif (LL_ATON_PLATFORM == LL_ATON_PLAT_ZC706) || (LL_ATON_PLATFORM == LL_ATON_PLAT_IWAVE) ||                          \
    (LL_ATON_PLATFORM == LL_ATON_PLAT_BITTWARE)
#define ATON_PLAT_HAS_FFLUSH (1)
extern uint8_t *get_zynq_aton_base(void);
#define NVIC_EnableIRQ(x)
#define NVIC_DisableIRQ(x)
#define ATON_BASE          (get_zynq_aton_base())
/* Timer clock is 100MHz. Compute timeout accordingly. */
#define ATON_EPOCH_TIMEOUT (ATON_EPOCH_TIMEOUT_MS * 100 * 1000)

/* PC based Orlando Simulator platform */
#elif (LL_ATON_PLATFORM == LL_ATON_PLAT_SWEMUL)
#define ATON_PLAT_HAS_FFLUSH (1)

#define __WFE()
#define NVIC_EnableIRQ(x)
#define NVIC_DisableIRQ(x)
#define ATON_EPOCH_TIMEOUT (ATON_EPOCH_TIMEOUT_MS * 1000)

/* Bare metal Cortex-M TLM simulator platform as used by MCD */
#elif (LL_ATON_PLATFORM == LL_ATON_PLAT_TLM_MCD)
#define ATON_PLAT_HAS_FFLUSH (1)

#include "cnn1_top_mapping.h"

#define ATON_BASE             CNN1_TOP_BASEADDR
#define SYSMEM1_BASE          AXI_MEM0
#define SYSMEM2_BASE          AXI_MEM1
#define SYSMEM3_BASE          AXI_MEM2
#define SYSMEM4_BASE          AXI_MEM3
#define SYSMEM_LENGTH         0x00020000
#define DUMMY_MEM_BASE        AXI_FLASH
#define DUMP_CONFIG_BASE_ADDR ATON_BASE + 0x108000
#define CDMA_BASE_ADDR        ATON_BASE + 0x100000

#define __WFE() esw_sleep()
#define __DSB()
#define NVIC_EnableIRQ(x)
#define NVIC_DisableIRQ(x)
#define ATON_EPOCH_TIMEOUT (ATON_EPOCH_TIMEOUT_MS * 1000)

/* PC based Test Explorer application platform */
#elif (LL_ATON_PLATFORM == LL_ATON_PLAT_TSTXPL)
#define ATON_PLAT_HAS_FFLUSH (1)

#define __WFE() (complete_dmas())
#define __DSB()
#define NVIC_EnableIRQ(x)
#define NVIC_DisableIRQ(x)
uintptr_t get_aton_base(void);
void complete_dmas(void);
#define ATON_BASE          (get_aton_base())
#define ATON_EPOCH_TIMEOUT (ATON_EPOCH_TIMEOUT_MS * 1000)

#elif (LL_ATON_PLATFORM == LL_ATON_PLAT_EC_TRACE)
#include "ll_aton_ec_trace.h"

#define ATON_PLAT_HAS_FFLUSH (1)

#define ATON_BASE (get_ec_aton_base())

#define __WFE()
#define __DSB()
#define NVIC_EnableIRQ(x)
#define NVIC_DisableIRQ(x)
#define ATON_EPOCH_TIMEOUT (ATON_EPOCH_TIMEOUT_MS * 1000)

#define ATON_REG_WRITE_RELOC(regaddr, base, offset)                                                                    \
  ec_trace_write_reloc((uintptr_t)regaddr, (uint32_t)base, (uint32_t)offset)
#define ATON_REG_WRITE(regaddr, val) ec_trace_write((uintptr_t)regaddr, (uint32_t)val)
#define ATON_REG_WRITE_FIELD_RANGE(unitname, id, reg, start, fsize, val)                                               \
  ec_trace_reg_writefield(ec_trace_get_IP_id(ATON_##unitname##_BASE(id)),                                              \
                          ec_trace_get_REG_id(ATON_##unitname##_##reg##_OFFSET), start, fsize, val)
#define ATON_REG_WRITE_FIELD(unitname, id, reg, field, val)                                                            \
  ATON_REG_WRITE_FIELD_RANGE(unitname, id, reg, ATON_##unitname##_##reg##_##field##_LSB,                               \
                             ATON_##unitname##_##reg##_##field##_W, val)
#define ATON_REG_POLL(unitname, id, reg, field, val)                                                                   \
  ec_trace_reg_poll(ec_trace_get_IP_id(ATON_##unitname##_BASE(id)),                                                    \
                    ec_trace_get_REG_id(ATON_##unitname##_##reg##_OFFSET), ATON_##unitname##_##reg##_##field##_LSB,    \
                    ATON_##unitname##_##reg##_##field##_W, (uint32_t)val)

#elif (LL_ATON_PLATFORM == LL_ATON_PLAT_CENTAURI)
#define ATON_BASE             0xA0000000
#define SYSMEM1_BASE          0xA0080000
#define SYSMEM2_BASE          0xA00A0000
#define SYSMEM3_BASE          0xA00C0000
#define SYSMEM4_BASE          0xA00E0000
#define SYSMEM_LENGTH         0x00020000
#define DUMMY_MEM_BASE        0x40000000
#define DUMP_CONFIG_BASE_ADDR 0xA0108000
#define CDMA_BASE_ADDR        0xA0100000
#define OSPI2_BASE_ADDR       0x3D710000
#define OSPI1_BASE_ADDR       0x3D700000
#define OSPIIOM_BASE_ADDR     0x3D720000
#define OSPI_HSEL_SEL         0x3D83100C
#define OSPI1_MEM             0x60000000
#define OSPI2_MEM             0x80000000
#define ATON_EPOCH_TIMEOUT    (ATON_EPOCH_TIMEOUT_MS * 1000)

//#define ATON_INT_NR     (2 * 16 + 6 + 4) /* 16 streng, 6 convacc, 4 busif */
#define ATON_INT_NR           32 //>32 not supported yet
#if (LL_ATON_RT_MODE == LL_ATON_RT_ASYNC)
#error Only supported `LL_ATON_RT_MODE==LL_ATON_RT_POLLING`
#endif
#include "cm4ikmcu.h"

#elif (LL_ATON_PLATFORM == LL_ATON_PLAT_N64)
#define ATON_PLAT_HAS_FFLUSH                      (1)
#define ATON_BASE                                 0x3d83d000
#define ATON_EPOCH_TIMEOUT                        (ATON_EPOCH_TIMEOUT_MS * 1000)
#define ATON_STRENG_CID_CACHE_SET_ALLOC(a, b)     0
#define ATON_STRENG_CID_CACHE_SET_CACHEABLE(a, b) 0
#define ATON_STRENG_CID_CACHE_SET_CID(a, b)       0
#define ATON_STRENG_CID_CACHE_SET_LINESIZE(a, b)  0
#define ATON_STRENG_CID_CACHE_SET_PFETCH(a, b)    0

#elif (LL_ATON_PLATFORM == LL_ATON_PLAT_STM32N6)
// Cache maintenance
#include "mcu_cache.h"
#include "npu_cache.h"

#include "stm32n6xx.h"

#define ATON_N6_DRIVERS_v0200                                                                                          \
  ((0x0 << 24U) | /* MAIN */                                                                                           \
   (0x2 << 16) |  /* SUB1 */                                                                                           \
   (0x0 << 8) |   /* SUB2 */                                                                                           \
   (0x0)          /* RC */                                                                                             \
  )
#define ATON_GET_VERSION_RC(x)          ((x)&0xFF)
#define ATON_GET_VERSION_FIRST_THREE(x) ((x) >> 8)
#define ATON_IS_AT_LEAST_VERSION(x, v)                                                                                 \
  (((ATON_GET_VERSION_RC(x) == 0x0) && (ATON_GET_VERSION_FIRST_THREE(x) >= ATON_GET_VERSION_FIRST_THREE(v))) ||        \
   (((ATON_GET_VERSION_RC(x) != 0x0) && (ATON_GET_VERSION_RC(v) == 0x0)) &&                                            \
    (ATON_GET_VERSION_FIRST_THREE(x) > ATON_GET_VERSION_FIRST_THREE(v))) ||                                            \
   (((ATON_GET_VERSION_RC(x) != 0x0) && (ATON_GET_VERSION_RC(v) != 0x0)) && ((x) >= (v))))

#if !ATON_IS_AT_LEAST_VERSION(__STM32N6xx_HAL_VERSION, ATON_N6_DRIVERS_v0200)
#define CDNN0_IRQHandler NPU_END_OF_EPOCH_IRQHandler
#define CDNN1_IRQHandler NPU_INT1_IRQHandler
#define CDNN2_IRQHandler NPU_INT2_IRQHandler
#define CDNN3_IRQHandler NPU_INT3_IRQHandler

#define CDNN0_IRQn NPU_END_OF_EPOCH_IRQn
#define CDNN1_IRQn NPU_INT1_IRQn
#define CDNN2_IRQn NPU_INT2_IRQn
#define CDNN3_IRQn NPU_INT3_IRQn
#else // ATON_IS_AT_LEAST_VERSION
#define CDNN0_IRQHandler NPU0_IRQHandler
#define CDNN1_IRQHandler NPU1_IRQHandler
#define CDNN2_IRQHandler NPU2_IRQHandler
#define CDNN3_IRQHandler NPU3_IRQHandler

#define CDNN0_IRQn NPU0_IRQn
#define CDNN1_IRQn NPU1_IRQn
#define CDNN2_IRQn NPU2_IRQn
#define CDNN3_IRQn NPU3_IRQn
#endif // ATON_IS_AT_LEAST_VERSION

#define ATON_PLAT_HAS_FFLUSH (0)
#if defined(CPU_IN_SECURE_STATE)
#define ATON_BASE NPU_BASE_S
#else
#define ATON_BASE NPU_BASE_NS
#endif
#define ATON_EPOCH_TIMEOUT (ATON_EPOCH_TIMEOUT_MS * 1000)

#elif (LL_ATON_PLATFORM == LL_ATON_PLAT_STM32H7P)
/* Cache maintenance */
//#include "mcu_cache.h" /* TODO */
//#include "npu_cache.h" /* TODO */
#include "stm32h7p4xx.h"

#define CDNN0_IRQHandler NPU_IT0_IRQHandler
#define CDNN1_IRQHandler NPU_IT1_IRQHandler
#define CDNN2_IRQHandler NPU_IT2_IRQHandler
#define CDNN3_IRQHandler NPU_IT2_IRQHandler /* Only three interrupts here */

#define CDNN0_IRQn NPU_IT0_IRQn
#define CDNN1_IRQn NPU_IT1_IRQn
#define CDNN2_IRQn NPU_IT2_IRQn
#define CDNN3_IRQn NPU_IT2_IRQn
#define ATON_BASE  0x520E0000 /* Not yet defined in HAL */

#define ATON_PLAT_HAS_FFLUSH (0)
#define ATON_EPOCH_TIMEOUT   (ATON_EPOCH_TIMEOUT_MS * 1000)

/* Stellar P3 support: TODO when information is available */
#elif (LL_ATON_PLATFORM == LL_ATON_PLAT_STELLARP3)
/* Cache maintenance */
//#include "mcu_cache.h" /* TODO */
//#include "npu_cache.h" /* TODO */
//#include "stellarp3.h"

#define CDNN0_IRQHandler NPU_IT0_IRQHandler
#define CDNN1_IRQHandler NPU_IT1_IRQHandler
#define CDNN2_IRQHandler NPU_IT2_IRQHandler
#define CDNN3_IRQHandler NPU_IT3_IRQHandler

#define CDNN0_IRQn 506 /* To R52 Kite Cluster */
#define CDNN1_IRQn 507 /* To R52 Kite Cluster */
#define CDNN2_IRQn 92  /* To Cortex-M4 */
#define CDNN3_IRQn 93  /* To Cortex-M4 */
#define ATON_BASE  0x75600000

#define ATON_PLAT_HAS_FFLUSH (0)
#define ATON_EPOCH_TIMEOUT   (ATON_EPOCH_TIMEOUT_MS * 1000)

#else
#error No target platform is specified. Please define macro `LL_ATON_PLATFORM`
#endif

// may be included just here because of the platform dependent definitions (above all `ATON_BASE`)
#include "ATON.h"
#include "ll_aton_rcompat.h"

/* Default macro for physical to virtual address translation (direct mapping) */
#ifndef ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR
#define ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR(address) (address)
#endif // !ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR

/* Default macro for virtual to physical address translation (direct mapping) */
#ifndef ATON_LIB_VIRTUAL_TO_PHYSICAL_ADDR
#define ATON_LIB_VIRTUAL_TO_PHYSICAL_ADDR(address) (address)
#endif // !ATON_LIB_VIRTUAL_TO_PHYSICAL_ADDR

// converts signed shift (negative value are left shift) to HW encoding Range 39:0. No shift 16
#ifndef ATON_SHIFT
#define ATON_SHIFT(x) ((x) + 16)
#endif // ATON_SHIFT

// Generic boilerplate for enabling an ATON unit
#define ATON_ENABLE(unitname, id) ATON_REG_WRITE_FIELD(unitname, id, CTRL, EN, 1)

// Generic boilerplate for disabling, stopping pending transactions (aka "clearing"), and clearing the configuration of
// an ATON unit
#ifndef ATON_DISABLE_CLR_CONFCLR
#define ATON_DISABLE_CLR_CONFCLR(unitname, id)                                                                         \
  do                                                                                                                   \
  {                                                                                                                    \
    t = ATON_##unitname##_CTRL_DT;                                                                                     \
    t = ATON_##unitname##_CTRL_SET_EN(t, 0);                                                                           \
    t = ATON_##unitname##_CTRL_SET_CLR(t, 1);                                                                          \
    ATON_##unitname##_CTRL_SET(id, t);                                                                                 \
    /* wait for unit to terminate clearing of configuration registers */                                               \
    ATON_REG_POLL(unitname, id, CTRL, CLR, 0);                                                                         \
                                                                                                                       \
    t = ATON_##unitname##_CTRL_DT;                                                                                     \
    t = ATON_##unitname##_CTRL_SET_CONFCLR(t, 1);                                                                      \
    ATON_##unitname##_CTRL_SET(id, t);                                                                                 \
    /* wait for unit to terminate clearing of configuration registers */                                               \
    ATON_REG_POLL(unitname, id, CTRL, CONFCLR, 0);                                                                     \
  } while (0)
#endif // ATON_DISABLE_CLR_CONFCLR

#ifndef ATON_REG_WRITE_RELOC
#define ATON_REG_WRITE_RELOC(regaddr, base, offset)                                                                    \
  do                                                                                                                   \
  {                                                                                                                    \
    *(volatile uint32_t *)(uintptr_t)(regaddr) = (base) + (offset);                                                    \
  } while (0)
#endif // ATON_REG_WRITE_RELOC

#ifndef ATON_REG_WRITE
#define ATON_REG_WRITE(regaddr, val)                                                                                   \
  do                                                                                                                   \
  {                                                                                                                    \
    *(volatile uint32_t *)(uintptr_t)(regaddr) = (val);                                                                \
  } while (0)
#endif // ATON_REG_WRITE

#ifndef ATON_REG_WRITE_FIELD_RANGE
#define ATON_REG_WRITE_FIELD_RANGE(unitname, id, reg, start, fsize, val)                                               \
  do                                                                                                                   \
  {                                                                                                                    \
    uint32_t t = ATON_##unitname##_##reg##_GET(id);                                                                    \
    t = ATON_SET_FIELD(t, start, fsize, val);                                                                          \
    ATON_##unitname##_##reg##_SET(id, t);                                                                              \
  } while (0)
#endif // ATON_REG_WRITE_FIELD_RANGE

#ifndef ATON_REG_WRITE_FIELD
#define ATON_REG_WRITE_FIELD(unitname, id, reg, field, val)                                                            \
  do                                                                                                                   \
  {                                                                                                                    \
    uint32_t t = ATON_##unitname##_##reg##_GET(id);                                                                    \
    t = ATON_##unitname##_##reg##_SET_##field(t, val);                                                                 \
    ATON_##unitname##_##reg##_SET(id, t);                                                                              \
  } while (0)
#endif // ATON_REG_WRITE_FIELD

#ifndef ATON_REG_POLL
#define ATON_REG_POLL(unitname, id, reg, field, val)                                                                   \
  do                                                                                                                   \
  {                                                                                                                    \
    while (ATON_##unitname##_##reg##_GET_##field(ATON_##unitname##_##reg##_GET(id)) != val)                            \
    {                                                                                                                  \
    }                                                                                                                  \
  } while (0)
#endif // ATON_REG_POLL

/* Interrupt line(s) to use */
/* ATON_STD_IRQ_LINE MUST be within the range of available ATON interrupt lines, i.e. within [0 - 3] */
#ifndef ATON_STD_IRQ_LINE
#define ATON_STD_IRQ_LINE 0
#endif

#if (ATON_STD_IRQ_LINE != 0) && (ATON_STD_IRQ_LINE != 1) && (ATON_STD_IRQ_LINE != 2) && (ATON_STD_IRQ_LINE != 3)
#error invalid interrupt line number `ATON_STD_IRQ_LINE` (must be in range [0 - 3])
#endif // ATON_STD_IRQ_LINE

// Beyond macros may be defined just here
#define ATON_STD_IRQHandler                  LL_ATON_CONCAT3(CDNN, ATON_STD_IRQ_LINE, _IRQHandler)
#define ATON_STD_IRQn                        LL_ATON_CONCAT3(CDNN, ATON_STD_IRQ_LINE, _IRQn)
#define ATON_STD_INTANDMSK                   LL_ATON_CONCAT(INTANDMSK, ATON_STD_IRQ_LINE)
#define ATON_STD_INTORMSK                    LL_ATON_CONCAT(INTORMSK, ATON_STD_IRQ_LINE)
#define ATON_INTCTRL_STD_INTANDMSK_SET(DATA) ATON_INTCTRL_INTANDMSK_SET(0, ATON_STD_IRQ_LINE, DATA)
#define ATON_INTCTRL_STD_INTANDMSK_GET       ATON_INTCTRL_INTANDMSK_GET(0, ATON_STD_IRQ_LINE)
#define ATON_INTCTRL_STD_INTORMSK_SET(DATA)  ATON_INTCTRL_INTORMSK_SET(0, ATON_STD_IRQ_LINE, DATA)
#define ATON_INTCTRL_STD_INTORMSK_GET        ATON_INTCTRL_INTORMSK_GET(0, ATON_STD_IRQ_LINE)
#if (ATON_INT_NR > 32)
#define ATON_INTCTRL_STD_INTORMSK_H_SET(DATA)  ATON_INTCTRL_INTORMSK_H_SET(0, ATON_STD_IRQ_LINE, DATA)
#define ATON_INTCTRL_STD_INTANDMSK_H_SET(DATA) ATON_INTCTRL_INTANDMSK_H_SET(0, ATON_STD_IRQ_LINE, DATA)
#endif // (ATON_INT_NR > 32)

#ifdef __cplusplus
}
#endif

#endif // __LL_ATON_PLATFORM_H
