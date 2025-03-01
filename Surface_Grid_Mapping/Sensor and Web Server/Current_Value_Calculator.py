import asyncio
import bleak
from bleak import BleakScanner, BleakClient
import struct
import numpy as np
from scipy.signal import butter, filtfilt
from scipy.fft import fft, fftfreq
from statistics import mean
import dash
from dash import dcc, html
from dash.dependencies import Input, Output
import plotly.graph_objs as go
from collections import deque
import threading

CHARACTERISTIC_UUID = "7299c6c1-4170-4e80-b2a9-746152969b3f"

# Constants from the Sensor and Microcontroller
ADC_SUPPLY_VOLTAGE       = 3.6   # Supply Voltage from RA4W1 (3.6V)
SENSOR_REFERENCE_VOLTAGE = 2.5   # Sensor Reference Voltage (2.5V)
MAX_ADC_VALUE            = 16383 # Maximum ADC Value from 14-Bit ADC
SENSOR_SENSITIVITY       = 1     # Sensor Sensitivity at 1V/Gauss

SAMPLE_RATE = 1000  # Sampling Rate in Hz
NYQUIST = SAMPLE_RATE / 2.0

# Band-Pass Filter Design for 50Hz ± 5Hz
LOW_CUTOFF = 45 / NYQUIST
HIGH_CUTOFF = 55 / NYQUIST
B, A = butter(2, [LOW_CUTOFF, HIGH_CUTOFF], btype='band')

# Buffers to Store Received Magnetic Field Data
buffer_x = []
buffer_y = []
buffer_z = []
BUFFER_SIZE = 20

# List to Store Current Values for Averaging
ble_current_values = []

# Initialize a Deque to Store the Current Values for the Graph
current_values = deque(maxlen=20)  # Stores the Last 20 Values

# Variable to keep Track of which Axis Data we are Receiving
current_axis = "X"

async def scan_for_device():
    print("Scanning for the Renesas RA4W1 MCU...")
    while True:
        devices = await BleakScanner.discover()
        for device in devices:
            if device.name == "ADC_BLE":
                print(f"Found your Device! Device Name: {device.name} with Device Address: {device.address}")
                return device.address
        await asyncio.sleep(0.1)  # Reduced Delay between the Scans

async def connect_and_subscribe(Address):
    while True:  # Loop to handle Reconnection attempts
        try:
            async with BleakClient(Address) as client:
                print(f"Connected to {Address}")

                await client.start_notify(CHARACTERISTIC_UUID, notification_handler)
                print("Subscribed to All Notifications. Receiving Data...")

                # Keep the Connection Alive and Process Notifications
                while True:
                    if not client.is_connected:
                        raise Exception("Client Disconnected")
                    await asyncio.sleep(1)  # Adjusted Sleep Interval for Connection checks

        except Exception as e:
            print(f"Connection Lost, Reconnecting... Error: {str(e)}")
            await asyncio.sleep(5)  # Delay before Reconnecting

def convert_little_endian_to_decimal(Hex_Data):
    Decimal_Value = struct.unpack('<H', Hex_Data)[0]
    return Decimal_Value

def convert_adc_to_voltage(ADC_Value):
    Sensor_Voltage = (ADC_Value / MAX_ADC_VALUE) * ADC_SUPPLY_VOLTAGE
    return Sensor_Voltage

def convert_voltage_to_gauss(Sensor_Voltage):
    Gauss_Value = (Sensor_Voltage - SENSOR_REFERENCE_VOLTAGE) / SENSOR_SENSITIVITY
    return Gauss_Value

def apply_filter(buffer):
    return filtfilt(B, A, buffer)

