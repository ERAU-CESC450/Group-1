#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
    typedef void (*TaskFunction_t)(void*);
    typedef struct TaskHandle* TaskHandle_t;

    int xTaskCreate(TaskFunction_t taskCode,
        const char* name,
        uint32_t stackWords,
        void* params,
        uint32_t priority,
        TaskHandle_t* outHandle);

    void vTaskStartScheduler(void);
    void vTaskDelay(uint32_t delayMs);
    void taskYIELD(void);
    uint32_t xTaskGetTickCount(void);

    typedef struct QueueHandle* QueueHandle_t;
    QueueHandle_t xQueueCreate(uint32_t length, uint32_t itemSize);
    int xQueueSend(QueueHandle_t q, const void* item, uint32_t timeoutMs);
    int xQueueReceive(QueueHandle_t q, void* outItem, uint32_t timeoutMs);
    void vQueueDelete(QueueHandle_t q);

    // Counting semaphore and mutex support used by the Module 8 design.
    typedef struct SemaphoreHandle* SemaphoreHandle_t;

    SemaphoreHandle_t xSemaphoreCreateCounting(
        uint32_t maxCount,
        uint32_t initialCount);

    SemaphoreHandle_t xSemaphoreCreateMutex(void);

    int xSemaphoreTake(
        SemaphoreHandle_t semaphore,
        uint32_t timeoutMs);

    int xSemaphoreGive(
        SemaphoreHandle_t semaphore);

    int xSemaphoreGiveFromISR(
        SemaphoreHandle_t semaphore);

    void vSemaphoreDelete(
        SemaphoreHandle_t semaphore);

#ifdef __cplusplus
}
#endif
