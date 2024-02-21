@echo off
echo Read flash
cd avrdude-6.4-mingw32
avrdude -C avrdude.conf -c avrispmkII  -P usb -p attiny261 -U eeprom:r:../eeprom.hex:i -v
PAUSE