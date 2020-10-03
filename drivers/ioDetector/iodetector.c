/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-09-28     DELL       the first version
 */
#include "iodetector.h"


typedef struct{
    rt_tick_t _RisingEdgeTime[ioDETETOR_MAX_NUMBER];
    rt_tick_t _FailingEdgeTime[ioDETETOR_MAX_NUMBER];
    uint8_t _installed_number;
    uint8_t _io_old_status[ioDETETOR_MAX_NUMBER];
    uint8_t _io_status[ioDETETOR_MAX_NUMBER];
    uint8_t* _io_source[ioDETETOR_MAX_NUMBER];
    rt_tick_t _gap_min_time[ioDETETOR_MAX_NUMBER];
    rt_tick_t _gap_max_time[ioDETETOR_MAX_NUMBER];
    char* _name[ioDETETOR_MAX_NUMBER];
    io_status_e _status[ioDETETOR_MAX_NUMBER];
    io_status_e _old_status[ioDETETOR_MAX_NUMBER];
    void(*_p[ioDETETOR_MAX_NUMBER])(char*,uint8_t,io_status_e);
}iodetector_base_t;

static iodetector_base_t base_ ;



int iodetector_install(char* name,\
        uint8_t* io_source,\
        uint16_t gap_min_time,\
        uint16_t gap_max_time,
        void(*p)(char*,uint8_t,io_status_e)){
    if(base_._installed_number   < ioDETETOR_MAX_NUMBER){
        base_._name[base_._installed_number] = name;
        base_._io_source[base_._installed_number] = io_source;
        base_._gap_min_time[base_._installed_number] = rt_tick_from_millisecond(gap_min_time);
        base_._gap_max_time[base_._installed_number] = rt_tick_from_millisecond(gap_max_time);
        base_._p[base_._installed_number] = p;
        base_._installed_number ++;

    }
    else{
        return -1;
    }
    return 0;
}


//300ms 即可
void iodetector_update(void){
    for(uint8_t i=0;i<base_._installed_number;i++){
        //获取最新状态
        base_._io_status[i] = *(base_._io_source[i]);

        //记录上升沿时间
        if( base_._io_status[i] !=  base_._io_old_status[i] &&  base_._io_status[i] ){
            base_._RisingEdgeTime[i] = rt_tick_get();
        }

        //记录下降沿时间
        if( base_._io_status[i] !=  base_._io_old_status[i] &&  base_._io_old_status[i] ){
            base_._FailingEdgeTime[i] = rt_tick_get();
        }



        //计算高电平和低电平维持时间
        int edgeGap_f = base_._RisingEdgeTime[i] - base_._FailingEdgeTime[i];
        int edgeGap_r = base_._FailingEdgeTime[i] - base_._RisingEdgeTime[i];

        //如果脉宽在合适范围内则判断为闪烁(任意边沿时间不为0)
        if (  ((edgeGap_r > base_._gap_min_time[i] && edgeGap_r < base_._gap_max_time[i]) ||\
               (edgeGap_f > base_._gap_min_time[i] && edgeGap_f < base_._gap_max_time[i]))&&\
                (base_._FailingEdgeTime[i] !=0 && base_._RisingEdgeTime[i] !=0)){
            if(base_._status[i] != ioFLASH){
                base_._status[i] = ioFLASH;
                if( base_._p[i] != NULL){
                    base_._p[i](base_._name[i],i,base_._status[i]);
                }
            }
        }
        //如果边沿时间较长或者为0，则认为是稳态
        if( rt_tick_get() - base_._RisingEdgeTime[i] > base_._gap_max_time[i] &&\
                rt_tick_get() - base_._FailingEdgeTime[i]> base_._gap_max_time[i]){
            if(base_._io_status[i]){
                if(base_._status[i] != ioOFF){
                    base_._status[i] = ioOFF;
                    if( base_._p[i] != NULL){
                        base_._p[i](base_._name[i],i,base_._status[i]);
                    }
                }

            }
            else{
                if(base_._status[i] != ioON){
                    base_._status[i] = ioON;
                    if( base_._p[i] != NULL){
                        base_._p[i](base_._name[i],i,base_._status[i]);
                    }
                }

            }
        }
        //记录上一个状态
        base_._io_old_status[i] =  base_._io_status[i];
        base_._old_status[i] = base_._status[i];
    }
}
