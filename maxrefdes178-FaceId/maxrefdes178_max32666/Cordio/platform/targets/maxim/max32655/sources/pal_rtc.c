/*************************************************************************************************/
/*!
 * \file
 *
 * \brief  RTC clock implementation.
 *
 * Copyright (c) 2019-2020 Packetcraft, Inc.  All rights reserved.
 * Packetcraft, Inc. confidential and proprietary.
 *
 * IMPORTANT.  Your use of this file is governed by a Software License Agreement
 * ("Agreement") that must be accepted in order to download or otherwise receive a
 * copy of this file.  You may not use or copy this file for any purpose other than
 * as described in the Agreement.  If you do not agree to all of the terms of the
 * Agreement do not use this file and delete all copies in your possession or control;
 * if you do not have a copy of the Agreement, you must contact Packetcraft, Inc. prior
 * to any use, copying or further distribution of this software.
 */
/*************************************************************************************************/

#include "pal_rtc.h"
#include "pal_led.h"
#include "pal_sys.h"
#include "wut.h"
#include "lp.h"
#include "mxc_device.h"
#include "wut_regs.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/

/**************************************************************************************************
  Global Functions
**************************************************************************************************/

/**************************************************************************************************
  Global Variables
**************************************************************************************************/
static struct {
  uint32_t baseVal;   /* Account for rollover at compare value */
  PalRtcState_t state;
} palRtcCb;

/*************************************************************************************************/
/*!
 *  \brief  Interrupt service routine.
 *
 *  \return None.
 */
/*************************************************************************************************/
void WUT_IRQHandler(void)
{
  PalLedOn(PAL_LED_ID_CPU_ACTIVE);
  MXC_WUT_IntClear();
  palRtcCb.state = PAL_RTC_STATE_UNINIT;
  NVIC_ClearPendingIRQ(WUT_IRQn);
  PalRtcDisableCompareIrq(0);

  /* Account for rollover at compare value */
  palRtcCb.baseVal += MXC_WUT_GetCompare();

  /* Wait for the counter to rollover */
  while(MXC_WUT->cnt == MXC_WUT->cmp) {}
}

/*************************************************************************************************/
/*!
 *  \brief  Get the state of the RTC.
 *
 *  \return PAL_RTC_STATE_READY if active, PAL_RTC_STATE_UNINIT if inactive.
 */
/*************************************************************************************************/
PalRtcState_t PalRtcGetState(void)
{
  return palRtcCb.state;
}

/*************************************************************************************************/
/*!
 *  \brief  Set the RTC capture compare value.
 *
 *  \param  channelId   Channel ID Number.
 *  \param  value   Set new value for compare value.
 */
/*************************************************************************************************/
void PalRtcCompareSet(uint8_t channelId, uint32_t value)
{
  /* Account for rollover at compare value */
  uint32_t compareVal = value - palRtcCb.baseVal;
  PAL_SYS_ASSERT(channelId == 0);

  MXC_WUT_SetCompare(compareVal);
  NVIC_EnableIRQ(WUT_IRQn);
}

/*************************************************************************************************/
/*!
 *  \brief  Tickless timer initialization routine.
 */
/*************************************************************************************************/
void PalRtcInit(void)
{
  palRtcCb.baseVal = 0;

  /* Init WUT */
  mxc_wut_cfg_t cfg;
  cfg.mode = MXC_WUT_MODE_CONTINUOUS;
  cfg.cmp_cnt = PAL_MAX_RTC_COUNTER_VAL;

  MXC_WUT_Init(MXC_WUT_PRES_1);
  MXC_WUT_Config(&cfg);

#ifndef __riscv
  MXC_LP_EnableWUTAlarmWakeup();
  NVIC_SetPriority(WUT_IRQn, 0x80);    /* medium priority */
#endif
  
  NVIC_ClearPendingIRQ(WUT_IRQn);

  /* Enable WUT */
  MXC_WUT_Enable();
}

/*************************************************************************************************/
/*!
 *  \brief  Get current time.
 *
 *  \return RTC time.
 *
 *  \note   Caution must be taken when performing calculations with RTC time. RTC value bit width
 *          and resolution may vary.
 */
/*************************************************************************************************/
uint32_t PalRtcCounterGet(void)
{
  uint32_t count = MXC_WUT_GetCount();

  /* Account for the rollover at compare value */
  count += palRtcCb.baseVal;

  return count;
}

/*************************************************************************************************/
/*!
 *  \brief  Function for starting the RTC timer.
 *
 *  \param  channelId   Channel ID Number.
 */
/*************************************************************************************************/
void PalRtcEnableCompareIrq(uint8_t channelId)
{
  PAL_SYS_ASSERT(channelId == 0);

  palRtcCb.state = PAL_RTC_STATE_READY;
  NVIC_EnableIRQ(WUT_IRQn);
}

/*************************************************************************************************/
/*!
 *  \brief  Function for stopping the RTC timer.
 *
 *  \param  channelId   Channel ID Number.
 */
/*************************************************************************************************/
void PalRtcDisableCompareIrq(uint8_t channelId)
{
  PAL_SYS_ASSERT(channelId == 0);
  palRtcCb.state = PAL_RTC_STATE_UNINIT;

  NVIC_DisableIRQ(WUT_IRQn);
}
