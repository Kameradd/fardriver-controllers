#include "tasks.hpp"
#include "fardriver_controller.hpp"

extern FardriverController *g_controller;

void keepAliveTask(void *pv) {
    TickType_t lastWake = xTaskGetTickCount();

    while (1) {
        vTaskDelayUntil(&lastWake, kKeepAliveIntervalMs);
        g_controller->keepAlive();
    }
}

void initKeepAliveTask() {
    xTaskCreatePinnedToCore(keepAliveTask, "KeepAlive",
        kStackKeepAlive, nullptr, kPriorityKeepAlive,
        &g_keepAliveTask, 0);
}
