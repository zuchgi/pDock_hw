/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-19     CXLTET01       the first version
 */
#ifndef DRIVERS_USRTIME_USRTIME_H_
#define DRIVERS_USRTIME_USRTIME_H_
#include <rtthread.h>
#include <stm32f1xx.h>
#include <time.h>

time_t  timingSystemGetSystemTime(void);
int timingSystemUsrInit(void);
int timingServerTimeSet(time_t now);
#endif /* DRIVERS_USRTIME_USRTIME_H_ */
