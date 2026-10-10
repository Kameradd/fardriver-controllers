#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <cstdint>
#include "dashboard_data.hpp"

static constexpr size_t kRingBufferSize = 512;

extern TaskHandle_t g_frameParserTask;
extern TaskHandle_t g_commandWriteTask;
extern TaskHandle_t g_keepAliveTask;
extern TaskHandle_t g_telemetryTask;
extern TaskHandle_t g_safetyMonitorTask;
extern TaskHandle_t g_displayTask;
extern TaskHandle_t g_loggerTask;
extern TaskHandle_t g_loraTask;

static constexpr UBaseType_t kPriorityFrameParser   = 8;
static constexpr UBaseType_t kPriorityCommandWrite  = 7;
static constexpr UBaseType_t kPriorityKeepAlive     = 5;
static constexpr UBaseType_t kPriorityTelemetry     = 6;
static constexpr UBaseType_t kPrioritySafetyMonitor = 9;
static constexpr UBaseType_t kPriorityDisplay       = 4;
static constexpr UBaseType_t kPriorityLogger        = 3;
static constexpr UBaseType_t kPriorityLoRa          = 4;

static constexpr uint32_t kStackFrameParser   = 4096;
static constexpr uint32_t kStackCommandWrite  = 3072;
static constexpr uint32_t kStackKeepAlive     = 2048;
static constexpr uint32_t kStackTelemetry     = 4096;
static constexpr uint32_t kStackSafetyMonitor = 3072;
static constexpr uint32_t kStackDisplay       = 3072;
static constexpr uint32_t kStackLogger        = 4096;
static constexpr uint32_t kStackLoRa          = 2048;

struct TelemetryItem {
    uint8_t addr;
    uint8_t data[12];
};

struct WriteCommand {
    uint8_t addr;
    uint16_t value;
    bool critical;
};

struct DisplayUpdate {
    DashboardData_t data;
    bool is_alert;
    uint8_t alert_code;
};

struct LogEntry {
    char csv_line[80];
};

struct LoRaPacket {
    char payload[64];
};

extern QueueHandle_t g_telemetryQueue;
extern QueueHandle_t g_writeQueue;
extern QueueHandle_t g_displayQueue;
extern QueueHandle_t g_logQueue;
extern QueueHandle_t g_loraQueue;

extern SemaphoreHandle_t g_dataMutex;

static constexpr UBaseType_t kTelemetryQueueDepth = 10;
static constexpr UBaseType_t kWriteQueueDepth     = 8;
static constexpr UBaseType_t kDisplayQueueDepth   = 16;
static constexpr UBaseType_t kLogQueueDepth       = 32;
static constexpr UBaseType_t kLoRaQueueDepth      = 8;

static constexpr TickType_t kKeepAliveIntervalMs  = pdMS_TO_TICKS(1000);
static constexpr TickType_t kSafetyIntervalMs     = pdMS_TO_TICKS(200);
static constexpr TickType_t kTelemetryTimeoutMs   = pdMS_TO_TICKS(100);
static constexpr TickType_t kDisplayIntervalMs    = pdMS_TO_TICKS(100);
static constexpr TickType_t kLoggerIntervalMs     = pdMS_TO_TICKS(1000);
static constexpr TickType_t kLoRaIntervalMs       = pdMS_TO_TICKS(200);
static constexpr TickType_t kReconnectTimeoutMs   = pdMS_TO_TICKS(5000);
