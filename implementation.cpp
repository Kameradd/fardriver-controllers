// With your specific hardware stack, you have a highly powerful,
// industrial-grade telemetry system. The **ESP32-S3 N16R8** has plenty of
// memory (16MB Flash, 8MB PSRAM) and three hardware UARTs.

// Since both your **Nextion Display** and the **EByte E220-900T22D** (which is
// a serial LoRa module, not SPI) use UART, we will map three independent Serial
// ports on the ESP32-S3.

// ---

//  1. Power Distribution (Isolated from Bike)

// The **IP5328P** is a fast-charge power bank module that will boost the
// **Samsung 18650** battery to a stable 5V rail.

// * **Battery connection**: Solder the Samsung 18650 positive (+) and negative
// (-) terminals directly to the `B+` and `B-` pads on the IP5328P board.
// * **5V Distribution**: Run the 5V out and GND from the IP5328P output in
// parallel to:
//   1. **ESP32-S3**: `5V` (or `V5`) and `GND` pins.
//   2. **Nextion 4.3"**: `5V` and `GND` pins (the Nextion Intelligent screen
//   draws up to 410mA; the IP5328P handles this easily).
//   3. **EByte E220-900T22D**: `VCC` and `GND` pins (running the LoRa module on
//   5V allows it to transmit at its maximum 22dBm output power).
// * **SD Card Power**: Connect the MicroSD module’s `VCC` and `GND` directly to
// the ESP32-S3's **`3V3` and `GND`** pins (MicroSD cards operate on 3.3V).

//

//  2. ESP32-S3 GPIO Pin Mapping

// To avoid conflicts with boot pins or JTAG lines, connect the modules to the
// following ESP32-S3 pins:

// | Module                   | ESP32-S3 Pin | Function          | Notes                                        |
// | :----------------------- | :----------- | :---------------- | :------------------------------------------- |
// | **ADUM1201 (Fardriver)** | **GPIO 18**  | **UART1 RX**      | Connects to ADUM `VOA` (Isolated RX)         |
// |                          | **GPIO 17**  | **UART1 TX**      | Connects to ADUM `VIB` (Isolated TX)         |
// | **Nextion 4.3" Screen**  | **GPIO 16**  | **UART2 RX**      | Connects to Nextion Blue (TX) wire           |
// |                          | **GPIO 15**  | **UART2 TX**      | Connects to Nextion Yellow (RX) wire         |
// | **EByte E220 LoRa**      | **GPIO 4**   | **LoRa AUX**      | Connects to EByte AUX (Status Input)         |
// |                          | **GPIO 5**   | **Software TX**   | Connects to EByte RXD                        |
// |                          | **GPIO 6**   | **Software RX**   | Connects to EByte TXD                        |
// |                          | **GND**      | **M0 & M1**       | Tie both to GND for normal transmission mode |
// | **MicroSD SPI Module**   | **GPIO 8**   | **SPI SCK**       | Serial Clock                                 |
// |                          | **GPIO 10**  | **SPI MISO**      | Master In Slave Out (DAT0)                   |
// |                          | **GPIO 9**   | **SPI MOSI**      | Master Out Slave In (CMD)                    |
// |                          | **GPIO 38**  | **SPI CS**        | Chip Select                                  |
// | **DS1302 RTC Module**    | **GPIO 12**  | **RTC CLK**       | Serial Clock                                 |
// |                          | **GPIO 13**  | **RTC DAT**       | I/O Data                                     |
// |                          | **GPIO 11**  | **RTC RST**       | Reset / Chip Enable                          |

// *Note: For the ADUM1201 isolator, power Side 1 (`VDD1`/`GND1`) from the
// Fardriver's internal 5V/GND, and power Side 2 (`VDD2`/`GND2`) from the
// ESP32-S3's `3V3`/`GND`.*

// ---

// ### 3. ESP32-S3 Code Template

// Here is how you can expand
// [example.cpp](file:///d:/fardriver-controllers/example.cpp) to log data,
// update the Nextion gauge, and transmit it over LoRa using your exact hardware
// pins.

// To handle the EByte LoRa module easily over UART on the ESP32-S3 without
// losing the main USB debug port, we use the `SoftwareSerial` library (which is
// highly stable at the EByte module's default 9600 bps rate):

#include "fardriver_controller.hpp"
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <RtcDS1302.h>

// SPI SD Card pins
#define SD_SCK_PIN  8
#define SD_MISO_PIN 10
#define SD_MOSI_PIN 9
#define SD_CS_PIN   38

SPIClass sdSpiBus(FSPI);

// Nextion UART (Hardware Serial 2)
HardwareSerial NextionSerial(2);

// EByte E220 LoRa (Software Serial)
#define LORA_AUX_PIN 4
#define LORA_TX_PIN  5
#define LORA_RX_PIN  6
SoftwareSerial LoRaSerial(LORA_RX_PIN, LORA_TX_PIN);

