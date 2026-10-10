#include "tasks.hpp"
#include <HardwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <RtcDS1302.h>
#include <cstdio>

HardwareSerial nextionSerial(2);
static constexpr int kLoraAuxPin = 4;
SoftwareSerial loraSerial(6, 5); // RX = GPIO 6, TX = GPIO 5

ThreeWire rtcWire(13, 12, 11);
RtcDS1302<ThreeWire> rtc(rtcWire);
File g_logFile;

static constexpr int kSdSCK  = 8;
static constexpr int kSdMISO = 10;
static constexpr int kSdMOSI = 9;
static constexpr int kSdCS   = 38;

SPIClass sdSpi(FSPI);

static void sendNextionCmd(const char *cmd) {
    nextionSerial.print(cmd);
    nextionSerial.write(0xFF);
    nextionSerial.write(0xFF);
    nextionSerial.write(0xFF);
}

static void sendNextionTxt(const char *widget, const char *val) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%s.txt=\"%s\"", widget, val);
    sendNextionCmd(buf);
}

static void sendNextionVal(const char *widget, int32_t val) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%s.val=%ld", widget, static_cast<long>(val));
    sendNextionCmd(buf);
}

void displayTask(void *pv) {
    while (1) {
        DisplayUpdate update;
        if (xQueueReceive(g_displayQueue, &update, kDisplayIntervalMs) == pdTRUE) {
            // 1. Output raw 15-byte DashboardData_t packed frame for binary displays
            nextionSerial.write(reinterpret_cast<const uint8_t *>(&update.data), sizeof(DashboardData_t));

            // 2. Update Nextion HMI widgets with scaled & formatted values
            char formatBuf[32];

            // Voltage (e.g. 7240 -> 72.40 V)
            snprintf(formatBuf, sizeof(formatBuf), "%.2f", update.data.voltage_raw / 100.0f);
            sendNextionTxt("tVolt", formatBuf);
            sendNextionVal("nVolt", update.data.voltage_raw);

            // Current (e.g. 1500 -> 150.0 A)
            snprintf(formatBuf, sizeof(formatBuf), "%.1f", update.data.current_raw / 10.0f);
            sendNextionTxt("tAmp", formatBuf);
            sendNextionVal("nAmp", update.data.current_raw);

            // Battery SOC (0..100%)
            snprintf(formatBuf, sizeof(formatBuf), "%u%%", update.data.battery_soc);
            sendNextionTxt("tSoc", formatBuf);
            sendNextionVal("nSoc", update.data.battery_soc);

            // Temperatures (deg C)
            snprintf(formatBuf, sizeof(formatBuf), "%d C", update.data.temp_motor);
            sendNextionTxt("tTmpM", formatBuf);
            sendNextionVal("nTmpM", update.data.temp_motor);

            snprintf(formatBuf, sizeof(formatBuf), "%d C", update.data.temp_controller);
            sendNextionTxt("tTmpC", formatBuf);
            sendNextionVal("nTmpC", update.data.temp_controller);

            snprintf(formatBuf, sizeof(formatBuf), "%d C", update.data.temp_battery);
            sendNextionTxt("tTmpB", formatBuf);
            sendNextionVal("nTmpB", update.data.temp_battery);

            // Controller Status bitmask
            snprintf(formatBuf, sizeof(formatBuf), "0x%04X", update.data.controller_status);
            sendNextionTxt("tStat", formatBuf);
            sendNextionVal("nStat", update.data.controller_status);

            // Vehicle Speed (e.g. 855 -> 85.5 km/h)
            snprintf(formatBuf, sizeof(formatBuf), "%.1f", update.data.vehicle_speed_raw / 10.0f);
            sendNextionTxt("tSpeed", formatBuf);
            sendNextionVal("nSpeed", update.data.vehicle_speed_raw);

            // Motor RPM
            snprintf(formatBuf, sizeof(formatBuf), "%u", update.data.motor_rpm);
            sendNextionTxt("tRpm", formatBuf);
            sendNextionVal("nRpm", update.data.motor_rpm);

            // CRC8 checksum
            sendNextionVal("nCrc", update.data.crc8);

            // Alert status box if alert flag is active
            if (update.is_alert) {
                snprintf(formatBuf, sizeof(formatBuf), "ALERT: Code %u", update.alert_code);
                sendNextionTxt("tAlert", formatBuf);
            }
        }
    }
}

void initDisplayTask() {
    nextionSerial.begin(9600, SERIAL_8N1, 16, 15);
    xTaskCreatePinnedToCore(displayTask, "Display",
        kStackDisplay, nullptr, kPriorityDisplay,
        &g_displayTask, 1);
}

static bool g_sdMounted = false;

void loggerTask(void *pv) {
    while (1) {
        LogEntry entry;
        if (xQueueReceive(g_logQueue, &entry, kLoggerIntervalMs) == pdTRUE) {
            if (g_sdMounted && g_logFile) {
                g_logFile.println(entry.csv_line);
                g_logFile.flush();
            }
        }
    }
}

void initLoggerTask() {
    sdSpi.begin(kSdSCK, kSdMISO, kSdMOSI, kSdCS);
    if (!SD.begin(kSdCS, sdSpi)) {
        Serial.println("[LOGGER] SD Card not detected / failed to mount. SD logging skipped (system continuing).");
        g_sdMounted = false;
    } else {
        Serial.println("[LOGGER] SD Card mounted successfully.");
        g_sdMounted = true;
    }

    rtc.Begin();
    if (!rtc.IsDateTimeValid()) {
        rtc.SetDateTime(RtcDateTime(__DATE__, __TIME__));
    }
    rtc.SetIsRunning(true);

    if (g_sdMounted) {
        RtcDateTime now = rtc.GetDateTime();
        char filename[32];
        snprintf(filename, sizeof(filename),
                 "/LOG_%04u-%02u-%02u_%02u-%02u-%02u.CSV",
                 now.Year(), now.Month(), now.Day(),
                 now.Hour(), now.Minute(), now.Second());
        g_logFile = SD.open(filename, FILE_WRITE);
        if (g_logFile) {
            g_logFile.println("Voltage,Current,Speed,Temp");
            g_logFile.flush();
        }
    }

    xTaskCreatePinnedToCore(loggerTask, "Logger",
        kStackLogger, nullptr, kPriorityLogger,
        &g_loggerTask, 1);
}

void loraTask(void *pv) {
    while (1) {
        LoRaPacket packet;
        if (xQueueReceive(g_loraQueue, &packet, kLoRaIntervalMs) == pdTRUE) {
            loraSerial.println(packet.payload);
        }
    }
}

void initLoRaTask() {
    pinMode(kLoraAuxPin, INPUT);
    loraSerial.begin(9600);
    xTaskCreatePinnedToCore(loraTask, "LoRa",
        kStackLoRa, nullptr, kPriorityLoRa,
        &g_loraTask, 1);
}
