#include "log_worker.h"

#include "rtos_api.h"
#include "system_stats.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <iomanip>
#include <iostream>

namespace
{
    constexpr std::size_t kMaxLogText = 384;

    enum class LogRecordKind : uint32_t
    {
        Message,
        WriterComplete
    };

    struct LogRecord
    {
        LogRecordKind kind;
        uint32_t tick;
        std::array<char, kMaxLogText> text;
    };

    QueueHandle_t g_logQueue = nullptr;
    SemaphoreHandle_t g_logReady = nullptr;
    SemaphoreHandle_t g_consoleMutex = nullptr;
    uint32_t g_writerTaskCount = 0;
    uint32_t g_sendTimeoutMs = 0;

    bool QueueRecord(const LogRecord& record, uint32_t timeoutMs)
    {
        if (!g_logQueue || !g_logReady)
            return false;

        if (!xQueueSend(g_logQueue, &record, timeoutMs))
            return false;

        // The queue and counting semaphore have the same capacity. Under the
        // one-send/one-give design this give should always succeed. The bounded
        // yield loop preserves synchronization if the host scheduler briefly
        // exposes a timing race.
        while (!xSemaphoreGive(g_logReady))
            taskYIELD();

        return true;
    }

    void PrintRecord(const LogRecord& record)
    {
        if (!g_consoleMutex || !xSemaphoreTake(g_consoleMutex, 1000))
        {
            StatsRecordLogSyncError();
            return;
        }

        std::cout << "[t=" << std::setw(5)
            << record.tick << " ms] "
            << record.text.data() << std::endl;

        if (!xSemaphoreGive(g_consoleMutex))
            StatsRecordLogSyncError();
    }
}

int LogWorkerInit(uint32_t queueLength,
    uint32_t writerTaskCount,
    uint32_t sendTimeoutMs)
{
    if (queueLength == 0 || writerTaskCount == 0)
        return 0;

    g_logQueue = xQueueCreate(queueLength, sizeof(LogRecord));
    g_logReady = xSemaphoreCreateCounting(queueLength, 0);
    g_consoleMutex = xSemaphoreCreateMutex();
    g_writerTaskCount = writerTaskCount;
    g_sendTimeoutMs = sendTimeoutMs;

    if (!g_logQueue || !g_logReady || !g_consoleMutex)
    {
        LogWorkerShutdown();
        return 0;
    }

    return 1;
}

void LogWorkerShutdown()
{
    vSemaphoreDelete(g_consoleMutex);
    vSemaphoreDelete(g_logReady);
    vQueueDelete(g_logQueue);

    g_consoleMutex = nullptr;
    g_logReady = nullptr;
    g_logQueue = nullptr;
    g_writerTaskCount = 0;
    g_sendTimeoutMs = 0;
}

bool LogWorkerSubmit(const std::string& message)
{
    LogRecord record{};
    record.kind = LogRecordKind::Message;
    record.tick = xTaskGetTickCount();

    const std::size_t copyLength =
        std::min(message.size(), record.text.size() - 1);

    std::memcpy(record.text.data(), message.data(), copyLength);
    record.text[copyLength] = '\0';

    const uint32_t start = xTaskGetTickCount();
    const bool queued = QueueRecord(record, g_sendTimeoutMs);
    const uint32_t waited = xTaskGetTickCount() - start;

    if (queued)
        StatsRecordLogQueued(waited);
    else
        StatsRecordLogDrop(waited);

    return queued;
}

void LogWorkerNotifyTaskComplete()
{
    LogRecord record{};
    record.kind = LogRecordKind::WriterComplete;
    record.tick = xTaskGetTickCount();
    record.text[0] = '\0';

    // Task completion is a control record and must not be lost. A bounded send
    // is retried until the worker makes room in the queue.
    while (!QueueRecord(record, 100))
        taskYIELD();
}

void LogWorkerTask(void*)
{
    uint32_t writersRemaining = g_writerTaskCount;

    while (writersRemaining > 0)
    {
        if (!xSemaphoreTake(g_logReady, 1000))
            continue;

        LogRecord record{};

        if (!xQueueReceive(g_logQueue, &record, 0))
        {
            StatsRecordLogSyncError();
            continue;
        }

        if (record.kind == LogRecordKind::WriterComplete)
        {
            --writersRemaining;
            continue;
        }

        PrintRecord(record);
        StatsRecordLogWritten();
    }
}
