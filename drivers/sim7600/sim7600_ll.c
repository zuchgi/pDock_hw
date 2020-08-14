/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-18     CXLTET01       the first version
 */
#include "sim7600.h"
#include <at.h>
#include <time.h>
#include <board.h>
#include <rtdevice.h>
#include <monitor/monitor.h>
#include <string.h>
#include <stdio.h>
#include "usrTime/usrTime.h"

#define DBG_TAG "sim7600"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static rb_sta_t* pvar;
static MCMBaseStatus_t mcmBase;
//用来检测模块重启是否成功
//1.复位模块，并开启定时器
//2.如果接收到pb down消息后则关闭该定时器
//3.如果没有接收到pb down 定时器超时，复位模块
static rt_timer_t mcmRebootTimer = NULL;
//用于检测服务器是否一致保持连接
//(server 会每10s发送"+ACK: 159101010"消息，消息内容为时间戳，topic为"common/msg")
//1.创建好连接后，开启定时器
//2.如果模块接收到ack命令则重启该定时器
//3.如果没有收到定时器超时复位模块
 rt_timer_t serverRebootTimer = NULL;

//private function



static void urcPowerOn(struct at_client *client,const char *data, rt_size_t size)
{
    LOG_I("SIM7600 Power ON!");
    mcmBase.isPowerOn = 1;
    pvar->isFirstStart = 1;
}

static void urcSMSDone(struct at_client *client,const char *data, rt_size_t size)
{
    /* 接收到服务器发送数据 */
    LOG_I("SIM7600 SMS DONE!");
    mcmBase.isSMSDone = 1;
}

static void urcPBDone(struct at_client *client,const char *data, rt_size_t size)
{

    /* 设备启动信息 */
    LOG_I("SIM7600 PB DONE!");
    mcmBase.isPBDone = 1;
    //关闭启动监控定时器
    if(mcmRebootTimer != NULL){
        rt_timer_stop(mcmRebootTimer);
    }

}

static void urcMQTTRxMessage(struct at_client *client,const char *data, rt_size_t size)
{
    /* 设备启动信息 */
    int device_id;
    int cmd_id;
    int value;
    int n=0;
    time_t time_ = 0;
    LOG_D("urcMQTTRxMessage !");
    LOG_D("data:%s,length:%d",data,size);
    if(strstr(data,"+CMD: ")){
        n = sscanf(data,"+CMD: %d,%d,%d",&device_id,&cmd_id,&value);
        if(n ==3){
            if(device_id == getBoardId()){
                switch(cmd_id){
                case 0:
                    rt_hw_cpu_reset();
                    break;
                case 1:
                    mqttReconnect();
                    break;
                case 2:
                    mqttReconnect();
                    break;

                }
            }
        }
    }
    else{
        if(strstr(data,"+ACK: ")){
            n = sscanf(data,"+ACK: %ld",&time_);
            if(n == 1){
                if(serverRebootTimer != NULL){
                    rt_timer_start(serverRebootTimer);
                }
                timingServerTimeSet(time_);
                rt_pin_write(LED1_PIN,1);
                rt_thread_mdelay(20);
                rt_pin_write(LED1_PIN,0);
            }
        }
    }

}

static struct at_urc urc_table[] = {
    {"RDY",      "\r\n",  urcPowerOn},
    {"SMS DONE", "\r\n",  urcSMSDone},
    {"PB DONE",  "\r\n",  urcPBDone},
    {"+ACK: ",  "\r\n",  urcMQTTRxMessage},
    {"+CMD: ",  "\r\n",  urcMQTTRxMessage},
};

static int atClientPortInit(void)
{
    /* 添加多种 URC 数据至 URC 列表中，当接收到同时匹配 URC 前缀和后缀的数据，执行 URC 函数  */
    at_set_urc_table(urc_table, sizeof(urc_table) / sizeof(urc_table[0]));
    return RT_EOK;
}
static void onMcmRebootTimerTimeout(void* parameter){
    LOG_E("MCM power on error! -> Reset!");
    s7DeInit();
}
static void onServerRebootTimerTimeout(void* parameter){
//    LOG_E("MCM Server ACK error! -> Reset!");
//    s7DeInit();
}

void s7Init(void){
    pvar =  monitorGetpVar();
    at_client_init("uart1",512);
    atClientPortInit();
    mcmBase.conmunicationErrorRetryTimes = DEFAULT_RETRY_TIMES;
    //创建定时器用于重启，
    //!!!!!!!!!!!非常重要，定时器需要使用RT_TIMER_FLAG_SOFT_TIMER模式
    mcmRebootTimer = rt_timer_create("mcm",\
            onMcmRebootTimerTimeout , \
                NULL,\
                rt_tick_from_millisecond(20000),\
                RT_TIMER_FLAG_ONE_SHOT|RT_TIMER_FLAG_SOFT_TIMER);
    serverRebootTimer = rt_timer_create("server",\
            onServerRebootTimerTimeout , \
                NULL,\
                rt_tick_from_millisecond(60000),\
                RT_TIMER_FLAG_ONE_SHOT|RT_TIMER_FLAG_SOFT_TIMER);
    s7DeInit();
}

