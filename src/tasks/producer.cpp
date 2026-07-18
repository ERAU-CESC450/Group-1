#include "producer.h"
#include "rtos_api.h"
#include "ipc.h"
#include "messages.h"
#include "safe_log.h"

#include <string>

namespace
{
	constexpr int kMessages = 12;
	constexpr uint32_t kPeriodMs = 100;
	constexpr uint32_t kSendTimeoutMs = 2000;
	constexpr int kPauseAfterSend = 6;
	constexpr uint32_t kPauseMs = 1500;
}

void SensorProducerTask(void *params)
{
	const char *name = params ? static_cast<const char *>(params) : "Producer";

	for (int i = 0; i < kMessages; ++i)
	{
		SystemMessage msg;
		msg.kind = MsgKind::Data;
		msg.seq = static_cast<uint32_t>(i + 1);
		msg.tick = xTaskGetTickCount();
		msg.payload = static_cast<uint32_t>(20 + (i % 5));

		const uint32_t t0 = xTaskGetTickCount();
		const int ok = IpcSend(msg, kSendTimeoutMs);
		const uint32_t waited = xTaskGetTickCount() - t0;

		if (!ok)
		{
			SafeLogWrite(std::string("[") + name + "] DROP seq-" + std::to_string(msg.seq) + " (queue full, send timed out)", 0);
		}
		else if (waited > 20)
		{
			SafeLogWrite(std::string("[") + name + "] sent seq-" + std::to_string(msg.seq) + " payload-" + std::to_string(msg.payload) + " <- backpressure: waited " + std::to_string(waited) + "ms for space", 0);
		}
		else
		{
			SafeLogWrite(std::string("[") + name + "] sent seq-" + std::to_string(msg.seq) + " payload-" + std::to_string(msg.payload), 0);
		}

		vTaskDelay(kPeriodMs);

		if (i + 1 == kPauseAfterSend)
		{
			SafeLogWrite(std::string("[") + name + "] pausing " + std::to_string(kPauseMs) + "ms (consumer will see a receive timeout)", 0);
			vTaskDelay(kPauseMs);
		}
	}


	SystemMessage stop;
	stop.kind = MsgKind::Stop;
	stop.seq = 0;
	stop.tick = xTaskGetTickCount();
	stop.payload = 0;
	IpcSend(stop, kSendTimeoutMs);
	SafeLogWrite(std::string("[") + name + "] all data sent; STOP enqueued.", 0);
}