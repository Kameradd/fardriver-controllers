#pragma once
#include <cstdint>
#include <cstring>
#include "fardriver_data.hpp"
#include "fardriver_message.hpp"
#include "fardriver_crc.hpp"
#include "parameter_validator.hpp"

enum class EReadError : uint8_t {
    Success = 0,
    NotEnoughBytesAvailable = 1,
    IncorrectMessageStart = 2,
    UnhandledMessageHeader = 3,
    CouldNotVerifyCRC = 4,
    ResyncInProgress = 5,
};

struct ReadResult {
    EReadError error;
    uint8_t addr;
};

enum class EWriteError : uint8_t {
    Success = 0,
    ValidationFailed = 1,
    ACKTimeout = 2,
    CRCError = 3,
    ReadbackMismatch = 4,
    NoResponse = 5,
};

struct WriteResult {
    EWriteError error;
    uint8_t addr;
};

struct FardriverSerial {
    uint32_t (*write)(const uint8_t *data, uint32_t length);
    uint32_t (*read)(uint8_t *data, uint32_t length);
    uint32_t (*available)(void);
    void (*delay_ms)(uint32_t ms);
};

class FardriverController {
public:
    FardriverController(FardriverSerial *serial, FardriverData *data)
        : serial_(serial), data_(data) {
        message_.reset();
        dropped_bytes_ = 0;
    }

    ReadResult read();
    WriteResult writeAddr(uint8_t addr, uint16_t value);
    WriteResult writeAddrRaw(uint8_t addr, uint8_t byte1, uint8_t byte2);
    WriteResult writeSysCmd(uint8_t command);

    void open();
    void keepAlive();
    void reset();

    void sendDetectPacket();
    void sendACK(uint8_t index);

    bool sendPacket(uint8_t index, const uint8_t *packetData, uint32_t length);
    bool verifyCRCMessage(uint8_t index, const uint8_t *fileCRC, uint32_t timeout_ms);

    uint32_t getDroppedBytes() const { return dropped_bytes_; }

private:
    void sendRS323Data(uint8_t command, uint8_t sub_command,
                       uint8_t value_1, uint8_t value_2);
    void computeAddressCRC(uint8_t *data, uint8_t length);
    uint16_t readWordFromSerial(uint32_t timeout_ms);

    FardriverSerial *serial_;
    FardriverData *data_;
    FardriverMessage message_;
    CRC crc_;
    uint32_t dropped_bytes_;
};
