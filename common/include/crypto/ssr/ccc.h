/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#pragma once

#include <stdint.h>
#include <crypto/ssr/registers.h>

int ssr_ccc_sha256_init(ssr_sha256_ctx *ctx);
int ssr_ccc_sha256_process(ssr_sha256_ctx *ctx, uint8_t *in, size_t inlen);
int ssr_ccc_sha256_done(ssr_sha256_ctx *ctx, uint8_t *out);
int ssr_ccc_sha256(uint8_t *in, size_t inlen, uint8_t *out);
int ssr_ccc_sha384_init(ssr_sha384_ctx *ctx);
int ssr_ccc_sha384_process(ssr_sha384_ctx *ctx, uint8_t *in, size_t inlen);
int ssr_ccc_sha384_done(ssr_sha384_ctx *ctx, uint8_t *out);
int ssr_ccc_sha384(uint8_t *in, size_t inlen, uint8_t *out);
