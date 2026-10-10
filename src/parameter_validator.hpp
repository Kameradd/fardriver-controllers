#pragma once
#include <cstdint>
#include <cstddef>

struct ParameterLimit {
    uint16_t addr;
    int32_t min_val;
    int32_t max_val;
    bool is_critical;
    const char *name;
};

struct ParamValidationResult {
    bool valid;
    const char *err_msg;
};

class ParameterValidator {
public:
    static ParamValidationResult validate(uint16_t addr, int32_t value);

    static const size_t kLimitCount = 28;
    static const ParameterLimit kLimits[kLimitCount];
};
