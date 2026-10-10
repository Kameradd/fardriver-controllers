#include "tasks.hpp"
#include "fardriver_data.hpp"
#include <Arduino.h>
#include <cstdio>

extern FardriverData g_fardriverData;
extern QueueHandle_t g_telemetryQueue;
extern QueueHandle_t g_displayQueue;
extern QueueHandle_t g_logQueue;
extern QueueHandle_t g_loraQueue;
extern SemaphoreHandle_t g_dataMutex;

enum DisplayWidget : uint8_t {
    kWVolt = 0,
    kWAmp = 1,
    kWSpeed = 2,
    kWTemp = 3,
    kWAlert = 4,
};

void telemetryTask(void *pv) {
    while (1) {
        TelemetryItem item;
        if (xQueueReceive(g_telemetryQueue, &item, kTelemetryTimeoutMs) != pdTRUE) {
            continue;
        }

        DashboardData_t dashData = {};
        float voltage = 0, current = 0, speed = 0, temp = 0, throttle = 0;
        if (xSemaphoreTake(g_dataMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
            dashData = createDashboardData(g_fardriverData);
            voltage  = g_fardriverData.getVoltage();
            current  = g_fardriverData.getLineCurrent();
            speed    = g_fardriverData.getSpeed();
            temp     = g_fardriverData.getMotorTemp();
            throttle = g_fardriverData.getThrottlePct();
            xSemaphoreGive(g_dataMutex);
        }

        DisplayUpdate dv = {dashData, false, 0};
        xQueueSend(g_displayQueue, &dv, 0);

        LogEntry le = {};
        snprintf(le.csv_line, sizeof(le.csv_line),
                 "%.1f,%.1f,%.1f,%.1f,%.1f", voltage, current, speed, temp, throttle);
        xQueueSend(g_logQueue, &le, 0);

        LoRaPacket lp = {};
        snprintf(lp.payload, sizeof(lp.payload),
                 "V%.1f,A%.1f,S%.1f,T%.1f,TH%.1f%%", voltage, current, speed, temp, throttle);
        Serial.printf("[TELEMETRY] %s\n", lp.payload);
        xQueueSend(g_loraQueue, &lp, 0);
    }
}

void initTelemetryTask() {
    xTaskCreatePinnedToCore(telemetryTask, "Telemetry",
        kStackTelemetry, nullptr, kPriorityTelemetry,
        &g_telemetryTask, 1);
}
