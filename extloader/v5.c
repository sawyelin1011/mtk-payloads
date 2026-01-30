/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2025 Shomy
 */

#include <stdint.h>

typedef struct
{
    int (*read)(uint8_t* buffer, uint32_t* length);
    int (*write)(uint8_t* buffer, uint32_t length);
    int (*log_to_pc)(const uint8_t* buffer, uint32_t length);
    int (*log_to_uart)(const uint8_t* buffer, uint32_t length);
} com_channel_struct;

__attribute__((section(".text.start"))) int cmd_boot_to(com_channel_struct* channel) {
    int status = 0;
    uint8_t hdr[16];
    uint32_t len = 16;
    channel->read(hdr, &len);

    uint8_t* target = (uint8_t*)(uintptr_t)(*(uint32_t*)hdr);
    uint32_t to_read = *(uint32_t*)(hdr + 8);
    channel->read(target, &to_read);
    channel->write((uint8_t*)&status, 4);

    void (*entry)(void *) = (void (*)(void *))((uintptr_t)target | 1);

    entry(&status);

    return status;
}
