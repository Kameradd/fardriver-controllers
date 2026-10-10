#include "fardriver_controller.hpp"

static constexpr uint8_t kMagicByte = 0xAA;
static constexpr uint8_t kFlagWrite = 1;
static constexpr uint8_t kFlagRead = 2;
static constexpr uint8_t kSysCmdMarker = 0x88;
static constexpr uint8_t kSysCmdAddr = 0xA0;
static constexpr uint32_t kWriteACKTimeoutMs = 200;
static constexpr uint32_t kReadbackWaitMs = 50;

void FardriverController::computeAddressCRC(uint8_t *data, uint8_t length) {
    uint8_t a = 0x3C;
    uint8_t b = 0x7F;
    for (uint8_t pos = 0; pos < length; ++pos) {
        uint8_t crc_i = a ^ data[pos];
        a = b ^ FardriverMessage::crcTableHi[crc_i];
        b = FardriverMessage::crcTableLo[crc_i];
    }
    data[length] = a;
    data[length + 1] = b;
}

ReadResult FardriverController::read() {
    if (serial_->available() < FardriverMessage::kRawLength) {
        return {EReadError::NotEnoughBytesAvailable, 0};
    }

    serial_->read(&message_.raw.start, 1);
    if (!message_.verifyStart()) {
        dropped_bytes_++;
        return {EReadError::IncorrectMessageStart, 0};
    }

    serial_->read(&message_.raw.start + 1, 1);

    if (message_.raw.header.flag != kFlagRead ||
        message_.raw.header.id > FardriverMessage::kValidIdMax) {
        return {EReadError::UnhandledMessageHeader, 0};
    }

    serial_->read(message_.data, FardriverMessage::kDataLength);
    serial_->read(message_.crc, FardriverMessage::kCRCLength);

    if (!message_.verifyCRC()) {
        return {EReadError::CouldNotVerifyCRC, 0};
    }

    uint8_t addr = FardriverMessage::lookupAddr(message_.raw.header.id);
    std::memcpy(data_->getAddr(addr), message_.data, FardriverMessage::kDataLength);

    return {EReadError::Success, addr};
}

void FardriverController::sendRS323Data(uint8_t command, uint8_t sub_command,
                                         uint8_t value_1, uint8_t value_2) {
    uint8_t data[8];
    data[0] = kMagicByte;
    data[1] = command;
    data[2] = ~command;
    data[3] = sub_command;
    data[4] = value_1;
    data[5] = value_2;
    data[6] = data[0] + data[1] + data[2] + data[3] + data[4] + data[5];
    data[7] = ~data[6];
    serial_->write(data, 8);
}

WriteResult FardriverController::writeAddrRaw(uint8_t addr,
                                                uint8_t byte1, uint8_t byte2) {
    uint8_t length = 6;
    uint8_t data[8];

    data[0] = kMagicByte;
    data[1] = 0xC0 + length;
    data[2] = addr;
    data[3] = addr;
    data[4] = byte1;
    data[5] = byte2;

    computeAddressCRC(data, length);
    serial_->write(data, length + 2);

    return {EWriteError::Success, addr};
}

WriteResult FardriverController::writeAddr(uint8_t addr, uint16_t value) {
    uint16_t combined = value;
    int16_t ival = static_cast<int16_t>(combined);

    ParamValidationResult vr = ParameterValidator::validate(addr, ival);
    if (!vr.valid) {
        return {EWriteError::ValidationFailed, addr};
    }

    uint8_t byte1 = combined & 0xFF;
    uint8_t byte2 = (combined >> 8) & 0xFF;

    WriteResult wResult = writeAddrRaw(addr, byte1, byte2);
    if (wResult.error != EWriteError::Success) {
        return wResult;
    }

    serial_->delay_ms(kReadbackWaitMs);

    uint16_t readback = readWordFromSerial(kReadbackWaitMs);

    if (static_cast<int16_t>(readback) != static_cast<int16_t>(combined)) {
        writeAddrRaw(addr, byte1, byte2);
        serial_->delay_ms(kReadbackWaitMs);
        readback = readWordFromSerial(kReadbackWaitMs);
        if (static_cast<int16_t>(readback) != static_cast<int16_t>(combined)) {
            return {EWriteError::ReadbackMismatch, addr};
        }
    }

    return {EWriteError::Success, addr};
}

