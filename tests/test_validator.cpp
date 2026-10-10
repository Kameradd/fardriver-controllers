#include <gtest/gtest.h>
#include "../src/parameter_validator.hpp"

TEST(ValidatorTest, InRangePasses) {
    ParamValidationResult r = ParameterValidator::validate(0x12, 100);
    EXPECT_TRUE(r.valid);
}

TEST(ValidatorTest, OutOfRangeFails) {
    ParamValidationResult r = ParameterValidator::validate(0x12, 99999);
    EXPECT_FALSE(r.valid);
}

TEST(ValidatorTest, NegativeAllowed) {
    ParamValidationResult r = ParameterValidator::validate(0x0C, -90);
    EXPECT_TRUE(r.valid);
}

TEST(ValidatorTest, NegativeTooLow) {
    ParamValidationResult r = ParameterValidator::validate(0x0C, -200);
    EXPECT_FALSE(r.valid);
}

TEST(ValidatorTest, UnknownAddrPasses) {
    ParamValidationResult r = ParameterValidator::validate(0xF4, 42);
    EXPECT_TRUE(r.valid);
}

TEST(ValidatorTest, CriticalParams) {
    for (size_t i = 0; i < ParameterValidator::kLimitCount; i++) {
        if (ParameterValidator::kLimits[i].is_critical) {
            ParamValidationResult r = ParameterValidator::validate(
                ParameterValidator::kLimits[i].addr,
                ParameterValidator::kLimits[i].max_val);
            EXPECT_TRUE(r.valid);
        }
    }
}
