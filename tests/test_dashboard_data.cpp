#include <gtest/gtest.h>
#include "../src/dashboard_data.hpp"

TEST(DashboardDataTest, StructSizeIs15Bytes) {
    EXPECT_EQ(sizeof(DashboardData_t), 15u);
}

TEST(DashboardDataTest, CRC8CalculationAndVerification) {
    DashboardData_t dash = {};
    dash.voltage_raw = 7240;        // 72.40 V
    dash.current_raw = 1500;        // 150.0 A
    dash.battery_soc = 85;          // 85%
    dash.temp_motor = 45;           // 45 C
    dash.temp_controller = 38;      // 38 C
    dash.temp_battery = 28;         // 28 C
    dash.controller_status = 0x0100;// Brake active
    dash.vehicle_speed_raw = 855;   // 85.5 km/h
    dash.motor_rpm = 3200;          // 3200 RPM

    // Calculate CRC8 over first 14 bytes
    dash.crc8 = calculateDashboardCRC8(
        reinterpret_cast<const uint8_t*>(&dash), sizeof(DashboardData_t) - 1);

    // Verify CRC passes
    EXPECT_TRUE(verifyDashboardCRC8(dash));

    // Tamper with data and verify CRC fails
    dash.voltage_raw += 1;
    EXPECT_FALSE(verifyDashboardCRC8(dash));
}

TEST(DashboardDataTest, ConversionFromFardriverData) {
    FardriverData fdData;
    fdData.addrE8.deci_volts = 724;  // 72.4 V -> 7240 raw
    fdData.addrE8.lineCurrent = 600; // 600 / 4 = 150.0 A -> 1500 raw
    fdData.addr0C.ZeroBattCoeff = 0;
    fdData.addr0C.FullBattCoeff = 1000;
    fdData.addrF4.motor_temp = 50;   // 50 C
    fdData.addrD6.MosTemp = 40;      // 40 C
    fdData.addrE2.brake = 1;
    fdData.addrE2.forward = 1;
    fdData.addrE2.MeasureSpeed = 2500;// 2500 RPM

    DashboardData_t dash = createDashboardData(fdData, 30);

    EXPECT_EQ(dash.voltage_raw, 7240);
    EXPECT_EQ(dash.current_raw, 1500);
    EXPECT_EQ(dash.temp_motor, 50);
    EXPECT_EQ(dash.temp_controller, 40);
    EXPECT_EQ(dash.temp_battery, 30);
    EXPECT_EQ(dash.motor_rpm, 2500);

    // Status bitmask should have Brake (bit 8) and Forward (bit 9)
    EXPECT_NE(dash.controller_status & (1 << 8), 0);
    EXPECT_NE(dash.controller_status & (1 << 9), 0);

    // CRC8 must be valid
    EXPECT_TRUE(verifyDashboardCRC8(dash));
}
