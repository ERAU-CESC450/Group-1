#pragma once
#include "messages.h"
#include <cstdint>

int IpcInit(uint32_t queueLength);
void IpcShutdown();
int IpcSend(const SystemMessage& message, uint32_t timeoutMs);
int IpcReceive(SystemMessage& out, uint32_t timeoutMs);
