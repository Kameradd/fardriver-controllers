#include "parameter_validator.hpp"

const ParameterLimit ParameterValidator::kLimits[kLimitCount] = {
    {0x00, 0,    0x7FFF,                           false, "VolCoeff"},
    {0x00, 0,    0x7FFF,                           false, "Voltage2Coeff"},
    {0x06, 0,    0xFF,                             false, "ThrottleLow"},
    {0x06, 0,    0xFF,                             false, "ThrottleHigh"},
    {0x06, 0,    0x7FFF,                           false, "FAIF"},
    {0x0C, -120, 120,                              true,  "PhaseOffset"},
    {0x0C, 0,    0xFFFF,                           false, "StartKI"},
    {0x0C, 0,    0xFFFF,                           false, "MidKI"},
    {0x0C, 0,    0xFFFF,                           false, "MaxKI"},
    {0x12, 0,    100,                              false, "PolePairs"},
    {0x12, 0,    12000,                            true,  "MaxSpeed"},
    {0x12, 0,    30000,                            true,  "RatedPower"},
    {0x12, 0,    3000,                             true,  "RatedVoltage"},
    {0x18, 0,    12000,                            false, "RatedSpeed"},
    {0x18, 0,    2000,                             true,  "MaxLineCurr"},
    {0x24, 0,    2000,                             true,  "CustomMaxLineCurr"},
    {0x24, 0,    3000,                             true,  "CustomMaxPhaseCurr"},
    {0x2A, 0,    3000,                             true,  "MaxPhaseCurr"},
    {0x1E, 0,    1500,                             true,  "LowVolProtect"},
    {0x24, 0,    2000,                             true,  "HighVolProtect"},
    {0x30, 0,    2000,                             false, "StopBackCurr"},
    {0x30, 0,    2000,                             false, "MaxBackCurr"},
    {0x63, 0,    2000,                             true,  "MaxLineCurr2"},
    {0x63, 0,    3000,                             true,  "MaxPhaseCurr2"},
    {0x82, 0,    150,                              true,  "MotorTempProtect"},
    {0x82, 0,    150,                              true,  "MosTempProtect"},
    {0xBE, 0,    0xFF,                             false, "AccCoeff"},
    {0xBE, 0,    0xFFFF,                           false, "TorqueCoeff"},
};

ParamValidationResult ParameterValidator::validate(uint16_t addr, int32_t value) {
    for (size_t i = 0; i < kLimitCount; i++) {
        if (kLimits[i].addr == addr) {
            if (value < kLimits[i].min_val || value > kLimits[i].max_val) {
                static const char msg[] = "Value out of range";
                return {false, kLimits[i].name};
            }
            return {true, nullptr};
        }
    }
    for (size_t i = 0; i < kLimitCount; i++) {
        if (kLimits[i].addr == (addr & 0xFE) || kLimits[i].addr == addr) {
            return {true, nullptr};
        }
    }
    return {true, nullptr};
}
