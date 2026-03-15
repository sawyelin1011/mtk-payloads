/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2025 Shomy
 */

#ifndef KEY_DERIVE_H
#define KEY_DERIVE_H

#include <stdint.h>
#include <crypto/sbrom/sbrom.h>

typedef enum {
    RPMB_KEY = 0,
    FDE_KEY = 1,
    // Figure out what's this: "TROD\"MG[UCUK"
} KeyType;

uint32_t key_derive(KeyType key_type, uint8_t *out, uint32_t len);

#endif /* KEY_DERIVE_H */
