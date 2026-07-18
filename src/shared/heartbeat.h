#pragma once
#include <cstdint>

// Shared timing/liveness state ("heartbeat registry"), lock-free atomics.
void HeartbeatInit(uint32_t nowTick);
void HeartbeatPublish(uint32_t nowTick);
uint32_t HeartbeatLastSeenTick(void);
uint32_t HeartbeatCount(void);
void HeartbeatSetActive(bool active);
bool HeartbeatActive(void);