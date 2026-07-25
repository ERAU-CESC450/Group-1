/*#include "rtos_api.h"
#include "FreeRTOSConfig.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <deque>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <functional>

// -----------------------------------------------------------------------------
// Task shim
// -----------------------------------------------------------------------------

struct TaskHandle
{
    std::string name;
    uint32_t priority;
    std::thread th;
};

static std::mutex g_mutex;
static std::vector<TaskHandle*> g_tasks;
static std::atomic<uint32_t> g_ticks{ 0 };
static std::atomic<bool> g_schedulerStarted{ false };

static void tickThreadFn()
{
    using namespace std::chrono;

    auto last = steady_clock::now();
    while (g_schedulerStarted.load(std::memory_order_relaxed))
    {
        std::this_thread::sleep_for(milliseconds(1)); // scheduler-friendly pacing

        auto now = steady_clock::now();
        auto elapsedMs = duration_cast<milliseconds>(now - last).count();

        if (elapsedMs > 0)
        {
            g_ticks.fetch_add(static_cast<uint32_t>(elapsedMs),
                std::memory_order_relaxed);
            last = now;
        }
    }
}

int xTaskCreate(TaskFunction_t taskCode,
    const char* name,
    uint32_t stackWords,// comment here stackWords,
    void* params,
    uint32_t priority,
    TaskHandle_t* outHandle)
{
    if (!taskCode || !name)
        return 0;

    auto* h = new TaskHandle();
    h->name = name;
    h->priority = priority;

    // NOTE: This shim does not implement priority scheduling.
    // It creates native threads; priority is recorded for later modules.
    h->th = std::thread([taskCode, params, nm = h->name]()
        {
            try {
                taskCode(params);
            }
            catch (...) {
                std::cerr << "[RTOS] Task '" << nm << "' terminated with an exception.\n";
            } });

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_tasks.push_back(h);
    }

    if (outHandle)
        *outHandle = h;
    return 1;
}

void vTaskStartScheduler(void)
{
    // In a real RTOS, tasks would begin only after the scheduler starts.
    // Here, threads begin immediately on xTaskCreate. Scheduler manages tick + join.
    g_schedulerStarted.store(true, std::memory_order_relaxed);

    std::thread tickThread(tickThreadFn);

    // Wait for all tasks to finish
    std::vector<TaskHandle*> tasksCopy;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        tasksCopy = g_tasks;
    }

    for (auto* t : tasksCopy)
    {
        if (t && t->th.joinable())
            t->th.join();
    }

    g_schedulerStarted.store(false, std::memory_order_relaxed);
    if (tickThread.joinable())
        tickThread.join();

    // cleanup
    for (auto* t : tasksCopy)
        delete t;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
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
// Queue shim (Module 6 IPC)
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

static bool wait_until(std::condition_variable& cv,
    std::unique_lock<std::mutex>& lock,
    uint32_t timeoutMs,
    const std::function<bool()>& pred)
{
    if (timeoutMs == 0)
        return pred();

    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    while (!pred())
    {
        if (cv.wait_until(lock, deadline) == std::cv_status::timeout)
            break;
    }
    return pred();
}

QueueHandle_t xQueueCreate(uint32_t length, uint32_t itemSize)
{
    if (length == 0 || itemSize == 0)
        return nullptr;

    auto* h = new QueueHandle();
    h->capacity = length;
    h->itemSize = itemSize;
    return h;
}

int xQueueSend(QueueHandle_t q, const void* item, uint32_t timeoutMs)
{
    if (!q || !item)
        return 0;

    std::unique_lock<std::mutex> lock(q->mtx);

    auto canPush = [&]()
        { return q->q.size() < q->capacity; };

    if (!wait_until(q->cvNotFull, lock, timeoutMs, canPush))
        return 0;

    std::vector<uint8_t> buf(q->itemSize);
    std::memcpy(buf.data(), item, q->itemSize);
    q->q.push_back(std::move(buf));

    lock.unlock();
    q->cvNotEmpty.notify_one();
    return 1;
}

int xQueueReceive(QueueHandle_t q, void* outItem, uint32_t timeoutMs)
{
    if (!q || !outItem)
        return 0;

    std::unique_lock<std::mutex> lock(q->mtx);

    auto canPop = [&]()
        { return !q->q.empty(); };

    if (!wait_until(q->cvNotEmpty, lock, timeoutMs, canPop))
        return 0;

    auto buf = std::move(q->q.front());
    q->q.pop_front();

    std::memcpy(outItem, buf.data(), q->itemSize);

    lock.unlock();
    q->cvNotFull.notify_one();
    return 1;
}

void vQueueDelete(QueueHandle_t q)
{
    if (!q)
        return;
    delete q;
}*/
#include "rtos_api.h"
#include "FreeRTOSConfig.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <deque>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <functional>

