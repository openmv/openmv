/**
 ******************************************************************************
 * @file    ll_aton_dbgtrc.c
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   ATON Debug and Trace unit LL module driver.
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

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "ll_aton.h"
#include "ll_aton_dbgtrc.h"
#include "ll_aton_util.h"

#if defined(ATON_DEBUG_TRACE_NUM)

/**
 * @brief Initializes the Debug and Trace unit
 * @param id Id of the target debug and trace unit to be initialized
 */
int LL_Dbgtrc_Init(int id)
{
  uint32_t t = ATON_DEBUG_TRACE_CTRL_DT(id);

  /* Enable clock and unit */
  LL_ATON_EnableClock(ATON_DEBUG_TRACE_CLKB_CLK(id));

  /* Start tracers, if available */
#ifdef ATON_DEBUG_TRACE_DEBUG_TRACE_N_TRACERS
#if ATON_DEBUG_TRACE_DEBUG_TRACE_N_TRACERS(0) > 0
  /* Start now */
  ATON_DEBUG_TRACE_START_LSB_SET(id, 0x0);
  ATON_DEBUG_TRACE_START_MSB_SET(id, 0x0);

  /* Stop never */
  ATON_DEBUG_TRACE_STOP_LSB_SET(id, 0xffffffff);
  ATON_DEBUG_TRACE_STOP_MSB_SET(id, 0xffffffff);

  /* Use timestamps as start/stop conditions */
  t = ATON_DEBUG_TRACE_CTRL_SET_TRACE_START_EVTYPE(t, 1);
  t = ATON_DEBUG_TRACE_CTRL_SET_TRACE_STOP_EVTYPE(t, 1);
#endif
#endif

  t = ATON_DEBUG_TRACE_CTRL_SET_EN(t, 1);

  ATON_DEBUG_TRACE_CTRL_SET(id, t);

  return 0;
}

/**
 * @brief Deinitializes the Debug and Trace unit
 * @param id Id of the target debug and trace unit to be deinitialized
 */
int LL_Dbgtrc_Deinit(int id)
{
  /* Disable unit and gate its clock */
  ATON_DEBUG_TRACE_CTRL_SET(id, 0);

  LL_ATON_DisableClock(ATON_DEBUG_TRACE_CLKB_CLK(id));

  return 0;
}

/**
 * @brief Enables Debug and Trace Unit clock via Clock Controller unit
 * @note This is used as method/workaround to start counters synchronously
 * @todo Fix for Puma design
 */
void LL_Dbgtrc_EnableClock(void)
{
  uint32_t t = ATON_CLKCTRL_BGATES_GET(0);
  t |= (1 << ATON_DEBUG_TRACE_CLKB_CLK(0));
  ATON_CLKCTRL_BGATES_SET(0, t);
}

/**
 * @brief Disables Debug and Trace Unit clock via Clock Controller unit
 * @note This is used as method/workaround to stop counters synchronously
 * @todo Fix for Puma design
 */
void LL_Dbgtrc_DisableClock(void)
{
  uint32_t t = ATON_CLKCTRL_BGATES_GET(0);
  t &= ~(1 << ATON_DEBUG_TRACE_CLKB_CLK(0));
  ATON_CLKCTRL_BGATES_SET(0, t);
}

/**
 * @brief Returns the status of current stall signals
 * @note  This function is deprecated, use LL_Dbgtrc_Read_IStall/LL_Dbgtrc_Read_OStall instead
 * @param iolink If not zero return the status of the Input Links. Return the Output Links otherwise
 * @retval Stall signals bitfield
 */
uint64_t LL_Dbgtrc_Read_Stalls(int id, unsigned char iolink)
{
  uint64_t stalls = 0;

  if (iolink)
  {
    stalls = (uint64_t)ATON_DEBUG_TRACE_ILINK_STATES_GET(id, 0);
    if (ATON_DEBUG_TRACE_ILINK_STATES_IDX_MAX(id) > 0)
      stalls |= (uint64_t)ATON_DEBUG_TRACE_ILINK_STATES_GET(id, 1) << 32;
  }
  else
  {
    stalls = (uint64_t)ATON_DEBUG_TRACE_OLINK_STATES_GET(id, 0);
    if (ATON_DEBUG_TRACE_ILINK_STATES_IDX_MAX(id) > 0)
      stalls |= (uint64_t)ATON_DEBUG_TRACE_OLINK_STATES_GET(id, 1) << 32;
  }

  return stalls;
}

/**
 * @brief Returns the status of a given input stall signal
 * @param signal Input stall signal id
 * @retval Status of the stall signal (0 or 1). -1 if error
 */
int LL_Dbgtrc_Read_IStall(int id, unsigned int signal)
{
  int idx = signal >> 5;
  int bitpos = signal & 0x1f;

  if (idx > ATON_DEBUG_TRACE_ILINK_STATES_IDX_MAX(id))
    return -1;

  return (ATON_DEBUG_TRACE_ILINK_STATES_GET(id, idx) >> bitpos) & 1;
}

/**
 * @brief Returns the status of a given output stall signal
 * @param signal Output stall signal id
 * @retval Status of the stall signal (0 or 1). -1 if error
 */
int LL_Dbgtrc_Read_OStall(int id, unsigned int signal)
{
  int idx = signal >> 5;
  int bitpos = signal & 0x1f;

  if (idx > ATON_DEBUG_TRACE_OLINK_STATES_IDX_MAX(id))
    return -1;

  return (ATON_DEBUG_TRACE_OLINK_STATES_GET(id, idx) >> bitpos) & 1;
}

