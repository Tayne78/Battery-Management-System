
echo Flash Slave with updated BMS firmware

avrdude -c avrispmkII  -P usb -p attiny261 -U eeprom:r:callibration_data.hex:i -v
timeout 2
avrdude -c avrispmkII  -P usb -p attiny261 -U flash:w:BMS_Slave.hex:i -v
timeout 2
avrdude -c avrispmkII  -P usb -p attiny261 -U eeprom:w:callibration_data.hex:i -v
PAUSE
