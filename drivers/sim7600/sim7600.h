/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-18     CXLTET01       the first version
 */
#ifndef DRIVERS_SIM7600_SIM7600_H_
#define DRIVERS_SIM7600_SIM7600_H_
#include <rtthread.h>
#include <stm32f1xx.h>


#define DEFAULT_RETRY_TIMES  5
#define DEFAULT_REPORT_INTERVAL  1000
#define DEFAULT_STATUS_UPDATE_INTERVAL  100

#define SIM7600_CMD_GAP_TIME 200

typedef struct{
    rt_bool_t isPowerOn;
    rt_bool_t isSMSDone;
    rt_bool_t isPBDone;
    rt_bool_t isSIMCardOK;
    rt_bool_t isMQTTConnected;
    rt_bool_t isSocketConnected;
    rt_bool_t isSubed;
    rt_bool_t isConfigOK;
    int32_t conmunicationErrorRetryTimes;

}MCMBaseStatus_t;

typedef enum{
    Operation_succeeded = 0,
    Unknown_error,
    Wrong_parameter,
    Wrong_date_and_time_calculated,
    Network_error,
    Time_zone_error,
    Time_out_error,
    AT_ERROR = -1

}SIM7600_Unsolicited_NTP_Codes;
#define TOPIC_MAX_LENGTH 32
#define CLIENTID_MAX_LENGTH 32

typedef struct{
    char pubTopic[TOPIC_MAX_LENGTH];
    char subTopic[TOPIC_MAX_LENGTH];
    char clientId0[CLIENTID_MAX_LENGTH];
    char clientId1[CLIENTID_MAX_LENGTH];
    int msgErrorCounter;
    int reportInterval;
}mqttClient_t;

void s7DeInit(void);
void s7Init(void);

int  s7Preconfig(void);
MCMBaseStatus_t* s7GetBaseStatus(void);

SIM7600_Unsolicited_NTP_Codes s7NTP_update(void);
int s7GetClock(struct tm*  t);
int s7GetSignalQuality(void);
void devMQTTClientInit(void);

int mqttInit(void);
int mqttSendMessage(int clientIndex,char* data,int qos);
int mqttReconnect(void);

#endif /* DRIVERS_SIM7600_SIM7600_H_ */