/**
 * @brief Initializes and starts a trigger
 * @param id Id of the target debug and trace unit
 * @param trigger The trigger to be used [0..7]
 * @param Trigger_InitStruct Trigger descriptor structure
 * @note  Triggers can be captured via CoreSight System Trace Macrocell (STM)
 */
int LL_Dbgtrc_Trigger_Init(int id, int trigger, LL_Dbgtrc_Trigger_InitTypedef *Trigger_InitStruct)
{
  volatile uint32_t *reg = (volatile uint32_t *)(ATON_DEBUG_TRACE_TRIG_ADDR(id, trigger));
  uint32_t t = ATON_DEBUG_TRACE_TRIG_DT;

  t = ATON_DEBUG_TRACE_TRIG_SET_SEL(t, Trigger_InitStruct->signal);
  t = ATON_DEBUG_TRACE_TRIG_SET_EVENT_TYPE(t, Trigger_InitStruct->evt_type);
  t = ATON_DEBUG_TRACE_TRIG_SET_FILTER(t, Trigger_InitStruct->filter);
  t = ATON_DEBUG_TRACE_TRIG_SET_EN(t, 1);
  *reg = t;

  return 0;
}

/**
 * @brief Generates software trigger
 * @param id Id of the target debug and trace unit
 * @param trigger Destination trigger [0..7]
 */
int LL_Dbgtrc_Trigger_Swtrig(int id, int trigger)
{
  volatile uint32_t *reg = (volatile uint32_t *)(ATON_DEBUG_TRACE_TRIG_ADDR(id, trigger));
  *reg |= (1 << ATON_DEBUG_TRACE_TRIG_SWTRIG_LSB);

  return 0;
}

/**
 * @brief Returns the overrunt status of a given trigger
 * @param id Id of the target debug and trace unit
 * @param trigger Destination trigger [0..7]
 * @retval Overrun bit status
 */
int LL_Dbgtrc_Trigger_GetOVR(int id, int trigger)
{
  uint32_t ovr;

  volatile uint32_t *reg = (volatile uint32_t *)(ATON_DEBUG_TRACE_TRIG_ADDR(id, trigger));
  ovr = *reg & ATON_DEBUG_TRACE_TRIG_OVR_MASK;

  return ovr == 0 ? 0 : 1;
}

/**
 * @brief Configure a counter
 * @param id Id of the target debug and trace unit
 * @param counter The counter to be used [0..15]
 * @param Counter_InitStruct Counter descriptor structure
 */
int LL_Dbgtrc_Counter_Init(int id, int counter, LL_Dbgtrc_Counter_InitTypdef *Counter_InitStruct)
{
  volatile uint32_t *reg = (volatile uint32_t *)(ATON_DEBUG_TRACE_EVENT_ADDR(id, counter));
  uint32_t t = ATON_DEBUG_TRACE_EVENT_DT;

  t = ATON_DEBUG_TRACE_EVENT_SET_SEL(t, Counter_InitStruct->signal);
  t = ATON_DEBUG_TRACE_EVENT_SET_EVENT_TYPE(t, Counter_InitStruct->evt_type);
  t = ATON_DEBUG_TRACE_EVENT_SET_WRAP(t, Counter_InitStruct->wrap);
  t = ATON_DEBUG_TRACE_EVENT_SET_CNT_DOWN(t, Counter_InitStruct->countdown);
  t = ATON_DEBUG_TRACE_EVENT_SET_INT_DISABLE(t, Counter_InitStruct->int_disable);

  *reg = t;

  reg = (volatile uint32_t *)(ATON_DEBUG_TRACE_EVENT_CNT_ADDR(id, counter));
  *reg = Counter_InitStruct->counter;

  return 0;
}

/**
 * @brief Start a counter
 * @param id Id of the target debug and trace unit
 * @param counter The counter to be used [0..15]
 * @note Do not use this when binding a Start event or the counter will start
 */
int LL_Dbgtrc_Counter_Start(int id, int counter)
{
  volatile uint32_t *reg = (volatile uint32_t *)(ATON_DEBUG_TRACE_EVENT_ADDR(id, counter));
  uint32_t t = *reg;

  t = ATON_DEBUG_TRACE_EVENT_SET_EN(t, 1);
  *reg = t;

  return 0;
}

/**
 * @brief Stop a counter
 * @param id Id of the target debug and trace unit
 * @param counter The counter to be used [0..15]
 * @note Do not use this when binding a Stop event or the counter will stop
 */
int LL_Dbgtrc_Counter_Stop(int id, int counter)
{
  volatile uint32_t *reg = (volatile uint32_t *)(ATON_DEBUG_TRACE_EVENT_ADDR(id, counter));
  uint32_t t = *reg;

  t = ATON_DEBUG_TRACE_EVENT_SET_EN(t, 0);
  *reg = t;

  return 0;
}

/**
 * @brief Returns the counter value of a given observer
 * @param id Id of the target debug and trace unit
 * @param counter The counter to be used [0..15]
 * @retval Counter's value
 */
uint32_t LL_Dbgtrc_Counter_Read(int id, int counter)
{
  volatile uint32_t *reg = (volatile uint32_t *)(ATON_DEBUG_TRACE_EVENT_CNT_ADDR(id, counter));
  return *reg;
}

