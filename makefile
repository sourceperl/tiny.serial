CC=avr-gcc
CFLAGS=-Wall -Os -DF_CPU=$(F_CPU) -mmcu=$(MCU)
MCU=attiny85
F_CPU=8000000UL
PROJECT=serial

AVRSIZE=avr-size
OBJDUMP=avr-objdump

OBJCOPY=avr-objcopy
BIN_FORMAT=ihex

PORT=/dev/ttyACM0
BAUD=19200
PROTOCOL=avrisp
PART=$(MCU)
AVRDUDE=avrdude -F -V

RM=rm -f

.PHONY: all
all: $(PROJECT).hex

$(PROJECT).hex: $(PROJECT).elf

$(PROJECT).elf: $(PROJECT).s

$(PROJECT).s: $(PROJECT).c

.PHONY: clean
clean:
	$(RM) $(PROJECT).elf $(PROJECT).hex $(PROJECT).s $(PROJECT).disasm

.PHONY: upload
upload: $(PROJECT).hex
	$(AVRDUDE) -c $(PROTOCOL) -p $(PART) -P $(PORT) -b $(BAUD) -U flash:w:$<

size:   $(PROJECT).elf
	$(AVRSIZE) $<

disasm:	$(PROJECT).elf
	$(OBJDUMP) -S $< > $(PROJECT).disasm


%.elf: %.s ; $(CC) $(CFLAGS) -s -o $@ $<

%.s: %.c ; $(CC) $(CFLAGS) -S -o $@ $<

%.hex: %.elf ; $(OBJCOPY) -O $(BIN_FORMAT) -R .eeprom $< $@
