/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-09-28     DELL       the first version
 */
#ifndef DRIVERS_IODETECTOR_IODETECTOR_H_
#define DRIVERS_IODETECTOR_IODETECTOR_H_
#include "board.h"

//能注册的io检测器最大数量
#define ioDETETOR_MAX_NUMBER 8
//io检测器缓存大小(uint8_t)
#define ioDETETOR_BUFFER_LENGTH 32

typedef enum{
    ioOFF,
    ioFLASH,
    ioON
}io_status_e;

int iodetector_install(char* name,\
        uint8_t* io_source,\
        uint16_t gap_min_time,\
        uint16_t gap_max_time,
        void(*p)(char*,uint8_t,io_status_e));
void iodetector_update(void);

#endif /* DRIVERS_IODETECTOR_IODETECTOR_H_ */
