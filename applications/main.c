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
#include <at.h>
#include <sim7600/sim7600.h>
#include <usrTime/usrTime.h>
#include <monitor/monitor.h>
#include <message/message.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       4096
#define THREAD_TIMESLICE        5

//���������MQTTû�б����ӻᱻ����
//1.ͬ������ȡʱ��
static int  serviceInit(void){
    int err =0;
//    NTPͬ�������ӻ�ȡʱ��
    err = timingSystemUsrInit();
    if( err == 0){
        //��ȡTopic ��Client ID
        devMQTTClientInit();
        //����MQTT����
        err = mqttInit();
        return err;
    }
    return -1;

}
static int appInit(void){
    monitorInit();
    return 0;
}

static rt_thread_t tidReport = RT_NULL;
static void reportThreadEntry(void *parameter)
{
    //������Ϣ�����Ϣ����
    messageInit();

    while (1)
    {
        //����Ѿ����ӣ����ɲ�������Ϣ
        messageUpdate();
        rt_thread_mdelay(DEFAULT_REPORT_INTERVAL);
    }
}

int main(void)
{
    uint8_t bling_counter;
    MCMBaseStatus_t* pmcmBS = s7GetBaseStatus();

    /*
     * 1.gpio init
     * 2.sim7600  AT client init
     */
    platformInit();
    /*
     **  ������ʱ���������Բɼ�����
     */
    appInit();
    tidReport = rt_thread_create("report",
                            reportThreadEntry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY,
                            THREAD_TIMESLICE);
    if (tidReport != RT_NULL)
        rt_thread_startup(tidReport);


    while(1){
        //������
        if(bling_counter > 9){
            bling_counter  = 0;
            rt_pin_write(LED0_PIN,1);
        }else{
            rt_pin_write(LED0_PIN,0);
        }
        bling_counter ++;
        //Ԥ����
        if(pmcmBS->isPBDone && pmcmBS->isConfigOK == 0 ){
            if(s7Preconfig() != -1){
                pmcmBS->isConfigOK = 1;
            }
            else {
                //Ԥ����ʧ�ܣ�����
                LOG_E("SIM7600 pre config error!");
                LOG_I("SIM7600 Reboot!");
                s7DeInit();
            }
        }


        if( pmcmBS->isConfigOK &&
            pmcmBS->isMQTTConnected == 0){
            serviceInit();
        }
        rt_thread_mdelay(100);
    }
}

