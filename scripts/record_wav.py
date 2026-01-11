import serial
import wave
import struct
import time

# ===== CONFIG =====
PORT = "COM6"          # CHANGE to your ESP32 port
BAUD = 115200
SAMPLE_RATE = 16000
OUTPUT_FILE = "mic_recording.wav"

ser = serial.Serial(PORT, BAUD, timeout=1)

wav = wave.open(OUTPUT_FILE, "w")
wav.setnchannels(1)
wav.setsampwidth(2)      # 16-bit PCM
wav.setframerate(SAMPLE_RATE)

print("Waiting for audio dump...")

samples = 0
last_data_time = time.time()

while True:
    line = ser.readline().decode(errors="ignore").strip()

    if line and (line.lstrip("-").isdigit()):
        sample = int(line)

        # clamp to int16
        sample = max(min(sample, 32767), -32768)

        wav.writeframes(struct.pack("<h", sample))
        samples += 1
        last_data_time = time.time()

    else:
        # stop if no data for a short while
        if samples > 0 and (time.time() - last_data_time) > 1.0:
            break

wav.close()
ser.close()

print(f"Saved {samples} samples to {OUTPUT_FILE}")
