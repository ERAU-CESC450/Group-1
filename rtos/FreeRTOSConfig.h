#pragma once
/*
  Placeholder configuration file for a FreeRTOS-style host build.

  In later modules, if you switch from this shim to the actual FreeRTOS
  kernel + host/posix port, keep this file and update the values per your
  chosen port and course guidance.
*/

#pragma once
#define configUSE_PREEMPTION                    1
#define configUSE_TIME_SLICING                  1
#define configTICK_RATE_HZ                      1000
#define configMAX_PRIORITIES                    7
#define configMINIMAL_STACK_SIZE                256
#define configTOTAL_HEAP_SIZE                   (64 * 1024)
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                  0
#define configCHECK_FOR_STACK_OVERFLOW          0
#define configUSE_MALLOC_FAILED_HOOK            0
#define configUSE_MUTEXES                       1
#define configUSE_COUNTING_SEMAPHORES           1