/**
 * @brief Binds the start of a counter to another counters' countdown event
 * @param id Id of the target debug and trace unit
 * @param srccounter The counter which triggers the Start event of the dstcounter
 * @param dstcounter The counter which starts upon srccounter contdown event
 * @TODO: add parameters checkers, e.g. throw error if source counter countdown is not enabled
 */
int LL_Dbgtrc_Counter_BindStart(int id, int srccounter, int dstcounter)
{
  volatile uint32_t *reg = (volatile uint32_t *)(ATON_DEBUG_TRACE_EVENT_ADDR(id, dstcounter));
  uint32_t t = *reg;

  t = ATON_DEBUG_TRACE_EVENT_SET_START_EVENT_EN(t, 1);
  t = ATON_DEBUG_TRACE_EVENT_SET_START_EVENT_SEL(t, srccounter);
  *reg = t;

  return 0;
}

/**
 * @brief Binds the stop of a counter to another counters' countdown event
 * @param id Id of the target debug and trace unit
 * @param srccounter The counter which triggers the Stop event of the dstcounter
 * @param dstcounter The counter which starts upon srccounter contdown event
 * @TODO: add parameters checkers, e.g. throw error if source counter countdown is not enabled
 */
int LL_Dbgtrc_Counter_BindStop(int id, int srccounter, int dstcounter)
{
  volatile uint32_t *reg = (volatile uint32_t *)(ATON_DEBUG_TRACE_EVENT_ADDR(id, dstcounter));
  uint32_t t = *reg;

  t = ATON_DEBUG_TRACE_EVENT_SET_STOP_EVENT_EN(t, 1);
  t = ATON_DEBUG_TRACE_EVENT_SET_STOP_EVENT_SEL(t, srccounter);
  *reg = t;

  return 0;
}

/**
 * @brief Configures a stopwatch
 * @param id The Debug and Trace unit id to be used
 * @param StopWatch_InitStruct Stopwatch descriptor structure
 */
int LL_Dbgtrc_Counter_StopWatch(int id, LL_Dbgtrc_StopWatch_InitTypdef *StopWatch_InitStruct)
{
  LL_Dbgtrc_Counter_InitTypdef counterConf = {0};

  /* Configure stopwatch start event */
  counterConf.signal = StopWatch_InitStruct->startSignal;
  counterConf.evt_type = StopWatch_InitStruct->startEvent;
  counterConf.wrap = 0;
  counterConf.countdown = 1;
  counterConf.int_disable = 1;
  counterConf.counter = 0;
  LL_Dbgtrc_Counter_Init(id, StopWatch_InitStruct->startCounter, &counterConf);

  /* Configure stopwatch stop event */
  counterConf.signal = StopWatch_InitStruct->stopSignal;
  counterConf.evt_type = StopWatch_InitStruct->stopEvent;
  counterConf.wrap = 0;
  counterConf.countdown = 1;
  counterConf.int_disable = 1;
  counterConf.counter = 0;
  LL_Dbgtrc_Counter_Init(id, StopWatch_InitStruct->stopCounter, &counterConf);

  /* Configure main counter, bind start/stop counters to it and start them */
  counterConf.signal = DBGTRC_VDD;
  counterConf.evt_type = DBGTRC_EVT_HI;
  counterConf.wrap = 0;
  counterConf.countdown = 0;
  counterConf.int_disable = 1;
  LL_Dbgtrc_Counter_Init(id, StopWatch_InitStruct->mainCounter, &counterConf);

  LL_Dbgtrc_Counter_BindStart(id, StopWatch_InitStruct->startCounter, StopWatch_InitStruct->mainCounter);
  LL_Dbgtrc_Counter_BindStop(id, StopWatch_InitStruct->stopCounter, StopWatch_InitStruct->mainCounter);

  LL_Dbgtrc_Counter_Start(id, StopWatch_InitStruct->startCounter);
  LL_Dbgtrc_Counter_Start(id, StopWatch_InitStruct->stopCounter);

  return 0;
}

/** Tracers support. Only build if hardware is available
 */
#ifdef ATON_DEBUG_TRACE_DEBUG_TRACE_N_TRACERS
#if ATON_DEBUG_TRACE_DEBUG_TRACE_N_TRACERS(0) > 0
/**
 * @brief Initializes a Tracer Unit
 * @param id Debug and Trace Unit ID to configure
 * @param tracer Tracer ID to configure [0..ATON_DEBUG_TRACE_DEBUG_TRACE_N_TRACERS-1]
 * @param Tracer_InitStruct Tracer initialization structure
 */
int LL_Dbgtrc_Tracer_Init(int id, int tracer, LL_Dbgtrc_Tracer_InitTypedef *Tracer_InitStruct)
{
  /* Enable associated Trigger unit */
  LL_Dbgtrc_Trigger_Init(id, tracer, &(Tracer_InitStruct->triggerconf));

  /* Configure Tracer unit. Use LL_Dbgtrc_Tracer_Start to start it */
  volatile uint32_t *reg = (volatile uint32_t *)(ATON_DEBUG_TRACE_TRACE_CTRL_ADDR(id, tracer));
  uint32_t t = *reg;
  t = ATON_DEBUG_TRACE_TRACE_CTRL_SET_USER_TOKEN(t, Tracer_InitStruct->token);
  t = ATON_DEBUG_TRACE_TRACE_CTRL_SET_VERBOSITY(t, Tracer_InitStruct->verbosity);
  *reg = t;

  return 0;
}

