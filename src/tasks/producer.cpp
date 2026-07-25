#include "producer.h"
#include "heartbeat.h"
#include "ipc.h"
#include "messages.h"
#include "rtos_api.h"
#include "safe_log.h"
#include "system_config.h"
#include "system_stats.h"
#include "timing.h"

#include <string>

void SensorProducerTask(void*)
{
    SafeLogTaskCompletionGuard logCompletion;
    const auto& config = GetSystemConfig();
    uint32_t nextReleaseTick = xTaskGetTickCount();
    IntervalTimer intervalTimer;

    for (int i = 0; i < config.messageCount; ++i)
    {
        const uint32_t now = xTaskGetTickCount();
        const uint32_t interval = IntervalTimerUpdate(intervalTimer, now);
        HeartbeatPublish(now);

        SystemMessage message{};
        message.kind = MsgKind::Data;
        message.seq = static_cast<uint32_t>(i + 1);
        message.tick = now;
        message.payload = static_cast<uint32_t>(20 + (i % 5));

        const uint32_t sendStart = xTaskGetTickCount();
        const int sent = IpcSend(message, config.sendTimeoutMs);
        const uint32_t waited = xTaskGetTickCount() - sendStart;

        if (sent)
        {
            StatsRecordSend(waited);

            SafeLogWrite(
                std::string("[PASS] Message sent: seq=") +
                std::to_string(message.seq) +
                " payload=" + std::to_string(message.payload) +
                " send_wait=" + std::to_string(waited) + "ms" +
                " interval=" +
                (interval == 0
                    ? std::string("n/a")
                    : std::to_string(interval) + "ms") +
                (interval == 0
                    ? std::string("")
                    : " jitter=" +
                    std::to_string(
                        TimingJitter(interval,
                            config.producerPeriodMs)) +
                    "ms"));
        }
        else
        {
            StatsRecordSendDrop(waited);

            SafeLogWrite(
                std::string("[FAIL] Message dropped: seq=") +
                std::to_string(message.seq) +
                " queue remained full for " +
                std::to_string(waited) + "ms");
        }

        const uint32_t lateness =
            TimingDelayUntil(nextReleaseTick,
                config.producerPeriodMs);

        if (lateness > 0)
        {
            StatsRecordProducerDeadlineMiss(lateness);

            SafeLogWrite(
                std::string("[FAIL] Producer deadline missed: seq=") +
                std::to_string(message.seq) +
                " late_by=" + std::to_string(lateness) + "ms");
        }
    }

    SystemMessage stop{};
    stop.kind = MsgKind::Stop;
    stop.tick = xTaskGetTickCount();

    while (!IpcSend(stop, 1000))
    {
        SafeLogWrite(
            "[FAIL] STOP message could not be queued because the IPC queue "
            "remained full; retrying.");
    }

    SafeLogWrite(
        "[PASS] Producer work completed and the STOP message was queued.");

    HeartbeatSetActive(false);
}
