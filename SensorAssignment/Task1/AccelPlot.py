import serial
import matplotlib.pyplot as plt
from collections import deque
import time

# FIX TO DO: Live Tracking on the plot
# Right now we're flushing the serial before reading which is correct, but we need to make sure we're reading the entire line, so we can try only reading full lines that exist

ser = serial.Serial('COM8', 9600, timeout=0.1)
ser.reset_input_buffer()
buffer_size = 10000

class DataBuffer:
    def __init__(self, size):
        self.x = deque(maxlen=size)
        self.y = deque(maxlen=size)
        self.z = deque(maxlen=size)
        self.roll = deque(maxlen=size)
        self.pitch = deque(maxlen=size)

data = DataBuffer(buffer_size)
plt.ion()
fig, (ax1, ax2) = plt.subplots(2, 1)

lines = {
    'x': ax1.plot([], [], label='X')[0],
    'y': ax1.plot([], [], label='Y')[0],
    'z': ax1.plot([], [], label='Z')[0],
    'roll': ax2.plot([], [], label='Roll')[0],
    'pitch': ax2.plot([], [], label='Pitch')[0]
}

ax1.set_title('Accelerometer')
ax1.set_ylim([-4, 4])
ax2.set_ylim([-90, 90])
ax1.legend()
ax2.legend()

def parse_line(line):
    try:
        # Remove any extra whitespace and split
        clean_line = line.strip().replace('\r', '').replace('\n', '')
        if not clean_line:
            return None
        # Split and convert to float, handling empty strings
        values = [float(x.strip()) for x in clean_line.split(',') if x.strip()]
        if len(values) == 5:
            return values
    except (ValueError, IndexError) as e:
        print(f"Parse error: {e} on line: {line}")
    return None

def update_plot():
    try:
        ser.reset_input_buffer()
        if ser.in_waiting:
            raw_line = ser.readline()
            print(f"Raw data: {raw_line}")  # Add this line
            line = raw_line.decode('utf-8', errors='ignore')
            print(f"Decoded data: {line}")  # And this line
            values = parse_line(line)
            
            if values:
                data.x.append(values[0])
                data.y.append(values[1])
                data.z.append(values[2])
                data.roll.append(values[3])
                data.pitch.append(values[4])
                print(f"Data: {values}")
                
                x_range = range(len(data.x))
                for name, buf in [('x', data.x), ('y', data.y), ('z', data.z),
                                ('roll', data.roll), ('pitch', data.pitch)]:
                    lines[name].set_data(x_range, buf)
                
                for ax in (ax1, ax2):
                    ax.relim()
                    ax.autoscale_view()
                
                plt.pause(0.001)
    except Exception as e:
        print(f"Error: {e}")
        return False
    return True

try:
    while update_plot():
        pass
finally:
    ser.close()
    plt.close()