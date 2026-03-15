CROSS_COMPILE    ?= arm-none-eabi-
CROSS_COMPILE_64 ?= aarch64-none-linux-gnu-

CC_ARM      := $(CROSS_COMPILE)gcc
OBJCOPY_ARM := $(CROSS_COMPILE)objcopy
SIZE_ARM    := $(CROSS_COMPILE)size

CC_ARM64      := $(CROSS_COMPILE_64)gcc
OBJCOPY_ARM64 := $(CROSS_COMPILE_64)objcopy
SIZE_ARM64    := $(CROSS_COMPILE_64)size

FEATURES   := uart crypto rpmb mmc mmc_rpmb
COMMON_DIR := ../common

include $(COMMON_DIR)/common.mk

LIBSEJ_DIR := ../libsej
OUTPUT_DIR := ../payloads
BUILD_DIR  := build

DA_XML_SRCS := \
	src/main.c \
	src/commands.c \
	src/protocol_functions.c \
	src/xml.c

LIBSEJ_SRCS := $(LIBSEJ_DIR)/src/sej.c

ALL_SRCS := $(DA_XML_SRCS) $(LIBSEJ_SRCS) $(COMMON_SRCS)

CFLAGS := $(COMMON_CFLAGS) \
	-Iinclude \
	-Ilib \
	-I$(LIBSEJ_DIR)/include \
	$(COMMON_INCLUDES)

LDFLAGS := $(COMMON_LDFLAGS) -lgcc

CFLAGS_ARM   := $(CFLAGS) -DARM -marm -mcpu=cortex-a9 -mno-unaligned-access
CFLAGS_ARM64 := $(CFLAGS) -DAARCH64 -march=armv8-a -mgeneral-regs-only -mno-outline-atomics

LDFLAGS_ARM   := $(LDFLAGS) -Tsrc/arch/arm/generic.ld
LDFLAGS_ARM64 := $(LDFLAGS) -Tsrc/arch/arm64/generic.ld

arm_obj   = $(patsubst %.c,$(BUILD_DIR)/arm/%.o,$(subst ../,,$(1)))
arm64_obj = $(patsubst %.c,$(BUILD_DIR)/aarch64/%.o,$(subst ../,,$(1)))

ARM_OBJS   := $(call arm_obj,$(ALL_SRCS))
ARM64_OBJS := $(call arm64_obj,$(ALL_SRCS))

ARM_ELF   := $(BUILD_DIR)/arm/da_xml_arm.elf
ARM_BIN   := $(OUTPUT_DIR)/da_xml_arm.bin

ARM64_ELF := $(BUILD_DIR)/aarch64/da_xml_arm64.elf
ARM64_BIN := $(OUTPUT_DIR)/da_xml_arm64.bin

.PHONY: all arm aarch64 clean

all: arm aarch64

arm: $(ARM_BIN)
aarch64: $(ARM64_BIN)

$(ARM_BIN): $(ARM_ELF)
	@mkdir -p $(dir $@)
	$(OBJCOPY_ARM) -O binary $< $@
	$(SIZE_ARM) $<
	@echo "Built: $@"

$(ARM_ELF): $(ARM_OBJS)
	@mkdir -p $(dir $@)
	$(CC_ARM) $(CFLAGS_ARM) $^ -o $@ $(LDFLAGS_ARM)

$(BUILD_DIR)/arm/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC_ARM) $(CFLAGS_ARM) -c $< -o $@

$(BUILD_DIR)/arm/common/%.o: $(COMMON_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC_ARM) $(CFLAGS_ARM) -c $< -o $@

$(BUILD_DIR)/arm/libsej/%.o: $(LIBSEJ_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC_ARM) $(CFLAGS_ARM) -c $< -o $@

$(ARM64_BIN): $(ARM64_ELF)
	@mkdir -p $(dir $@)
	$(OBJCOPY_ARM64) -O binary $< $@
	$(SIZE_ARM64) $<
	@echo "Built: $@"

$(ARM64_ELF): $(ARM64_OBJS)
	@mkdir -p $(dir $@)
	$(CC_ARM64) $(CFLAGS_ARM64) $^ -o $@ $(LDFLAGS_ARM64)

$(BUILD_DIR)/aarch64/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC_ARM64) $(CFLAGS_ARM64) -c $< -o $@

$(BUILD_DIR)/aarch64/common/%.o: $(COMMON_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC_ARM64) $(CFLAGS_ARM64) -c $< -o $@

$(BUILD_DIR)/aarch64/libsej/%.o: $(LIBSEJ_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC_ARM64) $(CFLAGS_ARM64) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(ARM_BIN) $(ARM64_BIN)
