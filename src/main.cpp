#include "tasks.hpp"
#include "ring_buffer.hpp"
#include "fardriver_controller.hpp"
#include "safety_monitor.hpp"
#include <Arduino.h>
#include <driver/uart.h>

RingBuffer<kRingBufferSize> g_rxRingBuffer;
FardriverData g_fardriverData;
SafetyConfig g_safetyConfig = kDefaultSafetyConfig;

QueueHandle_t g_telemetryQueue = nullptr;
QueueHandle_t g_writeQueue = nullptr;
QueueHandle_t g_displayQueue = nullptr;
QueueHandle_t g_logQueue = nullptr;
QueueHandle_t g_loraQueue = nullptr;
SemaphoreHandle_t g_dataMutex = nullptr;

TaskHandle_t g_frameParserTask = nullptr;
TaskHandle_t g_commandWriteTask = nullptr;
TaskHandle_t g_keepAliveTask = nullptr;
TaskHandle_t g_telemetryTask = nullptr;
TaskHandle_t g_safetyMonitorTask = nullptr;
TaskHandle_t g_displayTask = nullptr;
TaskHandle_t g_loggerTask = nullptr;
TaskHandle_t g_loraTask = nullptr;

static uint32_t serial_write(const uint8_t *data, uint32_t length) {
    return uart_write_bytes(UART_NUM_1, data, length);
}

static uint32_t serial_read(uint8_t *data, uint32_t length) {
    return uart_read_bytes(UART_NUM_1, data, length, pdMS_TO_TICKS(50));
}

static uint32_t serial_available(void) {
    size_t avail = 0;
    uart_get_buffered_data_len(UART_NUM_1, &avail);
    return avail;
}

static void serial_delay_ms(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

FardriverSerial g_fdSerial = {
    .write = serial_write,
    .read = serial_read,
    .available = serial_available,
    .delay_ms = serial_delay_ms
};

FardriverController *g_controller = nullptr;

void initFrameParserTask();
void initCommandWriteTask();
void initKeepAliveTask();
void initTelemetryTask();
void initSafetyMonitorTask();
void initDisplayTask();
void initLoggerTask();
void initLoRaTask();

void setup() {
    Serial.begin(115200);
    uint32_t t0 = millis();
    while (!Serial && (millis() - t0 < 2000)) {
        delay(10);
    }

    Serial.println();
    Serial.println("========================================");
    Serial.println("   Fardriver RTOS Firmware Starting     ");
    Serial.println("========================================");

    g_controller = new FardriverController(&g_fdSerial, &g_fardriverData);

    g_telemetryQueue = xQueueCreate(kTelemetryQueueDepth, sizeof(TelemetryItem));
    g_writeQueue = xQueueCreate(kWriteQueueDepth, sizeof(WriteCommand));
    g_displayQueue = xQueueCreate(kDisplayQueueDepth, sizeof(DisplayUpdate));
    g_logQueue = xQueueCreate(kLogQueueDepth, sizeof(LogEntry));
    g_loraQueue = xQueueCreate(kLoRaQueueDepth, sizeof(LoRaPacket));
    g_dataMutex = xSemaphoreCreateMutex();

    Serial.println("[INIT] Initializing RTOS tasks...");

    initFrameParserTask();
    initCommandWriteTask();
    initKeepAliveTask();
    initTelemetryTask();
    initSafetyMonitorTask();
    initDisplayTask();
    initLoggerTask();
    initLoRaTask();

    g_controller->open();

    Serial.println("[INIT] Controller serial interface opened.");
    Serial.println("[INIT] All tasks running. System ready.");
    Serial.println("----------------------------------------");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));

    float voltage = 0.0f, current = 0.0f, speed = 0.0f, temp = 0.0f, throttle = 0.0f;
    if (g_dataMutex && xSemaphoreTake(g_dataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        voltage  = g_fardriverData.getVoltage();
        current  = g_fardriverData.getLineCurrent();
        speed    = g_fardriverData.getSpeed();
        temp     = g_fardriverData.getMotorTemp();
        throttle = g_fardriverData.getThrottlePct();
        xSemaphoreGive(g_dataMutex);
    }

    char debugBuf[80];
    snprintf(debugBuf, sizeof(debugBuf),
             "V%.1f,A%.1f,S%.1f,T%.1f,TH%.1f%%", voltage, current, speed, temp, throttle);
    Serial.println(debugBuf);
}
