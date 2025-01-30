import serial
import matplotlib.pyplot as plt
from collections import deque
import time

ser = serial.Serial('COM8', 9600, timeout=0.1)
ser.reset_input_buffer()
buffer_size = 10000

class DataBuffer:
    def __init__(self, size):
        # Accelerometer data
        self.ax = deque(maxlen=size)
        
        self.ay = deque(maxlen=size)
        self.az = deque(maxlen=size)
        self.aroll = deque(maxlen=size)
        self.apitch = deque(maxlen=size)
        # Gyroscope data
        self.gx = deque(maxlen=size)
        self.gy = deque(maxlen=size)
        self.gz = deque(maxlen=size)
        self.groll = deque(maxlen=size)
        self.gpitch = deque(maxlen=size)

data = DataBuffer(buffer_size)
plt.ion()
fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(12, 8))

lines = {
    # Accelerometer lines
    'ax': ax1.plot([], [], label='X')[0],
    'ay': ax1.plot([], [], label='Y')[0],
    'az': ax1.plot([], [], label='Z')[0],
    'aroll': ax2.plot([], [], label='Roll')[0],
    'apitch': ax2.plot([], [], label='Pitch')[0],
    # Gyroscope lines
    'gx': ax3.plot([], [], label='X')[0],
    'gy': ax3.plot([], [], label='Y')[0],
    'gz': ax3.plot([], [], label='Z')[0],
    'groll': ax4.plot([], [], label='Roll')[0],
    'gpitch': ax4.plot([], [], label='Pitch')[0]
}

# Set up plot titles and labels
ax1.set_title('Accelerometer Raw Values')
ax2.set_title('Accelerometer Tilt Angles')
ax3.set_title('Gyroscope Raw Values')
ax4.set_title('Gyroscope Angles')

# Set initial y-axis limits
ax1.set_ylim([-4, 4])
ax2.set_ylim([-90, 90])
ax3.set_ylim([-500, 500])  # Adjust based on your gyroscope range
ax4.set_ylim([-90, 90])

# Enable legends
for ax in (ax1, ax2, ax3, ax4):
    ax.legend()
    ax.set_autoscale_on(True)
    ax.relim()
    ax.autoscale_view()

def parse_line(line):
    try:
        clean_line = line.strip().replace('\r', '').replace('\n', '')
        if not clean_line:
            return None
        values = [float(x.strip()) for x in clean_line.split(',') if x.strip()]
        if len(values) == 10:  # Updated to expect 10 values
            return values
    except (ValueError, IndexError) as e:
        print(f"Parse error: {e} on line: {line}")
    return None

def update_plot():
    try:
        ser.reset_input_buffer()
        if ser.in_waiting:
            line = ser.readline().decode('utf-8', errors='ignore')
            print(f"Decoded data: {line}")
            values = parse_line(line)
            
            if values:
                # Update accelerometer data
                data.ax.append(values[0])
                data.ay.append(values[1])
                data.az.append(values[2])
                data.aroll.append(values[3])
                data.apitch.append(values[4])
                # Update gyroscope data
                data.gx.append(values[5])
                data.gy.append(values[6])
                data.gz.append(values[7])
                data.groll.append(values[8])
                data.gpitch.append(values[9])
                
                print(f"Data: {values}")
                
                x_range = range(len(data.ax))
                # Update all line data
                for name, buf in [
                    ('ax', data.ax), ('ay', data.ay), ('az', data.az),
                    ('aroll', data.aroll), ('apitch', data.apitch),
                    ('gx', data.gx), ('gy', data.gy), ('gz', data.gz),
                    ('groll', data.groll), ('gpitch', data.gpitch)
                ]:
                    lines[name].set_data(x_range, buf)
                
                # Update all plot limits
                for ax in (ax1, ax2, ax3, ax4):
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