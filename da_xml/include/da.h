/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#ifndef DA_XML_DA_H
#define DA_XML_DA_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    STORAGE_UNKNOWN=0,
    STORAGE_EMMC=1,
    STORAGE_UFS=0x30,
} storage_type;

typedef struct {
    uint32_t sej_base;
    uint32_t tzcc_base;
    uint32_t da2_addr;
    uint32_t da2_size;
    storage_type storage;
    uint32_t usb_log;
} da_ctx_t;


typedef struct {
    uint64_t start;
    uint64_t length;
} address_range_t;

typedef struct {
    uint32_t start_sector;
    uint32_t sector_count;
} storage_range_t;

extern volatile da_ctx_t g_da_ctx;

#endif //DA_XML_DA_H