/**
 * @brief Starts a Tracer Unit
 * @param id Target Debug and Trace Unit ID
 * @param tracer Tracer ID to start [0..ATON_DEBUG_TRACE_DEBUG_TRACE_N_TRACERS-1]
 */
int LL_Dbgtrc_Tracer_Start(int id, int tracer)
{
  volatile uint32_t *reg = (volatile uint32_t *)(ATON_DEBUG_TRACE_TRACE_CTRL_ADDR(id, tracer));
  uint32_t t = *reg;
  t = ATON_DEBUG_TRACE_TRACE_CTRL_SET_EN(t, 1);
  *reg = t;

  return 0;
}

/**
 * @brief Stops a Tracer Unit
 * @param id Target Debug and Trace Unit ID
 * @param tracer Tracer ID to stop [0..ATON_DEBUG_TRACE_DEBUG_TRACE_N_TRACERS-1]
 */
int LL_Dbgtrc_Tracer_Stop(int id, int tracer)
{
  volatile uint32_t *reg = (volatile uint32_t *)(ATON_DEBUG_TRACE_TRACE_CTRL_ADDR(id, tracer));
  uint32_t t = *reg;
  t = ATON_DEBUG_TRACE_TRACE_CTRL_SET_EN(t, 0);
  *reg = t;

  return 0;
}

/**
 * @brief Enables Tracers Watchdog packet emission
 * @param id Target Debug and Trace Unit ID
 * @param period Watchdog period in clock cycles
 */
int LL_Dbgtrc_Tracer_Enable_Watchdog(int id, unsigned int period)
{
  uint32_t t = ATON_DEBUG_TRACE_WATCHDOG_DT;
  t = ATON_DEBUG_TRACE_WATCHDOG_SET_PERIOD(t, period);
  t = ATON_DEBUG_TRACE_WATCHDOG_SET_EN(t, 1);
  ATON_DEBUG_TRACE_WATCHDOG_SET(id, t);

  return 0;
}

/**
 * @brief DisablesTracers Watchdog packet emission
 * @param id Target Debug and Trace Unit ID
 */
int LL_Dbgtrc_Tracer_Disable_Watchdog(int id)
{
  uint32_t t = ATON_DEBUG_TRACE_WATCHDOG_GET(id);
  t = ATON_DEBUG_TRACE_WATCHDOG_SET_EN(t, 0);
  ATON_DEBUG_TRACE_WATCHDOG_SET(id, t);

  return 0;
}

/**
 * @brief Selects the Stream Engine in charge of emitting Tracers packets
 * @param id Debug and Trace Unit ID to configure
 * @param streng The Stream Engin ID to be used. Make sure it's not already used!
 * @param stream_addr Start address for trace emission
 * @param n Max trace emission length
 */
int LL_Dbgtrc_Tracer_Bind(int id, int streng, uint8_t *stream_addr, size_t n)
{
  LL_Streng_TensorInitTypeDef dma_out = {
      .dir = 1,
      .addr_base = {stream_addr},
      .offset_start = 0,
      .offset_end = n,
      .raw = 1,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = n,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 1,
      .nbits_in = 24,
      .nbits_out = 24,
      .nbits_unsigned = 0,
      .align_right = 0,
      .noblk = 0,
  };

  /* Connect Debug and Trace Unit to stream engine */
  const LL_Switch_InitTypeDef switch_init = {LL_Switch_Init_Dest() =
                                                 __atonn_getDstPortID(STRSWITCH, 0, STRENG, streng, 0),
                                             LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, DEBUG_TRACE, 0, 0),
                                             LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0};

  LL_Switch_Init(&switch_init, 1);

  /* Configure and start target stream engine */
  const LL_ATON_EnableUnits_InitTypeDef dma_unit = {{STRENG, streng}};
  LL_Streng_TensorInit(streng, &dma_out, 1);
  LL_ATON_EnableUnits_Init(&dma_unit, 1);

  return 0;
}
#endif
#endif

/* Examples */

/**
 * @brief Counts the number of interrupts triggered by a given unit
 * @param counter The Counter ID to be used [0..15]
 * @param unitirq The interrupt ID of the observed unit, e.g.: ATON_STRENG2_INT
 * @note Use LL_Dbgtrc_Counter_Read(0, counter) to retrieve the number of interrupts
 */
int LL_Dbgtrc_Count_IRQs(unsigned int counter, unsigned int unitirq)
{
  LL_Dbgtrc_Counter_InitTypdef dbgtrc_init = {
      .signal = DBGTRC_INTCTRL_SIGNALS + unitirq,
      .evt_type = DBGTRC_EVT_POSEDGE,
  };

  LL_Dbgtrc_Init(0);
  LL_Dbgtrc_Counter_Init(0, counter, &dbgtrc_init);
  LL_Dbgtrc_Counter_Start(0, counter);

  return 0;
}

/**
 * @brief Computes the duration of an epoch
 * @param counter The Counter ID to be used [0..15]
 * @param istreng The ID of the input stream engine triggering the start of epoch
 *        First rising edge of OSTRX_HENV signal marks the start of the epoch.
 * @param ostreng The ID of the output stream engine triggering the end of epoch
 *        Output stream engine interrupt rising edge marks the end of the epoch
 * @note Use LL_Dbgtrc_Counter_Read(0, counter) to retrieve the number of cycles the epoch lasted
 * @note Uses counter+1 and counter+2 as start/stop event sources
 * @note Uses input stream engine HENV signal to trigger counter start.
 *       Uses output stream engine interrupt to trigger counter stop
 */
