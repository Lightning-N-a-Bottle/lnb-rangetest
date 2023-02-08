"""
Semi-Autonomous range-testing program for LNB system

Author: Connor J. Farrell, CJF Consulting Services
"""
# Import Python System Libraries
import time
# Import Blinka Libraries
import busio
from digitalio import DigitalInOut, Direction, Pull
import board
# Import the SSD1306 module.
import adafruit_ssd1306
# Import RFM9x library
import adafruit_rfm9x

# Configure reset button
reset_button = DigitalInOut(board.D5)
reset_button.direction = Direction.INPUT
reset_button.pull = Pull.UP

# Initialize I2C interface.
i2c = busio.I2C(board.SCL, board.SDA)

# Configure 128x32 OLED Display
reset_pin = DigitalInOut(board.D4)
display = adafruit_ssd1306.SSD1306_I2C(128, 32, i2c, reset=reset_pin)

# Clear the display.
display.fill(0)
display.show()
width = display.width
height = display.height

# Initialize LoRa Radio
CS = DigitalInOut(board.CE1)
RESET = DigitalInOut(board.D25)
spi = busio.SPI(board.SCK, MOSI=board.MOSI, MISO=board.MISO)

# Configure LoRa Radio, lowest throughput but hopefully furthest range
rfm9x = adafruit_rfm9x.RFM9x(spi, CS, RESET, 915.0)
rfm9x.tx_power = 23 #TX Power in dBm, between 5 and 23, integer
rfm9x.signal_bandwidth = 7800 # [7800, 10400, 15600, 20800, 31250, 41700, 62500, 125000, 250000] - Signal bandwidth
rfm9x.coding_rate = 8 #[5, 6, 7, 8] - Higher = more FEC, lower = more throughput
rfm9x.spreading_factor = 12 #[6, 7, 8, ... 10, 11, 12] - Higher = better SNR tolerance, lower = more throughput
rfm9x.enable_crc = True

sent_packets = 0
received_packets = 0
last_rx_rssi = 0
last_rx_snr = 0
updateDisplay = True

last_tx_time = time.time()
pingTime = 1 # Time in seconds to wait between pings

# Send a reset
tx_data = bytes("RST!", "utf-8")
rfm9x.send(tx_data)

while True:
    # Send reset if we need to
    if not reset_button.value:
        sent_packets = 0
        received_packets = 0
        last_rx_rssi = 0
        last_rx_snr = 0
        updateDisplay = True
        tx_data = bytes("RST!", "utf-8")
        rfm9x.send(tx_data)
        continue

    # Time to ping?
    if(time.time() - last_tx_time <= pingTime):
        # too soon, check back l8r
        continue

    # Send packet to remote, increment count
    rfm9x.send(bytes("hi mom","utf-8"))
    sent_packets += 1
    updateDisplay = True

    # Get response from remote
    packet = rfm9x.receive()
    if(packet):
        # get info, get stats
        received_packets = str(packet, "utf-8")
        last_rx_rssi = rfm9x.last_rssi
        last_rx_snr = rfm9x.last_snr

    if(updateDisplay):
        display.fill(0)
        display.text('TX: {}, RX: {}'.format(sent_packets, received_packets), 15, 0, 1)
        display.text('RSSI: {}, SNR: {}'.format(last_rx_rssi, last_rx_snr), 15, 20, 1)
        display.show()
        updateDisplay = False
