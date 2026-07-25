/*#include "rtos_api.h"
#include "isr.h"
#include "../shared/safe_log.h"


#include <string>

void isr_signal(int latency)
{
	SafeLogWrite(std::string("\n        =========High Latency Detected : ") + std::to_string(latency) + "ms == flagging for protocol============\n", 0);

	vTaskDelay(50);
}*/
#include "isr.h"//M7 Tyler update 

#include "rtos_api.h"
#include "safe_log.h"

#include <cstdint>
#include <string>

namespace
{
	constexpr uint32_t kEventCount = 5;
	constexpr uint32_t kHandlerTimeoutMs = 2000;
	constexpr uint32_t kProcessingTimeMs = 75;

	SemaphoreHandle_t g_eventSemaphore = nullptr;

	// -----------------------------------------------------------------------------
	// ISR-equivalent
	// -----------------------------------------------------------------------------
	//
	// This function represents the interrupt service routine.
	//
	// ISR discipline:
	// - no logging
	// - no delay
	// - no blocking
	// - no application processing
	//
	// It only records the occurrence by giving the semaphore.
	//
	int SimulatedInterruptHandler()
	{
		return xSemaphoreGiveFromISR(
			g_eventSemaphore);
	}
}

bool InterruptSystemInit()
{
	// The semaphore can store all five simulated events
	// even if several arrive before the task processes them.
	g_eventSemaphore =
		xSemaphoreCreateCounting(
			8,
			0);

	return g_eventSemaphore != nullptr;
}

void InterruptSystemShutdown()
{
	vSemaphoreDelete(g_eventSemaphore);
	g_eventSemaphore = nullptr;
}

void SimulatedEventSourceTask(void *)
{
	// Uneven intervals demonstrate an asynchronous event source
	// that is independent of the producer and consumer loops.
	const uint32_t eventIntervalsMs[kEventCount] =
		{
			700,
			1100,
			450,
			1300,
			600};

	for (uint32_t eventNumber = 1;
		 eventNumber <= kEventCount;
		 ++eventNumber)
	{
		vTaskDelay(
			eventIntervalsMs[eventNumber - 1]);

		const uint32_t triggerTick =
			xTaskGetTickCount();

		const uint32_t isrStartTick =
			xTaskGetTickCount();

		const int signalResult =
			SimulatedInterruptHandler();

		const uint32_t isrDuration =
			xTaskGetTickCount() - isrStartTick;

		// These logs occur after the ISR-equivalent returns.
		// They are not inside the ISR.
		SafeLogWrite(
			"[EVENT SOURCE] asynchronous event " +
			std::to_string(eventNumber) +
			" triggered at tick=" +
			std::to_string(triggerTick));

		SafeLogWrite(
			"[EVENT SOURCE] ISR returned; duration=" +
			std::to_string(isrDuration) +
			"ms; ISR-safe semaphore=" +
			std::string(
				signalResult ? "GIVEN" : "FULL"));
	}

	SafeLogWrite(
		"[EVENT SOURCE] all simulated interrupts generated.");
}

void InterruptEventHandlerTask(void *)
{
	uint32_t handledEvents = 0;

	while (handledEvents < kEventCount)
	{
		const int eventReceived =
			xSemaphoreTake(
				g_eventSemaphore,
				kHandlerTimeoutMs);

		if (!eventReceived)
		{
			SafeLogWrite(
				"[EVENT TASK] waiting for asynchronous event...");

			continue;
		}

		++handledEvents;

		const uint32_t processingStart =
			xTaskGetTickCount();

		SafeLogWrite(
			"[EVENT TASK] received event " +
			std::to_string(handledEvents) +
			" at tick=" +
			std::to_string(processingStart) +
			"; processing in task context.");

		// This represents meaningful processing.
		// It is permitted here because this is task context,
		// not interrupt context.
		vTaskDelay(kProcessingTimeMs);

		SafeLogWrite(
			"[EVENT TASK] completed processing for event " +
			std::to_string(handledEvents) +
			" at tick=" +
			std::to_string(
				xTaskGetTickCount()));
	}

	SafeLogWrite(
		"[EVENT TASK] all asynchronous events handled.");
}
