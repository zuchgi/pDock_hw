/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-09-28     DELL       the first version
 */


#include <rtthread.h>
#include "slave.h"
#include "mb.h"
#include "user_mb_app.h"
#include "board.h"
#include <rtdevice.h>

#ifdef PKG_MODBUS_SLAVE_SAMPLE
#define SLAVE_ADDR      MB_SAMPLE_SLAVE_ADDR
#define PORT_NUM        MB_SLAVE_USING_PORT_NUM
#define PORT_BAUDRATE   MB_SLAVE_USING_PORT_BAUDRATE
#else
#define SLAVE_ADDR      0x01
#define PORT_NUM        3
#define PORT_BAUDRATE   115200
#endif

#define PORT_PARITY     MB_PAR_NONE

#define MB_POLL_THREAD_PRIORITY  10
#define MB_SEND_THREAD_PRIORITY  RT_THREAD_PRIORITY_MAX - 1

#define MB_POLL_CYCLE_MS 50

//extern USHORT usSRegHoldBuf[S_REG_HOLDING_NREGS];


static void mb_slave_poll(void *parameter)
{
    eMBInit(MB_RTU, SLAVE_ADDR, PORT_NUM, PORT_BAUDRATE, PORT_PARITY);
    eMBEnable();
    while (1)
    {
        eMBPoll();
        rt_pin_write(LED1_PIN,1);
        rt_thread_mdelay(MB_POLL_CYCLE_MS);
        rt_pin_write(LED1_PIN,0);
    }
}

int mb_slave_init(void)
{
    rt_thread_t tid1 = RT_NULL;

    tid1 = rt_thread_create("md_s_poll", mb_slave_poll, RT_NULL, 512, MB_POLL_THREAD_PRIORITY, 10);
    if (tid1 != RT_NULL)
    {
        rt_thread_startup(tid1);
    }
    else
    {
        goto __exit;
    }
    return RT_EOK;

__exit:
    if (tid1)
        rt_thread_delete(tid1);

    return -RT_ERROR;
}
