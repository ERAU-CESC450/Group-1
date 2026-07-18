#pragma once
#include <cstdint>

enum class MsgKind : uint32_t
{
	Data = 0,
	Stop = 1
};

struct SystemMessage
{
    MsgKind kind;
    uint32_t seq;
    uint32_t tick;
    uint32_t payload;
};