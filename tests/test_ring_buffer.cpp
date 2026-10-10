#include <gtest/gtest.h>
#include "../src/ring_buffer.hpp"

TEST(RingBufferTest, PushPop) {
    RingBuffer<512> rb;

    uint8_t out;
    EXPECT_FALSE(rb.pop(out));

    EXPECT_TRUE(rb.push(0xAA));
    EXPECT_EQ(rb.available(), 1);

    EXPECT_TRUE(rb.pop(out));
    EXPECT_EQ(out, 0xAA);
    EXPECT_EQ(rb.available(), 0);
}

TEST(RingBufferTest, WrapAround) {
    RingBuffer<16> rb;

    for (int i = 0; i < 16; i++) {
        EXPECT_TRUE(rb.push(static_cast<uint8_t>(i)));
    }
    EXPECT_EQ(rb.available(), 16);
    EXPECT_FALSE(rb.push(0xFF));

    uint8_t out;
    for (int i = 0; i < 16; i++) {
        EXPECT_TRUE(rb.pop(out));
        EXPECT_EQ(out, static_cast<uint8_t>(i));
    }
    EXPECT_FALSE(rb.pop(out));
}

TEST(RingBufferTest, OverflowCount) {
    RingBuffer<16> rb;

    for (int i = 0; i < 20; i++) {
        rb.push(static_cast<uint8_t>(i));
    }
    EXPECT_GT(rb.overflow_count(), 0);
}

TEST(RingBufferTest, Reset) {
    RingBuffer<16> rb;

    for (int i = 0; i < 8; i++) {
        rb.push(static_cast<uint8_t>(i));
    }
    rb.reset();
    EXPECT_EQ(rb.available(), 0);

    uint8_t out;
    EXPECT_FALSE(rb.pop(out));
}
