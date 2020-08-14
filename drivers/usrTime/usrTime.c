/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-19     CXLTET01       the first version
 */
#include <sim7600/sim7600.h>
#include <time.h>


#define DBG_TAG "sim7600"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>


int timingSystemUsrInit(void){
    struct tm    pCalendar;
    SIM7600_Unsolicited_NTP_Codes err = s7NTP_update();
    if(err == Operation_succeeded){
        if(s7GetClock(&pCalendar) !=0 ){
            LOG_E("Clock Init Error! -- get clock error！");
            return -1;
        }else {
            set_date(pCalendar.tm_year+1900,pCalendar.tm_mon+1,pCalendar.tm_mday);
            set_time(pCalendar.tm_hour,pCalendar.tm_min,pCalendar.tm_sec);
            return 0;
        }
    }
    else {
        LOG_E("Clock Init Error! -- NTP update error :%d！",err);
        return -1;
    }
}

int timingServerTimeSet(time_t now){
    struct tm*    t = gmtime(&now);
    LOG_D("%04d-%02d-%02dT%02d:%02d:%02dZ",\
                t->tm_year+1900,\
                t->tm_mon+1,\
                t->tm_mday,\
                t->tm_hour,\
                t->tm_min,\
                t->tm_sec);
    set_date(t->tm_year+1900,t->tm_mon+1,t->tm_mday);
    set_time(t->tm_hour,t->tm_min,t->tm_sec);
    return 0;

}
