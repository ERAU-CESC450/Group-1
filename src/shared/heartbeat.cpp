#include "heartbeat.h"

#include <atomic>

namespace
{
	std::atomic<uint32_t> g_lastSeenTick{ 0 };
	std::atomic<uint32_t> g_beatCount{ 0 };
	std::atomic<bool> g_active{ false };
}

void HeartbeatInit(uint32_t nowTick)
{
	g_lastSeenTick.store(nowTick, std::memory_order_relaxed);
	g_beatCount.store(0, std::memory_order_relaxed);
	g_active.store(true, std::memory_order_relaxed);
}

void HeartbeatPublish(uint32_t nowTick)
{
	g_lastSeenTick.store(nowTick, std::memory_order_relaxed);
	g_beatCount.fetch_add(1, std::memory_order_relaxed);
}

uint32_t HeartbeatLastSeenTick(void)
{
	return g_lastSeenTick.load(std::memory_order_relaxed);
}

uint32_t HeartbeatCount(void)
{
	return g_beatCount.load(std::memory_order_relaxed);
}

void HeartbeatSetActive(bool active)
{
	g_active.store(active, std::memory_order_relaxed);
}

bool HeartbeatActive(void)
{
	return g_active.load(std::memory_order_relaxed);
}