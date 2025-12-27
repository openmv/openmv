/**
 ******************************************************************************
 * @file    ll_aton_dbgtrc.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Header file of ATON Debug and Trace unit LL module driver.
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

#ifndef __LL_ATON_DBGTRC_H
#define __LL_ATON_DBGTRC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/* Monitored signals offsets list. TODO: move them in compatibility layer */
#define DBGTRC_GND               0
#define DBGTRC_VDD               1
#define SWITCH_OSTRX_STALL       2
#define DBGTRC_SWITCH_OSTRX_HENV 56
#define SWITCH_ISTRX_STALL       83
#define DBGTRC_SWITCH_ISTRX_HENV 165
#define EXT_TRIGGERS_SYNC        216
#define DBGTRC_INTCTRL_SIGNALS   220
#define BUS_INTERFACE_READ1X8B   276
#define BUS_INTERFACE_READ2X8B   278
#define BUS_INTERFACE_READ4X8B   280
#define BUS_INTERFACE_READ8X8B   282
#define BUS_INTERFACE_WRITE1X8B  284
#define BUS_INTERFACE_WRITE2X8B  286
#define BUS_INTERFACE_WRITE4X8B  288
#define BUS_INTERFACE_WRITE8X8B  290

/* Event type descriptors */
#define DBGTRC_EVT_LOW     0
#define DBGTRC_EVT_HI      1
#define DBGTRC_EVT_POSEDGE 2
#define DBGTRC_EVT_NEGEDGE 3

/* Get the cardinal number of a given destination port.
 * To be used, for example, with DBGTRC_SWITCH_ISTRX_HENV signals
 */
