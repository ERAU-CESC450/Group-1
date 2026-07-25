#pragma once

#include <cstdint>

int IsrInit(uint32_t eventQueueLength);
void IsrShutdown();

bool IsrSignalHighLatencyFromIsr(uint32_t latencyMs);
bool IsrSignalExternalPulseFromIsr(uint32_t pulseNumber);
void IsrRequestHandlerStop();

void EventHandlerTask(void* params);
void EventGeneratorTask(void* params);
