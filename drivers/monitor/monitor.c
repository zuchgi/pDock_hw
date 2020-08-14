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
#include <sim7600/sim7600.h>
#include <usrTime/usrTime.h>
#include "monitor.h"

#define DBG_TAG "monitor"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static rt_timer_t monitorTimer;
static  MCMBaseStatus_t* pmcmBS;

static rb_sta_t pvar;
//extern uint16_t adc_buf[];
static int devStatusBuffer[3][20];
static int devStatusBufferIndex = 0;
//存储状态变化的消息队列
rt_mq_t mqStatus = RT_NULL;
rt_mq_t mqWMStatus = RT_NULL;
rt_mq_t mqPlanStop = RT_NULL;
rt_mq_t mqPlanRepair = RT_NULL;
rt_mq_t mqProductName = RT_NULL;
//发送日报的事件
rt_event_t  evtReport = RT_NULL;
extern uint16_t adc_buf[];

static uint8_t devStatusUpdate(void);
static void statusChangedUpdate(void);
static int measureValueUpdate(void);
static uint8_t devWmStatusUpdate(void);


static int measureValueUpdate(void){
    for(uint8_t i=0;i<16;i++){
        pvar.vin[i] = (adc_buf[i] & 0xfff )* 2.5/4096.0;
    }
    for(uint8_t i=0;i<4;i++){
        pvar.wm_current[i] = pvar.vin[i]*5*50;
    }
    for(uint8_t i=0;i<4;i++){
        pvar.wm_voltage[i] = (pvar.vin[ADC_ISIN0P+i*2] - pvar.vin[ADC_ISIN0N+i*2])/8.2*249.5;
    }
    return 0;
}
static void onMonitorTimerTimeout(void* parameter){
#if SIMULATION
    time_t now;
    struct tm* tt;
    now = get_time();
    tt = gmtime(&now);
    if(tt->tm_min <= 10){
        pvar.run_sta =wm_force_stop;
    }
    else if(tt->tm_min <= 30){
        pvar.run_sta =wm_auto_run;
    }
    else if(tt->tm_min <= 35){
        pvar.run_sta =wm_auto_stop;
    }
    else if(tt->tm_min <= 50){
        pvar.run_sta =wm_m_run;
    }
    else if(tt->tm_min <= 55){
        pvar.run_sta =wm_m_stop;
    }
    else if(tt->tm_min <= 60){
        pvar.run_sta =wm_turn_off;
    }

#else
    //获取设备运行状态
    devStatusUpdate();
    //获取焊接状态
    devWmStatusUpdate();
    //判断设备状态是否变化，并存入消息队列
    statusChangedUpdate();
    //获取设备地址
    pvar.addr = getBoardId();
    //获取实时参数
    measureValueUpdate();
#endif
}

