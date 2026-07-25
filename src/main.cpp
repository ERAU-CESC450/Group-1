#include "consumer.h"
#include "heartbeat.h"
#include "ipc.h"
#include "isr.h"
#include "producer.h"
#include "rtos_api.h"
#include "safe_log.h"
#include "system_config.h"
#include "system_stats.h"
#include "watchdog.h"

#include <iomanip>
#include <iostream>
#include <string>

namespace
{
    const char* Result(bool passed)
    {
        return passed ? "PASS" : "FAIL";
    }

    bool CreateTask(void (*entry)(void*),
        const char* taskName,
        uint32_t priority)
    {
        TaskHandle_t handle = nullptr;
        return xTaskCreate(entry,
            taskName,
            256,
            nullptr,
            priority,
            &handle) != 0;
    }

    void PrintConfiguration(const SystemConfig& config)
    {
        std::cout << "Run configuration: " << config.modeName << '\n'
            << "IPC queue length: " << config.queueLength << '\n'
            << "Event queue length: " << config.eventQueueLength << '\n'
            << "Log queue length: " << config.logQueueLength << '\n'
            << "Log send timeout: " << config.logSendTimeoutMs << "ms\n"
            << "Producer period: " << config.producerPeriodMs << "ms\n"
            << "Consumer processing: " << config.consumerProcessMs << "ms\n"
            << "Send timeout: " << config.sendTimeoutMs << "ms\n"
            << "Receive timeout: " << config.receiveTimeoutMs << "ms\n"
            << "High-latency threshold: "
            << config.highLatencyThresholdMs << "ms\n\n";
    }

    void PrintSummary(const SystemConfig& config)
    {
        const auto stats = StatsSnapshot();
        const double averageLatency = stats.messagesReceived == 0
            ? 0.0
            : static_cast<double>(stats.totalLatencyMs) /
            static_cast<double>(stats.messagesReceived);

        const bool ipcPassed =
            stats.messagesSent == static_cast<uint32_t>(config.messageCount) &&
            stats.messagesReceived == stats.messagesSent &&
            stats.sendDrops == 0 &&
            stats.receiveTimeouts == 0;

        const bool timingPassed =
            stats.producerDeadlineMisses == 0;

        const bool latencyPassed =
            stats.maxLatencyMs <= config.highLatencyThresholdMs;

        const bool watchdogPassed =
            stats.watchdogFaults == 0;

        const bool eventPassed =
            stats.eventDrops == 0 &&
            stats.eventsSignaled == stats.eventsHandled;

        const bool loggingPassed =
            stats.logDrops == 0 &&
            stats.logSyncErrors == 0 &&
            stats.logsQueued == stats.logsWritten;

        const bool overallPassed =
            ipcPassed &&
            timingPassed &&
            latencyPassed &&
            watchdogPassed &&
            eventPassed &&
            loggingPassed;

        std::cout << "\n================ MODULE 8 SUMMARY ================\n"
            << "run_configuration=" << config.modeName << '\n'
            << "overall_result=" << Result(overallPassed) << '\n'
            << "ipc_result=" << Result(ipcPassed)
            << " sent=" << stats.messagesSent
            << " received=" << stats.messagesReceived
            << " drops=" << stats.sendDrops
            << " receive_timeouts=" << stats.receiveTimeouts << '\n'
            << "timing_result=" << Result(timingPassed)
            << " producer_deadline_misses="
            << stats.producerDeadlineMisses
            << " max_lateness_ms=" << stats.maxProducerLatenessMs
            << " max_send_wait_ms=" << stats.maxSendWaitMs << '\n'
            << std::fixed << std::setprecision(1)
            << "latency_result=" << Result(latencyPassed)
            << " average_latency_ms=" << averageLatency
            << " max_latency_ms=" << stats.maxLatencyMs << '\n'
            << "watchdog_result=" << Result(watchdogPassed)
            << " watchdog_faults=" << stats.watchdogFaults << '\n'
            << "event_result=" << Result(eventPassed)
            << " events_signaled=" << stats.eventsSignaled
            << " events_handled=" << stats.eventsHandled
            << " event_drops=" << stats.eventDrops << '\n'
            << "logging_result=" << Result(loggingPassed)
            << " logs_queued=" << stats.logsQueued
            << " logs_written=" << stats.logsWritten
            << " log_drops=" << stats.logDrops
            << " log_sync_errors=" << stats.logSyncErrors
            << " max_log_queue_wait_ms="
            << stats.maxLogQueueWaitMs << '\n'
            << "==================================================\n";
    }
}

int main(int argc, char* argv[])
{
    const bool baseline =
        argc > 1 && std::string(argv[1]) == "--baseline";

    const SystemConfig& config =
        baseline ? BaselineConfig() : StabilizedConfig();

    SetSystemConfig(&config);

    std::cout << "=====================================================\n"
        << "CESC 450 Module 8 Integration and Stabilization\n"
        << "=====================================================\n";

    PrintConfiguration(config);

    StatsReset();

    constexpr uint32_t kLoggingWriterTasks = 5;

    if (!SafeLogInit(config.logQueueLength,
        kLoggingWriterTasks,
        config.logSendTimeoutMs))
    {
        std::cerr << "[FAIL] Asynchronous logging initialization failed.\n";
        return 1;
    }
    HeartbeatInit(xTaskGetTickCount());

    if (!IpcInit(config.queueLength) ||
        !IsrInit(config.eventQueueLength))
    {
        std::cerr
            << "[FAIL] IPC, semaphore, or event queue initialization failed.\n";

        IpcShutdown();
        IsrShutdown();
        SafeLogShutdown();
        return 1;
    }

    const bool created =
        CreateTask(SafeLogWorkerTask, "LogWorker", 1) &&
        CreateTask(EventHandlerTask, "EventHandler", 4) &&
        CreateTask(WatchdogTask, "Watchdog", 4) &&
        CreateTask(SensorProducerTask, "Producer", 3) &&
        CreateTask(EventGeneratorTask, "EventSource", 3) &&
        CreateTask(LoggerConsumerTask, "Consumer", 2);

    if (!created)
    {
        std::cerr << "[FAIL] One or more tasks could not be created.\n";

        IpcShutdown();
        IsrShutdown();
        SafeLogShutdown();
        return 1;
    }

    SafeLogWrite(
        "[PASS] Scheduler started with tasks, timing, IPC, asynchronous logging, "
        "mutex protection, counting-semaphore signaling, watchdog monitoring, "
        "and deferred events.");

    vTaskStartScheduler();

    PrintSummary(config);

    IpcShutdown();
    IsrShutdown();
    SafeLogShutdown();
    return 0;
}