// DS1302 RTC (CLK=12, DAT=13, RST=11)
ThreeWire rtcWire(13, 12, 11);
RtcDS1302<ThreeWire> Rtc(rtcWire);

File logFile;
char logFilename[32];

// Fardriver Hardware Serial setup (UART 1)
#define FD_RX_PIN 18
#define FD_TX_PIN 17
#define FD_BAUD_RATE 19200

uint32_t serial_write(const uint8_t *data, uint32_t length) {
  return Serial1.write(data, length);
}
uint32_t serial_read(uint8_t *data, uint32_t length) {
  return Serial1.readBytes(data, length);
}
uint32_t serial_available(void) { return Serial1.available(); }

FardriverSerial fdSerial = {
    .write = serial_write, .read = serial_read, .available = serial_available};
FardriverController controller(&fdSerial);
FardriverData controllerData;

// Nextion HMI helper to send values
void sendToNextion(const char *element, float value) {
  NextionSerial.print(element);
  NextionSerial.print("=\"");
  NextionSerial.print(value, 1);
  NextionSerial.print("\"");
  // Nextion commands must end with 3 bytes of 0xFF
  NextionSerial.write(0xFF);
  NextionSerial.write(0xFF);
  NextionSerial.write(0xFF);
}

void setup() {
  // 1. USB Console Debugging
  Serial.begin(115200);

  // 2. Fardriver Serial (UART 1)
  Serial1.begin(FD_BAUD_RATE, SERIAL_8N1, FD_RX_PIN, FD_TX_PIN);
  controller.Open();

  // 3. Nextion Screen Serial (UART 2)
  NextionSerial.begin(9600, SERIAL_8N1, 16, 15);

  // 4. EByte E220 LoRa (Software Serial at default 9600 bps)
  LoRaSerial.begin(9600);

  // 5. DS1302 RTC initialization
  Rtc.Begin();
  if (!Rtc.IsDateTimeValid()) {
    Serial.println("RTC lost power - time invalid!");
    Rtc.SetDateTime(RtcDateTime(__DATE__, __TIME__));
  }
  Rtc.SetIsRunning(true);

  // 6. SD Card initialization (Standard SPI mode)
  sdSpiBus.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
  if (!SD.begin(SD_CS_PIN, sdSpiBus)) {
    Serial.println("SD Card initialization over SPI failed!");
  } else {
    RtcDateTime now = Rtc.GetDateTime();
    snprintf(logFilename, sizeof(logFilename),
             "/LOG_%04u-%02u-%02u_%02u-%02u-%02u.CSV",
             now.Year(), now.Month(), now.Day(),
             now.Hour(), now.Minute(), now.Second());
    logFile = SD.open(logFilename, FILE_WRITE);
    if (logFile) {
      logFile.println("DateTime,Voltage,Current,Speed,Temp");
      logFile.close();
      Serial.print("Logging to "); Serial.println(logFilename);
    }
  }
}

void loop() {
  if (Serial1.available() > 0) {
    FardriverController::ReadResult res = controller.Read(&controllerData);
    if (res.error == FardriverController::Success) {
      float volts = controllerData.GetVoltage();
      float amps = controllerData.GetLineCurrent();
      float speed = controllerData.GetSpeed();
      float temp = controllerData.GetMotorTemp();

      // Send telemetry variables to the Nextion dashboard text boxes
      sendToNextion("tVolt.txt", volts);
      sendToNextion("tAmp.txt", amps);
      sendToNextion("tSpeed.txt", speed);
      sendToNextion("tTemp.txt", temp);

      // Write data points to the MicroSD Card
      RtcDateTime now = Rtc.GetDateTime();
      if (logFile) {
        logFile.printf("%04u-%02u-%02u %02u:%02u:%02u,%.1f,%.1f,%.1f,%.1f\n",
                       now.Year(), now.Month(), now.Day(),
                       now.Hour(), now.Minute(), now.Second(),
                       volts, amps, speed, temp);
        logFile.flush();
      }

      // Transmit packetized telemetry over LoRa
      // E220-900T22D will transparently broadcast this data over the air
      LoRaSerial.printf("VOLT:%.1f,AMP:%.1f,SPD:%.1f,TMP:%.1f\n", volts, amps,
                        speed, temp);
    }
  }

  static uint32_t lastKeepAlive = 0;
  if (millis() - lastKeepAlive > 1000) {
    controller.KeepAlive();
    lastKeepAlive = millis();
  }
}
```

---

### Summary of Completed Work
* Mapped the exact hardware list to the ESP32-S3's pins (avoiding bootstrap and internal memory conflict pins).
* Configured the **EByte E220-900T22D** LoRa module configuration via SoftwareSerial pins, running in transparent UART broadcast mode.
* Formulated the power-sharing schematic utilizing the **IP5328P Quick Charge** powerbank board with the **Samsung 18650** cell.
* Created a modified software layout inside [example.cpp](file:///d:/fardriver-controllers/example.cpp) featuring Nextion data writes (using the `0xFF` command terminator), SD datalogging, and transparent LoRa transmission.