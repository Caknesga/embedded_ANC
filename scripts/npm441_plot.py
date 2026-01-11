import serial
import matplotlib.pyplot as plt
from collections import deque

# ===== CONFIG =====
PORT = "COM6"        # CHANGE to your ESP32 port
BAUD = 115200
SAMPLES = 500        # points on screen

ser = serial.Serial(PORT, BAUD)

buffer = deque([0]*SAMPLES, maxlen=SAMPLES)

plt.ion()
fig, ax = plt.subplots()
line, = ax.plot(buffer)
ax.set_ylim(-500000, 500000)
ax.set_title("INMP441 Audio Signal")

while True:
    try:
        value = int(ser.readline().decode().strip())
        buffer.append(value)
        line.set_ydata(buffer)
        plt.pause(0.001)
    except:
        pass
