/**
 ******************************************************************************
 * @file    ll_aton_osal_linux_bw.c
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   ATON LL Linux/Bittware platform-specific implementation
 * @note    To be used for Bittware PCI FPGA boards
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

#include "ll_aton_config.h"

#if (LL_ATON_OSAL == LL_ATON_OSAL_LINUX_BW)

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "hil.h"

#include "ll_aton_osal_linux_bw.h"
#include "ll_aton_runtime.h"
#include "ll_aton_util.h"

// Define beyond macro if you do NOT want to init/open/use the Bittware device
// (e.g. when executing under the control of a debugger)
// #define BW_DONT_USE
#define BW_DEVICE 0

#ifdef RASTA_SVN_REV
#define BW_NUM_OF_DEVICES (ATON_INTCTRL_VERSION_HOSTINTS_DT(0))
#else
#define BW_NUM_OF_DEVICES (ATON_INTCTRL_VERSION_HOSTINTS_DT)
#endif

HHil hil;
HDevice dev;
typedef void (*bw_irq_handler_t)(void);
static bw_irq_handler_t bw_irq_handlers[BW_NUM_OF_DEVICES] = {};

#if !defined(DUMP_DMA_STATE_FUNCTION)
#define __DUMP_DMA_STATE() /* Missing DMA state dump function */
#else
extern void DUMP_DMA_STATE_FUNCTION(void);
#define __DUMP_DMA_STATE() DUMP_DMA_STATE_FUNCTION()
#endif

#if (LL_ATON_RT_MODE == LL_ATON_RT_ASYNC)
pthread_mutex_t cs_lock;
sem_t wfe_sem;
#endif // (LL_ATON_RT_MODE == LL_ATON_RT_ASYNC)

/* Bittware interrupt handler */
void interrupt_handler_msi(unsigned int msi, void *param)
{
  if (bw_irq_handlers[msi])
  {
#if (LL_ATON_RT_MODE == LL_ATON_RT_ASYNC)
    bw_enter_cs();
#endif

    bw_irq_handlers[msi]();

#if (LL_ATON_RT_MODE == LL_ATON_RT_ASYNC)
    bw_exit_cs();
#endif
  }
}

void sighandler(int param)
{
  printf("Caught Signal %i, cleaning up\n", param);
  bw_uninit();
  exit(128 + param);
}

int bw_init(void)
{
#ifndef BW_DONT_USE
  int s;
  int ret;

#if (LL_ATON_RT_MODE == LL_ATON_RT_ASYNC)
  ret = pthread_mutex_init(&cs_lock, NULL);
  if (ret != 0)
  {
    fprintf(stderr, "Failed creating CS Mutex: %s\n", strerror(ret));
    exit(ret);
  }

  ret = sem_init(&wfe_sem, 0, 0);
  if (ret < 0)
  {
    fprintf(stderr, "Failed creating WFE semaphore: %s\n", strerror(errno));
    exit(errno);
  }
#endif // (LL_ATON_RT_MODE == LL_ATON_RT_ASYNC)

  if ((hil = hil_init(HILINIT_NO_OPTION)) == NULL)
  {
    printf("Error: could not open Bittware system handle\n");
    return 1;
  }

  /* Open the device */
  if ((dev = hil_open(hil, BW_DEVICE, HILOPEN_NO_OPTION)) == NULL)
  {
    fprintf(stderr, "Problem opening Bittware device %d\n", BW_DEVICE);
    hil_exit(hil);
    exit(-1);
  }
  else
  {
    printf("Opened successfully Bittware device\n");
  }

  /* Install Bittware interrupt handler */
  ret = hil_interrupt_enable_msi(dev, interrupt_handler_msi, NULL, 0xf, 0);
  if (ret < 0)
  {
    fprintf(stderr, "Error enabling interrupt: %s\n", hil_get_error_string(ret));
    hil_close(dev);
    hil_exit(hil);
    exit(ret);
  }
  else
  {
    printf("Bittware interrupt handler installed\n");
  }

  /* Intercept all signals so to release irq resources in case of segfaults and the like.
   * This should be done in bwpcidrv driver 'close' method but it's not.. */
  for (s = 0; s < SIGRTMAX; s++)
    signal(s, sighandler);

#endif // BW_DONT_USE

  return 0;
}

int bw_uninit(void)
{
  int ret = 0;

#ifndef BW_DONT_USE

#if (LL_ATON_RT_MODE == LL_ATON_RT_ASYNC)
  pthread_mutex_destroy(&cs_lock);
  sem_destroy(&wfe_sem);
#endif // (LL_ATON_RT_MODE == LL_ATON_RT_ASYNC)

  ret = hil_interrupt_disable(dev);
  if (ret < 0)
  {
    fprintf(stderr, "Error disabling interrupt: %s\n", hil_get_error_string(ret));
  }

  if (dev)
  {
    ret = hil_close(dev);
    if (ret < 0)
    {
      fprintf(stderr, "Error closing device: %s\n", hil_get_error_string(ret));
    }
  }

  if (hil)
  {
    ret = hil_exit(hil);
    if (ret < 0)
    {
      fprintf(stderr, "Error exiting HIL: %s\n", hil_get_error_string(ret));
    }
  }

#endif // BW_DONT_USE

  return ret;
}

int bw_install_irq(int irq_aton_line_nr, void (*handler)(void))
{
  bw_irq_handlers[irq_aton_line_nr] = handler;
  return 0;
}

int bw_uninstall_irq(int irq_aton_line_nr)
{
  bw_irq_handlers[irq_aton_line_nr] = NULL;
  return 0;
}

int bw_wfe(void)
{
#if (LL_ATON_RT_MODE == LL_ATON_RT_POLLING)
  return sched_yield();
#else
  struct timespec timeout;

  if (clock_gettime(CLOCK_REALTIME, &timeout) == -1)
  {
    printf("\n==> Failed waiting for interrupt\n");
    printf("==> clock_gettime: %s\n\n", strerror(errno));
    fflush(stdout);
    assert(0);
  }
  timeout.tv_sec += ATON_EPOCH_TIMEOUT_MS / 1000;

  int ret = sem_timedwait(&wfe_sem, &timeout);
  if (ret != 0)
  {
    printf("\n==> Failed waiting for interrupt\n");
    printf("==> sem_timedwait: %s\n\n", strerror(errno));
    __DUMP_DMA_STATE();
    fflush(stdout);
    assert(0);
  }

  return ret;
#endif
}

#if (LL_ATON_RT_MODE == LL_ATON_RT_ASYNC)
int bw_enter_cs(void)
{
  return pthread_mutex_lock(&cs_lock);
}

int bw_exit_cs(void)
{
  return pthread_mutex_unlock(&cs_lock);
}

int bw_post_event(void)
{
  return sem_post(&wfe_sem);
}
#endif // (LL_ATON_RT_MODE == LL_ATON_RT_ASYNC)

#endif // (LL_ATON_OSAL == LL_ATON_OSAL_LINUX_BW)
