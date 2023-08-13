@echo off
echo Disable clock divider in EEPROM
cd avrdude-6.4-mingw32
avrdude -C avrdude.conf -c avrispmkII  -P usb -p attiny261 -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m -v
PAUSE