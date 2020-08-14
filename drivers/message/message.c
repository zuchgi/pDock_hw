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
#include <monitor/monitor.h>
#include "message.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

#define DBG_TAG "message"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

extern rt_mq_t mqStatus;
extern rt_mq_t mqWMStatus;
extern rt_event_t evtReport;

extern rt_mq_t mqPlanStop ;
extern rt_mq_t mqPlanRepair ;
extern rt_mq_t mqProductName ;


static void msgGenerator(msg_type_e type,char* msg_buf,void* p){

    time_t now;
    struct tm* t;
    rb_sta_t *pvar = monitorGetpVar();
    devDailyReport_t* devDR = getDevDailyReport();
    now = time(NULL);
    t =gmtime(&now);
    status_changed_point_t* p_sdata =p;
    name_changed_point_t* p_ndata = p;

    switch(type){
        case devMeasurement:
            snprintf(msg_buf,MSG_LEN,"{\"Type\":\"Measurement\",\"Time\":\"%04d-%02d-%02dT%02d:%02d:%02dZ\",\"ID\":%d,\"U0\":%0.2f,\"U1\":%0.2f,\"U2\":%0.2f,\"U3\":%0.2f,\"I0\":%0.1f,\"I1\":%0.1f,\"I2\":%0.1f,\"I3\":%0.1f,\"Speed\":%.2f,\"Status\":%d,\"Length\":0}",\
                t->tm_year+1900,\
                t->tm_mon+1,\
                t->tm_mday,\
                t->tm_hour,\
                t->tm_min,\
                t->tm_sec,\
                pvar->addr,\
                pvar->wm_voltage[0],\
                pvar->wm_voltage[1],\
                pvar->wm_voltage[2],\
                pvar->wm_voltage[3],\
                pvar->wm_current[0],\
                pvar->wm_current[1],\
                pvar->wm_current[2],\
                pvar->wm_current[3],\
                pvar->speed,\
                pvar->run_sta);
            break;
        case devDailyReport:
            rt_snprintf(msg_buf,MSG_LEN,"{\"Type\":\"Report\",\"Time\":\"%04d-%02d-%02dT%02d:%02d:%02dZ\",\"ID\":%d,\"S0T\":%ld,\"S1T\":%ld,\"S2T\":%ld,\"S3T\":%ld,\"S4T\":%ld,\"S5T\":%ld,\"S6T\":%ld,\"S7T\":%ld,\"S8T\":%ld}",\
                        t->tm_year+1900,\
                        t->tm_mon+1,\
                        t->tm_mday,\
                        t->tm_hour,\
                        t->tm_min,\
                        t->tm_sec,\
                        pvar->addr,\
                        devDR->S0t,\
                        devDR->S1t,\
                        devDR->S2t,\
                        devDR->S3t,\
                        devDR->S4t,\
                        devDR->S5t,\
                        devDR->S6t,\
                        devDR->S7t,\
                        devDR->S8t);
            break;
        case devStatuesChanged:

            rt_snprintf(msg_buf,MSG_LEN,"{\"Type\":\"StatusChanged\",\"Time\":\"%04d-%02d-%02dT%02d:%02d:%02dZ\",\"ID\":%d,\"Status\":%d}",\
                        t->tm_year+1900,\
                        t->tm_mon+1,\
                        t->tm_mday,\
                        t->tm_hour,\
                        t->tm_min,\
                        t->tm_sec,\
                        pvar->addr,\
                        p_sdata->sta);
            break;
        case devPlanStopStatusChanged:
            rt_snprintf(msg_buf,MSG_LEN,"{\"Type\":\"Plan\",\"Time\":\"%04d-%02d-%02dT%02d:%02d:%02dZ\",\"ID\":%d,\"Status\":%d,\"planType\":\"stop\"}",\
                        t->tm_year+1900,\
                        t->tm_mon+1,\
                        t->tm_mday,\
                        t->tm_hour,\
                        t->tm_min,\
                        t->tm_sec,\
                        pvar->addr,\
                        p_sdata->sta);
            break;
        case devPlanRepairStatusChanged:
            rt_snprintf(msg_buf,MSG_LEN,"{\"Type\":\"Plan\",\"Time\":\"%04d-%02d-%02dT%02d:%02d:%02dZ\",\"ID\":%d,\"Status\":%d,\"planType\":\"repair\"}",\
                        t->tm_year+1900,\
                        t->tm_mon+1,\
                        t->tm_mday,\
                        t->tm_hour,\
                        t->tm_min,\
                        t->tm_sec,\
                        pvar->addr,\
                        p_sdata->sta);
            break;
        case devProductNameChanged:
            rt_snprintf(msg_buf,MSG_LEN,"{\"Type\":\"Model\",\"Time\":\"%04d-%02d-%02dT%02d:%02d:%02dZ\",\"ID\":%d,\"Model\":\"%s\",\"Status\":%d}",\
                        t->tm_year+1900,\
                        t->tm_mon+1,\
                        t->tm_mday,\
                        t->tm_hour,\
                        t->tm_min,\
                        t->tm_sec,\
                        pvar->addr,\
                        p_ndata->name,\
                        p_ndata->status);
            break;
        case devWeldingmachinStatusChanged:
            rt_snprintf(msg_buf,MSG_LEN,"{\"Type\":\"WMStatusChanged\",\"Time\":\"%04d-%02d-%02dT%02d:%02d:%02dZ\",\"ID\":%d,\"Status\":%d}",\
                        t->tm_year+1900,\
                        t->tm_mon+1,\
                        t->tm_mday,\
                        t->tm_hour,\
                        t->tm_min,\
                        t->tm_sec,\
                        pvar->addr,\
                        p_sdata->sta);
            break;
        case devOnline:
            rt_snprintf(msg_buf,MSG_LEN,"{\"Type\":\"Online\",\"Time\":\"%04d-%02d-%02dT%02d:%02d:%02dZ\",\"ID\":%d}",\
                        t->tm_year+1900,\
                        t->tm_mon+1,\
                        t->tm_mday,\
                        t->tm_hour,\
                        t->tm_min,\
                        t->tm_sec,\
                        pvar->addr);
            break;
        default:
            break;
        }
}


