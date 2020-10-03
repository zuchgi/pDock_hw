#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* Generated by Kconfiglib (https://github.com/ulfalizer/Kconfiglib) */

/* RT-Thread Kernel */

#define RT_NAME_MAX 8
#define RT_ALIGN_SIZE 4
#define RT_THREAD_PRIORITY_32
#define RT_THREAD_PRIORITY_MAX 32
#define RT_TICK_PER_SECOND 1000
#define RT_USING_OVERFLOW_CHECK
#define RT_USING_HOOK
#define RT_USING_IDLE_HOOK
#define RT_IDLE_HOOK_LIST_SIZE 4
#define IDLE_THREAD_STACK_SIZE 256
#define RT_USING_TIMER_SOFT
#define RT_TIMER_THREAD_PRIO 4
#define RT_TIMER_THREAD_STACK_SIZE 512
#define RT_DEBUG
#define RT_DEBUG_COLOR

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE
/* end of Inter-Thread communication */

/* Memory Management */

#define RT_USING_MEMPOOL
#define RT_USING_SMALL_MEM
#define RT_USING_HEAP
/* end of Memory Management */

/* Kernel Device Object */

#define RT_USING_DEVICE
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 256
#define RT_CONSOLE_DEVICE_NAME "uart4"
/* end of Kernel Device Object */
#define RT_VER_NUM 0x40002
/* end of RT-Thread Kernel */
#define ARCH_ARM
#define RT_USING_CPU_FFS
#define ARCH_ARM_CORTEX_M
#define ARCH_ARM_CORTEX_M3

/* RT-Thread Components */

#define RT_USING_COMPONENTS_INIT
#define RT_USING_USER_MAIN
#define RT_MAIN_THREAD_STACK_SIZE 2048
#define RT_MAIN_THREAD_PRIORITY 10

/* C++ features */

/* end of C++ features */

/* Command shell */

#define RT_USING_FINSH
#define FINSH_THREAD_NAME "tshell"
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES 5
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
#define FINSH_THREAD_PRIORITY 20
#define FINSH_THREAD_STACK_SIZE 4096
#define FINSH_CMD_SIZE 80
#define FINSH_USING_AUTH
#define FINSH_DEFAULT_PASSWORD "pussion"
#define FINSH_PASSWORD_MIN 6
#define FINSH_PASSWORD_MAX 8
#define FINSH_USING_MSH
#define FINSH_USING_MSH_DEFAULT
#define FINSH_USING_MSH_ONLY
#define FINSH_ARG_MAX 10
/* end of Command shell */

/* Device virtual file system */

/* end of Device virtual file system */

/* Device Drivers */

#define RT_USING_DEVICE_IPC
#define RT_PIPE_BUFSZ 512
#define RT_USING_SERIAL
#define RT_SERIAL_RB_BUFSZ 64
#define RT_USING_CAN
#define RT_CAN_USING_HDR
#define RT_USING_PIN
#define RT_USING_ADC
#define RT_USING_SPI

/* Using USB */

/* end of Using USB */
/* end of Device Drivers */

/* POSIX layer and C standard library */

#define RT_USING_LIBC
/* end of POSIX layer and C standard library */

/* Network */

/* Socket abstraction layer */

/* end of Socket abstraction layer */

/* Network interface device */

/* end of Network interface device */

/* light weight TCP/IP stack */

/* end of light weight TCP/IP stack */

/* AT commands */

/* end of AT commands */
/* end of Network */

/* VBUS(Virtual Software BUS) */

/* end of VBUS(Virtual Software BUS) */

/* Utilities */

/* end of Utilities */
/* end of RT-Thread Components */

/* RT-Thread online packages */

/* IoT - internet of things */

#define PKG_USING_FREEMODBUS
#define PKG_MODBUS_MASTER

/* advanced configuration */

#define RT_M_DISCRETE_INPUT_START 0
#define RT_M_DISCRETE_INPUT_NDISCRETES 16
#define RT_M_COIL_START 0
#define RT_M_COIL_NCOILS 64
#define RT_M_REG_INPUT_START 0
#define RT_M_REG_INPUT_NREGS 100
#define RT_M_REG_HOLDING_START 0
#define RT_M_REG_HOLDING_NREGS 100
#define RT_M_HD_RESERVE 0
#define RT_M_IN_RESERVE 0
#define RT_M_CO_RESERVE 0
#define RT_M_DI_RESERVE 0
/* end of advanced configuration */
#define PKG_MODBUS_MASTER_RTU
#define RT_MODBUS_MASTER_USE_CONTROL_PIN
#define MODBUS_MASTER_RT_CONTROL_PIN_INDEX 15
#define PKG_MODBUS_MASTER_SAMPLE
#define MB_SAMPLE_TEST_SLAVE_ADDR 1
#define MB_MASTER_USING_PORT_NUM 3
#define MB_MASTER_USING_PORT_BAUDRATE 115200
#define PKG_MODBUS_SLAVE

/* advanced configuration */

#define RT_S_DISCRETE_INPUT_START 0
#define RT_S_DISCRETE_INPUT_NDISCRETES 16
#define RT_S_COIL_START 0
#define RT_S_COIL_NCOILS 64
#define RT_S_REG_INPUT_START 0
#define RT_S_REG_INPUT_NREGS 100
#define RT_S_REG_HOLDING_START 0
#define RT_S_REG_HOLDING_NREGS 100
#define RT_S_HD_RESERVE 0
#define RT_S_IN_RESERVE 0
#define RT_S_CO_RESERVE 0
#define RT_S_DI_RESERVE 0
/* end of advanced configuration */
#define PKG_MODBUS_SLAVE_RTU
#define RT_MODBUS_SLAVE_USE_CONTROL_PIN
#define MODBUS_SLAVE_RT_CONTROL_PIN_INDEX 10
#define PKG_MODBUS_SLAVE_SAMPLE
#define MB_SAMPLE_SLAVE_ADDR 1
#define MB_SLAVE_USING_PORT_NUM 1
#define MB_SLAVE_USING_PORT_BAUDRATE 115200
#define PKG_USING_FREEMODBUS_LATEST_VERSION

/* Wi-Fi */

/* Marvell WiFi */

/* end of Marvell WiFi */

/* Wiced WiFi */

/* end of Wiced WiFi */
/* end of Wi-Fi */

/* IoT Cloud */

/* end of IoT Cloud */
/* end of IoT - internet of things */

/* security packages */

/* end of security packages */

/* language packages */

/* end of language packages */

/* multimedia packages */

/* end of multimedia packages */

/* tools packages */

/* end of tools packages */

/* system packages */

/* end of system packages */

/* peripheral libraries and drivers */

/* end of peripheral libraries and drivers */

/* miscellaneous packages */


/* samples: kernel and components samples */

/* end of samples: kernel and components samples */
/* end of miscellaneous packages */
/* end of RT-Thread online packages */

/* samples: kernel and components samples */

/* end of samples: kernel and components samples */
#define SOC_FAMILY_STM32
#define SOC_SERIES_STM32F1
#define SOC_STM32F103VCTX

#endif
