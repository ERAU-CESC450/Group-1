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
#include <iostream>
#include "rtos_api.h"
#include "TaskA.h"
#include "TaskB.h"

int main()
{
    std::cout << "=====================================================\n";
    std::cout << "CESC 450 Module 3 - Concurrent Avionics RTOS Active\n";
    std::cout << "=====================================================\n\n";

    // Create Task A (Telemetry Simulation)
    if (!xTaskCreate(vTaskA, "Task_A", 256, nullptr, 1, nullptr))
    {
        std::cerr << "CRITICAL: Failed to create Task A\n";
        return 1;
    }

    // Create Task B (System Monitor)
    if (!xTaskCreate(vTaskB, "Task_B", 256, nullptr, 1, nullptr))
    {
        std::cerr << "CRITICAL: Failed to create Task B\n";
        return 1;
    }

    std::cout << "Starting background RTOS scheduler...\n\n";
    vTaskStartScheduler();

    // The host shim runs until execution stops or tasks finish
    std::cout << "\nScheduler exited gracefully.\n";
    return 0;
}


