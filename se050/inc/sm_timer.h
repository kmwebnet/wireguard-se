/*
 *
 * Copyright 2016 NXP
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _SM_TIMER_H_
#define _SM_TIMER_H_
#include <linux/kernel.h>
#ifdef __gnu_linux__

#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Change this value to tick rate used by the controller */
#define TICK_RATE_HZ 1000
#define MS_TO_TICKS(msec) (( (msec) * (TICK_RATE_HZ) ) / (1000))

/* function used for delay loops */
uint32_t sm_initSleep(void);
void sm_sleep(uint32_t msec);
void sm_usleep(uint32_t microsec);

#ifdef __cplusplus
}
#endif
#endif // _SM_TIMER_H_
