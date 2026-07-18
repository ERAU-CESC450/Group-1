#include "ipc.h"
#include "rtos_api.h"

namespace
{
	QueueHandle_t g_queue = nullptr;
}

void IpcInit(uint32_t queueLength)
{
	g_queue = xQueueCreate(queueLength, sizeof(SystemMessage));
}

void IpcShutdown()
{
	vQueueDelete(g_queue);
	g_queue = nullptr;
}

int IpcSend(const SystemMessage &msg, uint32_t timeoutMs)
{
	return xQueueSend(g_queue, &msg, timeoutMs);
}

int IpcReceive(SystemMessage &out, uint32_t timeoutMs)
{
	return xQueueReceive(g_queue, &out, timeoutMs);
}