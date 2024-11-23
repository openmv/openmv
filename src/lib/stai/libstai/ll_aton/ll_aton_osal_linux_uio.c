/**
 ******************************************************************************
 * @file    ll_aton_osal_linux_uio.c
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   ATON LL Linux/UIO platform-specific implementation
 * @note    To be used for Zynq board (ZC104)
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

#if (LL_ATON_OSAL == LL_ATON_OSAL_LINUX_UIO)

#include <assert.h>
#include <fcntl.h>
#include <glob.h>
#include <inttypes.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ll_aton_osal_linux_uio.h"
#include "ll_aton_runtime.h"

//#define DEBUG
#ifdef DEBUG
#define TRACE() printf("%s@%d\n", __FUNCTION__, __LINE__);
#else
#define TRACE()
#endif

#if !defined(DUMP_DMA_STATE_FUNCTION)
#define __DUMP_DMA_STATE() /* Missing DMA state dump function */
#else
extern void DUMP_DMA_STATE_FUNCTION(void);
#define __DUMP_DMA_STATE() DUMP_DMA_STATE_FUNCTION()
#endif

typedef void (*uio_irq_handler_t)(void);

/* Beyond macro should correspond to the greatest ATON interrupt line number used by the runtime plus 1 - i.e. it MUST
 * be within range [1 - 4]) - and MUST be greater than the greatest ATON interrupt line number used by the runtime.
 * Requires that device-tree overlay provides at least `UIO_NUM_OF_DEVICES` interrupt lines. */
#define UIO_NUM_OF_DEVICES  (ATON_STD_IRQ_LINE + 1)
#define UIO_SYSFS_GLOB_PATH "/sys/devices/platform/amba/*.aton_irq*/uio/uio*"

static int uio_fds[UIO_NUM_OF_DEVICES] = {[0 ...(UIO_NUM_OF_DEVICES - 1)] = -1};
static uio_irq_handler_t uio_irq_handlers[UIO_NUM_OF_DEVICES] = {};
static struct pollfd poll_fds[UIO_NUM_OF_DEVICES] = {[0 ...(UIO_NUM_OF_DEVICES - 1)] = {.fd = -1, .events = POLLIN}};

/*** Initialization Functions ***/

void linux_init()
{
  int fd;
  glob_t globbuf = {};
  int ret;
  char *devname;
  char fullpath[128];

  TRACE();
  /* Look for potential uio devices */
  ret = glob(UIO_SYSFS_GLOB_PATH, 0, NULL, &globbuf);
  if (ret == GLOB_NOMATCH)
  {
    fprintf(stderr, "Could not find any aton-related UIO device entry under %s\n", UIO_SYSFS_GLOB_PATH);
    fflush(stdout);
    assert(0);
  }
  else if (ret < 0)
  {
    perror("Could not look at " UIO_SYSFS_GLOB_PATH);
    fflush(stdout);
    assert(0);
  }
  else if (globbuf.gl_pathc < UIO_NUM_OF_DEVICES)
  {
    fprintf(stderr, "Not enough UIO devices available (%ld vs %d)\n", globbuf.gl_pathc, UIO_NUM_OF_DEVICES);
    fflush(stdout);
    assert(0);
  }

  for (int i = 0; i < UIO_NUM_OF_DEVICES; i++)
  {
#ifdef DEBUG
    printf("FOUND: irq%d @ %s\n", i, globbuf.gl_pathv[i]);
#endif
    devname = strrchr(globbuf.gl_pathv[i], '/'); // assumes that `globbuf.gl_pathv` is ordered by irq line number
    if (!devname)
    {
      fprintf(stderr, "Looked at %s and found %s which I don't like\n", UIO_SYSFS_GLOB_PATH, globbuf.gl_pathv[i]);
      fflush(stdout);
      assert(0);
    }
    /* Build the full path */
    snprintf(fullpath, sizeof(fullpath) - 1, "/dev/%s", devname + 1);

#ifdef DEBUG
    printf("OPENING: %s\n", fullpath);
#endif
    fd = open(fullpath, O_RDWR);

    if (fd < 0)
    {
      perror("Error while opening UIO device");
      fflush(stdout);
      assert(0);
    }

    assert(uio_fds[i] == -1);
    assert(poll_fds[i].fd == -1);

    uio_fds[i] = fd;
    poll_fds[i].fd = fd;
  }

  /* Free the memory dynamically allocated by glob() */
  globfree(&globbuf);
}

void linux_uninit()
{
  TRACE();
  for (int i = 0; i < UIO_NUM_OF_DEVICES; i++)
  {
    if (uio_fds[i] >= 0)
    {
      close(uio_fds[i]);
      uio_fds[i] = -1;
      poll_fds[i].fd = -1;
    }
  }
}

void linux_install_irq(int irq_aton_line_nr, void (*handler)(void))
{
  TRACE();
  assert(irq_aton_line_nr < UIO_NUM_OF_DEVICES);
  uio_irq_handlers[irq_aton_line_nr] = handler;
}

/*** Enabling/Disabling Functions ***/

void linux_enable_irq(int irq_aton_line_nr, bool enable)
{
  TRACE();
  uint32_t info = /*htonl*/ ((enable ? 1 /* unmask */ : 0 /* mask */));

  if (irq_aton_line_nr < UIO_NUM_OF_DEVICES)
  {
    int fd = uio_fds[irq_aton_line_nr];
    ssize_t nb = write(fd, &info, sizeof(info));
    if (nb != (ssize_t)sizeof(info))
    {
      perror("write");
      close(fd);
      fflush(stdout);
      assert(0);
    }
  }
}

/*** Wait for interrupts ***/

void linux_wfe()
{
  TRACE();

  int ret = poll(poll_fds, UIO_NUM_OF_DEVICES, ATON_EPOCH_TIMEOUT_MS);
  if (ret == 0)
  {
    printf("timeout while waiting for interrupt\n");
    __DUMP_DMA_STATE();
    fflush(stdout);
    assert(0);
  }
  else if (ret < 0)
  {
    perror("error while waiting for interrupt\n");
    __DUMP_DMA_STATE();
    fflush(stdout);
    assert(0);
  }

  /* Normal case, there are interesting events somewhere */
  for (int i = 0; i < UIO_NUM_OF_DEVICES; i++)
  {
#ifdef DEBUG
    printf("Checking `poll_fds[%d]`\n", i);
#endif
    /* skip those with non-interesting events */
    if ((poll_fds[i].revents & POLLIN) == 0)
      continue;

    uint32_t info = 0;
    int fd = poll_fds[i].fd;
    ssize_t nb = read(fd, &info, sizeof(info));

    /* Treat unexpected return values as errors */
    if (nb != (ssize_t)sizeof(info))
    {
      perror("read()");
      fflush(stdout);
      assert(0);
    }

    /* Do something in response to the interrupt. */
#ifdef DEBUG
    printf("Interrupt Line #%d, Call #%" PRIu32 "!\n", i, info);
#endif
    if (uio_irq_handlers[i])
    {
#ifdef DEBUG
      printf("Calling interrupt handler!\n");
#endif
      uio_irq_handlers[i]();
    }
    /* XXX dirty -- acknowledge it so to trigger it again */
    linux_enable_irq(i, true);
  }
}

#endif // (LL_ATON_OSAL == LL_ATON_OSAL_LINUX_UIO)
