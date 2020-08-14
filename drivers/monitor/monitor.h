/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-19     CXLTET01       the first version
 */
#ifndef DRIVERS_MONITOR_H_
#define DRIVERS_MONITOR_H_
#include <rtthread.h>
#include <stm32f1xx.h>

#define DEV_STA_TYPES_COUNTER 9
//1 不管红灯状态，如果绿灯亮则运行 0 红灯具有最好优先级
#define REG_LAMP_ABNORMAL 1
//电路板板载基准电压
#define VREF (2.5)


typedef struct{
    int run_ar_sta;
    int run_mr_sta;
    int run_e_sta;
    int run_sta;
    int wm_sta;
    int isFirstStart;
    float vin[16];
    float wm_current[8];
    float wm_voltage[4];
    int addr;
    float speed;
    float isReportSent;
    uint8_t isPlanStop;
    uint8_t isPlanRepair;
    uint8_t isProductStart;
    uint8_t gap0[20];//不使用
    uint8_t gap1[20];//不使用
    uint8_t productName[20];
    uint8_t gap2[20];//不使用
    uint8_t gap3[20];//不使用
}rb_sta_t;

typedef enum{
    wm_turn_off = 0,
    wm_auto_run = 1,
    wm_auto_stop = 2,
    wm_m_run = 3,
    wm_m_stop = 4,
    wm_force_stop = 5,
    wm_unknow = 6
}wm_sta;
typedef struct{
    int sta;
    int rev;
    time_t time;

}status_changed_point_t;
typedef struct{
    uint8_t name[20];
    time_t time;
    int status;
    int rev;


}name_changed_point_t;
typedef struct{
    status_changed_point_t old;
    time_t time0;
    uint32_t st[DEV_STA_TYPES_COUNTER];

}daliy_report_base_t;

typedef struct{
    uint32_t S0t;
    uint32_t S1t;
    uint32_t S2t;
    uint32_t S3t;
    uint32_t S4t;
    uint32_t S5t;
    uint32_t S6t;
    uint32_t S7t;
    uint32_t S8t;
    time_t time;
}devDailyReport_t;

devDailyReport_t* getDevDailyReport(void);
rb_sta_t* monitorGetpVar(void);
void dailyReportUpdate(void);
int  monitorInit(void);

#endif /* DRIVERS_MONITOR_H_ */
