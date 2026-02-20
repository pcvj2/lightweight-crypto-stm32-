# WSC332 – Cyber-security for Embedded Systems
## Lightweight Cryptography on STM32F767ZI

**Target Hardware:** STM32F767ZI (NUCLEO-F767ZI)  
**Language:** C (STM32 firmware) + Python (ground station)  
**Toolchain:** STM32CubeIDE / ARM GCC  
**UART Baud Rate:** 115200

---

## Overview

This project implements and evaluates lightweight cryptographic security countermeasures on the STM32F767ZI microcontroller, framed around a simulated police surveillance drone (UAS) scenario. The STM32 board acts as the drone's compute board, communicating encrypted data to a host PC acting as the ground station over UART. A Python script running on the ground station receives and decrypts all transmitted data using a shared Blowfish DLL compiled from the same C source used on-board.

Security is implemented across three progressive levels:

| Level | Description |
|-------|-------------|
| 1 | Symmetric encryption (XTEA & Blowfish) for confidentiality and data integrity |
| 2 | SHA-256 firmware hash verification for secure boot |
| 3 | ECDSA digital signature generation and verification (attempted) |

---

## Repository Structure

```
lightweight-crypto-stm32-/
├── performance/            # Part 1: benchmarking projects
│   ├── xtea/               #   XTEA implementation + Makefile + READFILE.txt
│   ├── blowfish/           #   Blowfish implementation + Makefile + READFILE.txt
│   ├── xtea_sha256/        #   XTEA + SHA-256 firmware integrity
│   └── blowfish_sha256/    #   Blowfish + SHA-256 firmware integrity
├── case_study/             # Part 2: full drone mission implementation
│   ├── src/                #   STM32 firmware (encryption, UID, RNG, UART TX)
│   ├── ground_station.py   #   Python GS receiver and Blowfish decryption
│   ├── blowfish.dll        #   Compiled Blowfish DLL (MSYS2 / Windows)
│   └── READFILE.txt
├── results_report/         # Experimental results spreadsheet + written report (PDF)
├── presentation/           # Coursework slides (.pptx / PDF)
├── video/                  # Demo video
└── beakman_challenge/      # Bonus: real-time encrypted telemetry stream
```

---

## Part 1: Performance Benchmarking

All configurations were benchmarked on the STM32F767ZI. Execution time was measured over 100 iterations using the TIM11 hardware timer. Memory utilisation was read from the STM32CubeIDE build outputs.

| Model | Flag | Avg. Execution Time (µs) | Flash (KB) | RAM (KB) |
|-------|------|--------------------------|------------|----------|
| XTEA | -O0 | 77.46 | 28.82 | 2.58 |
| XTEA | -O3 | 27.52 | 23.63 | 2.58 |
| Blowfish | -O0 | 30.72 | 33.66 | 6.69 |
| Blowfish | -O3 | 5.42 | 32.01 | 6.69 |
| XTEA + SHA256 | -O0 | 147.50 | 34.83 | 2.75 |
| XTEA + SHA256 | -O3 | 96.11 | 29.66 | 2.75 |
| Blowfish + SHA256 | -O0 | 176.76 | 39.49 | 2.75 |
| Blowfish + SHA256 | -O3 | 89.78 | 37.83 | 2.75 |

Key findings:
- **Blowfish** consistently outperformed XTEA in raw execution speed (5.42 µs at -O3), though at a higher RAM cost (6.69 KB vs 2.58 KB)
- **XTEA** is the better choice for memory-constrained deployments
- **Compiler optimisation** had the largest impact on Blowfish, yielding a 5.6× speedup
- **SHA-256** added significant runtime overhead (~1.7× slowdown) and should be applied selectively (e.g. boot-time only)

---

## Part 2: Drone Mission Case Study

The case study simulates a complete drone-to-ground-station encrypted communication pipeline. All encryption is performed on the STM32F767ZI using **Blowfish with a 128-bit key**; decryption is handled by a Python script on the host PC using a Blowfish DLL compiled from the same C source (`blowfish.c / blowfish.h`) to guarantee identical behaviour.

### Mission Phases

**Phase 1 — Secure Boot (SHA-256 firmware integrity)**  
At startup, `FW_Hash_Verify()` from X-CUBE-CRYPTOLIB recomputes the firmware hash and compares it against a reference digest stored in Flash (embedded via `PostBuild_hash.bat`). Execution halts on mismatch.

**Phase 2 — Drone Identification (Encrypted UID)**  
The board's factory-programmed 96-bit unique ID is read via `HAL_GetUIDw0/1/2()`, split into two 64-bit blocks, and encrypted with Blowfish before transmission over UART.

**Phase 3 — Car Plate Identification (RNG + Encryption)**  
The STM32's hardware RNG generates a 6-digit number plate. It is encrypted with Blowfish and transmitted; the ground station decrypts and logs the plain plate.

**Phase 4 — Secret Message**  
The drone encrypts and transmits a 16-byte payload which decrypts to `"Amazing Ost \o/"`, verifying end-to-end correctness of the encrypted pipeline.

### Ground Station Setup

The Python ground station opens a serial connection via `pyserial`, receives encrypted 64-bit blocks, and decrypts them using `blowfish.dll` loaded via `ctypes`. The DLL was compiled from the same `blowfish.c` used on the STM32 using MSYS2:

```bash
gcc -shared -o blowfish.dll -DBUILD_DLL -fPIC blowfish.c
```

---

## ECDSA Signature Verification (Attempted — Level 3)

An ECDSA P-256 keypair was generated using OpenSSL. The private key was used to sign the SHA-256 firmware digest (`firmware.sig`), with the public key intended to be embedded in Flash for on-boot verification via X-CUBE-CRYPTOLIB. Key generation and signing succeeded; however, middleware incompatibilities on the STM32F7 platform prevented full integration. The final implementation uses SHA-256 integrity verification only.

---

## Beakman Challenge (Bonus)

A real-time encrypted telemetry stream was implemented to simulate continuous drone flight data. Every second, the STM32 generates and Blowfish-encrypts simulated sensor data — roll, pitch, yaw, latitude/longitude, altitude, battery level, and scanned plate numbers — and transmits it over UART. The Python ground station decrypts and plots altitude, battery, and yaw in real-time using Matplotlib. A descent sequence is triggered automatically when battery reaches 10%.

---

## Demo

https://github.com/pcvj2/lightweight-crypto-stm32-/raw/main/video/F130812_Cybersecurity_CW_Video.mp4

> If the video doesn't play inline, it can be found in the [`video/`](video/) folder.

---

## Getting Started

### Prerequisites

- STM32CubeIDE
- ST-LINK V2/V3 programmer
- Python 3 with `pyserial` and `matplotlib` (`pip install pyserial matplotlib`)
- Serial terminal for raw monitoring (e.g. PuTTY) — baud rate **115200**
- MSYS2 (Windows) if recompiling `blowfish.dll`

### Flash the STM32 Firmware

1. Open STM32CubeIDE and import the project from `performance/` or `case_study/`
2. Select `Debug` (-O0) or `Release` (-O3) build configuration
3. Build: **Project → Build Project**
4. Flash: **Run → Run**

For SHA-256 projects, run `PostBuild_hash.bat` after building to embed the firmware hash, then re-flash using STM32CubeProgrammer.

### Run the Ground Station

```bash
cd case_study/
python ground_station.py   # update COM port in script as needed
```
