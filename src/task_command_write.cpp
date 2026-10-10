#include "tasks.hpp"
#include "fardriver_controller.hpp"
#include <cstring>

extern FardriverController *g_controller;
extern QueueHandle_t g_writeQueue;

void commandWriteTask(void *pv) {
    while (1) {
        WriteCommand cmd;
        if (xQueueReceive(g_writeQueue, &cmd, portMAX_DELAY) != pdTRUE) continue;

        WriteResult result = g_controller->writeAddr(cmd.addr, cmd.value);

        if (result.error != EWriteError::Success) {
            char alert[64];
            snprintf(alert, sizeof(alert), "W:ERR addr=0x%02X v=%d e=%d",
                     cmd.addr, cmd.value, static_cast<int>(result.error));
            LoRaPacket lp;
            std::strncpy(lp.payload, alert, sizeof(lp.payload));
            xQueueSend(g_loraQueue, &lp, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void initCommandWriteTask() {
    xTaskCreatePinnedToCore(commandWriteTask, "CmdWrite",
        kStackCommandWrite, nullptr, kPriorityCommandWrite,
        &g_commandWriteTask, 0);
}
