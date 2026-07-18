#include "timing.h"

uint32_t IntervalTimerUpdate(IntervalTimer& t, uint32_t nowTick)
{
    if (!t.primed)
    {
        t.primed = true;
        t.lastTick = nowTick;
        return 0;
    }

    const uint32_t delta = nowTick - t.lastTick;
    t.lastTick = nowTick;
    return delta;
}

int32_t TimingJitter(uint32_t actualInterval, uint32_t expectedPeriod)
{
    return static_cast<int32_t>(actualInterval) - static_cast<int32_t>(expectedPeriod);
}

uint32_t TimingElapsed(uint32_t startTick, uint32_t nowTick)
{
    return nowTick - startTick;
}