CROSS_COMPILE ?= arm-none-eabi-

CC      := $(CROSS_COMPILE)gcc
LD      := $(CROSS_COMPILE)gcc
OBJCOPY := $(CROSS_COMPILE)objcopy

BUILD_DIR := build
TARGET    := build/extloader
OUTPUT := ../payloads/extloader_v5

SRC := v5.c
OBJ := $(BUILD_DIR)/v5.o

CFLAGS := -std=gnu99 -Os -mthumb -mcpu=cortex-a9 -fno-builtin-printf -fno-strict-aliasing
CFLAGS += -fno-builtin-memcpy -mno-unaligned-access -DPRINTF_DISABLE_SUPPORT_FLOAT=1 -fPIE
LDFLAGS := -T linker.x -nodefaultlibs -nostdlib -Wl,--no-warn-rwx-segments

ifeq ($(DEBUG),1)
CFLAGS += -DDEBUG
endif

all: $(TARGET).bin

$(TARGET).bin: $(OBJ)
	$(LD) $(LDFLAGS) -o $(TARGET).elf $(OBJ)
	$(OBJCOPY) -O binary $(TARGET).elf $(OUTPUT).bin

$(OBJ): $(SRC)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
