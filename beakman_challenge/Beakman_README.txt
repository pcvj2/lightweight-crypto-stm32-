Secure Drone Telemetry – Blowfish Encryption Project
Overview
This project demonstrates a simulated, secure telemetry system between a STM32767ZI and a PC. The STM32 firmware simulates drone data, encrypts it using Blowfish and sends it over UART. A Python script decrypts the data and displays it in real-time using live plots.
STM32 Features
•	Firmware integrity check using SHA-256
•	Encrypted drone UID and car plate
•	Encrypted secret message
•	Simulated flight data
•	Blowfish encryption of all data
Python Ground Station
•	Loads and uses blowfish.dll
•	Displays all transmitted encrypted data
•	Plots Altitude, Battery and Yaw over time

Requirements
Hardware
•	STM32767ZI
•	USB Connection over UART

Ground Station (Python Script)
•	Python 3.9+
•	blowfish.dll compiled from C
•	Python packages
	o pip install pyserial matplotlib re time struct

SHA-256
•	Host_Tools' PostBuild_hash.bat
•	STMCubeProgrammer


How to Run
STM32 Firmware
•	Build main.c to STM32 board using STMCubeIDE
•	Use Host_Tools PostBuild_hash.bat to generate correct hash for application's "ProjectName.bin"
•	Program hash to the board using STMCubeProgrammer
•	Flash main.c to STM32 board using STMCubeIDE
Python Script
•	Ensure Python script COM port is mapped correctly
•	Ensure that blowfish.dll is on path
•	Run python beakman_decryptor.py in command terminal (or similar)
•	Watch live telemetry output and plots

Folder Structure
SecureDroneTelemetry/
├── STM32_Firmware/       # Contains main.c
├── Python_GroundStation/ # Contains blowfish_pycode.py and blowfish.dll
└── README.md
