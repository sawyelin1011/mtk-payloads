/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#include <stdint.h>
#include <libc.h>
#include <crypto/tzcc.h>
#include <crypto/key_derive.h>
#include <debug.h>

#if defined(__aarch64__) && defined(crypto_ssr)

#include <crypto/ssr/ssr.h>

#endif

static const uint8_t label_rpmb[]    = { 'R','P','M','B',' ','K','E','Y' };
static const uint8_t salt_rpmb[]     = { 'S','A','S','I' };

// FDE
static const uint8_t label_sqnc[]    = { 'S','Q','N','C','!','L','F','Z' };
static const uint8_t salt_sqnc[]     = { 'T','B','T','J' };

// TEE
static const uint8_t label_trod[]    = { 'T','R','O','D','"','M','G','[' };
static const uint8_t salt_trod[]     = { 'U','C','U','K' };

// AES IMG ENC
static const uint8_t label_fw[]      = { 'F','I','R','M','W','A','R','E' };
static const uint8_t salt_fw[]       = { 'E','N','C','C' };

// Motorola RPMB
static const uint8_t label_custom[]  = { 'C','C','U','S','T','O','M','M' };
static const uint8_t salt_custom[]   = { 'M','O','T','O' };

// ??
static const uint8_t label_base[]    = { 'B','A','S','E','_','K','E','Y' };
static const uint8_t salt_base[]     = { '9','5','2','7' };

static const uint8_t label_unk0[]    = { 'C','B','T','F','Z',0xAB,'e',0x60};
static const uint8_t salt_unk0[]     = { '8','6','3','8'};

static const uint8_t label_unk1[]    = { 'A','@','R','D','^','J','D','X'};
static const uint8_t salt_unk1[]     = { '8','4','1','6'};

key_attr_t key_attr_table[] = {
    {
        .src_key  = 1,
        .label    = (uint8_t *)label_rpmb,
        .label_sz = sizeof(label_rpmb),
        .salt     = (uint8_t *)salt_rpmb,
        .salt_sz  = sizeof(salt_rpmb),
        .policy   = 22 // S-CHIP
    },
    {
        .src_key  = 1,
        .label    = (uint8_t *)label_sqnc,
        .label_sz = sizeof(label_sqnc),
        .salt     = (uint8_t *)salt_sqnc,
        .salt_sz  = sizeof(salt_sqnc),
        .policy   = 22
    },
    {
        .src_key  = 1,
        .label    = (uint8_t *)label_trod,
        .label_sz = sizeof(label_trod),
        .salt     = (uint8_t *)salt_trod,
        .salt_sz  = sizeof(salt_trod),
        .policy   = 22
    },
    {
        .src_key  = 2,
        .label    = (uint8_t *)label_fw,
        .label_sz = sizeof(label_fw),
        .salt     = (uint8_t *)salt_fw,
        .salt_sz  = sizeof(salt_fw),
        .policy   = 23 // TODO: Find out what this policy is
    },
    {
        .src_key  = 0,
        .label    = NULL,
        .label_sz = 0,
        .salt     = NULL,
        .salt_sz  = 0,
        .policy   = 0
    },
    {
        // https://github.com/bkerler/mtkclient/blob/main/mtkclient/Library/Hardware/hwcrypto_dxcc.py#L1120-L1121
        .src_key  = 1,
        .label    = (uint8_t *)label_custom,
        .label_sz = sizeof(label_custom),
        .salt     = (uint8_t *)salt_custom,
        .salt_sz  = sizeof(salt_custom),
        .policy   = 22
    },
    {
        .src_key  = 1,
        .label    = (uint8_t *)label_base,
        .label_sz = sizeof(label_base),
        .salt     = (uint8_t *)salt_base,
        .salt_sz  = sizeof(salt_base),
        .policy   = 22
    },
    {
        .src_key  = 1,
        .label    = (uint8_t *)label_unk0,
        .label_sz = sizeof(label_unk0),
        .salt     = (uint8_t *)salt_unk0,
        .salt_sz  = sizeof(salt_unk0),
        .policy   = 22
    },
    {
        .src_key  = 1,
        .label    = (uint8_t *)label_unk1,
        .label_sz = sizeof(label_unk1),
        .salt     = (uint8_t *)salt_unk1,
        .salt_sz  = sizeof(salt_unk1),
        .policy   = 22
    },
};

uint32_t key_derive(KeyType key_type, uint8_t* out, uint32_t len) {
    if (key_type >= sizeof(key_attr_table) / sizeof(key_attr_t)) {
        printf("%s: key_type=%d out of range (max=%zu)\n", __func__, key_type, sizeof(key_attr_table) / sizeof(key_attr_t));
        memset(out, 0, len);
        return 0;
    }

    key_attr_t key_attr = key_attr_table[key_type];

    uint8_t *label   = key_attr.label;
    uint8_t *context = key_attr.salt;

    if (label && context) {
        uint32_t total_size = key_attr.label_sz + key_attr.salt_sz;

        if (total_size > len) {
            printf("%s: total_size=%u exceeds out_len=%u\n", __func__, total_size, len);
            return -1;
        }

        printf("%s: key_type=%d label_sz=%u salt_sz=%u\n", __func__, key_type, key_attr.label_sz, key_attr.salt_sz);
        printf("%s: label='%.*s' context='%.*s'\n", __func__, key_attr.label_sz, label, key_attr.salt_sz, context);

        int status = tzcc_key_derive(label, key_attr.label_sz, context, key_attr.salt_sz, out, len);

#if defined(__aarch64__) && defined(crypto_ssr)
        if (status != 0)
            status = ssr_key_derive(label, key_attr.label_sz, context, key_attr.salt_sz, out, len);
#endif

        return status;
    }

    memset(out, 0, len);
    return -2;
}
