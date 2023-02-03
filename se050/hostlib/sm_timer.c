// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023 kmwebnet <kmwebnet@gmail.com>. All Rights Reserved.
 */


#include <linux/delay.h>
#include "sm_timer.h"

uint32_t sm_initSleep()
{
    return 0;
}

/**
 * Implement a blocking (for the calling thread) wait for a number of milliseconds.
 */
void sm_sleep(uint32_t msec)
{
    mdelay (msec);
}

/**
 * Implement a blocking (for the calling thread) wait for a number of microseconds
 */
void sm_usleep(uint32_t microsec)
{
    udelay(microsec);
}