#define GET_STRSWTCH_DEST(X) ((X - ATON_STRSWITCH_DSTSTRENG0_OFFSET) >> 2)

  /**
   * @brief Configuration structure for Debug and Trace unit triggers
   */
  typedef struct
  {
    unsigned int signal;
    unsigned char evt_type;
    unsigned int filter;
  } LL_Dbgtrc_Trigger_InitTypedef;

  /**
   * @brief Configuration structure for Debug and Trace unit counters
   */
  typedef struct
  {
    unsigned int signal;       /* Signal to monitor */
    unsigned char evt_type;    /* Event type (low, hi, posedge, negedge) */
    unsigned char wrap;        /* Wrap counter */
    unsigned char countdown;   /* Count-up/down. Interrupt can be fired on countdown to zero */
    unsigned char int_disable; /* Mask interrupt */
    uint32_t counter;          /* Counter value */
  } LL_Dbgtrc_Counter_InitTypdef;

  /**
   * @brief Configuration structure for Debug and Trace unit tracers
   */
  typedef struct
  {
    LL_Dbgtrc_Trigger_InitTypedef triggerconf; /* Trigger configuration */
    unsigned char verbosity;                   /* Tracer verbosity */
    unsigned int token;                        /* Optional user token */
  } LL_Dbgtrc_Tracer_InitTypedef;

  /**
   * @brief Configuration structure for a 'stopwatch'
   * To be used with LL_Dbgtrc_Counter_StopWatch function
   */
  typedef struct
  {
    unsigned char mainCounter;  /* ID of the main counter */
    unsigned char startCounter; /* ID of the start event counter */
    unsigned int startSignal;   /* Signal triggering the start event */
    unsigned char startEvent;   /* Event type of the start condition */
    unsigned char stopCounter;  /* ID of the stop event counter */
    unsigned int stopSignal;    /* Signal triggering the stop event */
    unsigned char stopEvent;    /* Event type of the stop condition */
  } LL_Dbgtrc_StopWatch_InitTypdef;

  /* Global */
  int LL_Dbgtrc_Init(int id);
  int LL_Dbgtrc_Deinit(int id);
  void LL_Dbgtrc_EnableClock(void);
  void LL_Dbgtrc_DisableClock(void);

  uint64_t LL_Dbgtrc_Read_Stalls(int id, unsigned char iolink);
  int LL_Dbgtrc_Read_IStall(int id, unsigned int signal);
  int LL_Dbgtrc_Read_OStall(int id, unsigned int signal);

  /* Triggers */
  int LL_Dbgtrc_Trigger_Init(int id, int trigger, LL_Dbgtrc_Trigger_InitTypedef *Trigger_InitStruct);
  int LL_Dbgtrc_Trigger_Swtrig(int id, int trigger);
  int LL_Dbgtrc_Trigger_GetOVR(int id, int trigger);

  /* Counters management */
  int LL_Dbgtrc_Counter_Init(int id, int counter, LL_Dbgtrc_Counter_InitTypdef *Counter_InitStruct);
  int LL_Dbgtrc_Counter_Start(int id, int counter);
  int LL_Dbgtrc_Counter_Stop(int id, int counter);
  uint32_t LL_Dbgtrc_Counter_Read(int id, int counter);
  int LL_Dbgtrc_Counter_BindStart(int id, int srccounter, int dstcounter);
  int LL_Dbgtrc_Counter_BindStop(int id, int srccounter, int dstcounter);
  int LL_Dbgtrc_Counter_StopWatch(int id, LL_Dbgtrc_StopWatch_InitTypdef *StopWatch_InitStruct);

  /* Tracers management */
  int LL_Dbgtrc_Tracer_Init(int id, int tracer, LL_Dbgtrc_Tracer_InitTypedef *Tracer_InitStruct);
  int LL_Dbgtrc_Tracer_Bind(int id, int streng, uint8_t *stream_addr, size_t n);
  int LL_Dbgtrc_Tracer_Start(int id, int tracer);
  int LL_Dbgtrc_Tracer_Stop(int id, int tracer);
  int LL_Dbgtrc_Tracer_Enable_Watchdog(int id, unsigned int period);
  int LL_Dbgtrc_Tracer_Disable_Watchdog(int id);

  /* Examples */

  /* Counts IRQs emitted by a given unit */
  int LL_Dbgtrc_Count_IRQs(unsigned int counter, unsigned int unitirq);

  /* Compute Epoch Length by monitoring two stream engines */
  int LL_Dbgtrc_Count_Epoch_Len(unsigned int counter, unsigned int istreng, unsigned int ostreng);

  /* Counts stall/active signal periods */
  int LL_Dbgtrc_Count_Stalls(unsigned int counter, unsigned char iostall, unsigned int signal, unsigned int hilow);

  /* Burst Lengths computations */
  int LL_Dbgtrc_Count_BurstsLen(unsigned int counter, unsigned char busif, unsigned char readwrite);
  int LL_Dbgtrc_BurstLenBenchStart(unsigned int counter);
  int LL_Dbgtrc_BurstLenGet(unsigned int counter, unsigned int *counters);
  int LL_Dbgtrc_GetTotalTranfers(unsigned int counter_id, unsigned int *totalWrites, unsigned int *totalReads);
  int LL_Dbgtrc_LogTransfers(unsigned int counter);
  int LL_Dbgtrc_LogTransfers_epoch(unsigned int counter_id, unsigned int *counters, int first);

  /* Count external trigger events. Useful, for example, to detect if a camera has emitted sync signal */
  int LL_Dbgtrc_Count_ExtTrigger(unsigned int counter, unsigned char trigger);

  /* Monitor Stream Engines' active times. Useful to detect epochs' stall time ratios */
  int LL_Dbgtrc_Count_StrengActive_Config(uint32_t istreng, uint32_t ostreng, unsigned int counter);
  int LL_Dbgtrc_Count_StrengActive_Start(uint32_t istreng, uint32_t ostreng, unsigned int counter);
  int LL_Dbgtrc_Count_StrengActive_Stop(uint32_t istreng, uint32_t ostreng, unsigned int counter);
  unsigned int LL_Dbgtrc_Count_StrengActive_GetMAX(uint32_t istreng, uint32_t ostreng, unsigned int counter,
                                                   unsigned int *argmax);
  int LL_Dbgtrc_Count_StrengActive_Print(uint32_t istreng, uint32_t ostreng, unsigned int counter);

  /* Monitor input Stream Engines' HENV signal */
  int LL_Dbgtrc_Count_StrengHENV_Config(uint32_t istreng, unsigned int counter);
  int LL_Dbgtrc_Count_StrengHENV_Start(uint32_t istreng, unsigned int counter);
  int LL_Dbgtrc_Count_StrengHENV_Stop(uint32_t istreng, unsigned int counter);
  int LL_Dbgtrc_Count_StrengHENV_Print(uint32_t istreng, unsigned int counter);

  int LL_Dbgtrc_SynchronousCountersTest(void);

#ifdef __cplusplus
}
#endif

#endif
