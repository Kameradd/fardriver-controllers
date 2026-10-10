#pragma once
#include <cstdint>

enum class SafetyAlert {
    None = 0,
    PhaseCurrentHigh = 1,
    MotorTempHigh = 2,
    MosTempHigh = 3,
    VoltageLow = 4,
    VoltageHigh = 5,
    LineCurrentHigh = 6,
    SpeedOvershoot = 7,
};

struct SafetyEvent {
    SafetyAlert alert;
    float current_value;
    float threshold;
    uint32_t tick;
};

struct SafetyConfig {
    float max_phase_current_A;
    float max_line_current_A;
    float max_motor_temp_C;
    float max_mos_temp_C;
    float min_voltage_V;
    float max_voltage_V;
    float max_speed_kmh;
    uint8_t violation_threshold;
};

static constexpr SafetyConfig kDefaultSafetyConfig = {
    300.0f,   200.0f,   110.0f,
    90.0f,    10.0f,    100.0f,
    120.0f,   3
};
