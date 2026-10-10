#include "tasks.hpp"
#include "safety_monitor.hpp"
#include "fardriver_data.hpp"
#include <cstdio>

extern FardriverData g_fardriverData;
extern QueueHandle_t g_displayQueue;
extern QueueHandle_t g_logQueue;
extern QueueHandle_t g_loraQueue;
extern SemaphoreHandle_t g_dataMutex;
extern SafetyConfig g_safetyConfig;

enum AlertWidget : uint8_t {
    kWAlertBox = 4,
};

static uint8_t violation_counts[8] = {0};

static SafetyAlert checkSafety(const FardriverData &data, const SafetyConfig &cfg) {
    float phaseA = data.getPhaseACurrent();
    float phaseC = data.getPhaseCCurrent();
    float voltage = data.getVoltage();
    float lineCurr = data.getLineCurrent();
    float motorTemp = static_cast<float>(data.addrF4.motor_temp);
    float mosTemp = static_cast<float>(data.addrD6.MosTemp);
    float speed = data.getSpeed();

    if (phaseA > cfg.max_phase_current_A) return SafetyAlert::PhaseCurrentHigh;
    if (phaseC > cfg.max_phase_current_A) return SafetyAlert::PhaseCurrentHigh;
    if (lineCurr > cfg.max_line_current_A) return SafetyAlert::LineCurrentHigh;
    if (motorTemp > cfg.max_motor_temp_C) return SafetyAlert::MotorTempHigh;
    if (mosTemp > cfg.max_mos_temp_C) return SafetyAlert::MosTempHigh;
    if (voltage < cfg.min_voltage_V) return SafetyAlert::VoltageLow;
    if (voltage > cfg.max_voltage_V) return SafetyAlert::VoltageHigh;
    if (speed > cfg.max_speed_kmh) return SafetyAlert::SpeedOvershoot;

    return SafetyAlert::None;
}

static void pushAlert(SafetyAlert alert, float value) {
    DashboardData_t dashData = createDashboardData(g_fardriverData);
    DisplayUpdate dv = {dashData, true, static_cast<uint8_t>(alert)};
    xQueueSend(g_displayQueue, &dv, 0);

    LogEntry le = {};
    snprintf(le.csv_line, sizeof(le.csv_line),
             "ALERT,%d,%.1f", static_cast<int>(alert), value);
    xQueueSend(g_logQueue, &le, 0);

    LoRaPacket lp = {};
    snprintf(lp.payload, sizeof(lp.payload),
             "ALERT:%d,VAL:%.1f", static_cast<int>(alert), value);
    xQueueSend(g_loraQueue, &lp, 0);
}

void safetyMonitorTask(void *pv) {
    TickType_t lastWake = xTaskGetTickCount();
    SafetyAlert prev_alert = SafetyAlert::None;

    while (1) {
        vTaskDelayUntil(&lastWake, kSafetyIntervalMs);

        SafetyAlert alert;
        float current_val = 0;
        if (xSemaphoreTake(g_dataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            alert = checkSafety(g_fardriverData, g_safetyConfig);
            switch (alert) {
            case SafetyAlert::PhaseCurrentHigh:
                current_val = g_fardriverData.getPhaseACurrent(); break;
            case SafetyAlert::LineCurrentHigh:
                current_val = g_fardriverData.getLineCurrent(); break;
            case SafetyAlert::MotorTempHigh:
                current_val = static_cast<float>(g_fardriverData.addrF4.motor_temp); break;
            case SafetyAlert::MosTempHigh:
                current_val = static_cast<float>(g_fardriverData.addrD6.MosTemp); break;
            case SafetyAlert::VoltageLow:
            case SafetyAlert::VoltageHigh:
                current_val = g_fardriverData.getVoltage(); break;
            case SafetyAlert::SpeedOvershoot:
                current_val = g_fardriverData.getSpeed(); break;
            default: break;
            }
            xSemaphoreGive(g_dataMutex);
        } else {
            continue;
        }

        uint8_t aidx = static_cast<uint8_t>(alert);

        if (alert != SafetyAlert::None) {
            if (alert == prev_alert) {
                violation_counts[aidx]++;
                if (violation_counts[aidx] >= g_safetyConfig.violation_threshold) {
                    pushAlert(alert, current_val);
                    violation_counts[aidx] = 0;
                }
            } else {
                for (int i = 0; i < 8; i++) violation_counts[i] = 0;
                violation_counts[aidx] = 1;
            }
        } else {
            for (int i = 0; i < 8; i++) violation_counts[i] = 0;
        }

        prev_alert = alert;
    }
}

void initSafetyMonitorTask() {
    xTaskCreatePinnedToCore(safetyMonitorTask, "SafetyMon",
        kStackSafetyMonitor, nullptr, kPrioritySafetyMonitor,
        &g_safetyMonitorTask, 1);
}
