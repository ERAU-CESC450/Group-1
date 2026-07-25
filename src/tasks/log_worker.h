#pragma once

#include <cstdint>
#include <string>

// Creates the asynchronous logging queue, counting semaphore, and console mutex.
// writerTaskCount is the number of application tasks that will call
// LogWorkerNotifyTaskComplete() before they exit.
int LogWorkerInit(uint32_t queueLength,
    uint32_t writerTaskCount,
    uint32_t sendTimeoutMs);

void LogWorkerShutdown();

// Queues one timestamped log message. The calling task performs only a bounded
// queue send; console output is deferred to LogWorkerTask().
bool LogWorkerSubmit(const std::string& message);

// Called once by each task that can submit log messages. The worker exits only
// after all registered writer tasks have completed and all earlier log records
// have been printed.
void LogWorkerNotifyTaskComplete();

// RTOS task entry point that owns normal console output.
void LogWorkerTask(void* params);
