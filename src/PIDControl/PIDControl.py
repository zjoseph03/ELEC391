import serial
import matplotlib.pyplot as plt
import numpy as np
from collections import defaultdict
import argparse
import time
from matplotlib.widgets import Button, SpanSelector, RectangleSelector

def generate_sample_data():
    """Generate sample data for testing the visualization without serial connection"""
    data = defaultdict(lambda: {'time': [], 'output': [], 'angle': []})
    kp_values = [2.0, 4.0, 6.0, 8.0]
    
    for kp in kp_values:
        # Generate sample time points
        times = np.linspace(0, 10, 500)
        
        # Generate oscillations with different characteristics for each Kp
        frequency = 0.5 + kp/10  # Higher Kp = faster oscillation
        amplitude = 10 + kp*5    # Higher Kp = larger amplitude
        damping = 0.2 - kp/50    # Higher Kp = less damping
        
        # Generate outputs with damped oscillation
        outputs = amplitude * np.exp(-damping*times) * np.sin(2*np.pi*frequency*times)
        angles = outputs / 10
        
        data[kp]['time'] = times
        data[kp]['output'] = outputs
        data[kp]['angle'] = angles
        
    return data

def analyze_pid_data(port='COM8', baud=115200, use_sample_data=False):
    """Main function to collect and analyze PID data"""
    
    # Either generate sample data or collect from serial port
    if use_sample_data:
        print("Using sample data for testing...")
        data = generate_sample_data()
    else:
        # Configure serial port
        try:
            ser = serial.Serial(port, baud, timeout=1)
            time.sleep(2)  # Allow time for connection
        except Exception as e:
            print(f"Error opening serial port: {e}")
            print("Using sample data instead...")
            data = generate_sample_data()
            use_sample_data = True
            
        if not use_sample_data:
            # Data storage by Kp value
            data = defaultdict(lambda: {'time': [], 'output': [], 'angle': []})
            current_kp = None
            start_time = None
            
            print(f"Reading from {port} at {baud} baud...")
            print("Press Ctrl+C to stop data collection and view analysis")
            
            try:
                while True:
                    if ser.in_waiting:
                        line = ser.readline().decode('utf-8').strip()
                        
                        # Extract data from the serial output
                        if "Kp=" in line:
                            parts = line.split(',')
                            kp = float(parts[0].split('=')[1])
                            output = float(parts[1].split('=')[1])
                            angle = float(parts[2].split('=')[1]) if len(parts) > 2 else 0
                            
                            # Record first timestamp
                            if start_time is None:
                                start_time = time.time()
                            
                            # Detect Kp changes
                            if current_kp != kp:
                                current_kp = kp
                                print(f"New Kp: {current_kp}")
                            
                            # Store data
                            elapsed = time.time() - start_time
                            data[kp]['time'].append(elapsed)
                            data[kp]['output'].append(output)
                            data[kp]['angle'].append(angle)
                        
            except KeyboardInterrupt:
                print("Data collection stopped.")
                ser.close()
    
    # Plot results
    fig = plt.figure(figsize=(15, 10))
    ax1 = plt.subplot(2, 1, 1)
    
    # Store lines by Kp value for filtering
    lines = {}
    kp_values = sorted(data.keys())
    global selected_points
    selected_points = []  # To store clicked points for measurement
    
    # Main output plot
    max_time = 0
    for kp in kp_values:
        times = np.array(data[kp]['time'])
        outputs = np.array(data[kp]['output'])
        
        # Adjust time to start from 0 for each Kp
        if len(times) > 0:
            times = times - times[0]
            if times[-1] > max_time:
                max_time = times[-1]
            
            line, = ax1.plot(times, outputs, label=f'Kp = {kp}')
            lines[kp] = line
            
            # Find and mark oscillations
            if len(outputs) > 20:
                # Find peaks for oscillation analysis
                peak_indices = np.where((outputs[1:-1] > outputs[0:-2]) & 
                                       (outputs[1:-1] > outputs[2:]))[0] + 1
                
                if len(peak_indices) >= 2:
                    # Calculate oscillation period
                    periods = np.diff([times[i] for i in peak_indices])
                    avg_period = np.mean(periods)
                    
                    # Calculate amplitude
                    peak_values = [outputs[i] for i in peak_indices]
                    trough_indices = np.where((outputs[1:-1] < outputs[0:-2]) & 
                                            (outputs[1:-1] < outputs[2:]))[0] + 1
                    trough_values = [outputs[i] for i in trough_indices] if len(trough_indices) > 0 else [min(outputs)]
                    
                    amplitude = (np.mean(peak_values) - np.mean(trough_values)) / 2
                    
                    # Mark peaks with red dots
                    ax1.plot([times[i] for i in peak_indices], 
                              [outputs[i] for i in peak_indices], 
                              'ro', markersize=4)
                    
                    # Add text annotation
                    ax1.annotate(f"Period: {avg_period:.2f}s\nAmplitude: {amplitude:.2f}", 
                                 xy=(times[peak_indices[-1]], outputs[peak_indices[-1]]),
                                 xytext=(10, 10), textcoords='offset points',
                                 bbox=dict(boxstyle="round,pad=0.3", fc="yellow", alpha=0.7))
                    
                    print(f"For Kp={kp}: Period={avg_period:.2f}s, Amplitude={amplitude:.2f}")
    
    # Create more x-axis ticks
    max_time = np.ceil(max_time)
    tick_interval = 0.1  # Tick every 100ms
    ticks = np.arange(0, max_time + tick_interval, tick_interval)
    ax1.set_xticks(ticks)
    ax1.set_xticklabels([f"{t:.1f}" if t % 0.5 == 0 else "" for t in ticks], rotation=90, fontsize=8)
    
    # Add minor ticks between major ticks
    ax1.xaxis.set_minor_locator(plt.MultipleLocator(0.02))  # Minor tick every 20ms
    
    ax1.set_title('PID Output vs Time for Different Kp Values')
    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('PID Output')
    ax1.legend()
    ax1.grid(True, which='both', linestyle='--', alpha=0.7)
    
    # Add zoom reset button
    zoom_ax = plt.axes([0.8, 0.01, 0.1, 0.04])
    zoom_button = Button(zoom_ax, 'Reset Zoom')
    
    def reset_zoom(event):
        ax1.set_xlim(0, max_time)
        ax1.set_autoscaley_on(True)
        ax1.relim()
        ax1.autoscale_view()
        fig.canvas.draw()
    
    zoom_button.on_clicked(reset_zoom)
    
    # Add filtering buttons
    button_axes = []
    buttons = []
    
    # "Show All" button
    all_button_ax = plt.axes([0.2, 0.01, 0.1, 0.04])
    all_button = Button(all_button_ax, 'Show All')
    
    def show_all(event):
        for line in lines.values():
            line.set_visible(True)
        fig.canvas.draw()
    
    all_button.on_clicked(show_all)
    
    # Individual Kp buttons
    for i, kp in enumerate(kp_values):
        button_ax = plt.axes([0.35 + i*0.1, 0.01, 0.08, 0.04])
        button = Button(button_ax, f'Kp={kp}')
        
        def show_only(event, k=kp):
            for key, line in lines.items():
                line.set_visible(key == k)
            fig.canvas.draw()
        
        button.on_clicked(show_only)
        buttons.append(button)
    
    # Point selection and measurement
    point_text = ax1.text(0.02, 0.95, '', transform=ax1.transAxes, 
                         verticalalignment='top', bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))
    
    def on_click(event):
        if event.inaxes == ax1:
            # Add clicked point to selection
            selected_points.append((event.xdata, event.ydata))
            
            # Show point
            ax1.plot(event.xdata, event.ydata, 'go', markersize=8)
            
            # Update text display
            if len(selected_points) >= 1:
                point_info = f"Point {len(selected_points)}: ({event.xdata:.3f}s, {event.ydata:.3f})"
                
                if len(selected_points) >= 2:
                    # Calculate distance between last two points
                    p1 = selected_points[-2]
                    p2 = selected_points[-1]
                    time_diff = abs(p2[0] - p1[0])
                    value_diff = abs(p2[1] - p1[1])
                    
                    point_info += f"\nTime diff: {time_diff:.3f}s\nValue diff: {value_diff:.3f}"
                    
                    # Draw a line between the points
                    ax1.plot([p1[0], p2[0]], [p1[1], p2[1]], 'g--', alpha=0.7)
                    
                point_text.set_text(point_info)
                fig.canvas.draw()
    
    fig.canvas.mpl_connect('button_press_event', on_click)
    
    # Add clear points button
    clear_ax = plt.axes([0.65, 0.01, 0.1, 0.04])
    clear_button = Button(clear_ax, 'Clear Points')
    
    def clear_points(event):
        global selected_points
        selected_points = []
        point_text.set_text('')
        # Redraw plot to clear green points and lines
        for line in ax1.get_lines():
            if line not in lines.values():
                line.remove()
        fig.canvas.draw()
    
    clear_button.on_clicked(clear_points)
    
    # Enable pan/zoom with toolbar
    def line_select_callback(eclick, erelease):
        # Get the corners of the selection
        x1, y1 = eclick.xdata, eclick.ydata
        x2, y2 = erelease.xdata, erelease.ydata
        # Set the limits
        ax1.set_xlim(min(x1, x2), max(x1, x2))
        ax1.set_ylim(min(y1, y2), max(y1, y2))
        fig.canvas.draw()
    
    rs = RectangleSelector(ax1, line_select_callback,
                           useblit=True,
                           button=[1],  # Left mouse button
                           minspanx=5, minspany=5,
                           spancoords='pixels',
                           interactive=True)
    
    # FFT Analysis
    ax2 = plt.subplot(2, 1, 2)
    for kp in sorted(data.keys()):
        if len(data[kp]['output']) > 20:  # Need enough data for FFT
            output_values = np.array(data[kp]['output'])
            output_values = output_values - np.mean(output_values)
            
            # Compute FFT
            times = np.array(data[kp]['time'])
            sample_rate = len(times) / (times[-1] - times[0])
            fft_result = np.abs(np.fft.rfft(output_values))
            freqs = np.fft.rfftfreq(len(output_values), 1/sample_rate)
            
            # Find dominant frequency (excluding very low freq)
            min_freq_idx = max(1, int(0.2 * len(freqs)))
            dominant_idx = np.argmax(fft_result[min_freq_idx:]) + min_freq_idx
            dominant_freq = freqs[dominant_idx]
            oscillation_period = 1/dominant_freq if dominant_freq > 0 else 0
            
            ax2.plot(freqs, fft_result, label=f'Kp = {kp}, Period ≈ {oscillation_period:.2f}s')
    
    # Create more x-axis ticks for frequency plot
    freq_max = 5  # Maximum frequency to display
    freq_interval = 0.1  # Tick every 0.1 Hz
    freq_ticks = np.arange(0, freq_max + freq_interval, freq_interval)
    ax2.set_xticks(freq_ticks)
    ax2.set_xticklabels([f"{f:.1f}" if f % 0.5 == 0 else "" for f in freq_ticks], rotation=90, fontsize=8)
    
    # Add minor ticks for frequencies
    ax2.xaxis.set_minor_locator(plt.MultipleLocator(0.02))  # Minor tick every 0.02 Hz
    
    ax2.set_title('Frequency Domain Analysis')
    ax2.set_xlabel('Frequency (Hz)')
    ax2.set_ylabel('Amplitude')
    ax2.legend()
    ax2.grid(True, which='both', linestyle='--', alpha=0.7)
    ax2.set_xlim(0, freq_max)  # Focus on lower frequencies
    
    plt.subplots_adjust(bottom=0.15)  # Make room for buttons
    plt.tight_layout(rect=[0, 0.1, 1, 0.95])
    plt.show()
    
    # Ziegler-Nichols calculator
    print("\nEnter the ultimate gain (Ku) and period (Tu) for Ziegler-Nichols calculations:")
    try:
        ku = float(input("Ultimate gain (Ku, where stable oscillation occurs): "))
        tu = float(input("Oscillation period (Tu, in seconds): "))
        
        print("\nZiegler-Nichols PID parameters:")
        print(f"P only:  Kp = {0.5*ku:.4f}")
        print(f"PI:      Kp = {0.45*ku:.4f}, Ki = {0.54*ku/tu:.4f}")
        print(f"PID:     Kp = {0.6*ku:.4f}, Ki = {1.2*ku/tu:.4f}, Kd = {0.075*ku*tu:.4f}")
    except ValueError:
        print("Invalid input. Skipping calculations.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="PID Tuning Analysis Tool")
    parser.add_argument("--port", default="COM8", help="Serial port")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate")
    parser.add_argument("--sample", action="store_true", help="Use sample data instead of serial")
    args = parser.parse_args()
    
    analyze_pid_data(args.port, args.baud, args.sample)