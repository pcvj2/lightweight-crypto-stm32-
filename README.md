# Lightweight Cryptography on STM32 — AES, XTEA & Blowfish Case Study

A coursework project benchmarking and comparing three symmetric encryption algorithms — **AES-128**, **XTEA**, and **Blowfish** — deployed on an STM32 microcontroller. The project evaluates each algorithm's suitability for resource-constrained embedded systems through performance profiling, code size analysis, and a practical case study.

---

## Repository Structure

```
lightweight-crypto-stm32-/
├── case_study/         # Case study implementation and analysis
├── performance/        # Benchmarking code and timing measurements
├── beakman_challenge/  # Cryptographic challenge task
├── results_report/     # Written report and analysis
├── presentation/       # Slides for coursework presentation
└── video/              # Demo or submission video
```

---

## Algorithms Implemented

| Algorithm | Block Size | Key Size | Rounds |
|-----------|-----------|----------|--------|
| AES-128   | 128-bit   | 128-bit  | 10     |
| XTEA      | 64-bit    | 128-bit  | 64     |
| Blowfish  | 64-bit    | 32–448-bit | 16   |

---

## Target Hardware

- **Platform:** STM32 microcontroller
- **Language:** C
- **Toolchain:** ARM GCC / STM32CubeIDE (Makefile-based build)

---

## Key Objectives

- Implement AES, XTEA, and Blowfish in C for a bare-metal STM32 environment
- Measure and compare encryption/decryption throughput (cycles/byte or time per block)
- Analyse RAM and Flash footprint for each algorithm
- Identify the most suitable cipher for a given embedded use case
- Complete the Beakman challenge cryptographic task

---

## Performance Metrics

Benchmarks were collected on-device and captured via UART or SWO trace. Metrics include:

- **Execution time** per encryption/decryption operation
- **Code size** (Flash usage) for each algorithm
- **RAM usage** (stack and heap overhead)
- **Throughput** in bytes per second

Results and analysis are documented in [`results_report/`](results_report/).

---

## Getting Started

### Prerequisites

- STM32CubeIDE or an ARM GCC toolchain
- ST-LINK programmer/debugger
- Serial terminal (e.g. PuTTY, minicom) for UART output

### Build & Flash

```bash
# Clone the repository
git clone https://github.com/pcvj2/lightweight-crypto-stm32-.git
cd lightweight-crypto-stm32-

# Build using Make (adjust target as needed)
make

# Flash to STM32 via ST-LINK
make flash
```

> If using STM32CubeIDE, import the project and build/debug through the IDE directly.

---

## Results Summary

Full results and discussion are in the [`results_report/`](results_report/) directory. In general:

- **XTEA** offers the best balance of code size and performance on constrained STM32 targets — consistent with its original design intent for resource-limited systems
- **AES-128** provides the strongest security margin and hardware acceleration support, at the cost of higher code complexity
- **Blowfish** has a large key schedule overhead (expensive initialisation), making it less suitable for applications requiring frequent key changes

---

## Beakman Challenge

The [`beakman_challenge/`](beakman_challenge/) folder contains the solution to a cryptographic challenge task set as part of the coursework, demonstrating practical application of the implemented ciphers.

---

## Author

**Pavle** — MSc Advanced Microelectronics Systems Engineering, University of Bristol  
BEng Electronic & Electrical Engineering (First Class), Loughborough University
