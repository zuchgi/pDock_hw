/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-19     CXLTET01       the first version
 */
#ifndef DRIVERS_MESSAGE_MESSAGE_H_
#define DRIVERS_MESSAGE_MESSAGE_H_
#include <rtthread.h>
#include <stm32f1xx.h>
#include <monitor/monitor.h>


#define PUBLISH_ERROR_TIME 3
#define MSG_LEN 256


typedef enum{
    devMeasurement,
    devDailyReport,
    devStatuesChanged,
    devWeldingmachinStatusChanged,
    devPlanStopStatusChanged,
    devPlanRepairStatusChanged,
    devProductNameChanged,
    devOnline,
}msg_type_e;

void messageInit(void);
void messageUpdate(void);


#endif /* DRIVERS_MESSAGE_MESSAGE_H_ */