WriteResult FardriverController::writeSysCmd(uint8_t command) {
    return writeAddrRaw(kSysCmdAddr, kSysCmdMarker, command);
}

void FardriverController::open() {
    sendRS323Data(0x13, 0x07, 0x01, 0xF1);
}

void FardriverController::keepAlive() {
    sendRS323Data(0x13, 0x07, 0x5F, 0x5F);
}

void FardriverController::reset() {
    writeSysCmd(0x05);
}

void FardriverController::sendDetectPacket() {
    uint8_t message[8];
    message[0] = 0x5A;
    message[1] = 0xAA;
    message[2] = 0x33;
    message[3] = 0x03;
    message[4] = 0x33;
    message[5] = 0x3C;
    message[6] = 0xFD;
    message[7] = 0xFE;
    serial_->write(message, 8);
}

void FardriverController::sendACK(uint8_t index) {
    uint8_t message[8];
    message[0] = 0x5A;
    message[1] = 0xBB;
    message[2] = index;
    message[3] = 0x72;
    message[4] = 0x73;
    message[5] = 0x74;
    message[6] = 0x75;
    message[7] = 0x76;
    serial_->write(message, 8);
}

bool FardriverController::sendPacket(uint8_t index, const uint8_t *packetData,
                                       uint32_t length) {
    if (length > 2048) return false;

    static uint8_t message[2048 + 3 + 4];

    message[0] = 0x5A;
    message[1] = 0xA5;
    message[2] = index;
    std::memcpy(&message[3], packetData, length);
    if (2048 - length > 0) {
        std::memset(&message[3 + length], 0xFF, 2048 - length);
    }
    crc_.reset();
    crc_.add(&message[2], 1 + 2048);
    crc_.assign(&message[3 + 2048]);
    serial_->write(message, 3 + 2048 + 4);
    return true;
}

bool FardriverController::verifyCRCMessage(uint8_t index,
                                              const uint8_t *fileCRC,
                                              uint32_t timeout_ms) {
    uint32_t elapsed = 0;
    const uint32_t kPollIntervalMs = 5;

    while (serial_->available() < 16) {
        serial_->delay_ms(kPollIntervalMs);
        elapsed += kPollIntervalMs;
        if (elapsed >= timeout_ms) return false;
    }

    uint8_t message[16] = {0};
    serial_->read(message, 16);

    if (message[0] != 0xAA) return false;
    if (message[1] != 0x1F) return false;
    if (message[2] != message[3]) return false;

    uint8_t *readCRC = &message[3];
    for (uint8_t i = 0; i < 8; i++) {
        if (fileCRC[i] != readCRC[i]) return false;
    }
    return true;
}

uint16_t FardriverController::readWordFromSerial(uint32_t timeout_ms) {
    uint32_t elapsed = 0;
    const uint32_t kPollIntervalMs = 5;

    while (serial_->available() < FardriverMessage::kRawLength) {
        serial_->delay_ms(kPollIntervalMs);
        elapsed += kPollIntervalMs;
        if (elapsed >= timeout_ms) return 0xFFFF;
    }

    uint8_t raw[16];
    serial_->read(raw, 16);

    if (raw[0] != kMagicByte) return 0xFFFF;
    uint8_t id = raw[1];
    if (id > FardriverMessage::kValidIdMax) return 0xFFFF;

    {
        uint8_t a = 0x3C;
        uint8_t b = 0x7F;
        for (uint8_t i = 0; i < 14; i++) {
            uint8_t crc_i = a ^ raw[i];
            a = b ^ FardriverMessage::crcTableHi[crc_i];
            b = FardriverMessage::crcTableLo[crc_i];
        }
        if (raw[14] != a || raw[15] != b) return 0xFFFF;
    }

    uint8_t addr = FardriverMessage::lookupAddr(id);
    return *reinterpret_cast<uint16_t *>(raw + 2);
}
