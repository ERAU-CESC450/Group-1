#include "ipc.h"
#include "rtos_api.h"

namespace
{
    QueueHandle_t g_queue = nullptr;
}

int IpcInit(uint32_t queueLength)
{
    g_queue = xQueueCreate(queueLength, sizeof(SystemMessage));
    return g_queue ? 1 : 0;
}

void IpcShutdown()
{
    vQueueDelete(g_queue);
    g_queue = nullptr;
}

int IpcSend(const SystemMessage& message, uint32_t timeoutMs)
{
    return xQueueSend(g_queue, &message, timeoutMs);
}

int IpcReceive(SystemMessage& out, uint32_t timeoutMs)
{
    return xQueueReceive(g_queue, &out, timeoutMs);
}