def perform_fft_and_filter(buffer):
    # Perform FFT on the Buffer
    N = len(buffer)
    yf = fft(buffer)
    xf = fftfreq(N, 1 / SAMPLE_RATE)[:N // 2]

    # Apply the Band-Pass Filter to the Buffer
    filtered_buffer = apply_filter(buffer)

    # Return the Final Filtered Value
    return filtered_buffer[-1]

async def notification_handler(Sender, Data):
    global buffer_x, buffer_y, buffer_z, current_axis, ble_current_values, current_values

    if len(Data) == 2:  # Expecting 2 Bytes (16-Bit Value)
        # Decode the ADC Value from the Received Data
        adc_value = convert_little_endian_to_decimal(Data)
        sensor_voltage = convert_adc_to_voltage(adc_value)
        gauss_value = convert_voltage_to_gauss(sensor_voltage)

        # Store the Value in the appropriate Buffer based on the Current Axis
        if current_axis == "X":
            buffer_x.append(gauss_value)
            current_axis = "Y"
        elif current_axis == "Y":
            buffer_y.append(gauss_value)
            current_axis = "Z"
        elif current_axis == "Z":
            buffer_z.append(gauss_value)
            current_axis = "X"  # Cycle back to X

            # When all Buffers are full, apply the FFT and Filter, then calculate the final Resultant Magnetic Field and Current
            if len(buffer_x) >= BUFFER_SIZE and len(buffer_y) >= BUFFER_SIZE and len(buffer_z) >= BUFFER_SIZE:
                filtered_x = perform_fft_and_filter(buffer_x)
                filtered_y = perform_fft_and_filter(buffer_y)
                filtered_z = perform_fft_and_filter(buffer_z)

                B_resultant_gauss = np.sqrt(filtered_x**2 + filtered_y**2 + filtered_z**2)
                B_resultant_tesla = B_resultant_gauss * 1e-4  # Convert Gauss to Tesla
                mu_0 = 4 * np.pi * 1e-7  # N/A^2 (Permeability of free space)
                r = 0.4  # Distance in Meters (assumed)
                current = (2 * np.pi * r * B_resultant_tesla) / mu_0

                # Store the Current Value for Averaging
                ble_current_values.append(current)

                # Calculate and print the Average Current every 10 Processed Buffers
                if len(ble_current_values) >= 10:
                    avg_current = mean(ble_current_values)
                    print("\nAverage Current After FFT and Filtering:")
                    print(f"Average Current: {avg_current:.6f} A\n")

                    # Append the Average Current to the Deque for Real-Time Graphing
                    current_values.append(avg_current)

                    # Clear the list after displaying the Average
                    ble_current_values.clear()

                # Clear the Buffers to start collecting New Data
                buffer_x.clear()
                buffer_y.clear()
                buffer_z.clear()

        await asyncio.sleep(0.05)  # Introduce a Small Delay to slow down the Processing

    else:
        print(f"Unexpected Data Length from {Sender}: {len(Data)} bytes received")

# Create a Dash app
app = dash.Dash(__name__)

# Layout of the Dash app
app.layout = html.Div([
    html.H1("Digital Gauge and Real-Time Graph"),
    dcc.Graph(id='gauge'),
    dcc.Graph(id='live-graph'),
    dcc.Interval(
        id='interval-component',
        interval=1000,  # Update every second
        n_intervals=0
    )
])

# Callback to update both the gauge and the graph
@app.callback(
    [Output('gauge', 'figure'),
     Output('live-graph', 'figure')],
    [Input('interval-component', 'n_intervals')]
)
def update_output(n):
    if current_values:
        current_value = current_values[-1]  # Get the latest current value
    else:
        current_value = 0  # Default value if no data is available yet

    # Create the gauge figure
    gauge_fig = go.Figure(go.Indicator(
        mode="gauge+number",
        value=current_value,
        title={'text': "Current (A)"},
        gauge={'axis': {'range': [0, 10]},
               'bar': {'color': "darkblue"},
               'steps': [{'range': [0, 5], 'color': "lightgray"},
                         {'range': [5, 10], 'color': "gray"}],
               'threshold': {'line': {'color': "red", 'width': 4}, 'thickness': 0.75, 'value': 7.5}}  # Threshold for alert
    ))

    # Create the live graph figure
    graph_fig = go.Figure(
        data=[go.Scatter(x=list(range(len(current_values))), y=list(current_values), mode='lines+markers')],
        layout=go.Layout(
            title="Current Over Time",
            xaxis=dict(range=[0, 20]),
            yaxis=dict(range=[0, 10]),
        )
    )

    return gauge_fig, graph_fig

def run_ble_loop():
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    device_address = loop.run_until_complete(scan_for_device())
    
    if device_address:
        loop.run_until_complete(connect_and_subscribe(device_address))

if __name__ == "__main__":
    # Run the BLE loop in a separate thread
    ble_thread = threading.Thread(target=run_ble_loop, daemon=True)
    ble_thread.start()

    # Run the Dash app
    app.run_server(host='0.0.0.0', port=8050, debug=True)