char out_buf[MSG_LEN];

static rt_mq_t mqDaliyReport = RT_NULL;
void messageInit(void){

    mqDaliyReport = rt_mq_create("mqRPT",
                                sizeof(devDailyReport_t),
                                5,
                                RT_IPC_FLAG_FIFO);

}

void messageUpdate(void){

    //获取SIM7600模块基本状态
    MCMBaseStatus_t* pmcmBS = s7GetBaseStatus();
    //获取测量板基本值
    rb_sta_t* pvar =  monitorGetpVar();
    //声明一个临时状态点
    status_changed_point_t  temp_p;
    //消息发送错误计数器
    int tx_error_conter = 0;
    rt_uint32_t recved = 0;
    //日报处理和计算
    dailyReportUpdate();
    //send devMeasurement message
    if(pmcmBS->isMQTTConnected){
        //第一次连接成功，发送上线消息
        if(pvar->isFirstStart){
            //产生对应的消息帧
            msgGenerator(devOnline,out_buf,RT_NULL);
            //发送消息
            while(mqttSendMessage(0,out_buf,0) != 0 && tx_error_conter < PUBLISH_ERROR_TIME){
               //发送失败计数
                tx_error_conter ++;
                rt_thread_mdelay(1000);
            }
            //
            if(tx_error_conter < PUBLISH_ERROR_TIME ){
                //Send OK
                pvar->isFirstStart = 0;
                LOG_D("Oline MSG send OK!");
            }else{
                //应用层消息发送失败
                LOG_E("Oline MSG send Error!");
            }
        }
        //延时
        rt_thread_mdelay(10);
        //产生并发送实时测量值，不对判断发送是否成功
        msgGenerator(devMeasurement,out_buf,RT_NULL);
        mqttSendMessage(0,out_buf,0);
        //等待消息队列中有状态变化点
        if (mqStatus != RT_NULL) {
            if(rt_mq_recv(mqStatus,&temp_p, sizeof(status_changed_point_t),RT_WAITING_NO) == RT_EOK){
                msgGenerator(devStatuesChanged,out_buf,&temp_p);
                while(mqttSendMessage(0,out_buf,0) != 0 && tx_error_conter < PUBLISH_ERROR_TIME){
                    tx_error_conter ++;
                    rt_thread_mdelay(1000);
                }
                rt_thread_mdelay(10);
                if(tx_error_conter < PUBLISH_ERROR_TIME ){
                    //Send OK
                    LOG_D("Status MSG send OK!");
                }else{
                    LOG_E("Status MSG send Error!");
                    //Send Error rewrite data to mq
                    //应用层消息发送失败了，需要把消息再放入消息队列
                    rt_mq_send(mqStatus,&temp_p, sizeof(status_changed_point_t));
                }
            }
        }
        //焊机焊接状态的变化
        if(mqWMStatus != RT_NULL){
            if(rt_mq_recv(mqWMStatus,&temp_p, sizeof(status_changed_point_t),RT_WAITING_NO) == RT_EOK){
                msgGenerator(devWeldingmachinStatusChanged,out_buf,&temp_p);
                while(mqttSendMessage(0,out_buf,0) != 0 && tx_error_conter < PUBLISH_ERROR_TIME){
                    tx_error_conter ++;
                    rt_thread_mdelay(1000);
                }
                rt_thread_mdelay(10);
                if(tx_error_conter < PUBLISH_ERROR_TIME ){
                    //Send OK
                    LOG_D("wmStatus MSG send OK!");
                }else{
                    LOG_E("wmStatus MSG send Error!");
                    //Send Error rewrite data to mq
                    rt_mq_send(mqWMStatus,&temp_p, sizeof(status_changed_point_t));
                }
            }
        }

        //计划维修状态的变化
        if(mqPlanRepair != RT_NULL){
            if(rt_mq_recv(mqPlanRepair,&temp_p, sizeof(status_changed_point_t),RT_WAITING_NO) == RT_EOK){
                msgGenerator(devPlanRepairStatusChanged,out_buf,&temp_p);
                while(mqttSendMessage(0,out_buf,0) != 0 && tx_error_conter < PUBLISH_ERROR_TIME){
                    tx_error_conter ++;
                    rt_thread_mdelay(1000);
                }
                rt_thread_mdelay(10);
                if(tx_error_conter < PUBLISH_ERROR_TIME ){
                    //Send OK
                    LOG_D("mqPlanRepair MSG send OK!");
                }else{
                    LOG_E("mqPlanRepair MSG send Error!");
                    //Send Error rewrite data to mq
                    rt_mq_send(mqPlanRepair,&temp_p, sizeof(status_changed_point_t));
                }
            }
        }

        //计划停机状态的变化
        if(mqPlanStop != RT_NULL ){
            if(rt_mq_recv(mqPlanStop,&temp_p, sizeof(status_changed_point_t),RT_WAITING_NO) == RT_EOK){
                msgGenerator(devPlanStopStatusChanged,out_buf,&temp_p);
                while(mqttSendMessage(0,out_buf,0) != 0 && tx_error_conter < PUBLISH_ERROR_TIME){
                    tx_error_conter ++;
                    rt_thread_mdelay(1000);
                }
                rt_thread_mdelay(10);
                if(tx_error_conter < PUBLISH_ERROR_TIME ){
                    //Send OK
                    LOG_D("mqPlanStop MSG send OK!");
                }else{
                    LOG_E("mqPlanStop MSG send Error!");
                    //Send Error rewrite data to mq
                    rt_mq_send(mqPlanStop,&temp_p, sizeof(status_changed_point_t));
                }
            }
        }
        //产品型号状态的变化
         if(mqProductName != RT_NULL ){
             if(rt_mq_recv(mqProductName,&temp_p, sizeof(name_changed_point_t),RT_WAITING_NO) == RT_EOK){
                 msgGenerator(devProductNameChanged,out_buf,&temp_p);
                 while(mqttSendMessage(0,out_buf,0) != 0 && tx_error_conter < PUBLISH_ERROR_TIME){
                     tx_error_conter ++;
                     rt_thread_mdelay(1000);
                 }
                 rt_thread_mdelay(10);
                 if(tx_error_conter < PUBLISH_ERROR_TIME ){
                     //Send OK
                     LOG_D("mqProductName MSG send OK!");
                 }else{
                     LOG_E("mqProductName MSG send Error!");
                     //Send Error rewrite data to mq
                     rt_mq_send(mqProductName,&temp_p, sizeof(name_changed_point_t));
                 }
             }
         }


        //接收并清除事件
//        if(evtReport != RT_NULL){
//            rt_event_recv(evtReport,
//                                0x01,
//                                RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
//                                RT_WAITING_NO,
//                                &recved);
//            if(recved & 0x01){
//                //接收到0x01事件
//                LOG_I("Start send Report!");
//                msgGenerator(devDailyReport,out_buf,&temp_p);
//                while(mqttSendMessage(0,out_buf,0) != 0 && tx_error_conter < PUBLISH_ERROR_TIME){
//                    tx_error_conter ++;
//                    rt_thread_mdelay(1000);
//                }
//                rt_thread_mdelay(10);
//                if(tx_error_conter < PUBLISH_ERROR_TIME ){
//                    //Send OK
//                    LOG_D("Report MSG send OK!");
//                    pvar->isReportSent = 1;
//
//                }else{
//                    //再把事件置位
//                    rt_event_send(evtReport,0x01);
//                    LOG_E("Report MSG send Error!");
//                }
//
//            }
//        }
    }
}
