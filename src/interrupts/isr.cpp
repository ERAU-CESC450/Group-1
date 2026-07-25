#include "isr.h"
#include "rtos_api.h"
#include "safe_log.h"
#include "system_config.h"
#include "system_stats.h"

#include <string>

namespace
{
    enum class EventKind : uint32_t
    {
        HighLatency,
        ExternalPulse,
        Stop
    };

    struct SystemEvent
    {
        EventKind kind;
        uint32_t tick;
        uint32_t value;
    };

    QueueHandle_t g_eventQueue = nullptr;
    SemaphoreHandle_t g_eventReady = nullptr;

    bool SignalFromIsr(EventKind kind, uint32_t value)
    {
        if (!g_eventQueue || !g_eventReady)
        {
            StatsRecordEventSignal(false);
            return false;
        }

        const SystemEvent event{
            kind,
            xTaskGetTickCount(),
            value
        };

        // ISR-style work remains short and bounded:
        // 1. The event data is placed in the queue without blocking.
        // 2. The counting semaphore wakes the deferred handler task.
        // 3. No logging, delay, or extended processing occurs here.
        const bool queued =
            xQueueSend(g_eventQueue, &event, 0) != 0;

        const bool signaled =
            queued &&
            xSemaphoreGiveFromISR(g_eventReady) != 0;

        const bool passed = queued && signaled;
        StatsRecordEventSignal(passed);
        return passed;
    }
}

int IsrInit(uint32_t eventQueueLength)
{
    g_eventQueue =
        xQueueCreate(eventQueueLength, sizeof(SystemEvent));

    g_eventReady =
        xSemaphoreCreateCounting(eventQueueLength, 0);

    if (!g_eventQueue || !g_eventReady)
    {
        vQueueDelete(g_eventQueue);
        vSemaphoreDelete(g_eventReady);

        g_eventQueue = nullptr;
        g_eventReady = nullptr;
        return 0;
    }

    return 1;
}

void IsrShutdown()
{
    vSemaphoreDelete(g_eventReady);
    vQueueDelete(g_eventQueue);

    g_eventReady = nullptr;
    g_eventQueue = nullptr;
}

bool IsrSignalHighLatencyFromIsr(uint32_t latencyMs)
{
    return SignalFromIsr(EventKind::HighLatency, latencyMs);
}

bool IsrSignalExternalPulseFromIsr(uint32_t pulseNumber)
{
    return SignalFromIsr(EventKind::ExternalPulse, pulseNumber);
}

void IsrRequestHandlerStop()
{
    if (!g_eventQueue || !g_eventReady)
        return;

    const SystemEvent stop{
        EventKind::Stop,
        xTaskGetTickCount(),
        0
    };

    while (!xQueueSend(g_eventQueue, &stop, 100))
        taskYIELD();

    // This function runs in task context, so the normal give is used.
    while (!xSemaphoreGive(g_eventReady))
        taskYIELD();
}

void EventHandlerTask(void*)
{
    SafeLogTaskCompletionGuard logCompletion;
    const auto& config = GetSystemConfig();

    for (;;)
    {
        // The counting semaphore blocks the task until an event is ready.
        if (!xSemaphoreTake(g_eventReady,
            config.eventReceiveTimeoutMs))
        {
            continue;
        }

        SystemEvent event{};

        if (!xQueueReceive(g_eventQueue, &event, 0))
        {
            SafeLogWrite(
                "[FAIL] Counting semaphore was released without a matching "
                "queued event.");
            continue;
        }

        if (event.kind == EventKind::Stop)
        {
            SafeLogWrite(
                "[PASS] Deferred event handler received its stop event.");
            break;
        }

        StatsRecordEventHandled();

        if (event.kind == EventKind::HighLatency)
        {
            SafeLogWrite(
                std::string("[FAIL] High-latency event handled outside ISR context: ") +
                std::to_string(event.value) +
                "ms, signaled_at_tick=" +
                std::to_string(event.tick));
        }
        else
        {
            SafeLogWrite(
                std::string("[PASS] External pulse handled outside ISR context: pulse=") +
                std::to_string(event.value) +
                " signaled_at_tick=" +
                std::to_string(event.tick));
        }
    }
}

void EventGeneratorTask(void*)
{
    SafeLogTaskCompletionGuard logCompletion;
    const auto& config = GetSystemConfig();

    for (int i = 1; i <= config.externalEventCount; ++i)
    {
        vTaskDelay(config.externalEventPeriodMs);

        const bool passed =
            IsrSignalExternalPulseFromIsr(
                static_cast<uint32_t>(i));

        SafeLogWrite(
            std::string(passed
                ? "[PASS] External pulse signaled through the event queue and counting semaphore: pulse="
                : "[FAIL] External pulse could not be queued or signaled: pulse=") +
            std::to_string(i));
    }
}
