/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-19     CXLTET01       the first version
 */
#include "sim7600.h"
#include <at.h>
#include <time.h>
#include <board.h>
#include <rtdevice.h>
#include <stdio.h>
#include <string.h>

#define DBG_TAG "sim7600_mqtt"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>


const char* sim7600ErrorInfo[32]={
        "0 successful",
        "1 failed",
        "2 bad UTF-8 string",
        "3 sock connect fail",
        "4 sock create fail",
        "5 sock close fail",
        "6 message receive fail",
        "7 network open fail",
        "8 network close fail",
        "9 network not opened",
        "10 client index error",
        "11 no connection",
        "12 invalid parameter",
        "13 not supported operation",
        "14 client is busy",
        "15 require connection fail",
        "16 sock sending fail",
        "17 timeout",
        "18 topic is empty",
        "19 client is used",
        "20 client not acquire resource",
        "21 client not release",
        "22 length out of range",
        "23 network is opened",
        "24 packet fail",
        "25 DNS error",
        "26 socket is closed by server",
        "27 connection refused: unaccepted protocol version",
        "28 connection refused: identifier rejected",
        "29 connection refused: server unavailable",
        "30 connection refused: bad user name or password",
        "31 connection refused: not authorized"
};

static mqttClient_t mqttClient;
static MCMBaseStatus_t* s7bs;
extern rt_timer_t serverRebootTimer;

static  char*  getSIM7600ErrorInfo(int err){
    if(err >=0 && err < 32){
        return (char*)sim7600ErrorInfo[err];
    }else{
        return "Unknown error OR Get error code failed!";
    }
}

static void getDevPubTopic(char* buf){
    rt_snprintf(buf,TOPIC_MAX_LENGTH,"CAT/device/%d/tx",getBoardId());
}
static void getDevSubTopic(char* buf){
    rt_snprintf(buf,TOPIC_MAX_LENGTH,"common/msg",getBoardId());
}
static void getDevClientID(int clientId,char* buf){
    rt_snprintf(buf,CLIENTID_MAX_LENGTH,"CAT.dev.id-%d-%d",getBoardId(),clientId);
}
void devMQTTClientInit(void){
    s7bs = s7GetBaseStatus();
    getDevPubTopic(mqttClient.pubTopic);
    getDevSubTopic(mqttClient.subTopic);
    getDevClientID(0,mqttClient.clientId0);
    getDevClientID(1,mqttClient.clientId1);
    mqttClient.reportInterval = DEFAULT_REPORT_INTERVAL;
}


static void onMQTTStarted(void)
{
    LOG_I("Mqtt Start OK!");
}
static void onMQTTStartError(void)
{
    LOG_E("Mqtt  Start Error!");
    s7bs->isMQTTConnected = 0;
    rt_pin_write(LED2_PIN,0);
}
#if USE_MQTT_STOP
static void onMQTTStoped(void)
{
    LOG_D("Mqtt Stop OK!");
    s7bs->isMQTTConnected = 0;
    rt_pin_write(LED2_PIN,0);
}
static void onMQTTStopError(void)
{
    LOG_E("Mqtt  Stop Error!");
}
#endif

static void onMQTTConnected(int clientid)
{
    LOG_I("Mqtt client%d connected!",clientid);
    if(clientid == 1){
        s7bs->isMQTTConnected = 1;
        rt_pin_write(LED2_PIN,1);
        if(serverRebootTimer != NULL){
            rt_timer_start(serverRebootTimer);
        }
    }

}
static void onMQTTConnectError(int clientid)
{
    LOG_E("Mqtt client%d connect Error!",clientid);
    s7bs->isMQTTConnected = 0;
    rt_pin_write(LED2_PIN,0);
}

static void onMQTTSubOK(int clientid)
{
    LOG_I("Mqtt client%d Sub OK!",clientid);
    s7bs->isSubed = 1;
}
static void onMQTTSubError(int clientid)
{
    LOG_E("Mqtt client%d Sub Error!",clientid);
    s7bs->isSubed = 0;
}

