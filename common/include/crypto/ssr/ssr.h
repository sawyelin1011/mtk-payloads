/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#pragma once

/* SSR (Scalable Security Root) */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <crypto/ssr/registers.h>

int ssr_hw_init(void);
uint64_t ssr_lcs_get(void);
void set_ssr_base(uintptr_t base_addr);
uintptr_t get_ssr_base(void);

// KDF
int ssr_key_derive(const uint8_t *label, uint32_t label_len, const uint8_t *context, uint32_t context_len, uint8_t *out, uint32_t out_len);

// SHA2
int ssr_sha256(uint8_t* in, size_t inlen, uint8_t* out);
int ssr_sha256_init(ssr_sha256_ctx* ctx);
int ssr_sha256_process(ssr_sha256_ctx* ctx, uint8_t* in, size_t inlen);
int ssr_sha256_done(ssr_sha256_ctx *ctx, uint8_t* out);

// SHA3
int ssr_sha384(uint8_t* in, size_t inlen, uint8_t* out);
int ssr_sha384_init(ssr_sha384_ctx* ctx);
int ssr_sha384_process(ssr_sha384_ctx* ctx, uint8_t* in, size_t inlen);
int ssr_sha384_done(ssr_sha384_ctx *ctx, uint8_t* out);

// RSA
int ssr_rsa_2048(uint32_t* sign, uint8_t* exp, uint32_t* modulus, uint32_t* result);
int ssr_rsa_3072(uint32_t* sign, uint8_t* exp, uint32_t* modulus, uint32_t* result);

// Ellpictic
int ssr_ecdh_gen_key_p256(uint8_t* priv_key, uint8_t* pub_x, uint8_t* pub_y);
int ssr_ecdh_gen_key_p384(uint8_t* priv_key, uint8_t* pub_x, uint8_t* pub_y);
int ssr_ecdsa_sign_p256(uint8_t* priv_key, uint8_t* ephem_k, uint8_t* hash, uint8_t* sign_r, uint8_t* sign_s);
int ssr_ecdsa_sign_p384(uint8_t* priv_key, uint8_t* ephem_k, uint8_t* hash, uint8_t* sign_r, uint8_t* sign_s);
int ssr_ecdsa_vfy_p256(uint8_t* pub_x, uint8_t* pub_y, uint8_t* hash, uint8_t* sig_r, uint8_t* sig_s, uint8_t* sig_r_prime);
int ssr_ecdsa_vfy_p384(uint8_t* pub_x, uint8_t* pub_y, uint8_t* hash, uint8_t* sig_r, uint8_t* sig_s, uint8_t* sig_r_prime);
