# Имя программы и собранного бинарника
TARGET = pi_controller

# файлы программы
C_SOURCES = \
src/pi_controller.c \
src/simplePID.c \
src/simplePID_a.s


# название контроллера и частота для компилятора
MCU = atmega16
F_CPU = 8000000

# параметры для AVRDUDE
DUDE_MCU = m16
PORT = COM3
PORTSPEED = 115200

# DEFINы
DEFINES = \
-D__AVR_ATmega16__ \
-DF_CPU=$(F_CPU)UL

# путь к каталогу с GCC
AVRCCDIR = c:/avr-gcc/bin/

#само название компилятора, мало ли, вдруг оно когда-нибудь поменяется
CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size
OBJDUMP = avr-objdump
NM = avr-nm
AVRDUDE = avrdude

# каталог в который будет осуществляться сборка, что бы не засерать остальные каталоги
BUILD_DIR = build

#флаги для компилятора 
OPT = -Os
#C_FLAGS = -mmcu=$(MCU) $(OPT) -Wall

CFLAGS = -mmcu=$(MCU)
CFLAGS += $(OPT)
CFLAGS += -std=gnu99
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
#CFLAGS += -ffunction-sections -fdata-sections
# Флаги `-ffunction-sections` и `-fdata-sections` указывают компилятору создать раздел 
# для каждой функции и переменной, что позволит исключить их на индивидуальной основе, 
# а не только на уровне файла
CFLAGS += -Wall
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wa,-adhlns=$(BUILD_DIR)/$(@F).lss
# Не работает без libiconv на windows, но должно работать на linux
#CFLAGS += -finput-charset=UTF-8 -fexec-charset=cp1251
#CFLAGS += -Wa,-adhlns=$(BUILD_DIR)/$(TARGET).lss

LDFLAGS = 
#LDFLAGS = -Wl,--gc-sections
#LDFLAGS += -Wl,--print-gc-sections
# Флаг `--gc-sections` указывает компоновщику опускать разделы, на которые нет ссылок



# пути к заголовочным файлам
C_INCLUDES =  \
-Ic:/avr-gcc/avr/include \
-Iinc

# служебные переменные
OBJ_FILES = $(C_SOURCES:.c=.o)
ASM_FILES = $(C_SOURCES:.c=.s)
OUT_OBJ = $(addprefix $(BUILD_DIR)/, $(notdir $(OBJ_FILES)))

## Intel Hex file production flags
HEX_FLASH_FLAGS = -j .text -j .data

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

MSG_LINKING = Linking:
MSG_COMPILING = Compiling:
MSG_FLASH = Preparing HEX file:
MSG_GEN_ASM = Generate assembler listing:

# правила для сборки

all: $(TARGET).hex $(TARGET).eep $(TARGET).lss

$(TARGET).hex: $(TARGET).elf
	@echo $(MSG_FLASH) $@
	$(AVRCCDIR)$(OBJCOPY) $(HEX_FLASH_FLAGS) -O ihex $(BUILD_DIR)/$< $(BUILD_DIR)/$@

$(TARGET).elf: $(OBJ_FILES) $(ASM_FILES)
	@echo
	@echo $(MSG_LINKING) $@
	copy src\simplePID_a.s build\simplePID_a.s
	$(AVRCCDIR)$(CC) -mmcu=$(MCU) $(LDFLAGS) $(OUT_OBJ) -o $(BUILD_DIR)/$@

$(TARGET).eep:  $(TARGET).elf
	-$(AVRCCDIR)$(OBJCOPY) $(HEX_EEPROM_FLAGS) -O ihex $(BUILD_DIR)/$< $(BUILD_DIR)/$@ || exit 0

$(TARGET).lss: $(TARGET).elf
	$(AVRCCDIR)$(OBJDUMP) -h -S $(BUILD_DIR)/$< > $(BUILD_DIR)/$@

%.o: %.c
	@echo $(MSG_COMPILING) $<
	$(AVRCCDIR)$(CC) -c $(CFLAGS) $(DEFINES) $(C_INCLUDES) $< -o $(BUILD_DIR)/$(@F)

%.s: %.c
	@echo $(MSG_GEN_ASM) $<
	$(AVRCCDIR)$(CC) -S -g3 $(CFLAGS) $(DEFINES) $(C_INCLUDES) $< -o $(BUILD_DIR)/$(@F)

clean:
	rmdir /S /Q $(BUILD_DIR)

prog: $(TARGET).hex
	$(AVRCCDIR)$(AVRDUDE) -p $(DUDE_MCU) -c wiring -P $(PORT) -b $(PORTSPEED) -U flash:w:$(BUILD_DIR)/$(TARGET).hex

isp: $(TARGET).hex
	$(AVRCCDIR)$(AVRDUDE) -p $(DUDE_MCU) -c usbasp -B8 -U flash:w:$(BUILD_DIR)/$(TARGET).hex

write_bootloader:
	$(AVRCCDIR)$(AVRDUDE) -p $(DUDE_MCU) -c usbasp -B8 -U flash:w:optiboot_flash_atmega328p_UART0_19200_8000000L_B5.hex

read_eeprom:
	$(AVRCCDIR)$(AVRDUDE) -p $(DUDE_MCU) -c arduino -P $(PORT) -b $(PORTSPEED) -U eeprom:r:eeprom.hex:i

write_eeprom: $(TARGET).eep
	$(AVRCCDIR)$(AVRDUDE) -p $(DUDE_MCU) -c arduino -P $(PORT) -b $(PORTSPEED) -U eeprom:w:$(BUILD_DIR)/$(TARGET).eep

size:
	$(AVRCCDIR)$(SIZE) $(BUILD_DIR)/$(TARGET).elf

analyze:
	$(AVRCCDIR)$(NM) -S --size-sort -t decimal $(BUILD_DIR)/$(TARGET).elf

## Other dependencies
-include $(shell mkdir $(BUILD_DIR) 2>NUL)
