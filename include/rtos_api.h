#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void (*TaskFunction_t)(void*);

    typedef struct TaskHandle* TaskHandle_t;

    // -----------------------------------------------------------------------------
    // Tasks
    // -----------------------------------------------------------------------------

    // Return 1 for success, 0 for failure
    int xTaskCreate(TaskFunction_t taskCode,
        const char* name,
        uint32_t stackWords,
        void* params,
        uint32_t priority,
        TaskHandle_t* outHandle);

    // Start the scheduler (non-returning in a typical RTOS).
    // In this shim, it runs until all created tasks have exited.
    void vTaskStartScheduler(void);

    // Delay current task by milliseconds (host-based).
    void vTaskDelay(uint32_t delayMs);

    // Cooperative yield hint.
    void taskYIELD(void);

    // Basic diagnostics
    uint32_t xTaskGetTickCount(void);

    // -----------------------------------------------------------------------------
    // Message Queues (IPC) - minimal RTOS-style API for Module 6
    // -----------------------------------------------------------------------------

    typedef struct QueueHandle* QueueHandle_t;

    // Create a fixed-size queue of length items, each itemSize bytes.
    QueueHandle_t xQueueCreate(uint32_t length, uint32_t itemSize);

    // Send an item to the queue.
    // timeoutMs:
    //   - 0      : non-blocking (fail immediately if full)
    //   - >0     : wait up to timeoutMs for space
    // Returns 1 on success, 0 on failure/timeout.
    int xQueueSend(QueueHandle_t q, const void* item, uint32_t timeoutMs);

    // Receive an item from the queue into outItem (must be itemSize bytes).
    // timeoutMs:
    //   - 0      : non-blocking (fail immediately if empty)
    //   - >0     : wait up to timeoutMs for an item
    // Returns 1 on success, 0 on failure/timeout.
    int xQueueReceive(QueueHandle_t q, void* outItem, uint32_t timeoutMs);

    // Optional cleanup (safe in host shim; not required for Module 6 but useful)
    void vQueueDelete(QueueHandle_t q);

#ifdef __cplusplus
}
#endif