int LL_Dbgtrc_Count_Epoch_Len(unsigned int counter, unsigned int istreng, unsigned int ostreng)
{
  LL_Dbgtrc_StopWatch_InitTypdef stopwatch_init = {
      .mainCounter = counter,
      .startCounter = counter + 1,
      .startSignal = DBGTRC_SWITCH_OSTRX_HENV + ATON_STRSWITCH_0_LINK_STRENG_0_0 + istreng,
      .startEvent = DBGTRC_EVT_POSEDGE,
      .stopCounter = counter + 2,
      .stopSignal = DBGTRC_INTCTRL_SIGNALS + ATON_STRENG_INT(0) + ostreng,
      .stopEvent = DBGTRC_EVT_POSEDGE,
  };

  LL_Dbgtrc_Init(0);
  LL_Dbgtrc_Counter_StopWatch(0, &stopwatch_init);

  return 0;
}

/** @defgroup Stream Engines' Stall signals monitoring
 * @{
 */

/**
 * @brief Counts the number cycles a given Stream Switch stall signal is hi or low
 * @param counter The counter to be used
 * @param iostall If non-zero count the ISTALL events. If zero count the OSTALL events
 * @param signal The signal to be monitored. E.g.: GET_STRSWTCH_DEST(ATON_STRSWITCH_DSTSTRENG0_OFFSET) for ISTALL or
 * ATON_LINK_STRENG0 for OSTALL
 * @param Set to 1 to count when the stall signal is hi. Set to 0 to count when the signal is low
 * @note If a unit is idle, its stall signal is high
 */
int LL_Dbgtrc_Count_Stalls(unsigned int counter, unsigned char iostall, unsigned int signal, unsigned int hilow)
{
  signal += (iostall ? SWITCH_ISTRX_STALL : SWITCH_OSTRX_STALL);
  unsigned int event_type = hilow ? DBGTRC_EVT_HI : DBGTRC_EVT_LOW;

  LL_Dbgtrc_Counter_InitTypdef dbgtrc_init = {
      .signal = signal,
      .evt_type = event_type,
  };

  LL_Dbgtrc_Init(0);
  LL_Dbgtrc_Counter_Init(0, counter, &dbgtrc_init);
  LL_Dbgtrc_Counter_Start(0, counter);

  return 0;
}

/**
 * @brief Configures a group of counters to measure the number of cycles a given set of stream engines are active (i.e.:
 * not stalled) during an epoch execution
 * @param istreng the input stream engine mask
 * @param ostreng the output stream engine mask
 * @param counter The Debug and Trace counter ID to start with
 * @retval Number of counters used. Negative error code otherwise
 * @note This function is meant to be called from Epoch PRE START callback
 * @note To be used in conjunction with LL_Dbgtrc_Count_StrengActive_Start/Stop()
 */
int LL_Dbgtrc_Count_StrengActive_Config(uint32_t istreng, uint32_t ostreng, unsigned int counter)
{
  int i;
  int ncounters = 0;
  LL_Dbgtrc_Counter_InitTypdef counter_init = {0};

  counter_init.evt_type = DBGTRC_EVT_LOW;
  counter_init.wrap = 0;
  counter_init.countdown = 0;
  counter_init.int_disable = 1;
  counter_init.counter = 0;

  for (i = 0; i < ATON_STRENG_NUM; i++)
  {
    if (istreng & (1 << i))
    {
      counter_init.signal = SWITCH_OSTRX_STALL + 0 + i;
      LL_Dbgtrc_Counter_Init(0, counter, &counter_init);
      counter++;
      ncounters++;
    }

    if (ostreng & (1 << i))
    {
      counter_init.signal = SWITCH_ISTRX_STALL + 0 + i;
      LL_Dbgtrc_Counter_Init(0, counter, &counter_init);
      counter++;
      ncounters++;
    }
  }

  return ncounters;
}

/**
 * @brief Starts a set of counters configured with LL_Dbgtrc_Count_StrengActive_Config
 * @param istreng the input stream engine mask
 * @param ostreng the output stream engine mask
 * @param counter The Debug and Trace counter ID to start with
 * @retval Zero if ok, error code otherwise
 * @note This function is meant to be called from Epoch Start callback
 * @note To be used in conjunction with LL_Dbgtrc_Count_StrengActive_Stop()
 */
int LL_Dbgtrc_Count_StrengActive_Start(uint32_t istreng, uint32_t ostreng, unsigned int counter)
{
  int i;
  for (i = 0; i < ATON_STRENG_NUM; i++)
  {
    if ((istreng | ostreng) & (1 << i))
      LL_Dbgtrc_Counter_Start(0, counter++);
  }

  return 0;
}

/**
 * @brief Stops the counters being started with LL_Dbgtrc_Count_StrengActive_Start
 * @param istreng the input stream engine mask
 * @param ostreng the output stream engine mask
 * @param counter The Debug and Trace counter ID to start with
 * @retval Zero if ok, error code otherwise
 * @note This function is meant to be called from Epoch Stop callback
 * @note To be used in conjunction with LL_Dbgtrc_Count_StrengActive_Config/Start()
 */
int LL_Dbgtrc_Count_StrengActive_Stop(uint32_t istreng, uint32_t ostreng, unsigned int counter)
{
  int i;
  for (i = 0; i < ATON_STRENG_NUM; i++)
  {
    if ((istreng | ostreng) & (1 << i))
      LL_Dbgtrc_Counter_Stop(0, counter++);
  }

  return 0;
}

