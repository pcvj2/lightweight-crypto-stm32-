import serial
import re
import struct
import time
import matplotlib.pyplot as plt
from collections import deque
from ctypes import (
    cdll, Structure, c_uint32, POINTER, byref
)

# === Load Blowfish DLL ===
# Make sure path matches the compiled C library used for decryption
DLL_PATH = r"C:\Users\Pavle\Documents\Uni\Cybersecurity\casestudy_new\blowfish.dll"
lib = cdll.LoadLibrary(DLL_PATH)

# === Define Blowfish C structure ===
class BLOWFISH_CTX(Structure):
    _fields_ = [
        ("P", c_uint32 * 18),
        ("S", (c_uint32 * 256) * 4)
    ]

# === Set argument types for Blowfish C functions ===
lib.Blowfish_Init.argtypes = [POINTER(BLOWFISH_CTX), POINTER(c_uint32), c_uint32]
lib.Blowfish_Encrypt.argtypes = [POINTER(BLOWFISH_CTX), POINTER(c_uint32), POINTER(c_uint32)]
lib.Blowfish_Decrypt.argtypes = [POINTER(BLOWFISH_CTX), POINTER(c_uint32), POINTER(c_uint32)]

# === Blowfish Key (Must match STM32 firmware key) ===
BF_KEY = (c_uint32 * 4)(0xABCDEFAB, 0xCDEFABCD, 0xEFABCDEF, 0xABCDEFAB)

# === Initialize Blowfish context ===
ctx = BLOWFISH_CTX()
lib.Blowfish_Init(byref(ctx), BF_KEY, 16)

# === Open UART Port ===
ser = serial.Serial('COM6', baudrate=115200, timeout=2)
print("Listening for STM32 UART...\n")

# === Store state ===
secret_blocks = {}   # Holds decrypted blocks of a secret message
uid_blocks = {}      # Holds decrypted UID block1 and block2
telemetry = {}       # Current telemetry frame values

# === Live Plot Setup ===
history_len = 60  # Number of telemetry points to keep

time_history = deque(maxlen=history_len)
alt_history = deque(maxlen=history_len)
batt_history = deque(maxlen=history_len)
yaw_history = deque(maxlen=history_len)

plt.ion()
fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(10, 8))
fig.suptitle("Live Drone Telemetry", fontsize=16)

# === Live Plot Updater ===
def update_live_plot(t, lat, lon, alt, batt, roll, pitch, yaw):
    time_history.append(t)
    alt_history.append(alt)
    batt_history.append(batt)
    yaw_history.append(yaw)

    ax1.clear()
    ax2.clear()
    ax3.clear()

    ax1.plot(time_history, alt_history, label='Altitude (m)', color='tab:blue')
    ax2.plot(time_history, batt_history, label='Battery (%)', color='tab:green')
    ax3.plot(time_history, yaw_history, label='Yaw (°)', color='tab:orange')

    for ax in (ax1, ax2, ax3):
        ax.legend(loc="upper right")
        ax.grid(True)

    ax1.set_ylabel("Altitude (m)")
    ax2.set_ylabel("Battery (%)")
    ax3.set_ylabel("Yaw (°)")
    ax3.set_xlabel("Time (s)")

    plt.pause(0.001)

# === Decrypt a single 64-bit block from STM32 telemetry ===
def decrypt_block(high_hex, low_hex):
    xl = c_uint32(int(low_hex, 16))   # lower word
    xr = c_uint32(int(high_hex, 16))  # upper word
    lib.Blowfish_Decrypt(byref(ctx), byref(xl), byref(xr))
    return xl.value.to_bytes(4, 'little') + xr.value.to_bytes(4, 'little')

# === Handle one incoming UART line ===
def handle_line(line):
    global secret_blocks, uid_blocks, telemetry

    print("Received:", line)
    match = re.search(r'\{([0-9a-fA-F]{8})\s+([0-9a-fA-F]{8})\}', line)
    if not match:
        return

    high, low = match.groups()
    decrypted = decrypt_block(high, low)

    # UID Reconstruction
    if "UID block 1" in line:
        uid_blocks[1] = decrypted
    elif "UID block 2" in line:
        uid_blocks[2] = decrypted
        if 1 in uid_blocks:
            uid0 = int.from_bytes(uid_blocks[1][:4], 'little')
            uid1 = int.from_bytes(uid_blocks[1][4:], 'little')
            uid2 = int.from_bytes(uid_blocks[2][4:], 'little')
            print(f"[RECONSTRUCTED UID] {uid2:08X} {uid1:08X} {uid0:08X}")
            uid_blocks = {}

    # Secret Message Assembly
    elif "block1" in line:
        secret_blocks[1] = decrypted
    elif "block2" in line:
        secret_blocks[2] = decrypted
        if 1 in secret_blocks and 2 in secret_blocks:
            full = secret_blocks[1] + secret_blocks[2]
            try:
                print(f"[DECRYPTED SECRET] \"{full.decode('utf-8')}\"")
            except UnicodeDecodeError:
                print(f"[!] Could not decode secret: {full.hex()}")
            secret_blocks = {}

    # Car Plate Decryption
    elif "plate" in line:
        telemetry['plate'] = int.from_bytes(decrypted[4:], 'little')

    # Position
    elif "TELE_POS" in line:
        telemetry['lat'] = int.from_bytes(decrypted[:4], 'little')
        telemetry['lon'] = int.from_bytes(decrypted[4:], 'little')

    # Altitude and Battery
    elif "TELE_ALT" in line:
        telemetry['alt'] = int.from_bytes(decrypted[:4], 'little')
        telemetry['batt'] = int.from_bytes(decrypted[4:], 'little')

    # Roll and Pitch
    elif "TELE_RP" in line:
        try:
            telemetry['roll'] = struct.unpack('<i', decrypted[:4])[0] / 100.0
            telemetry['pitch'] = struct.unpack('<i', decrypted[4:])[0] / 100.0
        except:
            telemetry['roll'] = telemetry['pitch'] = float('nan')

    # Yaw
    elif "TELE_YM" in line:
        try:
            telemetry['yaw'] = struct.unpack('<i', decrypted[:4])[0] / 100.0
        except:
            telemetry['yaw'] = float('nan')

    # === When full telemetry frame is available ===
    if {'lat', 'lon', 'alt', 'batt', 'roll', 'pitch', 'yaw'}.issubset(telemetry):
        print("\n  TELEMETRY FRAME")
        print("──────────────────────────────")
        print(f"Position : LAT  {telemetry['lat']:<8} LON  {telemetry['lon']}")
        print(f"Altitude : {telemetry['alt']} m")
        print(f"Roll     : {telemetry['roll']}°")
        print(f"Pitch    : {telemetry['pitch']}°")
        print(f"Yaw      : {telemetry['yaw']}°")
        print(f"Battery  : {telemetry['batt']} %")

        if 'plate' in telemetry:
            print("──────────────────────────────")
            print("\n   SCANNED PLATE")
            print("──────────────────────────────")
            print(f"Plate    : {telemetry['plate']:06d}")

        print("──────────────────────────────")

        # Update the live telemetry plot
        update_live_plot(
            time.time(),
            telemetry['lat'], telemetry['lon'],
            telemetry['alt'], telemetry['batt'],
            telemetry['roll'], telemetry['pitch'], telemetry['yaw']
        )

        # Reset for next frame
        telemetry = {}

# === Continuous UART read loop ===
try:
    while True:
        line = ser.readline().decode(errors='ignore').strip()
        if line:
            handle_line(line)
except KeyboardInterrupt:
    print("Exiting.")
finally:
    ser.close()
