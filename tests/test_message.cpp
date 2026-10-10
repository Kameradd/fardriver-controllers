#include <gtest/gtest.h>
#include "../src/fardriver_message.hpp"

TEST(MessageTest, CRCVerification) {
    uint8_t raw[16] = {
        0xAA, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    FardriverMessage::computeCRC(raw, 16);

    FardriverMessage msg;
    std::memcpy(msg.getRaw(), raw, 16);
    EXPECT_TRUE(msg.verifyCRC());
}

TEST(MessageTest, BadCRC) {
    uint8_t raw[16] = {
        0xAA, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF
    };

    FardriverMessage msg;
    std::memcpy(msg.getRaw(), raw, 16);
    EXPECT_FALSE(msg.verifyCRC());
}

TEST(MessageTest, VerifyStart) {
    FardriverMessage msg;
    msg.reset();
    EXPECT_FALSE(msg.verifyStart());

    msg.raw.start = 0xAA;
    EXPECT_TRUE(msg.verifyStart());
}

TEST(MessageTest, LookupAddrBounds) {
    EXPECT_EQ(FardriverMessage::lookupAddr(0), 0xE2);
    EXPECT_EQ(FardriverMessage::lookupAddr(54), 0xFA);

    EXPECT_EQ(FardriverMessage::lookupAddr(55), 0);
    EXPECT_EQ(FardriverMessage::lookupAddr(63), 0);
}

TEST(MessageTest, LookupAddrKnown) {
    EXPECT_EQ(FardriverMessage::lookupAddr(3), 0x00);
    EXPECT_EQ(FardriverMessage::lookupAddr(4), 0x06);
    EXPECT_EQ(FardriverMessage::lookupAddr(5), 0x0C);
    EXPECT_EQ(FardriverMessage::lookupAddr(10), 0x18);
}
