# SPDX-FileCopyrightText: 2021 ladyada for Adafruit Industries
# SPDX-License-Identifier: MIT

"""
This test will initialize the display using displayio and draw a solid green
background, a smaller purple rectangle, and some yellow text.
"""

import board
import displayio
import busio

import ssl
import wifi
import socketpool
import adafruit_requests
import time
import json

# Starting in CircuitPython 9.x fourwire will be a seperate internal library
# rather than a component of the displayio library
try:
    from fourwire import FourWire
except ImportError:
    from displayio import FourWire
from adafruit_st7789 import ST7789

# First set some parameters used for shapes and text
BORDER = 20
FONTSCALE = 1
BACKGROUND_COLOR = 0x00F300  # Bright Green
FOREGROUND_COLOR = 0xAA0088  # Purple
TEXT_COLOR = 0xFFFF00

# Release any resources currently in use for the displays
displayio.release_displays()


spi = busio.SPI(
    board.GP18,  # clock / SCL
    board.GP19,  # MOSI / SDA
)

display_bus = FourWire(
    spi, command=board.GP20, chip_select=board.GP17, reset=board.GP21
)

display = ST7789(
    display_bus, rotation=270, width=240, height=135, rowstart=40, colstart=53
)


wifi.radio.connect("WifiSmith", "suzyetcoco")


pool = socketpool.SocketPool(wifi.radio)

#  prints MAC address to REPL
print("My MAC addr:", [hex(i) for i in wifi.radio.mac_address])

#  prints IP address to REPL
print("My IP address is", wifi.radio.ipv4_address)

pool = socketpool.SocketPool(wifi.radio)
requests = adafruit_requests.Session(pool, ssl.create_default_context())

url = "https://api.dashdoc.eu/api/web/moderation/smiirl/"

while True:
    try:
        #  pings adafruit quotes
        print("Fetching text from %s" % url)
        #  gets the quote from adafruit quotes
        response = requests.get(url)
        print("-" * 40)
        #  prints the response to the REPL
        print("Text Response: ", response.text)
        print("-" * 40)
        resp_data = json.loads(response.text)
        response.close()
        #  delays for 1 minute
        print("The value is:", resp_data.get("number"))
        time.sleep(5)
    # pylint: disable=broad-except
    except Exception as e:
        print("Error:\n", str(e))
        print("Resetting microcontroller in 10 seconds")
        time.sleep(10)
        microcontroller.reset()
