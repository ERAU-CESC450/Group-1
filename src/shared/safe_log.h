#ifndef CESC450_SAFE_LOG_H
#define CESC450_SAFE_LOG_H

#include <cstdint>
#include <string>

int SafeLogInit(uint32_t queueLength,
    uint32_t writerTaskCount,
    uint32_t sendTimeoutMs);

void SafeLogShutdown();
void SafeLogWrite(const std::string& message, uint32_t color = 0);
void SafeLogTaskComplete();
void SafeLogWorkerTask(void* params);

class SafeLogTaskCompletionGuard
{
public:
    SafeLogTaskCompletionGuard() = default;

    ~SafeLogTaskCompletionGuard()
    {
        SafeLogTaskComplete();
    }

    SafeLogTaskCompletionGuard(const SafeLogTaskCompletionGuard&) = delete;
    SafeLogTaskCompletionGuard& operator=(
        const SafeLogTaskCompletionGuard&) = delete;
};

#endif // CESC450_SAFE_LOG_H
