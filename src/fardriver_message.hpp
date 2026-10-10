#pragma once
#include <cstdint>
#include <cstddef>

struct FardriverMessage {
    static constexpr const uint8_t crcTableLo[256] = {
        0, 192, 193, 1, 195, 3, 2, 194, 198, 6, 7, 199, 5, 197, 196, 4,
        204, 12, 13, 205, 15, 207, 206, 14, 10, 202, 203, 11, 201, 9, 8, 200,
        216, 24, 25, 217, 27, 219, 218, 26, 30, 222, 223, 31, 221, 29, 28, 220,
        20, 212, 213, 21, 215, 23, 22, 214, 210, 18, 19, 211, 17, 209, 208, 16,
        240, 48, 49, 241, 51, 243, 242, 50, 54, 246, 247, 55, 245, 53, 52, 244,
        60, 252, 253, 61, 255, 63, 62, 254, 250, 58, 59, 251, 57, 249, 248, 56,
        40, 232, 233, 41, 235, 43, 42, 234, 238, 46, 47, 239, 45, 237, 236, 44,
        228, 36, 37, 229, 39, 231, 230, 38, 34, 226, 227, 35, 225, 33, 32, 224,
        160, 96, 97, 161, 99, 163, 162, 98, 102, 166, 167, 103, 165, 101, 100, 164,
        108, 172, 173, 109, 175, 111, 110, 174, 170, 106, 107, 171, 105, 169, 168, 104,
        120, 184, 185, 121, 187, 123, 122, 186, 190, 126, 127, 191, 125, 189, 188, 124,
        180, 116, 117, 181, 119, 183, 182, 118, 114, 178, 179, 115, 177, 113, 112, 176,
        80, 144, 145, 81, 147, 83, 82, 146, 150, 86, 87, 151, 85, 149, 148, 84,
        156, 92, 93, 157, 95, 159, 158, 94, 90, 154, 155, 91, 153, 89, 88, 152,
        136, 72, 73, 137, 75, 139, 138, 74, 78, 142, 143, 79, 141, 77, 76, 140,
        68, 132, 133, 69, 135, 71, 70, 134, 130, 66, 67, 131, 65, 129, 128, 64
    };

    static constexpr const uint8_t crcTableHi[256] = {
        0, 193, 129, 64, 1, 192, 128, 65, 1, 192, 128, 65, 0, 193, 129, 64,
        1, 192, 128, 65, 0, 193, 129, 64, 0, 193, 129, 64, 1, 192, 128, 65,
        1, 192, 128, 65, 0, 193, 129, 64, 0, 193, 129, 64, 1, 192, 128, 65,
        0, 193, 129, 64, 1, 192, 128, 65, 1, 192, 128, 65, 0, 193, 129, 64,
        1, 192, 128, 65, 0, 193, 129, 64, 0, 193, 129, 64, 1, 192, 128, 65,
        0, 193, 129, 64, 1, 192, 128, 65, 1, 192, 128, 65, 0, 193, 129, 64,
        0, 193, 129, 64, 1, 192, 128, 65, 1, 192, 128, 65, 0, 193, 129, 64,
        1, 192, 128, 65, 0, 193, 129, 64, 0, 193, 129, 64, 1, 192, 128, 65,
        1, 192, 128, 65, 0, 193, 129, 64, 0, 193, 129, 64, 1, 192, 128, 65,
        0, 193, 129, 64, 1, 192, 128, 65, 1, 192, 128, 65, 0, 193, 129, 64,
        0, 193, 129, 64, 1, 192, 128, 65, 1, 192, 128, 65, 0, 193, 129, 64,
        1, 192, 128, 65, 0, 193, 129, 64, 0, 193, 129, 64, 1, 192, 128, 65,
        0, 193, 129, 64, 1, 192, 128, 65, 1, 192, 128, 65, 0, 193, 129, 64,
        1, 192, 128, 65, 0, 193, 129, 64, 0, 193, 129, 64, 1, 192, 128, 65,
        1, 192, 128, 65, 0, 193, 129, 64, 0, 193, 129, 64, 1, 192, 128, 65,
        0, 193, 129, 64, 1, 192, 128, 65, 1, 192, 128, 65, 0, 193, 129, 64
    };

    static constexpr const size_t kFlashReadAddrSize = 55;
    static constexpr const uint8_t flashReadAddr[kFlashReadAddrSize] = {
        0xE2, 0xE8, 0xEE, 0x00, 0x06, 0x0C, 0x12,
        0xE2, 0xE8, 0xEE, 0x18, 0x1E, 0x24, 0x2A,
        0xE2, 0xE8, 0xEE, 0x30, 0x5D, 0x63, 0x69,
        0xE2, 0xE8, 0xEE, 0x7C, 0x82, 0x88, 0x8E,
        0xE2, 0xE8, 0xEE, 0x94, 0x9A, 0xA0, 0xA6,
        0xE2, 0xE8, 0xEE, 0xAC, 0xB2, 0xB8, 0xBE,
        0xE2, 0xE8, 0xEE, 0xC4, 0xCA, 0xD0,
        0xE2, 0xE8, 0xEE, 0xD6, 0xDC, 0xF4, 0xFA
    };

    static constexpr const uint8_t kValidIdMax = kFlashReadAddrSize - 1;

    static constexpr uint8_t kMagicByte = 0xAA;
    static constexpr uint8_t kDataLength = 12;
    static constexpr uint8_t kRawLength = 16;
    static constexpr uint8_t kCRCLength = 2;

    uint8_t *getRaw() {
        return reinterpret_cast<uint8_t *>(this);
    }

    const uint8_t *getRaw() const {
        return reinterpret_cast<const uint8_t *>(this);
    }

    void reset() {
        raw.start = 0;
        header_raw = 0;
        for (uint8_t i = 0; i < kDataLength; i++) data[i] = 0;
        crc[0] = 0;
        crc[1] = 0;
    }

    bool verifyStart() const {
        return raw.start == kMagicByte;
    }

    static uint8_t lookupAddr(uint8_t id) {
        if (id > kValidIdMax) return 0;
        return flashReadAddr[id];
    }

    bool verifyCRC() const {
        uint8_t a = 0x3C;
        uint8_t b = 0x7F;
        for (uint8_t i = 0; i < kRawLength - kCRCLength; i++) {
            uint8_t crc_i = a ^ getRaw()[i];
            a = b ^ crcTableHi[crc_i];
            b = crcTableLo[crc_i];
        }
        return crc[0] == a && crc[1] == b;
    }

    static void computeCRC(uint8_t *msg, uint8_t length) {
        uint8_t a = 0x3C;
        uint8_t b = 0x7F;
        for (uint8_t i = 0; i < length - 2; i++) {
            uint8_t crc_i = a ^ msg[i];
            a = b ^ crcTableHi[crc_i];
            b = crcTableLo[crc_i];
        }
        msg[length - 2] = a;
        msg[length - 1] = b;
    }

    union {
        struct {
            uint8_t start;
            struct {
                uint8_t id : 6;
                uint8_t flag : 2;
            } header;
        } raw;
        uint16_t header_raw;
    };

    uint8_t data[kDataLength];
    uint8_t crc[kCRCLength];
};