// -----------------------------------------------------------------------------
// Task shim
// -----------------------------------------------------------------------------

struct TaskHandle
{
    std::string name;
    uint32_t priority;
    std::thread th;
};

static std::mutex g_mutex;
static std::vector<TaskHandle *> g_tasks;
static std::atomic<uint32_t> g_ticks{0};
static std::atomic<bool> g_schedulerStarted{false};

static void tickThreadFn()
{
    using namespace std::chrono;

    auto last = steady_clock::now();
    while (g_schedulerStarted.load(std::memory_order_relaxed))
    {
        std::this_thread::sleep_for(milliseconds(1)); // scheduler-friendly pacing

        auto now = steady_clock::now();
        auto elapsedMs = duration_cast<milliseconds>(now - last).count();

        if (elapsedMs > 0)
        {
            g_ticks.fetch_add(static_cast<uint32_t>(elapsedMs),
                              std::memory_order_relaxed);
            last = now;
        }
    }
}

int xTaskCreate(TaskFunction_t taskCode,
                const char *name,
                uint32_t /*stackWords*/,
                void *params,
                uint32_t priority,
                TaskHandle_t *outHandle)
{
    if (!taskCode || !name)
        return 0;

    auto *h = new TaskHandle();
    h->name = name;
    h->priority = priority;

    // NOTE: This shim does not implement priority scheduling.
    // It creates native threads; priority is recorded for later modules.
    h->th = std::thread([taskCode, params, nm = h->name]()
                        {
            try {
                taskCode(params);
            }
            catch (...) {
                std::cerr << "[RTOS] Task '" << nm << "' terminated with an exception.\n";
            } });

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_tasks.push_back(h);
    }

    if (outHandle)
        *outHandle = h;
    return 1;
}

void vTaskStartScheduler(void)
{
    // In a real RTOS, tasks would begin only after the scheduler starts.
    // Here, threads begin immediately on xTaskCreate. Scheduler manages tick + join.
    g_schedulerStarted.store(true, std::memory_order_relaxed);

    std::thread tickThread(tickThreadFn);

    // Wait for all tasks to finish
    std::vector<TaskHandle *> tasksCopy;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        tasksCopy = g_tasks;
    }

    for (auto *t : tasksCopy)
    {
        if (t && t->th.joinable())
            t->th.join();
    }

    g_schedulerStarted.store(false, std::memory_order_relaxed);
    if (tickThread.joinable())
        tickThread.join();

    // cleanup
    for (auto *t : tasksCopy)
        delete t;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
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
// Queue shim (Module 6 IPC)
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

static bool wait_until(std::condition_variable &cv,
                       std::unique_lock<std::mutex> &lock,
                       uint32_t timeoutMs,
                       const std::function<bool()> &pred)
{
    if (timeoutMs == 0)
        return pred();

    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    while (!pred())
    {
        if (cv.wait_until(lock, deadline) == std::cv_status::timeout)
            break;
    }
    return pred();
}

QueueHandle_t xQueueCreate(uint32_t length, uint32_t itemSize)
{
    if (length == 0 || itemSize == 0)
        return nullptr;

    auto *h = new QueueHandle();
    h->capacity = length;
    h->itemSize = itemSize;
    return h;
}