/**
 * @brief Prints statistics on stream engine's active periods
 * @param istreng the input stream engine mask
 * @param ostreng the output stream engine mask
 * @param counter The Debug and Trace counter ID to start with
 * @retval Zero if ok, error code otherwise
 * @note To be used in conjunction with LL_Dbgtrc_Count_StrengActive_Config/Start/Stop()
 */
int LL_Dbgtrc_Count_StrengActive_Print(uint32_t istreng, uint32_t ostreng, unsigned int counter)
{
  int i;

  LL_ATON_PRINTF("STRENG active: ");
  for (i = 0; i < ATON_STRENG_NUM; i++)
  {
    if ((istreng | ostreng) & (1 << i))
      LL_ATON_PRINTF("%" PRIu32 "  ", LL_Dbgtrc_Counter_Read(0, counter++));
  }
  LL_ATON_PRINTF("\n");

  return 0;
}

/**
 * @brief Returns the biggest value of the counters configured and started with
 * LL_Dbgtrc_Count_StrengActive_Config/Start()
 * @param istreng the input stream engine mask
 * @param ostreng the output stream engine mask
 * @param counter The Debug and Trace counter ID to start with
 * @param argmax The ID of Stream Engine with the max active time
 * @retval The value of bigger counter
 * @note This function is meant to be called from Epoch Stop callback
 * @note To be used in conjunction with LL_Dbgtrc_Count_StrengActive_Start/Stop()
 */
unsigned int LL_Dbgtrc_Count_StrengActive_GetMAX(uint32_t istreng, uint32_t ostreng, unsigned int counter,
                                                 unsigned int *argmax)
{
  int i;
  unsigned int maxcount = 0;

  for (i = 0; i < ATON_STRENG_NUM; i++)
  {
    if ((istreng | ostreng) & (1 << i))
    {
      if (LL_Dbgtrc_Counter_Read(0, counter) > maxcount)
      {
        maxcount = LL_Dbgtrc_Counter_Read(0, counter);
        *argmax = i;
      }
      counter++;
    }
  }

  return maxcount;
}

/**
 * @}
 */

/** @defgroup HENV counters
 * oHENV signal is high when the corresponding link is transferring data so it can be used to measure link activity
 * of the input stream engines (i.e.: reading data) and check whether there are bootlenecks in reading from memory.
 * @{
 */

/**
 * @brief Configures a group of counters to measure the number of cycles a given set of input stream engines have HENV
 * signal high
 * @param istreng the input stream engine mask
 * @param counter The Debug and Trace counter ID to start with
 * @retval Number of counters used. Negative error code otherwise
 * @note This function is meant to be called from Epoch PRE START callback
 * @note To be used in conjunction with LL_Dbgtrc_Count_StrengHENV_Start/Stop/Print()
 */
int LL_Dbgtrc_Count_StrengHENV_Config(uint32_t istreng, unsigned int counter)
{
  int i;
  int ncounters = 0;
  LL_Dbgtrc_Counter_InitTypdef counter_init = {0};

  counter_init.evt_type = DBGTRC_EVT_HI;
  counter_init.wrap = 0;
  counter_init.countdown = 0;
  counter_init.int_disable = 1;
  counter_init.counter = 0;

  for (i = 0; i < ATON_STRENG_NUM; i++)
  {
    if (istreng & (1 << i))
    {
      counter_init.signal = DBGTRC_SWITCH_OSTRX_HENV + ATON_STRSWITCH_0_LINK_STRENG_0_0 + i;
      LL_Dbgtrc_Counter_Init(0, counter, &counter_init);
      counter++;
      ncounters++;
    }
  }

  return ncounters;
}

/**
 * @brief Starts the counters previously configured with LL_Dbgtrc_Count_StrengHENV_Config
 * @param istreng the input stream engine mask
 * @param counter The Debug and Trace counter ID to start with
 */
int LL_Dbgtrc_Count_StrengHENV_Start(uint32_t istreng, unsigned int counter)
{
  int i;
  for (i = 0; i < ATON_STRENG_NUM; i++)
  {
    if (istreng & (1 << i))
      LL_Dbgtrc_Counter_Start(0, counter++);
  }

  return 0;
}

/**
 * @brief Stops the counters previously started with LL_Dbgtrc_Count_StrengHENV_Start
 * @param istreng the input stream engine mask
 * @param counter The Debug and Trace counter ID to start with
 */
int LL_Dbgtrc_Count_StrengHENV_Stop(uint32_t istreng, unsigned int counter)
{
  int i;
  for (i = 0; i < ATON_STRENG_NUM; i++)
  {
    if (istreng & (1 << i))
      LL_Dbgtrc_Counter_Start(0, counter++);
  }

  return 0;
}

/**
 * @brief Prints the contents of the counters configured with LL_Dbgtrc_Count_StrengHENV_Config
 * @param istreng the input stream engine mask
 * @param counter The Debug and Trace counter ID to start with
 */
int LL_Dbgtrc_Count_StrengHENV_Print(uint32_t istreng, unsigned int counter)
{
  int i;

  LL_ATON_PRINTF("STRENG HENV: ");
  for (i = 0; i < ATON_STRENG_NUM; i++)
  {
    if (istreng & (1 << i))
      LL_ATON_PRINTF("%" PRIu32 "  ", LL_Dbgtrc_Counter_Read(0, counter++));
  }
  LL_ATON_PRINTF("\n");

  return 0;
}

