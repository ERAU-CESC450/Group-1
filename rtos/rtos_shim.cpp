#include "rtos_api.h"
#include "FreeRTOSConfig.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <deque>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// -----------------------------------------------------------------------------
// Task shim
// -----------------------------------------------------------------------------

struct TaskHandle
{
    std::string name;
    uint32_t priority;
    std::thread th;
};

static std::mutex g_taskListMutex;
static std::vector<TaskHandle*> g_tasks;
static std::atomic<uint32_t> g_ticks{ 0 };
static std::atomic<bool> g_schedulerStarted{ false };
static std::mutex g_startMutex;
static std::condition_variable g_startCv;

static void tickThreadFn()
{
    using namespace std::chrono;

    auto last = steady_clock::now();
    while (g_schedulerStarted.load(std::memory_order_relaxed))
    {
        std::this_thread::sleep_for(milliseconds(1));

        const auto now = steady_clock::now();
        const auto elapsedMs = duration_cast<milliseconds>(now - last).count();

        if (elapsedMs > 0)
        {
            g_ticks.fetch_add(
                static_cast<uint32_t>(elapsedMs),
                std::memory_order_relaxed);
            last = now;
        }
    }
}

int xTaskCreate(TaskFunction_t taskCode,
    const char* name,
    uint32_t /*stackWords*/,
    void* params,
    uint32_t priority,
    TaskHandle_t* outHandle)
{
    if (!taskCode || !name)
        return 0;

    auto* handle = new TaskHandle();
    handle->name = name;
    handle->priority = priority;

    // The desktop shim uses native threads. Tasks are held at a start barrier
    // until vTaskStartScheduler() is called. Priority is recorded to preserve
    // the intended RTOS design, but native priority scheduling is not emulated.
    handle->th = std::thread([taskCode, params, taskName = handle->name]()
        {
            {
                std::unique_lock<std::mutex> lock(g_startMutex);
                g_startCv.wait(lock, []
                    {
                        return g_schedulerStarted.load(std::memory_order_relaxed);
                    });
            }

            try
            {
                taskCode(params);
            }
            catch (...)
            {
                (void)taskName;
                std::cerr << "[FAIL] A task terminated with an exception.\n";
            }
        });

    {
        std::lock_guard<std::mutex> lock(g_taskListMutex);
        g_tasks.push_back(handle);
    }

    if (outHandle)
        *outHandle = handle;

    return 1;
}

void vTaskStartScheduler(void)
{
    g_schedulerStarted.store(true, std::memory_order_relaxed);

    std::thread tickThread(tickThreadFn);
    g_startCv.notify_all();

    std::vector<TaskHandle*> tasksCopy;
    {
        std::lock_guard<std::mutex> lock(g_taskListMutex);
        tasksCopy = g_tasks;
    }

    for (auto* task : tasksCopy)
    {
        if (task && task->th.joinable())
            task->th.join();
    }

    g_schedulerStarted.store(false, std::memory_order_relaxed);

    if (tickThread.joinable())
        tickThread.join();

    for (auto* task : tasksCopy)
        delete task;

    {
        std::lock_guard<std::mutex> lock(g_taskListMutex);
        g_tasks.clear();
    }
}

void vTaskDelay(uint32_t delayMs)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
}

void taskYIELD(void)
{
    std::this_thread::yield();
}

uint32_t xTaskGetTickCount(void)
{
    return g_ticks.load(std::memory_order_relaxed);
}

// -----------------------------------------------------------------------------
// Queue shim (IPC)
// -----------------------------------------------------------------------------

struct QueueHandle
{
    uint32_t capacity = 0;
    uint32_t itemSize = 0;

    std::mutex mtx;
    std::condition_variable cvNotEmpty;
    std::condition_variable cvNotFull;
    std::deque<std::vector<uint8_t>> q;
};

static bool waitUntil(std::condition_variable& cv,
    std::unique_lock<std::mutex>& lock,
    uint32_t timeoutMs,
    const std::function<bool()>& predicate)
{
    if (timeoutMs == 0)
        return predicate();

    const auto deadline = std::chrono::steady_clock::now() +
        std::chrono::milliseconds(timeoutMs);

    while (!predicate())
    {
        if (cv.wait_until(lock, deadline) == std::cv_status::timeout)
            break;
    }

    return predicate();
}

QueueHandle_t xQueueCreate(uint32_t length, uint32_t itemSize)
{
    if (length == 0 || itemSize == 0)
        return nullptr;

    auto* queue = new QueueHandle();
    queue->capacity = length;
    queue->itemSize = itemSize;
    return queue;
}

int xQueueSend(QueueHandle_t queue, const void* item, uint32_t timeoutMs)
{
    if (!queue || !item)
        return 0;

    std::unique_lock<std::mutex> lock(queue->mtx);

    const auto canPush = [&]()
        {
            return queue->q.size() < queue->capacity;
        };

    if (!waitUntil(queue->cvNotFull, lock, timeoutMs, canPush))
        return 0;

    std::vector<uint8_t> buffer(queue->itemSize);
    std::memcpy(buffer.data(), item, queue->itemSize);
    queue->q.push_back(std::move(buffer));

    lock.unlock();
    queue->cvNotEmpty.notify_one();
    return 1;
}

