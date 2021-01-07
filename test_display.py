#!/usr/bin/env python
# coding=utf-8

# Bibliotheken importieren

#LUMA
from luma.core.render import canvas
from luma.core.interface.serial import spi
from luma.core.render import canvas
from luma.oled.device import sh1106

#from .lib_oled96 import ssd1306
from smbus import SMBus
from lib_oled96.lib_oled96 import ssd1306

# Display einrichten
i2cbus = SMBus(1)            # 0 = Raspberry Pi 1, 1 = Raspberry Pi > 1
oled = ssd1306(i2cbus)

# Ein paar Abkürzungen, um den Code zu entschlacken
draw = oled.canvas

# Display zum Start löschen
oled.cls()
oled.display()

# Hallo Welt
draw.text((1, 16), "TrainControll 2021", fill=1)
draw.text((1, 40), "", fill=1)

# Ausgaben auf Display schreiben
oled.display()