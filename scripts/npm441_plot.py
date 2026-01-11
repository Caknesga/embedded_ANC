import serial
import math
import matplotlib.pyplot as plt
from collections import deque

# ---- SERIAL ----
PORT = "COM6"      # change if needed
BAUD = 115200

# ---- RMS SETTINGS ----
BLOCK = 256        # RMS computed over this many samples (increase for smoother)
HISTORY = 200      # points displayed

ser = serial.Serial(PORT, BAUD, timeout=1)

rms_history = deque([0]*HISTORY, maxlen=HISTORY)
block_samples = []

plt.ion()
fig, ax = plt.subplots()
line, = ax.plot(list(rms_history))
ax.set_title("Real-time RMS from ESP32 samples")
ax.set_xlabel("Frame")
ax.set_ylabel("RMS")

while True:
    try:
        s = ser.readline().decode(errors="ignore").strip()
        if not s:
            continue

        # accept signed integers
        if not s.lstrip("-").isdigit():
            continue

        sample = int(s)
        block_samples.append(sample)

        # compute RMS once we have a block
        if len(block_samples) >= BLOCK:
            # RMS = sqrt(mean(x^2))
            mean_sq = sum(x*x for x in block_samples) / len(block_samples)
            rms = int(math.sqrt(mean_sq))
            block_samples.clear()

            rms_history.append(rms)
            line.set_ydata(list(rms_history))
            line.set_xdata(range(len(rms_history)))
            ax.relim()
            ax.autoscale_view(scalex=False, scaley=True)
            plt.pause(0.01)

    except KeyboardInterrupt:
        break

ser.close()
