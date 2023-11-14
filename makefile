	# Makefile for programming AVR in Geany 
	
	##############	VARIABLES		###################
	
	### Commands ###
	COMPILE     			= "F:\AVRToolchain\avr8-gnu-toolchain-win32_x86_64\bin\avr-gcc.exe" -Wall -Os -mmcu=$(DEVICE)
	REMOVE					=  C:\MinGW\msys\1.0\bin\rm.exe
	AVRDUDE 				= "F:\AVRDude\avrdude.exe"
	AVR_OBJCOPY		= "F:\AVRToolchain\avr8-gnu-toolchain-win32_x86_64\bin\avr-objcopy.exe"
	AVR_SIZE				= "F:\AVRToolchain\avr8-gnu-toolchain-win32_x86_64\bin\avr-size.exe"
	
	### Flash data ###
	DEVICE     				= atmega328p
	# built in arduino USB-TTL
	PROGRAMMER 		= arduino	
	BAUD       				= 115200
	FLASHFILE 			= main.hex
	PORT 						= COM5
	#can see info about args in system("avrdude")
	AVRDUDE_FLAGS 	= -p $(DEVICE) -c $(PROGRAMMER) -P $(PORT) -b $(BAUD) -D
	
	### File names ###
	FILENAME   			= main
	
##############	BUILD		#######################
	
default: compile flash 

compile:
	$(COMPILE) -c "src\$(FILENAME).c" -o "build\$(FILENAME).o"
	$(COMPILE) -o "build\$(FILENAME).elf" "build\$(FILENAME).o"
	$(AVR_OBJCOPY) -j .text -j .data -O ihex "build\$(FILENAME).elf"  "build\$(FILENAME).hex" 
	$(AVR_SIZE) --format=avr --mcu=$(DEVICE) "build\$(FILENAME).elf"

flash: "build\$(FILENAME).hex"
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:w:$<

clean :
	$(REMOVE) "build\$(FILENAME).o"
	$(REMOVE) "build\$(FILENAME).elf"
	$(REMOVE) "build\$(FILENAME).hex"
