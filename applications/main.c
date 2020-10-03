/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-09-09     RT-Thread    first version
 */

#include <rtthread.h>
#include <board.h>
#include <rtdevice.h>
#include <monitor/monitor.h>
#include "modbus/slave.h"

#define DBG_TAG "main"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>



static int appInit(void){
    monitorInit();
    mb_slave_init();
    return 0;
}

int main(void)
{
    uint8_t bling_counter;

    platformInit();
    appInit();

    while(1){
        //ÐÄÌøµÆ
        if(bling_counter > 9){
            bling_counter  = 0;
            rt_pin_write(LED0_PIN,1);
        }else{
            rt_pin_write(LED0_PIN,0);
        }
        bling_counter ++;

        rt_thread_mdelay(100);
    }
}

