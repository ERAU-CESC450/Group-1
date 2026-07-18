#pragma once

#include <string>
#include <cstdint>

// Initializes the logging system.
void SafeLogInit();

// Cleans up the logging system.
void SafeLogShutdown();

// Thread-safe console logging.
// The color parameter is reserved for future use.
void SafeLogWrite(const std::string& message,
    uint32_t color = 0);