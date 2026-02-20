# WSC332 – Cyber-security for Embedded Systems
## Coursework: Lightweight Cryptography on STM32F767

**Module:** WSC332 – Cyber-security for Embedded Systems  
**Institution:** Loughborough University  
**Target Hardware:** STM32F767 (NUCLEO-F767ZI)  
**Language:** C  
**Toolchain:** STM32CubeIDE / ARM GCC

---

## Overview

This coursework applies embedded security countermeasures to a simulated police surveillance drone (UAS) scenario. The STM32F767 board acts as the drone's compute board, communicating with a ground station (GS) over UART. Security is implemented across three progressive levels, covering symmetric encryption, firmware integrity via hashing, and digital signature verification.

The project is split into two parts:

| Part | Description | Weight |
|------|-------------|--------|
| Part 1 | Deployment and performance assessment of cryptography algorithms | 55% |
| Part 2 | Mission validation (Security Level 3 + drone mission phases) | 25% |
| Report | Written report (up to 10 pages, PDF) | 20% |

---

## Security Levels

**Level 1 — Confidentiality & Data Integrity**  
Deploy XTEA and Blowfish symmetric encryption algorithms and benchmark them against AES. Performance metrics (execution time, memory footprint) are collected under `-O0` and `-O3` compiler optimisation flags.

**Level 2 — Firmware Integrity**  
SHA-256 is used to compute a hash of the firmware binary, which is appended to detect firmware corruption at boot. The performance overhead of adding SHA-256 to both XTEA and Blowfish configurations is measured.

**Level 3 — Firmware Signature & Protected Storage**  
A digital signature (RSA-2048 or ECDSA-256) is generated for the firmware binary. The public key is embedded in the firmware and stored in a write-protected Flash sector. On boot, the firmware hash is re-computed and verified against the stored signature before execution is permitted.

---

## Repository Structure

```
lightweight-crypto-stm32-/
├── performance/        # Part 1: XTEA, Blowfish, AES, SHA256 STM32CubeIDE projects
│   ├── xtea/           #   - algorithm source, Makefile, READFILE.txt
│   ├── blowfish/
│   ├── xtea_sha256/
│   └── blowfish_sha256/
├── case_study/         # Part 2: Mission case study implementation
│   ├── src/            #   - drone mission phases (identification, encryption, signature)
│   ├── Makefile
│   └── READFILE.txt
├── results_report/     # Spreadsheet of experimental results + written report (PDF)
├── presentation/       # Coursework presentation slides (.pptx / PDF)
├── video/              # Demo video recording (if submitted)
└── beakman_challenge/  # Bonus challenge: INS attitude data encrypted with XTEA/Blowfish
```

---

## Part 1: Performance Benchmarking

All four algorithm configurations were benchmarked on the STM32F767 with the following metrics:

| Model | Compilation Flag | Av. Execution Time | Memory Utilisation |
|-------|-----------------|--------------------|--------------------|
| XTEA | -O0 | | |
| XTEA | -O3 | | |
| Blowfish | -O0 | | |
| Blowfish | -O3 | | |
| XTEA + SHA256 | -O0 | | |
| XTEA + SHA256 | -O3 | | |
| Blowfish + SHA256 | -O0 | | |
| Blowfish + SHA256 | -O3 | | |

> Full results are in [`results_report/`](results_report/).

**Algorithm references:**
- XTEA: Needham & Wheeler, "Tea extensions," University of Cambridge, 1997
- Blowfish: https://github.com/pcvj2/blowfish/blob/master/blowfish.c

---

## Part 2: Drone Mission Phases

The case study simulates a police drone performing a traffic surveillance mission. Communication between the STM32F767 (drone) and a host PC (ground station) is over UART/serial.

The four mission phases are:

1. **Security Level 3 verification** — firmware signature is checked at startup; execution halts if verification fails
2. **Drone identification** — the ground station requests the board's unique ID; the drone encrypts and transmits it using XTEA or Blowfish
3. **Car plate identification** — the on-board RNG generates a 6-digit car plate number; it is encrypted and sent to the GS, which decrypts and displays it
4. **Secret message** — the drone transmits the hex string `416D617A696E67204F7374205C6F2F20`, which the GS decrypts and converts to a readable string

---

## Beakman Challenge (Bonus — up to 10 marks)

An Inertial Navigation System (INS) outputs attitude data (roll φ, pitch θ, yaw ψ) from gyroscope, accelerometer, and magnetometer sensors. This data is encrypted on-board using XTEA, transmitted to a host PC, and decrypted using XTEA or Blowfish on the receiving end.

---

## Demo

https://github.com/pcvj2/lightweight-crypto-stm32-/raw/main/video/demo.mp4

> If the video doesn't play inline, you can find it in the [`video/`](video/) folder.

---

## Getting Started

### Prerequisites

- STM32CubeIDE (or ARM GCC + `arm-none-eabi` toolchain)
- ST-LINK V2/V3 programmer
- Serial terminal (e.g. PuTTY, TeraTerm) — default baud rate: **115200**

### Build & Flash (STM32CubeIDE)

1. Open STM32CubeIDE and import the relevant project from `performance/` or `case_study/`
2. Select the desired build configuration (`Debug` for `-O0`, `Release` for `-O3`)
3. Build: **Project → Build Project**
4. Flash: **Run → Run** (or use the ST-LINK debug interface)

### Build (Makefile / Command Line)

```bash
cd performance/xtea/
make          # builds with default flags
make O3=1     # builds with -O3 optimisation
make flash    # flashes via OpenOCD (if configured)
```

See the `READFILE.txt` in each subfolder for algorithm-specific build and run instructions.
