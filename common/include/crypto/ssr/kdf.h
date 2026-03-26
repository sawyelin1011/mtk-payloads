/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#pragma once

#include <stdint.h>
#include <crypto/ssr/registers.h>

uint32_t ssr_kdf_kbkdf_cmac_counter(SsrKdfKeySel key_src, const uint8_t *sw_key, uint32_t key_alg, const uint8_t *in, uint32_t inlen, uint8_t *out, uint32_t out_len);
uint32_t ssr_kdf_hkdf_sha256(const uint8_t *ikm, uint32_t ikm_len, const uint8_t *info, uint32_t info_len, const uint8_t *salt, uint32_t salt_len, uint8_t *out,  uint32_t out_len);
