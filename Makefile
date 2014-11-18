TARGET=psxusb

MCU = at90usb1286      # Teensy++ 2.0

CFLAGS += -mmcu=$(MCU) -Os -ffunction-sections -Wall -W -std=gnu99
LDFLAGS += -Wl,-Map=$(TARGET).map,--cref -Wl,--relax -Wl,--gc-sections
OBJ=psxusb.o

.PHONY: all clean
all: $(TARGET).hex

clean:
	rm -f *.elf *.hex *.o

%.hex: %.elf
	avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

$(TARGET).elf: $(OBJ)
	avr-gcc $(CFLAGS) --output $@ $+

%.o: %.c
	avr-gcc $(CFLAGS) -c $<
