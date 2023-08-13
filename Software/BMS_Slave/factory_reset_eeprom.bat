@echo off
echo Write flash
cd avrdude-6.4-mingw32
avrdude -C avrdude.conf -c avrispmkII  -P usb -p attiny261 -U eeprom:w:../eeprom_fresh.hex:i -v
PAUSE