int xQueueReceive(QueueHandle_t queue, void* outItem, uint32_t timeoutMs)
{
    if (!queue || !outItem)
        return 0;

    std::unique_lock<std::mutex> lock(queue->mtx);

    const auto canPop = [&]()
        {
            return !queue->q.empty();
        };

    if (!waitUntil(queue->cvNotEmpty, lock, timeoutMs, canPop))
        return 0;

    auto buffer = std::move(queue->q.front());
    queue->q.pop_front();
    std::memcpy(outItem, buffer.data(), queue->itemSize);

    lock.unlock();
    queue->cvNotFull.notify_one();
    return 1;
}

void vQueueDelete(QueueHandle_t queue)
{
    delete queue;
}

// -----------------------------------------------------------------------------
// Counting semaphore and mutex shim
// -----------------------------------------------------------------------------

struct SemaphoreHandle
{
    uint32_t maxCount = 0;
    std::atomic<uint32_t> count{ 0 };
    bool isMutex = false;

    // Tasks wait on this condition variable. The ISR-style give operation does
    // not acquire waitMutex, which keeps the signal path short and non-blocking.
    std::mutex waitMutex;
    std::condition_variable cv;

    // Mutex ownership is tracked for task-context mutexes. This shim does not
    // implement priority inheritance because it does not emulate RTOS priorities.
    std::mutex ownerMutex;
    std::thread::id owner;
};

static SemaphoreHandle_t createSemaphore(
    uint32_t maxCount,
    uint32_t initialCount,
    bool isMutex)
{
    if (maxCount == 0 || initialCount > maxCount)
        return nullptr;

    auto* semaphore = new SemaphoreHandle();
    semaphore->maxCount = maxCount;
    semaphore->count.store(initialCount, std::memory_order_relaxed);
    semaphore->isMutex = isMutex;
    return semaphore;
}

SemaphoreHandle_t xSemaphoreCreateCounting(
    uint32_t maxCount,
    uint32_t initialCount)
{
    return createSemaphore(maxCount, initialCount, false);
}

SemaphoreHandle_t xSemaphoreCreateMutex(void)
{
    // A mutex is represented as a binary semaphore that begins available.
    return createSemaphore(1, 1, true);
}

static bool tryTakeSemaphore(SemaphoreHandle_t semaphore)
{
    uint32_t current = semaphore->count.load(std::memory_order_acquire);

    while (current > 0)
    {
        if (semaphore->count.compare_exchange_weak(
            current,
            current - 1,
            std::memory_order_acq_rel,
            std::memory_order_acquire))
        {
            if (semaphore->isMutex)
            {
                std::lock_guard<std::mutex> ownerLock(semaphore->ownerMutex);
                semaphore->owner = std::this_thread::get_id();
            }

            return true;
        }
    }

    return false;
}

int xSemaphoreTake(SemaphoreHandle_t semaphore, uint32_t timeoutMs)
{
    if (!semaphore)
        return 0;

    if (tryTakeSemaphore(semaphore))
        return 1;

    if (timeoutMs == 0)
        return 0;

    const auto deadline = std::chrono::steady_clock::now() +
        std::chrono::milliseconds(timeoutMs);

    std::unique_lock<std::mutex> lock(semaphore->waitMutex);

    for (;;)
    {
        if (tryTakeSemaphore(semaphore))
            return 1;

        const auto now = std::chrono::steady_clock::now();
        if (now >= deadline)
            return 0;

        // A short bounded wake interval also protects the host simulation from
        // a missed notification when xSemaphoreGiveFromISR() deliberately avoids
        // taking waitMutex.
        const auto pollDeadline = std::min(
            deadline,
            now + std::chrono::milliseconds(5));

        semaphore->cv.wait_until(lock, pollDeadline);
    }
}

static int giveCountingSemaphore(SemaphoreHandle_t semaphore)
{
    uint32_t current = semaphore->count.load(std::memory_order_relaxed);

    while (current < semaphore->maxCount)
    {
        if (semaphore->count.compare_exchange_weak(
            current,
            current + 1,
            std::memory_order_release,
            std::memory_order_relaxed))
        {
            semaphore->cv.notify_one();
            return 1;
        }
    }

    return 0;
}

int xSemaphoreGive(SemaphoreHandle_t semaphore)
{
    if (!semaphore)
        return 0;

    if (semaphore->isMutex)
    {
        {
            std::lock_guard<std::mutex> ownerLock(semaphore->ownerMutex);

            if (semaphore->owner != std::this_thread::get_id())
                return 0;

            semaphore->owner = std::thread::id{};
        }

        uint32_t expected = 0;
        if (!semaphore->count.compare_exchange_strong(
            expected,
            1,
            std::memory_order_release,
            std::memory_order_relaxed))
        {
            return 0;
        }

        semaphore->cv.notify_one();
        return 1;
    }

    // Normal task-context give. The atomic count keeps the critical section
    // short, and the condition variable wakes one blocked task.
    return giveCountingSemaphore(semaphore);
}

int xSemaphoreGiveFromISR(SemaphoreHandle_t semaphore)
{
    if (!semaphore || semaphore->isMutex)
        return 0;

    // ISR-style signaling is restricted to counting semaphores. It performs a
    // bounded atomic increment and notification, with no logging or delay.
    return giveCountingSemaphore(semaphore);
}

void vSemaphoreDelete(SemaphoreHandle_t semaphore)
{
    delete semaphore;
}
