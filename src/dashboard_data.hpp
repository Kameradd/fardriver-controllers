#pragma once
#include <cstdint>
#include <cstddef>
#include <algorithm>
#include "fardriver_data.hpp"

#pragma pack(push, 1)

// Dashboard Telemetry Data Packet (Total Size: 15 Bytes)
typedef struct __attribute__((__packed__)) {
    uint16_t voltage_raw;       // Scaled x100 (e.g., 7240 = 72.40 V)   [2 Bytes]
    int16_t  current_raw;       // Scaled x10  (e.g., 1500 = 150.0 A)   [2 Bytes]
    uint8_t  battery_soc;       // 0 to 100%                            [1 Byte]
    int8_t   temp_motor;        // -128 to +127 deg C                   [1 Byte]
    int8_t   temp_controller;   // -128 to +127 deg C                   [1 Byte]
    int8_t   temp_battery;      // -128 to +127 deg C                   [1 Byte]
    uint16_t controller_status; // Bitmask flags for faults/status       [2 Bytes]
    uint16_t vehicle_speed_raw; // Scaled x10  (e.g., 855 = 85.5 km/h)   [2 Bytes]
    uint16_t motor_rpm;         // Direct RPM value                     [2 Bytes]
    uint8_t  crc8;              // Frame integrity check                [1 Byte]
} DashboardData_t;

static_assert(sizeof(DashboardData_t) == 15, "DashboardData_t must be exactly 15 bytes");

#pragma pack(pop)

/**
 * @brief Calculate CRC-8 checksum for DashboardData_t (polynomial 0x07).
 * 
 * @param data Pointer to byte buffer.
 * @param len Number of bytes to process.
 * @return uint8_t Computed CRC8 checksum.
 */
inline uint8_t calculateDashboardCRC8(const uint8_t *data, size_t len) {
    uint8_t crc = 0x00;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = static_cast<uint8_t>((crc << 1) ^ 0x07);
            } else {
                crc = static_cast<uint8_t>(crc << 1);
            }
        }
    }
    return crc;
}

/**
 * @brief Verify frame integrity of a DashboardData_t packet.
 * 
 * @param dash The dashboard packet to verify.
 * @return true if calculated CRC matches dash.crc8, false otherwise.
 */
inline bool verifyDashboardCRC8(const DashboardData_t &dash) {
    uint8_t computed = calculateDashboardCRC8(
        reinterpret_cast<const uint8_t*>(&dash), sizeof(DashboardData_t) - 1);
    return computed == dash.crc8;
}

/**
 * @brief Convert FardriverData into a packed 15-byte DashboardData_t struct for display input.
 * 
 * @param fdData Reference to the current FardriverData.
 * @param tempBattery Battery temperature in deg C (default 25 deg C).
 * @return DashboardData_t Populated and CRC-signed dashboard packet.
 */
inline DashboardData_t createDashboardData(const FardriverData &fdData, int8_t tempBattery = 25) {
    DashboardData_t pkt;

    // 1. Voltage raw: Scaled x100 (e.g. 72.40 V -> 7240)
    float voltage = fdData.getVoltage();
    if (voltage < 0.0f) voltage = 0.0f;
    if (voltage > 655.35f) voltage = 655.35f;
    pkt.voltage_raw = static_cast<uint16_t>(voltage * 100.0f + 0.5f);

    // 2. Current raw: Scaled x10 (e.g. 150.0 A -> 1500)
    float current = fdData.getLineCurrent();
    if (current < -3276.8f) current = -3276.8f;
    if (current > 3276.7f) current = 3276.7f;
    pkt.current_raw = static_cast<int16_t>(current >= 0.0f ? (current * 10.0f + 0.5f) : (current * 10.0f - 0.5f));

    // 3. Battery SOC: 0 to 100%
    float soc = fdData.getBatteryP();
    if (soc < 0.0f) soc = 0.0f;
    if (soc > 100.0f) soc = 100.0f;
    pkt.battery_soc = static_cast<uint8_t>(soc + 0.5f);

    // 4. Temp Motor: -128 to +127 deg C
    int16_t mTemp = fdData.addrF4.motor_temp;
    if (mTemp < -128) mTemp = -128;
    if (mTemp > 127) mTemp = 127;
    pkt.temp_motor = static_cast<int8_t>(mTemp);

    // 5. Temp Controller (MOSFET): -128 to +127 deg C
    int16_t cTemp = fdData.addrD6.MosTemp;
    if (cTemp < -128) cTemp = -128;
    if (cTemp > 127) cTemp = 127;
    pkt.temp_controller = static_cast<int8_t>(cTemp);

    // 6. Temp Battery: -128 to +127 deg C
    pkt.temp_battery = tempBattery;

    // 7. Controller Status bitmask:
    //    Bit 0: Motor Hall error
    //    Bit 1: Throttle error
    //    Bit 2: Voltage protection
    //    Bit 3: Alarm protection
    //    Bit 4: Motor temp protection
    //    Bit 5: Controller temp protection
    //    Bit 6: Phase current overflow protection
    //    Bit 7: MOSFET check status
    //    Bit 8: Brake status
    //    Bit 9: Forward direction
    //    Bit 10: Reverse direction
    //    Bit 11: Phase lost alarm
    //    Bit 12: Low voltage stop
    //    Bit 13: Block status
    //    Bit 14: E-ABS active
    //    Bit 15: Error stop
    uint16_t status = 0;
    if (fdData.addrE2.motor_hall_error)              status |= (1 << 0);
    if (fdData.addrE2.throttle_error)                status |= (1 << 1);
    if (fdData.addrE2.voltage_protect)               status |= (1 << 2);
    if (fdData.addrE2.alarm_protect)                 status |= (1 << 3);
    if (fdData.addrE2.motor_temp_protect)            status |= (1 << 4);
    if (fdData.addrE2.controller_temp_protect)       status |= (1 << 5);
    if (fdData.addrE2.phase_current_overflow_protect) status |= (1 << 6);
    if (fdData.addrE2.MosfetCheck)                   status |= (1 << 7);
    if (fdData.addrE2.brake)                         status |= (1 << 8);
    if (fdData.addrE2.forward)                       status |= (1 << 9);
    if (fdData.addrE2.reverse)                       status |= (1 << 10);
    if (fdData.addrD6.PhaseLostAlarm)                 status |= (1 << 11);
    if (fdData.addrD6.LowVolStop)                    status |= (1 << 12);
    if (fdData.addrFA.BlockStat)                      status |= (1 << 13);
    if (fdData.addrFA.EABSStat)                       status |= (1 << 14);
    if (fdData.addrFA.ErrorStop)                      status |= (1 << 15);
    pkt.controller_status = status;

    // 8. Vehicle Speed raw: Scaled x10 (e.g. 85.5 km/h -> 855)
    float speed = fdData.getSpeed();
    if (speed < 0.0f) speed = 0.0f;
    if (speed > 6553.5f) speed = 6553.5f;
    pkt.vehicle_speed_raw = static_cast<uint16_t>(speed * 10.0f + 0.5f);

    // 9. Motor RPM: Direct value
    pkt.motor_rpm = fdData.addrE2.MeasureSpeed;

    // 10. CRC8 checksum computed over bytes 0..13
    pkt.crc8 = calculateDashboardCRC8(
        reinterpret_cast<const uint8_t*>(&pkt), sizeof(DashboardData_t) - 1);

    return pkt;
}
