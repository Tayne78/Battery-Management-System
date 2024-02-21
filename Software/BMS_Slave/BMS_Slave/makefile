# The MCU model, see https://www.nongnu.org/avr-libc/user-manual/using_tools.html for list
MCU=attiny261a
# Clock speed used (after divider), mostly for delay functions
CLK=2000000
# Name of project output file
PROJ=BMS_Slave
# List of source files
SOURCES=src/main.c\
../Modules/Manchester/manch_m.c\
../Modules/Status/status.c\
../Modules/Timing/timer.c\
../Modules/ADC/ADC.c\
##../Modules/One_Wire_COMM/communication.c

# List of object files generated from source files
OBJ=$(SOURCES:.c=.o)

 
all: $(PROJ).hex
 
# Generated object files
.c.o:
	avr-gcc -w -Os -DF_CPU=$(CLK) -Wall -mmcu=$(MCU) -I../Modules/ADC/ -I../Modules/One_Wire_COMM/ -I../Modules/Timing/ -I../Modules/Manchester/ -I../Modules/Status/ -c -gstabs -Wa,-ahlmsd=$*.lst -o $@ $<
 
# Generate executable
$(PROJ).elf: $(OBJ)
	avr-gcc -w -mmcu=$(MCU) -Wl,-Map=$(PROJ).map $(OBJ) -o $(PROJ).elf
 
# Create Intel hex format for programming
$(PROJ).hex: $(PROJ).elf
	avr-objcopy -O ihex -R .eeprom $(PROJ).elf $(PROJ).hex
# print memory-usage
	avr-size -C --mcu=$(MCU) $(PROJ).elf

# Befehl zum Löschen aller erzeugten Dateien
clean:
	del /Q *.hex *.elf *.o
	del $(OBJ)

install:

	avrdude -c avrispmkII  -P usb -p ATtiny261 -U flash:w:BMS_Slave.hex:i -v
	
# update: reads eeprom, flasches the code, write old eeprom-data
update:
	avrdude -c avrispmkII  -P usb -p attiny261 -U eeprom:r:callibration_data.hex:i -v
	avrdude -c avrispmkII  -P usb -p attiny261 -U flash:w:BMS_Slave.hex:i -v
	avrdude -c avrispmkII  -P usb -p attiny261 -U eeprom:w:callibration_data.hex:i -v