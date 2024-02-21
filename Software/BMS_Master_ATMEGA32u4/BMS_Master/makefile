# The MCU model, see https://www.nongnu.org/avr-libc/user-manual/using_tools.html for list
MCU=atmega32u4
# Clock speed used (after divider), mostly for delay functions
CLK=2000000
# Name of project output file
PROJ=BMS_Master
# List of source files
SOURCES=src/main.c\
../Modules/Timing/timer.c\
../Modules/Manchester/manch_m.c\
../Modules/Status/status.c\
../Modules/rs232/rs232.c
#../Modules/One_Wire_COMM/communication.c

 
# List of object files generated from source files
OBJ=$(SOURCES:.c=.o)

 
all: $(PROJ).hex
 
# Generated object files
.c.o:
	avr-gcc -w -Os -DF_CPU=$(CLK) -mmcu=$(MCU) -I../Modules/ADC/ -I../Modules/One_Wire_COMM/ -I../Modules/Timing/ -I../Modules/Manchester/ -I../Modules/Status/ -I../Modules/rs232 -c -gstabs -Wa,-ahlmsd=$*.lst -o $@ $<
 
# Generate executable
$(PROJ).elf: $(OBJ)
	avr-gcc -w -mmcu=$(MCU) -Wl,-Map=$(PROJ).map $(OBJ) -o $(PROJ).elf
 
# Create Intel hex format for programming
$(PROJ).hex: $(PROJ).elf
	avr-objcopy -O ihex -R .eeprom $(PROJ).elf $(PROJ).hex
 
clean:
	rm -f *.hex *.elf *.o
	rm $(OBJ)
 
# Install flashes the code
install:
	dfu-programmer atmega32u4 erase
	dfu-programmer atmega32u4 flash $(PROJ).hex
	dfu-programmer atmega32u4 start
	
# info, etc:
info:
	avr-nm -S BMS_Master.elf
	avr-objdump -D -m avr2 BMS_Master.hex  > BMS_Master.dump