/**
 * @}
 */

/** @defgroup Burst Length counters
 * @{
 */

/**
 * @brief Counts the burst lenghts on a given Bus Interface
 * @param counter The counter ID to start from. Four counters are used: counter, counter+1, counter+2, counter+3
 * @param busif The Bus Interface to monitor
 * @param readwrite If non-zero count the Read burst lengths. If zero, count the Write burst lenghts
 * @note  Uses four counters starting from 'counter' to count four possible burst lenghts [1, 2, 4, 8]
 */
int LL_Dbgtrc_Count_BurstsLen(unsigned int counter, unsigned char busif, unsigned char readwrite)
{
  unsigned int signal = readwrite ? BUS_INTERFACE_READ1X8B : BUS_INTERFACE_WRITE1X8B;
  int i;

  signal += busif;

  LL_Dbgtrc_Counter_InitTypdef dbgtrc_init = {
      .evt_type = DBGTRC_EVT_HI,
  };

  LL_Dbgtrc_Init(0);

  for (i = 0; i < 4; i++)
  {
    dbgtrc_init.signal = signal + 2 * i;
    LL_Dbgtrc_Counter_Init(0, counter + i, &dbgtrc_init);
    LL_Dbgtrc_Counter_Start(0, counter + i);
  }

  return 0;
}

/**
 * @brief Initialize and start counters to analyze Write and Read accesses on both Bus Interfaces
 * @param counter_id The counter to start from. Uses 16 counters starting from 'counter_id'
 */
int LL_Dbgtrc_BurstLenBenchStart(unsigned int counter_id)
{
  LL_Dbgtrc_Count_BurstsLen(counter_id, 0, 0);      /* Busif 0 writes */
  LL_Dbgtrc_Count_BurstsLen(counter_id + 4, 0, 1);  /* Busif 0 reads */
  LL_Dbgtrc_Count_BurstsLen(counter_id + 8, 1, 0);  /* Busif 1 writes */
  LL_Dbgtrc_Count_BurstsLen(counter_id + 12, 1, 1); /* Busif 1 reads */

  return 0;
}

/**
 * @brief Retrieves the Burst Length counters from Debug and Trace Unit and stores them in a given array
 * @param counter_id The counter ID to start from
 * @param counters Pointer to the buffer where to store the counter values. Must be at least 16 elements
 */
int LL_Dbgtrc_BurstLenGet(unsigned int counter_id, unsigned int *counters)
{
  int i;

  for (i = 0; i < 16; i++)
    counters[i] = LL_Dbgtrc_Counter_Read(0, counter_id + i);

  return 0;
}

/**
 * @brief Computes the total amount of written and read bytes transited on both Bus Interfaces
 * @param counter_id The counter ID to start from
 * @param totalWrites [OUT] Holds the total number of written bytes
 * @param totalReads [OUT] Holds the total number of read bytes
 * @note
 */
int LL_Dbgtrc_GetTotalTranfers(unsigned int counter_id, unsigned int *totalWrites, unsigned int *totalReads)
{
  int i;
  *totalWrites = *totalReads = 0;
  unsigned int counters[16];

  LL_Dbgtrc_BurstLenGet(counter_id, counters);

  /* Compute total writes. AWLEN is one of 0, 1, 3, 7 */
  for (i = 0; i < 4; i++)
  {
    *totalWrites += (1 << i) * counters[i];
    *totalWrites += (1 << i) * counters[8 + i];
  }

  /* AWSIZE is always 3 (8 byte beats) */
  *totalWrites <<= 3;

  /* Same for reads */
  for (i = 0; i < 4; i++)
  {
    *totalReads += (1 << i) * counters[4 + i];
    *totalReads += (1 << i) * counters[12 + i];
  }

  *totalReads <<= 3;

  return 0;
}

/**
 * @brief Prints to standard output Burst Length statistics
 * @param counter_id The counter to start from
 */
int LL_Dbgtrc_LogTransfers(unsigned int counter_id)
{
  unsigned int counters[16];
  unsigned int totalReads, totalWrites;

  LL_Dbgtrc_BurstLenGet(counter_id, counters);

  LL_ATON_PRINTF("\n");
  LL_ATON_PRINTF("------------------------------------------\n");
  LL_ATON_PRINTF("BURSTLEN            1      2      4      8\n");
  LL_ATON_PRINTF("------------------------------------------\n");
  LL_ATON_PRINTF("BUSIF0 Writes  %6u %6u %6u %6u\n", counters[0], counters[1], counters[2], counters[3]);
  LL_ATON_PRINTF("BUSIF0 Reads   %6u %6u %6u %6u\n", counters[4], counters[5], counters[6], counters[7]);
  LL_ATON_PRINTF("BUSIF1 Writes  %6u %6u %6u %6u\n", counters[8], counters[9], counters[10], counters[11]);
  LL_ATON_PRINTF("BUSIF1 Reads   %6u %6u %6u %6u\n", counters[12], counters[13], counters[14], counters[15]);
  LL_ATON_PRINTF("------------------------------------------\n");

  LL_Dbgtrc_GetTotalTranfers(counter_id, &totalWrites, &totalReads);

  LL_ATON_PRINTF("Total Writes: %u bytes\n", totalWrites);
  LL_ATON_PRINTF("Total Reads : %u bytes\n", totalReads);

  return 0;
}

