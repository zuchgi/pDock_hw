/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-10-26     ChenYong     first version
 * 2020-01-08     xiangxistu   add HSI configuration
 */

#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "sim7600/sim7600.h"

#define DBG_TAG "board"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define USE_HSI 1
static void boardGpioInit(void){
    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(LED2_PIN, PIN_MODE_OUTPUT);


    rt_pin_write(LED0_PIN, 0);
    rt_pin_write(LED1_PIN, 0);
    rt_pin_write(LED2_PIN, 0);

    rt_pin_mode(SIM7600_DCDC_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(SIM7600_DCDC_PIN,1);
    rt_pin_mode(SIM7600_POWER_PIN, PIN_MODE_OUTPUT_OD);//1.8V
    rt_pin_write(SIM7600_POWER_PIN,0);
    rt_pin_mode(SIM7600_RESET_PIN, PIN_MODE_OUTPUT_OD);//1.8V
    rt_pin_write(SIM7600_RESET_PIN,1);

    rt_pin_mode(SW0_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(SW1_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(SW2_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(SW3_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(SW4_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(SW5_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(SW6_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(SW7_PIN, PIN_MODE_INPUT_PULLUP);

    rt_pin_mode(DIN0_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(DIN1_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(DIN2_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(DIN3_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(DIN4_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(DIN5_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(DIN6_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(DIN7_PIN, PIN_MODE_INPUT_PULLUP);

    rt_pin_mode(RS485_EN, PIN_MODE_OUTPUT);

}
void platformInit(void){

    boardGpioInit();
    chipInint();
    s7Init();
    freeModbusInit();
}

int getBoardId(void){
    int rtval =0;
    if(rt_pin_read(SW0_PIN)){
    }
    else{
        rtval += 128;
    }
    if(rt_pin_read(SW1_PIN)){
    }
    else{
        rtval += 64;
    }
    if(rt_pin_read(SW2_PIN)){
    }
    else{
        rtval += 32;
    }
    if(rt_pin_read(SW3_PIN)){
    }
    else{
        rtval += 16;
    }
    if(rt_pin_read(SW4_PIN)){
    }
    else{
        rtval += 8;
    }
    if(rt_pin_read(SW5_PIN)){
    }
    else{
        rtval += 4;
    }
    if(rt_pin_read(SW6_PIN)){
    }
    else{
        rtval += 2;
    }
    if(rt_pin_read(SW7_PIN)){
    }
    else{
        rtval += 1 ;
    }
    return rtval;
}
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks
  */
#if USE_HSI
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
#else
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
#endif
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;


#if USE_HSI
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
#else
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
#endif
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  //add by tau for adc
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

int clock_information(void)
{
    LOG_D("System Clock information");
    LOG_D("SYSCLK_Frequency = %d", HAL_RCC_GetSysClockFreq());
    LOG_D("HCLK_Frequency   = %d", HAL_RCC_GetHCLKFreq());
    LOG_D("PCLK1_Frequency  = %d", HAL_RCC_GetPCLK1Freq());
    LOG_D("PCLK2_Frequency  = %d", HAL_RCC_GetPCLK2Freq());

    return RT_EOK;
}
INIT_BOARD_EXPORT(clock_information);
