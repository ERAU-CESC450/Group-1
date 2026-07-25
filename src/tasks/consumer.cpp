#include "consumer.h"
#include "ipc.h"
#include "isr.h"
#include "messages.h"
#include "rtos_api.h"
#include "safe_log.h"
#include "system_config.h"
#include "system_stats.h"

#include <string>

void LoggerConsumerTask(void*)
{
    SafeLogTaskCompletionGuard logCompletion;
    const auto& config = GetSystemConfig();

    for (;;)
    {
        SystemMessage message{};

        if (!IpcReceive(message, config.receiveTimeoutMs))
        {
            StatsRecordReceiveTimeout();

            SafeLogWrite(
                std::string("[FAIL] IPC receive timed out after ") +
                std::to_string(config.receiveTimeoutMs) + "ms");

            continue;
        }

        if (message.kind == MsgKind::Stop)
        {
            SafeLogWrite(
                "[PASS] STOP message received; no additional IPC data is "
                "expected.");

            IsrRequestHandlerStop();
            break;
        }

        const uint32_t latency =
            xTaskGetTickCount() - message.tick;

        StatsRecordReceive(latency);

        const bool highLatency =
            latency > config.highLatencyThresholdMs;

        if (highLatency)
        {
            IsrSignalHighLatencyFromIsr(latency);
        }

        SafeLogWrite(
            std::string(highLatency
                ? "[FAIL] Message received with high latency: seq="
                : "[PASS] Message received: seq=") +
            std::to_string(message.seq) +
            " payload=" + std::to_string(message.payload) +
            " latency=" + std::to_string(latency) + "ms");

        vTaskDelay(config.consumerProcessMs);
    }
}
