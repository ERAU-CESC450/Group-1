#include "timing.h"
#include "rtos_api.h"

uint32_t IntervalTimerUpdate(IntervalTimer& timer, uint32_t nowTick)
{
    if (!timer.primed)
    {
        timer.primed = true;
        timer.lastTick = nowTick;
        return 0;
    }

    const uint32_t delta = nowTick - timer.lastTick;
    timer.lastTick = nowTick;
    return delta;
}

int32_t TimingJitter(uint32_t actualInterval, uint32_t expectedPeriod)
{
    return static_cast<int32_t>(actualInterval) -
        static_cast<int32_t>(expectedPeriod);
}

uint32_t TimingElapsed(uint32_t startTick, uint32_t nowTick)
{
    return nowTick - startTick;
}

uint32_t TimingDelayUntil(uint32_t& nextReleaseTick, uint32_t periodMs)
{
    nextReleaseTick += periodMs;
    const uint32_t now = xTaskGetTickCount();

    if (now < nextReleaseTick)
    {
        vTaskDelay(nextReleaseTick - now);
        return 0;
    }

    const uint32_t lateness = now - nextReleaseTick;
    nextReleaseTick = now;
    return lateness;
}
