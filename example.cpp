#include "fardriver_controller.hpp"
#include <Arduino.h>


// Setup hardware serial (e.g., Serial2 on ESP32 or an STM32 UART port)
#define RX_PIN 16
#define TX_PIN 17
#define BAUD_RATE 19200

// 1. Map Arduino Serial methods to the Fardriver library structure
uint32_t serial_write(const uint8_t *data, uint32_t length) {
  return Serial2.write(data, length);
}

uint32_t serial_read(uint8_t *data, uint32_t length) {
  return Serial2.readBytes(data, length);
}

uint32_t serial_available(void) { return Serial2.available(); }

FardriverSerial fdSerial = {
    .write = serial_write, .read = serial_read, .available = serial_available};

// 2. Initialize controller and data structs
FardriverController controller(&fdSerial);
FardriverData controllerData;

void setup() {
  Serial.begin(115200);

  // Initialize Fardriver UART link
  Serial2.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);

  // Open communication with the controller
  controller.Open();
}

void loop() {
  // 3. Constantly read/parse telemetry packets sent by the controller
  if (Serial2.available() > 0) {
    FardriverController::ReadResult res = controller.Read(&controllerData);
    if (res.error == FardriverController::Success) {
      Serial.print("Successfully read packet address: 0x");
      Serial.println(res.addr, HEX);

      // Print live data using the built-in helper functions
      Serial.printf("Voltage: %.1f V\n", controllerData.GetVoltage());
      Serial.printf("Current: %.1f A\n", controllerData.GetLineCurrent());
      Serial.printf("Speed  : %.1f km/h\n", controllerData.GetSpeed());
      Serial.printf("Motor T: %.1f F\n", controllerData.GetMotorTemp());
    }
  }

  // 4. Send keep-alive commands periodically
  static uint32_t lastKeepAlive = 0;
  if (millis() - lastKeepAlive > 1000) {
    controller.KeepAlive();
    lastKeepAlive = millis();
  }
}
