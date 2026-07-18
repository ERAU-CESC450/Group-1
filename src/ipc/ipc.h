#pragma once
#include <cstdint>
#include "messages.h"

void IpcInit(uint32_t queueLength);
void IpcShutdown();



int IpcSend(const SystemMessage &msg, uint32_t timeoutMs);
int IpcReceive(SystemMessage &out, uint32_t timeoutMs);