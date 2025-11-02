XTEA and Blowfish Perfomance Harness
Overview
This firmware demonstrates the encryption and decryption using XTEA and Blowfish algorithms combined wiht a SHA-256 hashing. It is designed to run on an STM32767ZI and benchmarks the performance of four combinations, each conducted using O3 and O0 compilation falgs for analysis. 

STM32 Features
•	Firmware integrity check using SHA-256
•	Encrypted drone UID and car plate
•	Encrypted secret message
•	XTEA and Blowfish encryption and decryption over 100 runs


Requirements
Hardware
•	STM32767ZI
•	USB Connection over UART

Software
•	STMCubeIDE
•	STM32 Cryptographic Library (libSTM32CryptographicV3)
•	UART viewer (Putty)

SHA-256
•	Host_Tools' PostBuild_hash.bat
•	STMCubeProgrammer


How to Run
STM32 Firmware (if necessary, otherwise just flash and observe output using Putty)
•	Build main.c to STM32 board using STMCubeIDE
•	Use Host_Tools PostBuild_hash.bat to generate correct hash for application’s .bin
•	Program hash to the board using STMCubeProgrammer
•	Flash main.c to STM32 board using STMCubeIDE

Output
•	UART will print:
	Algorithm Encryption and Decryption
	100x benchmark in microseconds

Folder Structure
performance/
├── (XTEA or Blowfish)/        # Contains associated project files
└── README.md
