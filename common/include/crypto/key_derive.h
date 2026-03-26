/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2025-2026 Shomy
 */

#ifndef KEY_DERIVE_H
#define KEY_DERIVE_H

#include <stdint.h>
#include <crypto/sbrom/sbrom.h>

typedef enum {
    RPMB_KEY = 0,
    FDE_KEY = 1,
    TEE_KEY = 2,
    AES_IMG_ENC = 3,
    AES_CUSTOM = 4,
    CUSTOM_0 = 5, // Moto
    CUSTOM_1 = 6,
    BASE_KEY = 7, // Not sure what this is
} KeyType;

typedef struct {
    uint32_t src_key;
    uint8_t* label;
    uint32_t label_sz;
    uint8_t* salt;
    uint32_t salt_sz;
    uint32_t policy; // 22
} key_attr_t;

uint32_t key_derive(KeyType key_type, uint8_t *out, uint32_t len);

#endif /* KEY_DERIVE_H */
