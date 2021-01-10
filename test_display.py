#!/usr/bin/env python
# coding=utf-8

# Bibliotheken importieren

#LUMA
from luma.core.render import canvas
from luma.core.interface.serial import spi
from luma.core.render import canvas
from luma.oled.device import sh1106


from smbus import SMBus
from lib_oled96.lib_oled96 import ssd1306



def init_spi_display():
    serial = spi(port=0, device=1, gpio_DC=27, gpio_RST=24)
    device = sh1106(serial, rotate=0)

    with canvas(device) as draw:
        draw.rectangle(device.bounding_box, outline="white", fill="black")
        draw.text((30, 40), "Hello World", fill="white")




def init i2c_display():
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


if __name__ == "__main__":
    init_i2c_display()
    init_spi_display()
