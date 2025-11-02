import serial
import re
from ctypes import (
    cdll, Structure, c_uint32, POINTER, byref
)

# === Use Blowfish shared library from C code ===
DLL_PATH = r"C:\Users\Pavle\Documents\Uni\Cybersecurity\casestudy_new\blowfish.dll"
lib = cdll.LoadLibrary(DLL_PATH)

# === Define Blowfish context structure to match C struct layout ===
class BLOWFISH_CTX(Structure):
    _fields_ = [("P", c_uint32 * 18), ("S", (c_uint32 * 256) * 4)]

# === Setup C type Blowfish functions
lib.Blowfish_Init.argtypes = [POINTER(BLOWFISH_CTX), POINTER(c_uint32), c_uint32]
lib.Blowfish_Encrypt.argtypes = [POINTER(BLOWFISH_CTX), POINTER(c_uint32), POINTER(c_uint32)]
lib.Blowfish_Decrypt.argtypes = [POINTER(BLOWFISH_CTX), POINTER(c_uint32), POINTER(c_uint32)]

# === Blowfish key must match your STM32 firmware
BF_KEY = (c_uint32 * 4)(0xABCDEFAB, 0xCDEFABCD, 0xEFABCDEF, 0xABCDEFAB)

# === Initialize Blowfish cipher context with key
ctx = BLOWFISH_CTX()
lib.Blowfish_Init(byref(ctx), BF_KEY, 16)

# === Open serial port for receiving STM32 data
ser = serial.Serial('COM6', baudrate=115200, timeout=2)
print("Listening for STM32 UART...\n")

# === Track secret message and UID blocks
secret_blocks = {}
uid_blocks = {}

# === Decrypt a 64-bit ciphertext using Blowfish ===
def decrypt_block(high_hex, low_hex):
    xl = c_uint32(int(low_hex, 16))  # lower 32-bits
    xr = c_uint32(int(high_hex, 16))  # higher 32-bits

    lib.Blowfish_Decrypt(byref(ctx), byref(xl), byref(xr))

    # Reconstruct after decryption
    decrypted = xl.value.to_bytes(4, 'little') + xr.value.to_bytes(4, 'little')
    return decrypted

# === Process a single line of UART input ===
def handle_line(line):
    global secret_blocks, uid_blocks

    print("Received:", line)

    # Extract hex pair
    match = re.search(r'\{([0-9a-fA-F]{8})\s+([0-9a-fA-F]{8})\}', line)
    if not match:
        return

    high, low = match.groups()
    decrypted = decrypt_block(high, low)

    # === UID Block Encryption ===
    if "UID block 1" in line:
        uid_blocks[1] = decrypted
        h, l = int.from_bytes(decrypted[:4], 'little'), int.from_bytes(decrypted[4:], 'little')
        

    elif "UID block 2" in line:
        uid_blocks[2] = decrypted
        h, l = int.from_bytes(decrypted[:4], 'little'), int.from_bytes(decrypted[4:], 'little')
        
        # If both blocks received, reassemble and print UID
        if 1 in uid_blocks:
            # Match STM32 format: uid[2] uid[1] uid[0]
            uid0 = int.from_bytes(uid_blocks[1][:4], 'little')    # low part of block 1
            uid1 = int.from_bytes(uid_blocks[1][4:], 'little')    # high part of block 1
            uid2 = int.from_bytes(uid_blocks[2][4:], 'little')    # low part of block 2 (only relevant part)
            print(f"[RECONSTRUCTED UID] {uid2:08X} {uid1:08X} {uid0:08X}")
            uid_blocks = {}

    # === Encrypted car plate decryption ===
    elif "plate" in line:
        val = int.from_bytes(decrypted[4:], 'little')
        print(f"[DECRYPTED PLATE] {val:06d}")

    # === Secret message handling ===
    elif "block1" in line:
        secret_blocks[1] = decrypted

    elif "block2" in line:
        secret_blocks[2] = decrypted

        # If both message blocks received, reconstruct and decode
        if 1 in secret_blocks and 2 in secret_blocks:
            full = secret_blocks[1] + secret_blocks[2]
            try:
                print(f"[DECRYPTED SECRET] \"{full.decode('utf-8')}\"")
            except UnicodeDecodeError:
                print(f"[!] Could not decode secret: {full.hex()}")
            secret_blocks = {}

# === UART read loop: listens for incoming encrypted blocks and decrypts them ===
try:
    while True:
        line = ser.readline().decode(errors='ignore').strip()
        if line:
            handle_line(line)
except KeyboardInterrupt:
    print("Exiting.")
finally:
    ser.close()
