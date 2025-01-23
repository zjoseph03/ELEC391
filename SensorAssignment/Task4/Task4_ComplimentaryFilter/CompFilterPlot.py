import serial
import matplotlib.pyplot as plt
from collections import deque
import time
import numpy as np

ser = serial.Serial('COM8', 9600, timeout=0.1)
ser.reset_input_buffer()
buffer_size = 10000

class DataBuffer:
    def __init__(self, size):
        # Only store the roll-related data
        self.acc_roll = deque(maxlen=size)    # Accelerometer roll
        self.gyro_roll = deque(maxlen=size)   # Gyroscope roll rate
        self.filtered_roll = deque(maxlen=size)  # Filtered roll

data = DataBuffer(buffer_size)
plt.ion()
fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(13, 10))

lines = {
    'acc_roll': ax1.plot([], [], label='Accelerometer Roll', color='blue')[0],
    'gyro_roll': ax2.plot([], [], label='Gyroscope Roll Angle', color='red')[0],
    'filtered_roll': ax3.plot([], [], label='Filtered Roll', color='green')[0]
}

# Set up plot titles and labels
ax1.set_title('Unfiltered Accelerometer Roll Angle')
ax2.set_title('Unfiltered Gyroscope Angle')
ax3.set_title('Filtered Roll Angle')

# Set initial y-axis limits
ax1.set_ylim([-90, 90])
ax2.set_ylim([-90, 90])
ax3.set_ylim([-90, 90])

# Set specific y-axis ticks for filtered roll (every 10 degrees)
y_ticks = np.arange(-90, 91, 10)
ax3.set_yticks(y_ticks)
ax3.set_yticklabels([f'{int(tick)}°' for tick in y_ticks])
ax3.yaxis.grid(True, which='major', linestyle='-', alpha=0.7)
ax3.yaxis.grid(True, which='minor', linestyle=':', alpha=0.4)

# Enable legends and grids
for ax in (ax1, ax2, ax3):
    ax.legend()
    ax.grid(True)
    ax.set_xlabel('Samples')

# Add y-axis labels
ax1.set_ylabel('Angle (degrees)')
ax2.set_ylabel('Angular (degrees)')
ax3.set_ylabel('Angle (degrees)')

def parse_line(line):
    try:
        clean_line = line.strip().replace('\r', '').replace('\n', '')
        if not clean_line:
            return None
        values = [float(x.strip()) for x in clean_line.split(',') if x.strip()]
        if len(values) == 13:  # Still expect 13 values from Arduino
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
                # Only store roll-related values
                data.acc_roll.append(values[3])      # Accelerometer roll
                data.gyro_roll.append(values[12])     # Gyroscope roll rate
                data.filtered_roll.append(values[10]) # Filtered roll
                
                print(f"Roll Data - Acc: {values[3]:.2f}, Gyro: {values[8]:.2f}, Filtered: {values[10]:.2f}")
                
                x_range = range(len(data.acc_roll))
                
                # Update all line data
                lines['acc_roll'].set_data(x_range, data.acc_roll)
                lines['gyro_roll'].set_data(x_range, data.gyro_roll)
                lines['filtered_roll'].set_data(x_range, data.filtered_roll)
                
                # Update all plot limits while maintaining y-axis for filtered roll
                ax1.relim()
                ax1.autoscale_view()
                ax2.relim()
                ax2.autoscale_view()
                # Only autoscale x-axis for filtered roll
                ax3.set_xlim(min(x_range), max(x_range))
                
                plt.tight_layout()  # Adjust spacing between plots
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