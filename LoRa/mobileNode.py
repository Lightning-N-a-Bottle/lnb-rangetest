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

# Button A
btnA = DigitalInOut(board.D5)
btnA.direction = Direction.INPUT
btnA.pull = Pull.UP

# Button B
btnB = DigitalInOut(board.D6)
btnB.direction = Direction.INPUT
btnB.pull = Pull.UP

# Button C
btnC = DigitalInOut(board.D12)
btnC.direction = Direction.INPUT
btnC.pull = Pull.UP

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

# Ping Function
def ping():
    # ping the fixed node
    tx_data = bytes("ping", "utf-8")
    rfm9x.send(tx_data)
    print("Sent ping")

    linkStats = [0,-999, -999]

    # Wait for a reply, timeout if necessary
    packet = rfm9x.receive()
    if(packet):
        print("Got response")
        linkStats[0] = 1
        linkStats[1] = rfm9x.last_rssi
        linkStats[2] = rfm9x.last_snr

    # Return link stats
    return linkStats

# Link Quality Checker
def checkLink():
    print("linkcheck")
    linkQualities = []

    # Ping 10 times, append to list
    for i in range(10):
        display.text('packet {}/10'.format(i+1), 15, 20, 1)
        display.show()
        linkQualities.append(ping())

    print(linkQualities)
    # Return packet list
    return linkQualities

# Main
display.fill(0)
display.text('Waiting...', 15, 0, 1)
display.text('', 15, 20, 1)
display.show()

while True:
    # Reset/Clear Screen
    if not btnA.value:
        print("Clear Screen")
        display.fill(0)
        display.text('Waiting...', 15, 0, 1)
        display.text('', 15, 20, 1)
        display.show()
        continue

    # Single ping?
    if not btnB.value:
        print("Ping Button")
        display.fill(0)
        display.text('Pinging...', 15, 0, 1)
        display.text('', 15, 20, 1)
        display.show()
        res = ping()
        display.fill(0)
        display.text('Success: {}'.format(res[0]), 15, 0, 1)
        display.text('RSSI: {}, SNR: {}'.format(res[1], res[2]), 15, 20, 1)
        display.show()
        continue

    # do a test
    if not btnC.value:
        print("LQ Test")
        display.fill(0)
        display.text('Testing...', 15, 0, 1)
        display.text('', 15, 20, 1)
        display.show()
        # Get our 10 test values
        res = checkLink()

        print("returned from linkcheck")

        # variables to hold test stats
        avgRSSI = 0
        avgSNR = 0
        successfulPings = 0

        # Iterate over list, get values of interest
        for row in res:
            if(row[0] == 1):
                successfulPings += 1
                avgRSSI += row[1]
                avgSNR += [2]
            else:
                continue

        # more than 1 ping, average the RSSI/SNR
        if(successfulPings > 1):
            avgRSSI /= successfulPings
            avgSNR /= successfulPings

        # No pings?
        if(successfulPings == 0):
            avgRSSI = 0
            avgSNR = 0

        # Update display
        display.fill(0)
        display.text('Pings: {}'.format(successfulPings), 15, 0, 1)
        display.text('RSSI: {}, SNR: {}'.format(avgRSSI, avgSNR), 15, 20, 1)
        display.show()
        continue
