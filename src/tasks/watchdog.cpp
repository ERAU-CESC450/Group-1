#include "watchdog.h"
#include "heartbeat.h"
#include "rtos_api.h"
#include "safe_log.h"
#include "system_config.h"
#include "system_stats.h"
#include "timing.h"

#include <string>

void WatchdogTask(void*)
{
    SafeLogTaskCompletionGuard logCompletion;
    const auto& config = GetSystemConfig();
    bool faulted = false;

    while (HeartbeatActive())
    {
        const uint32_t now = xTaskGetTickCount();
        const uint32_t age =
            TimingElapsed(HeartbeatLastSeenTick(), now);

        if (age > config.watchdogDeadlineMs)
        {
            if (!faulted)
            {
                faulted = true;
                StatsRecordWatchdogFault();

                SafeLogWrite(
                    std::string("[FAIL] Watchdog heartbeat deadline exceeded: age=") +
                    std::to_string(age) +
                    "ms deadline=" +
                    std::to_string(config.watchdogDeadlineMs) + "ms");
            }
        }
        else if (faulted)
        {
            faulted = false;

            SafeLogWrite(
                std::string("[PASS] Watchdog heartbeat recovered: age=") +
                std::to_string(age) +
                "ms beats=" +
                std::to_string(HeartbeatCount()));
        }

        vTaskDelay(config.watchdogPeriodMs);
    }

    SafeLogWrite(
        "[PASS] Watchdog monitoring ended after producer completion.");
}
