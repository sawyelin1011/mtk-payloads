CROSS_COMPILE ?= arm-none-eabi-

CC      := $(CROSS_COMPILE)gcc
OBJCOPY := $(CROSS_COMPILE)objcopy
SIZE    := $(CROSS_COMPILE)size

FEATURES   := uart crypto rpmb mmc mmc_rpmb
COMMON_DIR := ../common

include $(COMMON_DIR)/common.mk

LIBSEJ_DIR := ../libsej
OUTPUT_DIR := ../payloads
BUILD_DIR  := build

DA_X_SRCS := $(wildcard src/*.c)
ASM_SRCS  := src/start.S


LIBSEJ_SRCS := $(LIBSEJ_DIR)/src/sej.c

ALL_SRCS := $(DA_X_SRCS) $(LIBSEJ_SRCS) $(EXTRA_COMMON_SRCS) $(COMMON_SRCS)

CFLAGS := $(COMMON_CFLAGS) \
	-mthumb -mcpu=cortex-a9 \
	-fno-stack-protector \
	-fPIE \
	-DV5 \
	-Iinclude \
	-I$(LIBSEJ_DIR)/include \
	$(COMMON_INCLUDES)

ASMFLAGS := -Iinclude \
	$(COMMON_INCLUDES)

LDFLAGS := $(COMMON_LDFLAGS) \
	-Tsrc/generic.ld -lgcc

c_obj   = $(patsubst %.c,$(BUILD_DIR)/%.o,$(subst ../,,$(1)))
asm_obj = $(patsubst %.S,$(BUILD_DIR)/%.o,$(1))

C_OBJS   := $(call c_obj,$(ALL_SRCS))
ASM_OBJS := $(call asm_obj,$(ASM_SRCS))
ALL_OBJS := $(C_OBJS) $(ASM_OBJS)

TARGET_ELF := $(BUILD_DIR)/da_x.elf
TARGET_BIN := $(OUTPUT_DIR)/da_x.bin

.PHONY: all clean

all: $(TARGET_BIN)

$(TARGET_BIN): $(TARGET_ELF)
	@mkdir -p $(dir $@)
	$(OBJCOPY) -O binary $< $@
	@echo "Built: $@"

$(TARGET_ELF): $(ALL_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	$(SIZE) $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $(ASMFLAGS) -c $< -o $@

$(BUILD_DIR)/common/%.o: $(COMMON_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/libsej/%.o: $(LIBSEJ_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET_BIN)
