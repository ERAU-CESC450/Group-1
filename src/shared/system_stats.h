#pragma once
#include <cstdint>

struct SystemStatsSnapshot
{
    uint32_t messagesSent;
    uint32_t messagesReceived;
    uint32_t sendDrops;
    uint32_t receiveTimeouts;
    uint32_t producerDeadlineMisses;
    uint32_t watchdogFaults;
    uint32_t eventsSignaled;
    uint32_t eventsHandled;
    uint32_t eventDrops;
    uint32_t maxSendWaitMs;
    uint32_t maxProducerLatenessMs;
    uint32_t maxLatencyMs;
    uint64_t totalLatencyMs;
    uint32_t logsQueued;
    uint32_t logsWritten;
    uint32_t logDrops;
    uint32_t logSyncErrors;
    uint32_t maxLogQueueWaitMs;
};

void StatsReset();
void StatsRecordSend(uint32_t waitMs);
void StatsRecordSendDrop(uint32_t waitMs);
void StatsRecordReceive(uint32_t latencyMs);
void StatsRecordReceiveTimeout();
void StatsRecordProducerDeadlineMiss(uint32_t lateMs);
void StatsRecordWatchdogFault();
void StatsRecordEventSignal(bool queued);
void StatsRecordEventHandled();
void StatsRecordLogQueued(uint32_t waitMs);
void StatsRecordLogWritten();
void StatsRecordLogDrop(uint32_t waitMs);
void StatsRecordLogSyncError();
SystemStatsSnapshot StatsSnapshot();