static rt_bool_t onMessageSentError(void){
    mqttClient.msgErrorCounter ++;
    if(mqttClient.msgErrorCounter >= 5){
        LOG_E("MQTT Message sent Error!");
        mqttClient.msgErrorCounter = 0;
        s7bs->isMQTTConnected = 0;
        rt_pin_write(LED2_PIN,0);
        return 1;
    }
    return 0;
}

static void onMessageSentOK(void){
    mqttClient.msgErrorCounter = 0;
    LOG_D("MQTT Message sent OK!");
}


static void onMQTTRetryError(void){
    mqttReconnect();

}
static int mqttStart(void) {
    at_response_t resp = RT_NULL;
    int rtval =0;
    int result;
    resp = at_create_resp(128, 4, rt_tick_from_millisecond(5000));
    at_exec_cmd(resp, "AT+CMQTTSTART");
    int n = at_resp_parse_line_args_by_kw(resp, "+CMQTTSTART","+CMQTTSTART: %d",&result);
    if(n==1 && result ==0 ){
        onMQTTStarted();
        rtval = 0;
    }else{
        onMQTTStartError();
        if(n !=1){
            rtval = -1;
        }
        else{
            rtval = result;
            LOG_E("Error code: %s",getSIM7600ErrorInfo(result));
        }
    }
    at_delete_resp(resp);
    return rtval;
}
#if USE_MQTT_STOP
static int mqttStop(void) {

    at_response_t resp = RT_NULL;
    int rtval =0 ;
    int result;
    resp = at_create_resp(128, 4, rt_tick_from_millisecond(200));
    at_exec_cmd(resp, "AT+CMQTTSTOP");
    int n = at_resp_parse_line_args_by_kw(resp, "+CMQTTSTOP","+CMQTTSTOP: %d",&result);
    if(n==1 && result ==0 ){
        onMQTTStoped();
        rtval = 0;
    }else{
        onMQTTStopError();

        if(n !=1){
            rtval = -1;
        }
        else{
            rtval = result;
            LOG_E("Error code: %s",getSIM7600ErrorInfo(result));
        }
    }
    at_delete_resp(resp);
    return rtval;
}
#endif

static int mqttACCQ(int clientIndex,char* clientId) {
    int rtval = 0;
    at_exec_cmd(NULL, "AT+CMQTTACCQ=%d,\"%s\"",clientIndex,clientId);
    return rtval;
}

static int mqttSetTopic(int clientIndex,char* topic){
    int rtval = 0;
    at_exec_cmd(NULL, "AT+CMQTTTOPIC=%d,%d\r\n",clientIndex,strlen(topic));
    rt_thread_mdelay(10);
    at_exec_cmd(NULL, topic);
    return rtval;
}

static int mqttSetPayload(int clientIndex,char* data){
    int rtval = 0;
    at_exec_cmd(NULL, "AT+CMQTTPAYLOAD=%d,%d\r\n",clientIndex,strlen(data));
    rt_thread_mdelay(10);
    at_exec_cmd(NULL, data);
    return rtval;
}

static int mqttConnect(int clientIndex,char* ip,int time,int type,char* name,char* pwd) {
    at_response_t resp = RT_NULL;
    int rtval =0;
    int rtClientIndex,err;
    resp = at_create_resp(128, 4, rt_tick_from_millisecond(5000));
    at_exec_cmd(resp, "AT+CMQTTCONNECT=%d,\"tcp://%s\",%d,%d,\"%s\",\"%s\"\r\n",clientIndex,ip,time,type,name,pwd);
    int n = at_resp_parse_line_args_by_kw(resp, "+CMQTTCONNECT","+CMQTTCONNECT: %d,%d",&rtClientIndex,&err);
    if(n==2 && err == 0 ){
        onMQTTConnected(rtClientIndex);
    }else{
        onMQTTConnectError(rtClientIndex);
        if(n !=1){
            rtval = -1;
        }
        else{
            rtval = err;
            LOG_E("Error code: %s",getSIM7600ErrorInfo(err));
        }
    }
    at_delete_resp(resp);
    return rtval;
}

