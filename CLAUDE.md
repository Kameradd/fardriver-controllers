# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

Fardriver-controllers is a C++ library and ESP32-S3 RTOS firmware for interfacing with Nanjing Fardriver electric motor controllers over serial (RS232). It supports reading telemetry, writing parameters, firmware flashing, and safety monitoring.

The repo contains both a **legacy library** (top-level `.hpp`/`.cpp` files) and a **new ESP32 RTOS firmware** (`src/` directory) that refactors the library into an RTOS-based architecture.

## Build & Test

### RTOS Firmware (PlatformIO / ESP32-S3)

```bash
# Build the main firmware
pio run -e esp32-s3-devkitc-1

# Upload to device
pio run -e esp32-s3-devkitc-1 -t upload

# Monitor serial output
pio run -e esp32-s3-devkitc-1 -t monitor

# Clean build
pio run -e esp32-s3-devkitc-1 -t clean
```

### Native Tests (Google Test)

```bash
# Build and run all tests
pio test -e test

# Run a specific test file (filter by suite name)
pio test -e test --filter "RingBufferTest.*"
pio test -e test --filter "MessageTest.*"
pio test -e test --filter "ValidatorTest.*"
```

The test environment builds `tests/*.cpp` against `src/` headers using the native platform + Google Test framework.

## Project Architecture

### RTOS Firmware (`src/` directory)

The firmware is a FreeRTOS-based system for ESP32-S3 that communicates with a Fardriver controller via UART1 (pins 17/18, 19200 baud).

**Global state** (`src/main.cpp`, `src/tasks.hpp`):
- `g_fardriverData` — 512-byte packed struct mirroring all controller memory (addresses 0x00–0xFA)
- `g_rxRingBuffer` — lock-free ring buffer for UART ISR byte ingestion
- `g_controller` — `FardriverController` instance managing serial protocol
- `g_safetyConfig` — configurable thresholds for safety monitoring
- Queue handles for telemetry, writes, display, logging, and LoRa
- `g_dataMutex` — mutex protecting shared FardriverData access

**Task priorities and stack sizes** (`src/tasks.hpp:19-33`):
| Task | Priority | Stack |
|---|---|---|
| SafetyMonitor | 9 | 3072 |
| FrameParser | 8 | 4096 |
| CommandWrite | 7 | 3072 |
| Telemetry | 6 | 4096 |
| KeepAlive | 5 | 2048 |
| Display | 4 | 3072 |
| LoRa | 4 | 2048 |
| Logger | 3 | 4096 |

**Tasks:**
- **FrameParser** (`src/task_frame_parser.cpp`) — UART ISR pushes bytes into `g_rxRingBuffer`; the task syncs on `0xAA`, parses 16-byte frames, validates CRC, copies data into `g_fardriverData`, and enqueues `TelemetryItem` to the telemetry queue.
- **CommandWrite** (`src/task_command_write.cpp`) — dequeues `WriteCommand` items and calls `g_controller->writeAddr()`. Sends LoRa alert on write error.
- **KeepAlive** (`src/task_keepalive.cpp`) — sends keep-alive packet every 1 second via `g_controller->keepAlive()`.
- **Telemetry** (`src/task_telemetry.cpp`) — pulls from telemetry queue, reads computed values (voltage, current, speed, motor temp) from `g_fardriverData` under mutex, dispatches to display, logging, and LoRa queues.
- **SafetyMonitor** (`src/task_safety.cpp`) — checks all parameters against `g_safetyConfig` thresholds every 200ms. Uses violation counting (consecutive violations must exceed `violation_threshold` before alerting). Sends alerts to display, logging, and LoRa.
- **Display** / **Logger** / **LoRa** (`src/tasks/task_peripherals.cpp`) — Nextion display via UART2, SD card CSV logging via standard SPI SD + RTC timestamp, and LoRa serial radio.

### Serial Protocol

- 16-byte frames: magic `0xAA`, 6-bit ID + 2-bit flags, 12 bytes data, 2-byte CRC.
- Write packets (8 bytes): magic, length+cmd, addr, addr_confirm, data, CRC.
- Commands sent as 8-byte RS323-style: magic, command, ~command, sub_command, v1, v2, sum, ~sum.
- CRC uses two 256-entry lookup tables (`crcTableLo`/`crcTableHi`) computed over bytes before the CRC field.

### Key Classes

- **FardriverController** (`src/fardriver_controller.hpp`) — serial read/write abstraction with address write + readback verification, firmware update (packet send + CRC verify), system commands, and reset.
- **FardriverMessage** (`src/fardriver_message.hpp`) — 16-byte message struct with CRC verification, ID-to-address lookup via `flashReadAddr` table.
- **FardriverData** (`src/fardriver_data.hpp`) — 512-byte packed struct mapping all controller memory addresses (0x00–0xFA) to typed substructs with computed getters (voltage, speed, current, temps, battery %).
- **ParameterValidator** (`src/parameter_validator.hpp`, `src/parameter_validator.cpp`) — 28 parameter limits with range validation and critical-parameter tagging.
- **CRC** (`src/fardriver_crc.hpp`) — CRC32 calculator used for firmware update verification.
- **RingBuffer** (`src/ring_buffer.hpp`) — lock-free SPSC ring buffer (power-of-2 capacity) with atomic head/tail and overflow tracking.
- **SafetyMonitor** (`src/safety_monitor.hpp`) — `SafetyConfig` thresholds and `SafetyAlert` enum for overcurrent/overtemp/undervolt/overspeed.

### Legacy Library (top-level files)

The original single-header/multi-file library (`fardriver.hpp`, `fardriver_controller.hpp`, `fardriver_message.hpp`, `fardriver_crc.hpp`) is the predecessor. The new `src/` directory is the active development target. The legacy files compile as a PlatformIO library (`library.json`) but the `src/` firmware uses local copies.
