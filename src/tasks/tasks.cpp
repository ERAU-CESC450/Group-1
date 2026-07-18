#include "tasks.h"
#include "rtos_api.h"
#include "timing.h"
#include "heartbeat.h"

#include <atomic>
#include <iostream>

namespace
{
    std::atomic<int> g_latestReading{ 0 };
    std::atomic<int> g_readingCount{ 0 };
}

static constexpr int kSensorIterations = 12;
static constexpr int kLoggerIterations = 8;
static constexpr uint32_t kSensorPeriodMs = 250;
static constexpr uint32_t kLoggerPeriodMs = 400;

// Inject one missed deadline so the watchdog has something to detect.
static constexpr int kStallAtIteration = 5;
static constexpr uint32_t kStallDurationMs = 1200;

// Task A - Sensor (producer) with timing instrumentation + heartbeat.
void SensorTask(void* params)
{
    const char* name = params ? static_cast<const char*>(params) : "Sensor";
    IntervalTimer period;

    for (int i = 0; i < kSensorIterations; ++i)
    {
        const uint32_t now = xTaskGetTickCount();
        const uint32_t interval = IntervalTimerUpdate(period, now);

        const int reading = 20 + (i % 5);
        g_latestReading.store(reading, std::memory_order_relaxed);
        g_readingCount.fetch_add(1, std::memory_order_relaxed);

        HeartbeatPublish(now);

        std::cout << "[" << name << "] reading #" << (i + 1) << " = " << reading;
        if (interval == 0)
            std::cout << " | interval=n/a (first beat)";
        else
            std::cout << " | interval=" << interval << "ms"
            << " jitter=" << TimingJitter(interval, kSensorPeriodMs) << "ms";
        std::cout << " (tick=" << now << ")\n";

        if (i == kStallAtIteration)
        {
            std::cout << "[" << name << "] !! simulating blocking/load for "
                << kStallDurationMs << "ms (will miss deadline) !!\n";
            vTaskDelay(kStallDurationMs); // no heartbeat during this gap
        }

        vTaskDelay(kSensorPeriodMs);
        taskYIELD();
    }

    std::cout << "[" << name << "] done producing " << kSensorIterations
        << " readings.\n";
    HeartbeatSetActive(false); // let the watchdog loop exit
}

// Task B - Logger (consumer), unchanged from Module 3.
void LoggerTask(void* params)
{
    const char* name = params ? static_cast<const char*>(params) : "Logger";

    for (int i = 0; i < kLoggerIterations; ++i)
    {
        const int count = g_readingCount.load(std::memory_order_relaxed);
        const int value = g_latestReading.load(std::memory_order_relaxed);

        std::cout << "    [" << name << "] status: latest=" << value
            << " after " << count << " readings"
            << " (tick=" << xTaskGetTickCount() << ")\n";

        vTaskDelay(kLoggerPeriodMs);
        taskYIELD();
    }

    std::cout << "    [" << name << "] done reporting.\n";
}