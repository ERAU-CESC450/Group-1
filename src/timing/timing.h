#pragma once
#include <cstdint>

// Timing instrumentation helpers (pure functions, no I/O).
struct IntervalTimer
{
	uint32_t lastTick = 0;
	bool primed = false;
};

// Elapsed ticks since the previous call (0 on the first call).
uint32_t IntervalTimerUpdate(IntervalTimer& t, uint32_t nowTick);

// actual - expected (+ = late, - = early).
int32_t TimingJitter(uint32_t actualInterval, uint32_t expectedPeriod);

uint32_t TimingElapsed(uint32_t startTick, uint32_t nowTick);