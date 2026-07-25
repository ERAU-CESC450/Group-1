#include "system_config.h"
#include <stdexcept>

namespace
{
    const SystemConfig kBaseline{
        "baseline-stress",
        3,
        4,
        32,
        10,
        100,
        250,
        200,
        400,
        450,
        50,
        220,
        300,
        600,
        3,
        24
    };

    const SystemConfig kStabilized{
        "stabilized",
        8,
        8,
        64,
        10,
        100,
        75,
        50,
        400,
        450,
        50,
        220,
        300,
        600,
        3,
        24
    };

    const SystemConfig* g_config = nullptr;
}

const SystemConfig& BaselineConfig()
{
    return kBaseline;
}

const SystemConfig& StabilizedConfig()
{
    return kStabilized;
}

void SetSystemConfig(const SystemConfig* config)
{
    g_config = config;
}

const SystemConfig& GetSystemConfig()
{
    if (!g_config)
        throw std::logic_error("System configuration was not initialized");
    return *g_config;
}
