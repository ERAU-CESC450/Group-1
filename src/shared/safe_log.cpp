#include "safe_log.h"

#include <iostream>
#include <mutex>

namespace
{
    std::mutex gLogMutex;
}

void SafeLogInit()
{
    // Nothing to initialize yet.
}

void SafeLogShutdown()
{
    // Nothing to clean up yet.
}

void SafeLogWrite(const std::string& message, uint32_t)
{
    std::lock_guard<std::mutex> lock(gLogMutex);

    std::cout << message << std::endl;
}