/**
 * @brief Prints to standard output Burst Length statistics per epoch
 * @param counter_id The counter to start from
 * @param counters The counters accumulated by some other means
 * @param first =0 prints header, =1 prints counters values, =2 prints footers
 */
int LL_Dbgtrc_LogTransfers_epoch(unsigned int counter_id, unsigned int *counters, int first)
{
  unsigned int totalReads, totalWrites;

  if (first == 0)
  {
    LL_ATON_PUTS("");
    LL_ATON_PRINTF(
        "-----------------------------------------------------------------------------------------------------------"
        "----------------------\n");
    LL_ATON_PRINTF(
        "BURSTLEN            1      2      4      8   |   1      2      4      8   |   1      2      4      8   |   "
        "1      2      4      8\n");
    LL_ATON_PRINTF(
        "                         BUSIF0 Writes                BUSIF0 Reads                BUSIF1 Writes            "
        "    BUSIF1 Reads\n");
    LL_ATON_PRINTF(
        "-----------------------------------------------------------------------------------------------------------"
        "----------------------\n");
  }
  else if (first == 1)
  {
    LL_ATON_PRINTF("            %6u %6u %6u %6u ", counters[0], counters[1], counters[2], counters[3]);
    LL_ATON_PRINTF("%6u %6u %6u %6u ", counters[4], counters[5], counters[6], counters[7]);
    LL_ATON_PRINTF("%6u %6u %6u %6u ", counters[8], counters[9], counters[10], counters[11]);
    LL_ATON_PRINTF("%6u %6u %6u %6u ", counters[12], counters[13], counters[14], counters[15]);
    LL_ATON_PUTS("");
  }
  else
  {
    LL_Dbgtrc_GetTotalTranfers(counter_id, &totalWrites, &totalReads);

    LL_ATON_PRINTF("\nTotal Writes: %u bytes\n", totalWrites);
    LL_ATON_PRINTF("Total Reads : %u bytes\n", totalReads);
  }

  return 0;
}

/**
 * @}
 */

/**
 * @brief Counts the external trigger signals occurences
 * @param counter The counter to be used
 * @param trigger The External Trigger signal to monitor [0..3]
 */
int LL_Dbgtrc_Count_ExtTrigger(unsigned int counter, unsigned char trigger)
{
  LL_Dbgtrc_Counter_InitTypdef dbgtrc_init = {.evt_type = DBGTRC_EVT_POSEDGE, .signal = EXT_TRIGGERS_SYNC + trigger};

  LL_Dbgtrc_Init(0);
  LL_Dbgtrc_Counter_Init(0, counter, &dbgtrc_init);
  LL_Dbgtrc_Counter_Start(0, counter);

  return 0;
}

/**
 * @brief Example showing how to start/stop counters synchronously
 * @note This is a workaroud for ATON RTL <= 1.6. Proper sync start/stop implemented later versions
 */
int LL_Dbgtrc_SynchronousCountersTest(void)
{
  int counter;
  LL_Dbgtrc_Counter_InitTypdef dbgtrc_init = {.evt_type = DBGTRC_EVT_HI, .signal = DBGTRC_VDD};

  LL_ATON_Init();
  LL_Dbgtrc_Init(0);

  /* Starts counters and stops them in reverse order. Shows counters scews
   */
  for (counter = 0; counter < 8; counter++)
    LL_Dbgtrc_Counter_Init(0, counter, &dbgtrc_init);

  for (counter = 0; counter < 8; counter++)
    LL_Dbgtrc_Counter_Start(0, counter);

  /* Wait for counter to reach given value */
  while (LL_Dbgtrc_Counter_Read(0, 0) < 100000)
    ;

  /* Stop counters in reverse order */
  for (counter = 8; counter > 0; counter--)
    LL_Dbgtrc_Counter_Stop(0, counter);

  /* Check scews */
  for (counter = 0; counter < 8; counter++)
    LL_ATON_PRINTF("Non synced counters: %u    %" PRIu32 "\n", (unsigned)counter, LL_Dbgtrc_Counter_Read(0, counter));

  /* Starts counters and stops them in reverse order. Gates clock to sync counters
   */
  LL_Dbgtrc_Counter_InitTypdef noevt_init = {.evt_type = DBGTRC_EVT_HI, .signal = DBGTRC_GND};

  /* Starts counter with no event */
  for (counter = 0; counter < 8; counter++)
  {
    LL_Dbgtrc_Counter_Init(0, counter, &noevt_init);
    LL_Dbgtrc_Counter_Start(0, counter);
  }

  /* Gate clock */
  LL_Dbgtrc_DisableClock();

  /* Re-configure with event to monitor */
  for (counter = 0; counter < 8; counter++)
  {
    LL_Dbgtrc_Counter_Init(0, counter, &dbgtrc_init);
    LL_Dbgtrc_Counter_Start(0, counter);
  }

  /* Give clock */
  LL_Dbgtrc_EnableClock();

  /* Wait for counter to reach given value */
  while (LL_Dbgtrc_Counter_Read(0, 0) < 100000)
    ;

  LL_Dbgtrc_DisableClock();

  /* Check scews */
  for (counter = 0; counter < 8; counter++)
    LL_ATON_PRINTF("Synced  counters:    %u    %" PRIu32 "\n", (unsigned)counter, LL_Dbgtrc_Counter_Read(0, counter));

  return 0;
}

#endif // ATON_DEBUG_TRACE_NUM
