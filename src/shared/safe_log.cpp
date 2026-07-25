#include "safe_log.h"
#include "log_worker.h"

int SafeLogInit(uint32_t queueLength,
    uint32_t writerTaskCount,
    uint32_t sendTimeoutMs)
{
    return LogWorkerInit(queueLength,
        writerTaskCount,
        sendTimeoutMs);
}

void SafeLogShutdown()
{
    LogWorkerShutdown();
}

void SafeLogWrite(const std::string& message,
    uint32_t)
{
    LogWorkerSubmit(message);
}

void SafeLogTaskComplete()
{
    LogWorkerNotifyTaskComplete();
}

void SafeLogWorkerTask(void* params)
{
    LogWorkerTask(params);
}
