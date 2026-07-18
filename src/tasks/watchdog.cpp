#include "watchdog.h"

#include "rtos_api.h"
#include "heartbeat.h"
#include "timing.h"

#include <cstdint>
#include <iostream>

namespace
{
    constexpr uint32_t kWatchPeriodMs = 200;
    constexpr uint32_t kDeadlineMs = 700; // > period + normal jitter
}

void WatchdogTask(void* params)
{
    const char* name = params ? static_cast<const char*>(params) : "Watchdog";
    bool faulted = false; // report transitions, not every poll

    while (HeartbeatActive())
    {
        const uint32_t now = xTaskGetTickCount();
        const uint32_t age = TimingElapsed(HeartbeatLastSeenTick(), now);
        const uint32_t beats = HeartbeatCount();

        if (age > kDeadlineMs)
        {
            if (!faulted)
            {
                std::cout << "  >> [" << name << "] FAULT: heartbeat LATE/STALLED - age="
                    << age << "ms > deadline=" << kDeadlineMs
                    << "ms (beats=" << beats << ")\n";
                faulted = true;
            }
            else
            {
                std::cout << "  >> [" << name << "] still waiting - age=" << age
                    << "ms (beats=" << beats << ")\n";
            }
        }
        else
        {
            if (faulted)
            {
                std::cout << "  >> [" << name << "] RECOVERED: heartbeat resumed - age="
                    << age << "ms (beats=" << beats << ")\n";
                faulted = false;
            }
            else
            {
                std::cout << "  -- [" << name << "] OK: age=" << age
                    << "ms (beats=" << beats << ")\n";
            }
        }

        vTaskDelay(kWatchPeriodMs);
    }

    std::cout << "  -- [" << name << "] monitoring stopped (system inactive).\n";
}