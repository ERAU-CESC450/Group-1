#include "rtos_api.h"
#include "isr.h"
#include "../shared/safe_log.h"


#include <string>

void isr_signal(int latency)
{
	SafeLogWrite(std::string("\n        =========High Latency Detected : ") + std::to_string(latency) + "ms == flagging for protocol============\n", 0);

	vTaskDelay(50);
}