/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy, R0rt1z2
 */

#pragma once

#include <stdint.h>

struct com_channel_struct {
    int (*read)(uint8_t *buffer, uint32_t *length);
    int (*write)(uint8_t *buffer, uint32_t length);
    int (*log_to_pc)(const uint8_t *buffer, uint32_t length);
    int (*log_to_uart)(const uint8_t *buffer, uint32_t length);
};

typedef int (*HHANDLE)(struct com_channel_struct *channel, const char *xml);

int cmd_boot_to(struct com_channel_struct *channel, const char *xml);
int cmd_patch_mem(struct com_channel_struct *channel, const char *xml);
int cmd_call_function(struct com_channel_struct *channel, const char *xml);
