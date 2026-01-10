import serial
import matplotlib.pyplot as plt
from collections import deque

ser = serial.Serial('COM6', 115200, timeout=0)

N = 300
data = deque([0]*N, maxlen=N)

plt.ion()
fig, ax = plt.subplots()
line_plot, = ax.plot(data)
ax.set_ylim(-20, 20)

while True:
    raw = ser.readline()
    if raw:
        try:
            val = int(raw.decode().strip().split()[0])
            data.append(val)
        except:
            pass

    line_plot.set_ydata(data)
    plt.pause(0.001)
