/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-19     CXLTET01       the first version
 */

#include <rtthread.h>
#include <board.h>
#include <rtdevice.h>
#include "monitor.h"
#include "ioDetector/iodetector.h"

#define DBG_TAG "monitor"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static rt_timer_t monitorTimer;
static rb_sta_t pvar;
extern uint16_t adc_buf[];

extern uint16_t usSRegHoldBuf[];
//每100ms 调用一次
static float adcRate;
static float Gain0;
static float Gain1;

static void inputUpdate(void)
{
    pvar._input[0] = rt_pin_read( DIN0_PIN);
    pvar._input[1] = rt_pin_read( DIN1_PIN);
    pvar._input[2] = rt_pin_read( DIN2_PIN);
    pvar._input[3] = rt_pin_read( DIN3_PIN);
    pvar._input[4] = rt_pin_read( DIN4_PIN);
    pvar._input[5] = rt_pin_read( DIN5_PIN);
    pvar._input[6] = rt_pin_read( DIN6_PIN);
    pvar._input[7] = rt_pin_read( DIN7_PIN);
}

static int measureValueUpdate(void){
    //计算ch0-ch7
    for(uint8_t i=0;i<8;i++){
        pvar.vin[i].val = (adc_buf[i] & 0xfff )* adcRate * Gain0;
    }

    //计算ch8-ch11
    //获取p和n
    for(uint8_t i=8;i<16;i++){
        pvar.vin[i].val = (adc_buf[i] & 0xfff )* adcRate ;
    }
    for(uint8_t i=0;i<4;i++){
        pvar.vin[i+8].val = (pvar.vin[ (i<<1) + 8].val -  pvar.vin[(i<<1) + 9].val)*Gain1;
    }

    return 0;
}

static void on_io_status_changed(char* ioname,uint8_t index,io_status_e status){
    LOG_D("%s status changed ! current is %d",ioname,(uint8_t)status);
    if(index  < 8){
        pvar.input[index] = (uint8_t)status;
    }

}

static void onMonitorTimerTimeout(void* parameter){
    //动态获取设备地址
    pvar.addr = getBoardId();
    //获取输入状态
    inputUpdate();
    //运行io检测
    iodetector_update();
    //获取实时电压值
    measureValueUpdate();
    rt_memcpy(&usSRegHoldBuf[0], &(pvar.vin[0]._data[0]), 12<<2);
    rt_memcpy(&usSRegHoldBuf[12<<1], &(pvar.input[0]), 8);
    rt_memcpy(&usSRegHoldBuf[(12<<1) + (8>>1)], &pvar.addr, 2);

}

static void iodetector_init(void){

    iodetector_install("IO0",&(pvar._input[0]),500,3000,on_io_status_changed);
    iodetector_install("IO1",&(pvar._input[1]),500,3000,on_io_status_changed);
    iodetector_install("IO2",&(pvar._input[2]),500,3000,on_io_status_changed);
    iodetector_install("IO3",&(pvar._input[3]),500,3000,on_io_status_changed);
    iodetector_install("IO4",&(pvar._input[4]),500,3000,on_io_status_changed);
    iodetector_install("IO5",&(pvar._input[5]),500,3000,on_io_status_changed);
    iodetector_install("IO6",&(pvar._input[6]),500,3000,on_io_status_changed);
    iodetector_install("IO7",&(pvar._input[7]),500,3000,on_io_status_changed);
}

int  monitorInit(void){

    adcRate = VREF/4096.0;
    Gain0 = 5.0;
    Gain1 = ((12400/49.9 + 1) * 0.2)/(0.2*8.2);
    iodetector_init();
    monitorTimer =  rt_timer_create("monitor",\
                onMonitorTimerTimeout , \
                NULL,\
                rt_tick_from_millisecond(100),\
                RT_TIMER_FLAG_PERIODIC);
    if(monitorTimer != NULL){
        rt_timer_start(monitorTimer);
        return 0;
    }

    return -1;

}

rb_sta_t* monitorGetpVar(void){
    return &pvar;
}