static int mqttSendPub2Server(int clientIndex,int qos,int interval) {

    at_response_t resp = RT_NULL;
    int rtval = 0;
    int rtClientIndex,err;
    resp = at_create_resp(128, 4, rt_tick_from_millisecond(2000));
    at_exec_cmd(resp, "AT+CMQTTPUB=%d,%d,%d\r\n",clientIndex,qos,interval);
    int n = at_resp_parse_line_args_by_kw(resp, "+CMQTTPUB","+CMQTTPUB: %d,%d",&rtClientIndex,&err);
    if(n==2 && err ==0 ){
        onMessageSentOK();
    }else{
        if(onMessageSentError()){
            onMQTTRetryError();
        }
        if(n !=1){
            rtval = -1;
        }
        else{
            rtval = err;
            LOG_E("Error code: %s",getSIM7600ErrorInfo(err));
        }
    }
    at_delete_resp(resp);
    return rtval;
}

static int mqttSetSubTopic(int clientIndex,char* topic,int qos){

    at_response_t resp = RT_NULL;
    int rtval = 0;
    int rtClientIndex,err;
    resp = at_create_resp(128, 4, rt_tick_from_millisecond(5000));
    at_exec_cmd(NULL, "AT+CMQTTSUB=%d,%d,%d\r\n",clientIndex,strlen(topic),qos);
    rt_thread_mdelay(10);
    at_exec_cmd(resp,topic);
    int n = at_resp_parse_line_args_by_kw(resp, "+CMQTTSUB","+CMQTTSUB: %d,%d",&rtClientIndex,&err);
    if(n==2 && err ==0 ){
       onMQTTSubOK(rtClientIndex);
    }else{
       onMQTTSubError(rtClientIndex);
       if(n != 2){
           rtval = -1;
       }
       else{
           rtval = err;
           LOG_E("Error code: %s",getSIM7600ErrorInfo(err));
       }
    }
    at_delete_resp(resp);
    return rtval;
}


int mqttSendMessage(int clientIndex,char* data,int qos){
    int err;
    err = mqttSetTopic(clientIndex,mqttClient.pubTopic);
    if(err == 0){
        rt_thread_mdelay(10);
        err = mqttSetPayload(clientIndex,data);
        if(err == 0){
            rt_thread_mdelay(10);
            err  =  mqttSendPub2Server(clientIndex,qos,60);
        }
    }
    return err;
}

int mqttInit(void){
    rt_thread_mdelay(20);
    int err = mqttStart();
    if(err == 0 ){
        rt_thread_mdelay(20);
        mqttACCQ(0,mqttClient.clientId0);
        rt_thread_mdelay(20);
        mqttACCQ(1,mqttClient.clientId1);
        if(err == 0 ){
            rt_thread_mdelay(20);
            err = mqttConnect(0,"47.93.209.68",60,1,"pussion","pussion");
            if(err == 0 ){
                rt_thread_mdelay(20);
                err = mqttConnect(1,"47.93.209.68",60,1,"pussion","pussion");
                if(err == 0){
                    rt_thread_mdelay(20);
                    err = mqttSetSubTopic(1,mqttClient.subTopic,0);
                    if(err != 0){
                        LOG_E("MQTT Client1 sub Error!");
                    }
                }else {
                    LOG_E("MQTT Client1 Connect Error!");
                }
            }else {
                LOG_E("MQTT Client0 Connect Error!");
            }
        }
    }
    if(err != 0){
        LOG_E("MQTT Start Error!");
        LOG_I("MQTT Reconnect!");
        mqttReconnect();
    }
    return err;
}

int mqttReconnect(void){
    LOG_I("MQTT Reconnecting!");
    s7DeInit();
    return 0;
}
