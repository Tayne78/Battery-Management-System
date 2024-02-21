@echo off
echo Flash Slave with updated BMS firmware
cd avrdude-6.4-mingw32
avrdude -C avrdude.conf -c avrispmkII  -P usb -p attiny261 -U eeprom:r:../callibration_data.hex:i -v
timeout 2
avrdude -C avrdude.conf -c avrispmkII  -P usb -p attiny261 -U flash:w:../.pio/build/attiny261a/firmware.hex:i -v
timeout 2
avrdude -C avrdude.conf -c avrispmkII  -P usb -p attiny261 -U eeprom:w:../callibration_data.hex:i -v
PAUSE