int xQueueSend(QueueHandle_t q, const void *item, uint32_t timeoutMs)
{
    if (!q || !item)
        return 0;

    std::unique_lock<std::mutex> lock(q->mtx);

    auto canPush = [&]()
    { return q->q.size() < q->capacity; };

    if (!wait_until(q->cvNotFull, lock, timeoutMs, canPush))
        return 0;

    std::vector<uint8_t> buf(q->itemSize);
    std::memcpy(buf.data(), item, q->itemSize);
    q->q.push_back(std::move(buf));

    lock.unlock();
    q->cvNotEmpty.notify_one();
    return 1;
}

int xQueueReceive(QueueHandle_t q, void *outItem, uint32_t timeoutMs)
{
    if (!q || !outItem)
        return 0;

    std::unique_lock<std::mutex> lock(q->mtx);

    auto canPop = [&]()
    { return !q->q.empty(); };

    if (!wait_until(q->cvNotEmpty, lock, timeoutMs, canPop))
        return 0;

    auto buf = std::move(q->q.front());
    q->q.pop_front();

    std::memcpy(outItem, buf.data(), q->itemSize);

    lock.unlock();
    q->cvNotFull.notify_one();
    return 1;
}

void vQueueDelete(QueueHandle_t q)
{
    if (!q)
        return;
    delete q;
}
// -----------------------------------------------------------------------------
// Counting semaphore shim
// -----------------------------------------------------------------------------

struct SemaphoreHandle
{
    uint32_t maxCount = 0;
    std::atomic<uint32_t> count{0};

    // These objects are used only when a task waits.
    // The ISR-safe give operation does not lock this mutex.
    std::mutex waitMutex;
    std::condition_variable cv;
};

SemaphoreHandle_t xSemaphoreCreateCounting(
    uint32_t maxCount,
    uint32_t initialCount)
{
    if (maxCount == 0 || initialCount > maxCount)
    {
        return nullptr;
    }

    auto *semaphore = new SemaphoreHandle();

    semaphore->maxCount = maxCount;
    semaphore->count.store(
        initialCount,
        std::memory_order_relaxed);

    return semaphore;
}

static bool tryTakeSemaphore(
    SemaphoreHandle_t semaphore)
{
    uint32_t current =
        semaphore->count.load(
            std::memory_order_acquire);

    while (current > 0)
    {
        if (semaphore->count.compare_exchange_weak(
                current,
                current - 1,
                std::memory_order_acq_rel,
                std::memory_order_acquire))
        {
            return true;
        }
    }

    return false;
}

int xSemaphoreGiveFromISR(
    SemaphoreHandle_t semaphore)
{
    if (!semaphore)
    {
        return 0;
    }

    uint32_t current =
        semaphore->count.load(
            std::memory_order_relaxed);

    while (current < semaphore->maxCount)
    {
        if (semaphore->count.compare_exchange_weak(
                current,
                current + 1,
                std::memory_order_release,
                std::memory_order_relaxed))
        {
            // Wake the task waiting for the event.
            // This operation does not wait for a mutex.
            semaphore->cv.notify_one();

            return 1;
        }
    }

    // The semaphore already contains the maximum
    // number of pending events.
    return 0;
}

int xSemaphoreTake(
    SemaphoreHandle_t semaphore,
    uint32_t timeoutMs)
{
    if (!semaphore)
    {
        return 0;
    }

    // First attempt without blocking.
    if (tryTakeSemaphore(semaphore))
    {
        return 1;
    }

    if (timeoutMs == 0)
    {
        return 0;
    }

    std::unique_lock<std::mutex> lock(
        semaphore->waitMutex);

    const bool eventAvailable =
        semaphore->cv.wait_for(
            lock,
            std::chrono::milliseconds(timeoutMs),
            [semaphore]()
            {
                return semaphore->count.load(
                           std::memory_order_acquire) > 0;
            });

    if (!eventAvailable)
    {
        return 0;
    }

    return tryTakeSemaphore(semaphore) ? 1 : 0;
}

void vSemaphoreDelete(
    SemaphoreHandle_t semaphore)
{
    delete semaphore;
}
