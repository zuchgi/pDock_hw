/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-19     CXLTET01       the first version
 */
#ifndef DRIVERS_MONITOR_H_
#define DRIVERS_MONITOR_H_
#include <rtthread.h>
#include <stm32f1xx.h>

//电路板板载基准电压
#define VREF (2.5)

typedef union{
    float val;
    uint8_t _data[4];
}float_u;
typedef struct{
    float_u vin[16];      //voltage
    uint8_t _input[8];  // 0 or 1
    uint8_t input[8];   //0 off 1 flash 2 on
    int addr;           //0-255
}rb_sta_t;



rb_sta_t* monitorGetpVar(void);
int  monitorInit(void);

#endif /* DRIVERS_MONITOR_H_ */
