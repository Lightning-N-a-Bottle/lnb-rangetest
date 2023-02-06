# lnb-example
This is the code from the online example
https://learn.adafruit.com/adafruit-radio-bonnets/rfm9x-raspberry-pi-setup

Steps to set up raspberry pi zero or Pi 4 (That is what I have tested with so far)

Step 1 - clean raspberry pi OS install
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install python3-pip
sudo pip3 install --upgrade setuptools

Step 2 - Install Circuit Python Libraries
<br>sudo pip3 install adafruit-circuitpython-ssd1306
<br>sudo pip3 install adafruit-circuitpython-framebuf
<br>sudo pip3 install adafruit-circuitpython-rfm9x
<br>wget https://github.com/adafruit/Adafruit_CircuitPython_framebuf/raw/main/examples/font5x8.bin // font
<br>wget https://raw.githubusercontent.com/adafruit/Adafruit_Learning_System_Guides/main/pi_radio/rfm9x_check.py
<br>wget https://github.com/adafruit/Adafruit_Learning_System_Guides/blob/main/pi_radio/radio_rfm9x.py

Step 2.5 - Check if system is set up correctly
run rfm9x_check.py  // Grab from this repository

Step 3
run radio_rfm9x.py  // Grab from this repository
