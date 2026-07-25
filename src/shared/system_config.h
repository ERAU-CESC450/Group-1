#ifndef CESC450_SYSTEM_CONFIG_H
#define CESC450_SYSTEM_CONFIG_H

#include <cstdint>

struct SystemConfig
{
    const char* modeName;

    uint32_t queueLength;
    uint32_t eventQueueLength;
    uint32_t logQueueLength;
    uint32_t logSendTimeoutMs;

    uint32_t producerPeriodMs;
    uint32_t consumerProcessMs;
    uint32_t sendTimeoutMs;
    uint32_t receiveTimeoutMs;
    uint32_t highLatencyThresholdMs;

    uint32_t watchdogPeriodMs;
    uint32_t watchdogDeadlineMs;

    uint32_t eventReceiveTimeoutMs;
    uint32_t externalEventPeriodMs;

    int externalEventCount;
    int messageCount;
};

const SystemConfig& BaselineConfig();
const SystemConfig& StabilizedConfig();

void SetSystemConfig(const SystemConfig* config);
const SystemConfig& GetSystemConfig();

#endif // CESC450_SYSTEM_CONFIG_H
