#!/bin/bash
esptool.py --port /dev/ttyUSB0 -b 115200 write_flash 0x00000 eagle.flash.bin 0x20000 eagle.irom0text.bin