void s7DeInit(void){
    mcmBase.isPowerOn = 0;
    mcmBase.isSMSDone = 0;
    mcmBase.isPBDone = 0;
    mcmBase.isSIMCardOK = 0;
    mcmBase.isSocketConnected = 0;
    mcmBase.isMQTTConnected = 0;
    mcmBase.isSubed = 0;
    mcmBase.isConfigOK = 0;
    LOG_I("MCM  Reset!");
    rt_pin_write(SIM7600_RESET_PIN,0);
    rt_thread_mdelay(500);
    rt_pin_write(SIM7600_RESET_PIN,1);
    rt_pin_write(LED2_PIN,0);
    //用于重启
    if(mcmRebootTimer != NULL){
        rt_timer_start(mcmRebootTimer);
    }
}

MCMBaseStatus_t* s7GetBaseStatus(void){
    return &mcmBase;
}



int  s7Preconfig(void){
    int isGetSQOK = 0;
    int retry_counter= 0;
    rt_thread_mdelay(SIM7600_CMD_GAP_TIME);
    //  ATE0 Echo mode off
    at_exec_cmd(NULL, "ATE0");
    rt_thread_mdelay(10);
    //  CNMP Preferred mode selection
    //  2 – Automatic
    //  13 – GSM Only
    //  14 – WCDMA Only
    //  38 – LTE Only
    at_exec_cmd(NULL, "AT+CNMP=2");
    rt_thread_mdelay(SIM7600_CMD_GAP_TIME);
    //CGDCONT Define PDP context
    at_exec_cmd(NULL, "AT+CGDCONT=1,\"IP\",\"CMNET\"");
    rt_thread_mdelay(300);
    while(isGetSQOK == 0){
        if(s7GetSignalQuality() != -1){
            //信号质量读取成功
            isGetSQOK = 1;
        }else {
            retry_counter ++;
            if(retry_counter >= DEFAULT_RETRY_TIMES){
                //尝试次数过多
                LOG_E("Get SQ try error!");
                return -1;
            }
        }
    }
    return 0;
}
#if 0
static int s7CheckSimCard(void){
    int rtval = 0;
    at_response_t resp = RT_NULL;
    resp = at_create_resp(128, 0, rt_tick_from_millisecond(1000));
    at_exec_cmd(resp,"AT+CPIN?");
    if(at_resp_get_line_by_kw(resp,"READY") == RT_NULL)
    {
         LOG_E("NO SIM Card!");
         rtval = -1;
    }else{
        LOG_D("SIM7600 SIMCard Inserted !");
        mcmBase.isSIMCardOK = 1;
    }
    at_delete_resp(resp);
    return rtval;
}
#endif

int s7GetClock(struct tm*  t){
    at_response_t resp = RT_NULL;
    int y,m,d,hh,mm,ss,zz;
    int rtval;
    resp = at_create_resp(256, 4, rt_tick_from_millisecond(2000));
    at_exec_cmd(resp, "AT+CCLK?");
    int n = at_resp_parse_line_args_by_kw(resp, \
            "+CCLK",\
            "+CCLK: \"%d/%d/%d,%d:%d:%d+%d\"",\
            &y,&m,&d,&hh,&mm,&ss,&zz);
    if(n == 7){
        t->tm_year = y+2000-1900;
        t->tm_mon = m-1;
        t->tm_mday = d;
        t->tm_hour = hh;
        t->tm_min = mm;
        t->tm_sec = ss;
        t->tm_isdst =0;
        t->tm_wday =0;
        t->tm_yday = 0;
        LOG_I("Current time : %04d-%02d-%02d %02d:%02d:%02d+%02d",y+2000,m,d,hh,mm,ss,zz);
        rtval = 0;
    }else{
        LOG_E("CSQ AT ERROR,get clock error!");
        rtval = -1;
    }
    at_delete_resp(resp);
    return rtval;
}

SIM7600_Unsolicited_NTP_Codes s7NTP_update(void){
    at_response_t resp = RT_NULL;
    int error_code;
    //发送命令设置NTP SERVER
    at_exec_cmd(NULL, "AT+CNTP= \"ntp7.aliyun.com\",0");
    rt_thread_mdelay(SIM7600_CMD_GAP_TIME);

    resp = at_create_resp(256, 4, rt_tick_from_millisecond(3000));
    at_exec_cmd(resp, "AT+CNTP");
    int n = at_resp_parse_line_args_by_kw(resp, "+CNTP","+CNTP: %d",&error_code);
    at_delete_resp(resp);
    if(n != 1){
        LOG_E("NTP AT ERROR! NTP update error!");
        return AT_ERROR;
    }
    else{
        LOG_I("NTP status : %d",error_code);
        return error_code;
    }



}
int s7GetSignalQuality(void){
    at_response_t resp = RT_NULL;
    int rtval;
    int a=0,b=0;
    int rssi;
    resp = at_create_resp(256, 4, rt_tick_from_millisecond(2000));
    at_exec_cmd(resp, "AT+CSQ");
    int n = at_resp_parse_line_args_by_kw(resp, "+CSQ","+CSQ: %d,%d",&a,&b);
    if(n==2 && a>=0 ){
        if(a < 99){
         rssi = a*2-113;
        }else{
            rssi =(a-100)-116;
        }
        rtval = 0;
        LOG_I("Rssi:%ddbm\r\n",rssi);
    }else{
        LOG_E("CSQ AT ERROR!,get signal quality error!");
        rtval = -1;
    }
    at_delete_resp(resp);
    return rtval;
}
