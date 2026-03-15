# Usage in case I forget:
#
#   COMMON_DIR := ../common
#   FEATURES   := uart
#   include $(COMMON_DIR)/common.mk
#
# Available features:
#   uart    - UART driver
#   crypto  - TZCC, SBROM, SHA256, HMAC-SHA256...
#   rpmb    - RPMB

COMMON_DIR ?= ../common

COMMON_SRCS := \
	$(COMMON_DIR)/debug.c \
	$(COMMON_DIR)/libc.c

COMMON_INCLUDES := \
	-I$(COMMON_DIR)/include


ifneq ($(filter uart,$(FEATURES)),)
COMMON_SRCS += $(COMMON_DIR)/drivers/uart/uart.c
endif

ifneq ($(filter crypto,$(FEATURES)),)
COMMON_SRCS += $(COMMON_DIR)/crypto/sbrom/sbrom.c \
            $(COMMON_DIR)/crypto/tzcc.c \
            $(COMMON_DIR)/crypto/hmac-sha256.c \
            $(COMMON_DIR)/crypto/sha256.c
endif

ifneq ($(filter mmc,$(FEATURES)),)
COMMON_SRCS += \
	$(COMMON_DIR)/storage/mmc/mmc.c
endif

ifneq ($(filter rpmb,$(FEATURES)),)
COMMON_SRCS += \
	$(COMMON_DIR)/security/rpmb.c
endif

ifneq ($(filter mmc_rpmb,$(FEATURES)),)
ifeq ($(filter mmc,$(FEATURES)),)
$(error mmc_rpmb requires mmc)
endif
ifeq ($(filter rpmb,$(FEATURES)),)
$(error mmc_rpmb requires rpmb)
endif

COMMON_SRCS += \
	$(COMMON_DIR)/storage/mmc/rpmb_mmc.c
endif


# Flags

COMMON_CFLAGS ?= \
	-std=gnu99 -Os -Wall -Wextra \
	-fno-strict-aliasing \
	-fno-builtin \
	-fno-omit-frame-pointer \
	-fno-pic -fno-pie

COMMON_LDFLAGS ?= \
	-nodefaultlibs -nostdlib \
	-Wl,--build-id=none,--no-warn-rwx-segments


common_objects = $(patsubst %.c,$(1)/%.o,$(subst ../,,$(COMMON_SRCS)))

define common_build_rule
$(1)/common/%.o: $(COMMON_DIR)/%.c
	@mkdir -p $$(dir $$@)
	$(2) $(3) $(COMMON_INCLUDES) -c $$< -o $$@
endef
