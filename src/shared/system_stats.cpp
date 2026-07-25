#include "system_stats.h"
#include <atomic>

namespace
{
    std::atomic<uint32_t> g_messagesSent{ 0 };
    std::atomic<uint32_t> g_messagesReceived{ 0 };
    std::atomic<uint32_t> g_sendDrops{ 0 };
    std::atomic<uint32_t> g_receiveTimeouts{ 0 };
    std::atomic<uint32_t> g_producerDeadlineMisses{ 0 };
    std::atomic<uint32_t> g_watchdogFaults{ 0 };
    std::atomic<uint32_t> g_eventsSignaled{ 0 };
    std::atomic<uint32_t> g_eventsHandled{ 0 };
    std::atomic<uint32_t> g_eventDrops{ 0 };
    std::atomic<uint32_t> g_maxSendWaitMs{ 0 };
    std::atomic<uint32_t> g_maxProducerLatenessMs{ 0 };
    std::atomic<uint32_t> g_maxLatencyMs{ 0 };
    std::atomic<uint64_t> g_totalLatencyMs{ 0 };
    std::atomic<uint32_t> g_logsQueued{ 0 };
    std::atomic<uint32_t> g_logsWritten{ 0 };
    std::atomic<uint32_t> g_logDrops{ 0 };
    std::atomic<uint32_t> g_logSyncErrors{ 0 };
    std::atomic<uint32_t> g_maxLogQueueWaitMs{ 0 };

    void UpdateMax(std::atomic<uint32_t>& target, uint32_t value)
    {
        uint32_t current = target.load(std::memory_order_relaxed);
        while (current < value &&
            !target.compare_exchange_weak(current, value, std::memory_order_relaxed))
        {
        }
    }
}

void StatsReset()
{
    g_messagesSent.store(0);
    g_messagesReceived.store(0);
    g_sendDrops.store(0);
    g_receiveTimeouts.store(0);
    g_producerDeadlineMisses.store(0);
    g_watchdogFaults.store(0);
    g_eventsSignaled.store(0);
    g_eventsHandled.store(0);
    g_eventDrops.store(0);
    g_maxSendWaitMs.store(0);
    g_maxProducerLatenessMs.store(0);
    g_maxLatencyMs.store(0);
    g_totalLatencyMs.store(0);
    g_logsQueued.store(0);
    g_logsWritten.store(0);
    g_logDrops.store(0);
    g_logSyncErrors.store(0);
    g_maxLogQueueWaitMs.store(0);
}

void StatsRecordSend(uint32_t waitMs)
{
    g_messagesSent.fetch_add(1, std::memory_order_relaxed);
    UpdateMax(g_maxSendWaitMs, waitMs);
}

void StatsRecordSendDrop(uint32_t waitMs)
{
    g_sendDrops.fetch_add(1, std::memory_order_relaxed);
    UpdateMax(g_maxSendWaitMs, waitMs);
}

void StatsRecordReceive(uint32_t latencyMs)
{
    g_messagesReceived.fetch_add(1, std::memory_order_relaxed);
    g_totalLatencyMs.fetch_add(latencyMs, std::memory_order_relaxed);
    UpdateMax(g_maxLatencyMs, latencyMs);
}

void StatsRecordReceiveTimeout()
{
    g_receiveTimeouts.fetch_add(1, std::memory_order_relaxed);
}

void StatsRecordProducerDeadlineMiss(uint32_t lateMs)
{
    g_producerDeadlineMisses.fetch_add(1, std::memory_order_relaxed);
    UpdateMax(g_maxProducerLatenessMs, lateMs);
}

void StatsRecordWatchdogFault()
{
    g_watchdogFaults.fetch_add(1, std::memory_order_relaxed);
}

void StatsRecordEventSignal(bool queued)
{
    g_eventsSignaled.fetch_add(1, std::memory_order_relaxed);
    if (!queued)
        g_eventDrops.fetch_add(1, std::memory_order_relaxed);
}

void StatsRecordEventHandled()
{
    g_eventsHandled.fetch_add(1, std::memory_order_relaxed);
}

void StatsRecordLogQueued(uint32_t waitMs)
{
    g_logsQueued.fetch_add(1, std::memory_order_relaxed);
    UpdateMax(g_maxLogQueueWaitMs, waitMs);
}

void StatsRecordLogWritten()
{
    g_logsWritten.fetch_add(1, std::memory_order_relaxed);
}

void StatsRecordLogDrop(uint32_t waitMs)
{
    g_logDrops.fetch_add(1, std::memory_order_relaxed);
    UpdateMax(g_maxLogQueueWaitMs, waitMs);
}

void StatsRecordLogSyncError()
{
    g_logSyncErrors.fetch_add(1, std::memory_order_relaxed);
}

SystemStatsSnapshot StatsSnapshot()
{
    return {
        g_messagesSent.load(),
        g_messagesReceived.load(),
        g_sendDrops.load(),
        g_receiveTimeouts.load(),
        g_producerDeadlineMisses.load(),
        g_watchdogFaults.load(),
        g_eventsSignaled.load(),
        g_eventsHandled.load(),
        g_eventDrops.load(),
        g_maxSendWaitMs.load(),
        g_maxProducerLatenessMs.load(),
        g_maxLatencyMs.load(),
        g_totalLatencyMs.load(),
        g_logsQueued.load(),
        g_logsWritten.load(),
        g_logDrops.load(),
        g_logSyncErrors.load(),
        g_maxLogQueueWaitMs.load()
    };
}
