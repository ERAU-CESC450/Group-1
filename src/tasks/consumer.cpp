#include "consumer.h"
#include "rtos_api.h"
#include "ipc.h"
#include "messages.h"
#include "safe_log.h"
#include "../interrupts/isr.h"

#include <string>

namespace
{
	constexpr uint32_t kRecvTimeoutMs = 400;
	constexpr uint32_t kProcessMs = 250;
}

void LoggerConsumerTask(void* params)
{
	const char* name = params ? static_cast<const char*>(params) : "Consumer";
	int received = 0;
	int timeouts = 0;

	for (;;)
	{
		SystemMessage msg;
		const int ok = IpcReceive(msg, kRecvTimeoutMs);

		if (!ok)
		{
			++timeouts;
			SafeLogWrite(std::string("   [") + name + "] receive timed out (" + std::to_string(kRecvTimeoutMs) + "ms) - waiting for producer...", 0);
			continue;
		}

		if (msg.kind == MsgKind::Stop)
		{
			SafeLogWrite(std::string("   [") + name + "] STOP received; processed " + std::to_string(received) + " messages, " + std::to_string(timeouts) + " receive timeouts.", 0);
			break;

		}


		++received;

		const uint32_t now = xTaskGetTickCount();
		const uint32_t latency = now - msg.tick;

		if (latency > 500)
		{
			isr_signal(latency);
		}
		SafeLogWrite(
			std::string("   [") + name +
			"] received seq-" + std::to_string(msg.seq) +
			" payload-" + std::to_string(msg.payload) +
			" latency=" + std::to_string(latency) + "ms",
			0);

		vTaskDelay(kProcessMs);
	}
}

