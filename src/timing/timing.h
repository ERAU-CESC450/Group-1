#pragma once
#include <cstdint>

struct IntervalTimer
{
    uint32_t lastTick = 0;
    bool primed = false;
};

uint32_t IntervalTimerUpdate(IntervalTimer& timer, uint32_t nowTick);
int32_t TimingJitter(uint32_t actualInterval, uint32_t expectedPeriod);
uint32_t TimingElapsed(uint32_t startTick, uint32_t nowTick);
uint32_t TimingDelayUntil(uint32_t& nextReleaseTick, uint32_t periodMs);
