/*
 * ============================================================
 * CESC 450 – Real-Time Embedded Systems Course Project
 * src/main.cpp
 *
 * PURPOSE:
 * This file serves as the central coordination point for the project.
 * It is responsible for:
 *
 *  - System initialization (host-based for this course)
 *  - RTOS startup
 *  - Creating and configuring tasks
 *
 * This file is extended incrementally across Modules 3–7.
 *
 * IMPORTANT GUIDELINES FOR STUDENTS:
 *  - Do NOT place all application logic in this file
 *  - Task implementations belong in src/tasks/
 *  - Timing logic belongs in src/timing/
 *  - IPC logic belongs in src/ipc/
 *  - Shared data and synchronization belong in src/shared/
 *
 * main.cpp should remain readable and focused on system setup.
 * ============================================================
 */


//Module 6 IPC 
#include <iostream>
#include "rtos_api.h"
#include "tasks.h"
#include "watchdog.h"
#include "heartbeat.h"
#include "ipc.h"
#include "safe_log.h"
#include "log_worker.h"
#include "producer.h"
#include "consumer.h"



int main()
{
    std::cout << "=====================================================\n";
    std::cout << "CESC 450 Module 6 Messages & IPC\n";
    std::cout << "=====================================================\n\n";

    SafeLogInit();
    IpcInit(3);

    static const char* kProducer = "Producer";
    static const char* kConsumer = "Consumer";

    TaskHandle_t p = nullptr;
	TaskHandle_t c = nullptr;

    if (!xTaskCreate(SensorProducerTask, "Producer,", 256, (void*)kProducer, 2, &p) ||
        !xTaskCreate(LoggerConsumerTask, "Consumer", 256, (void*)kConsumer, 1, &c))
    {
        std::cerr << "Failed to create IPC tasks\n";
        IpcShutdown();
        SafeLogShutdown();
        return 1;
    }
    
    std::cout << "Starting scheduler...\n";
    vTaskStartScheduler();

    std::cout << "\nScheduler exited (all tasks completed). Baseline OK.\n";

    return 0;
}