static uint8_t sta_old = 0xff;
static uint8_t wmsta_old = 0xff;
static uint8_t isPlanStop_old = 0xff;
static uint8_t isPlanRepair_old = 0xff;
static uint8_t isProductStart_old = 0xff;
static uint8_t productName_old[20];
static void statusChangedUpdate(void){

    //声明一个临时状态点
    status_changed_point_t  temp_p;
    name_changed_point_t temp_n;
    if(sta_old != pvar.run_sta){
        //设备运行状态发生变化
        temp_p.sta =  pvar.run_sta;
        temp_p.time = time(NULL);
        //将状态变化点存入队列
        if(mqStatus != RT_NULL){
            rt_mq_send(mqStatus,&temp_p, sizeof(status_changed_point_t));
        }

        sta_old = pvar.run_sta;
    }
    if(wmsta_old != pvar.wm_sta){
        //焊接状态发生变化
        temp_p.sta = pvar.wm_sta;
        temp_p.time = time(NULL);
        //将状态变化点存入消息队列
        if(mqWMStatus != RT_NULL){
            rt_mq_send(mqWMStatus,&temp_p, sizeof(status_changed_point_t));
        }
        wmsta_old = pvar.wm_sta;
    }
    if(isPlanStop_old != pvar.isPlanStop){
        temp_p.sta = pvar.isPlanStop;
        temp_p.time = time(NULL);
        //将状态变化点存入消息队列
        if(mqPlanStop != RT_NULL){
            rt_mq_send(mqPlanStop,&temp_p, sizeof(status_changed_point_t));
        }
        isPlanStop_old = pvar.isPlanStop;
    }
    if(isPlanRepair_old != pvar.isPlanRepair){
        temp_p.sta = pvar.isPlanRepair;
        temp_p.time = time(NULL);
        //将状态变化点存入消息队列
        if(mqPlanRepair != RT_NULL){
            rt_mq_send(mqPlanRepair,&temp_p, sizeof(status_changed_point_t));
        }
        isPlanRepair_old = pvar.isPlanRepair;
    }
//    if(rt_strcmp(productName_old, pvar.productName) !=0 && pvar.productName[0] !=0){
//        rt_memcpy(temp_n.name, pvar.productName, 20);
//        temp_n.time = time(NULL);
//        if(mqProductName != RT_NULL){
//            rt_mq_send(mqProductName,&temp_n, sizeof(name_changed_point_t));
//        }
//        rt_memcpy(productName_old, pvar.productName, 20);
//    }
    if(isProductStart_old != pvar.isProductStart){
        rt_memcpy(temp_n.name, pvar.productName, 20);
        temp_n.time = time(NULL);
        temp_n.status = pvar.isProductStart;
        if(mqProductName != RT_NULL){
            rt_mq_send(mqProductName,&temp_n, sizeof(name_changed_point_t));
        }
    }
    isProductStart_old = pvar.isProductStart;

}
static uint8_t devWmStatusUpdate(void){
    for(uint8_t index=0;index<4;index++){
        if(pvar.wm_current[index] > 20){
            pvar.wm_sta = 1;
            return 1;
        }
    }
    pvar.wm_sta = 0;
    return 0;
}
//每100ms 调用一次
static uint8_t devStatusUpdate(void)
{
    int isFillOk;
    int sum_ar,sum_mr,sum_e;
    int i;
    pvar.run_ar_sta = rt_pin_read(DIN5_PIN);
    pvar.run_mr_sta = rt_pin_read(DIN6_PIN);
    pvar.run_e_sta  = rt_pin_read(DIN7_PIN);
    if(devStatusBufferIndex < (20-1)){
        //数据还没填充好,继续填
        devStatusBufferIndex++;
        isFillOk  =0;
    }
    else{
        isFillOk = 1;
        //填满数据了,需要一次向前移动
        for( i=0;i<(20-1);i++){
            devStatusBuffer[0][i] = devStatusBuffer[0][i+1];
            devStatusBuffer[1][i] = devStatusBuffer[1][i+1];
            devStatusBuffer[2][i] = devStatusBuffer[2][i+1];
        }
    }
    //填充数据
    devStatusBuffer[0][devStatusBufferIndex] = pvar.run_ar_sta ;
    devStatusBuffer[1][devStatusBufferIndex] = pvar.run_mr_sta ;
    devStatusBuffer[2][devStatusBufferIndex] = pvar.run_e_sta ;
    sum_ar = 0;
    sum_mr =0;
    sum_e=0;
    //计算
    for(i=0;i<20;i++){
        sum_ar += devStatusBuffer[0][i];
        sum_mr += devStatusBuffer[1][i];
        sum_e += devStatusBuffer[2][i];
    }
    //判断
    if(isFillOk == 0 ){
        pvar.run_sta = wm_unknow;
        return pvar.run_sta;
    }
    else
    {
#if REG_LAMP_ABNORMAL
        //如果绿灯和黄灯都没亮,并且红灯亮了才是故障
        //绿灯常量
        if(sum_ar == 0){
            pvar.run_sta = wm_auto_run;
            return pvar.run_sta;
        }
        //黄灯常亮
        if(sum_mr == 0){
            pvar.run_sta = wm_m_run;
            return pvar.run_sta;
        }
        //绿灯闪,黄灯不亮
        if((sum_ar < 20)&&(sum_mr ==20)){
            pvar.run_sta = wm_auto_stop;
            return pvar.run_sta;
        }
        //黄灯闪,绿灯灭
        if((sum_mr < 20)&&(sum_ar ==20)){
            pvar.run_sta = wm_m_stop;
            return pvar.run_sta;
        }
        //三个灯都不亮
        if((sum_mr == 20)&&(sum_ar ==20)&&(sum_e ==20)){
            pvar.run_sta = wm_turn_off;
            return pvar.run_sta;
        }
        //红灯亮(最低优先级)
        if(sum_e < 20){
            pvar.run_sta = wm_force_stop;
            return pvar.run_sta;
        }
#else
        //正常情况,红灯亮就是故障
        //红灯亮(最高优先级)
        if(sum_e < 20){
            pvar.run_sta = wm_force_stop;
            return pvar.run_sta;
        }
        //绿灯常量
        if(sum_ar == 0) {
            pvar.run_sta = wm_auto_run;
            return pvar.run_sta;
        }
        //黄灯常亮
        if(sum_mr == 0){
            pvar.run_sta = wm_m_run;
            return pvar.run_sta;
        }
        //绿灯闪,黄灯不亮
        if((sum_ar < 20)&&(sum_mr ==20)&&(sum_e ==20)){
            pvar.run_sta = wm_auto_stop;
            return pvar.run_sta;
        }
        //黄灯闪,绿灯灭
        if((sum_mr < 20)&&(sum_ar ==20)&&(sum_e ==20)){
            pvar.run_sta = wm_m_stop;
            return pvar.run_sta;
        }
        //三个灯都不亮
        if((sum_mr == 20)&&(sum_ar ==20)&&(sum_e ==20)){
            pvar.run_sta = wm_turn_off;
            return pvar.run_sta;
        }

#endif
    }
    return 0;
}

