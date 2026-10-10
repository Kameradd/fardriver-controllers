#include "tasks.hpp"
#include "ring_buffer.hpp"
#include "fardriver_message.hpp"
#include "fardriver_data.hpp"
#include <driver/uart.h>
#include <cstring>

extern RingBuffer<kRingBufferSize> g_rxRingBuffer;
extern FardriverData g_fardriverData;
extern QueueHandle_t g_telemetryQueue;
extern SemaphoreHandle_t g_dataMutex;

void frameParserTask(void *pv) {
    uint8_t raw[FardriverMessage::kRawLength];
    enum State { SYNC, GATHER } state = SYNC;
    uint8_t idx = 0;
    uint8_t byte;

    while (1) {
        uint8_t chunk[64];
        int bytes_read = uart_read_bytes(UART_NUM_1, chunk, sizeof(chunk), pdMS_TO_TICKS(10));
        if (bytes_read > 0) {
            for (int i = 0; i < bytes_read; i++) {
                g_rxRingBuffer.push(chunk[i]);
            }
        }

        while (g_rxRingBuffer.pop(byte)) {
            switch (state) {
            case SYNC:
                if (byte == FardriverMessage::kMagicByte) {
                    raw[0] = byte;
                    idx = 1;
                    state = GATHER;
                }
                break;

            case GATHER:
                raw[idx++] = byte;
                if (idx >= FardriverMessage::kRawLength) {
                    state = SYNC;
                    idx = 0;

                    if (raw[0] != FardriverMessage::kMagicByte) continue;

                    uint8_t header = raw[1];
                    uint8_t id = header & 0x3F;
                    uint8_t flag = (header >> 6) & 0x03;

                    if (flag != 2 || id > FardriverMessage::kValidIdMax) continue;

                    uint8_t a = 0x3C;
                    uint8_t b = 0x7F;
                    for (uint8_t i = 0; i < 14; i++) {
                        uint8_t crc_i = a ^ raw[i];
                        a = b ^ FardriverMessage::crcTableHi[crc_i];
                        b = FardriverMessage::crcTableLo[crc_i];
                    }
                    if (raw[14] != a || raw[15] != b) continue;

                    uint8_t addr = FardriverMessage::lookupAddr(id);

                    TelemetryItem item;
                    item.addr = addr;
                    std::memcpy(item.data, raw + 2, 12);

                    if (xSemaphoreTake(g_dataMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
                        std::memcpy(g_fardriverData.getAddr(addr), item.data, 12);
                        xSemaphoreGive(g_dataMutex);
                    }

                    xQueueSend(g_telemetryQueue, &item, 0);
                }
                break;
            }
        }
    }
}

void initFrameParserTask() {
    uart_config_t uart_config = {
        .baud_rate = 19200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
#if defined(UART_SCLK_DEFAULT)
        .source_clk = UART_SCLK_DEFAULT,
#else
        .source_clk = UART_SCLK_APB,
#endif
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, 17, 18, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_1, kRingBufferSize * 2, 0, 0, nullptr, 0);

    xTaskCreatePinnedToCore(frameParserTask, "FrameParser",
        kStackFrameParser, nullptr, kPriorityFrameParser,
        &g_frameParserTask, 0);
}