int  monitorInit(void){

    mqStatus = rt_mq_create("mqSta",
                                    sizeof(status_changed_point_t),
                                    20,
                                    RT_IPC_FLAG_FIFO);
    if(mqStatus != RT_NULL){
        mqWMStatus = rt_mq_create("mqWMSta",
                                    sizeof(status_changed_point_t),
                                    20,
                                   RT_IPC_FLAG_FIFO);
        if(mqWMStatus != RT_NULL){
            evtReport = rt_event_create("evtRPT",RT_IPC_FLAG_PRIO);
            if(evtReport != RT_NULL){
                monitorTimer = rt_timer_create("monitor",\
                            onMonitorTimerTimeout , \
                            NULL,\
                            rt_tick_from_millisecond(DEFAULT_STATUS_UPDATE_INTERVAL),\
                            RT_TIMER_FLAG_PERIODIC);
                if(monitorTimer != NULL){
                    mqPlanStop = rt_mq_create("mqPlanS",
                                                sizeof(status_changed_point_t),
                                                20,
                                               RT_IPC_FLAG_FIFO);
                    if(mqPlanStop != RT_NULL){
                        mqPlanRepair = rt_mq_create("mqPlanR",
                                                    sizeof(status_changed_point_t),
                                                    20,
                                                   RT_IPC_FLAG_FIFO);
                        if(mqPlanRepair != RT_NULL){
                            mqProductName = rt_mq_create("mqName",
                                                        sizeof(name_changed_point_t),
                                                        20,
                                                       RT_IPC_FLAG_FIFO);
                            if (mqProductName != RT_NULL) {
                                rt_timer_start(monitorTimer);
                                pmcmBS = s7GetBaseStatus();
                            }
                        }

                    }
                    return 0;
                }
            }

        }
    }
    pvar.isReportSent = 0;

    return -1;
}

rb_sta_t* monitorGetpVar(void){
    return &pvar;
}


static daliy_report_base_t drb_;
static devDailyReport_t devDaliyReport;
static int daily_sm;

devDailyReport_t* getDevDailyReport(void){
    return &devDaliyReport;
}
void dailyReportUpdate(void){
    time_t now;
    struct tm* ptm_;
    now = time(NULL);

    //获取系统基本值
    rb_sta_t *pvar = monitorGetpVar();
    switch(daily_sm){
        case 0:
            //初始化日报计算变量
            drb_.time0 = (now/86400)*86400;
            drb_.old.time = drb_.time0;
            drb_.old.sta = 6;
            daily_sm = 1;
            break;
        case 1:
            if(drb_.old.sta != pvar->run_sta){
                //记录一个变化点
                drb_.st[drb_.old.sta] += now - drb_.old.time;
                drb_.old.time = now;
                drb_.old.sta = pvar->run_sta;
            }
            ptm_ = gmtime(&now);
            //UTC 19点以后即北京时间凌晨3点以后
            //如果日报被发送了，清除已经发送标志
            //等待第二天的凌晨3点继续发送报告
            if(ptm_->tm_hour > 19){
                if(pvar->isReportSent){
                    pvar->isReportSent = 0;
                }
            }
            //凌晨19点0分0秒(UTC)相当于北京时间第二天凌晨3点
            //这一个小时内，每秒都会判断日报是否被发送
            if(ptm_->tm_hour == 19 && pvar->isReportSent == 0){
                LOG_D("Time to generate Report!");
                rt_uint32_t recved = 0;
                if(evtReport != NULL){
                    //接收事件集，不清除
                    rt_event_recv(evtReport,
                                0x01,
                                RT_EVENT_FLAG_AND ,
                                RT_WAITING_NO,
                                &recved);
                    if(recved == 0){
                        //生成日报并清除
                        drb_.st[drb_.old.sta] += drb_.time0 + 86400 - drb_.old.time;
                        drb_.time0 = (now/86400)*86400;
                        drb_.old.time = drb_.time0;
                        drb_.old.sta = pvar->run_sta;
                        devDaliyReport.S0t = drb_.st[0];
                        devDaliyReport.S1t = drb_.st[1];
                        devDaliyReport.S2t = drb_.st[2];
                        devDaliyReport.S3t = drb_.st[3];
                        devDaliyReport.S4t = drb_.st[4];
                        devDaliyReport.S5t = drb_.st[5];
                        devDaliyReport.S6t =  86400 - (drb_.st[0] +drb_.st[1]+drb_.st[2]+drb_.st[3]+drb_.st[4]+drb_.st[5]);

                        LOG_I("Send evtReport event");
                        rt_event_send(evtReport,0x01);

                        for(int i=0;i<DEV_STA_TYPES_COUNTER;i++){
                            drb_.st[i] = 0;
                        }
                    }else{
                        LOG_I("Event evtReport is set,wait send report!");
                    }
                }else{
                    LOG_E("Event evtReport is NULL");
                }


            }
            break;
        default:
            break;
        }
}

