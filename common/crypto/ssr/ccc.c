/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <libc.h>
#include <crypto/ssr/top.h>
#include <crypto/ssr/registers.h>
#include <crypto/ssr/ccc.h>

int ssr_ccc_sha256_compress(ssr_sha256_ctx *ctx, const void *data, size_t len) {
    uintptr_t ccc   = ctx->dma_base_address;
    bool first = (ctx->reserved[0] == 0);

    flush_dcache_range((uintptr_t)data, len);

    uint64_t paddr = (uint64_t)(uintptr_t)vaddr_to_paddr((void *)data);
    uint64_t bit_len = (uint64_t)len * 8;

    bool high_paddr = (paddr >> 32) != 0;
    bool high_bitlen = (bit_len >> 32) != 0;

    uint32_t avail;
    do { avail = readl(ccc + 0x100); } while ((avail & 0xFF) < 6);

    uint32_t cmd0 = (first ? 0x30000000U : 0x31000000U)
                  | ((uint32_t)high_bitlen << 7)
                  | ((uint32_t)high_paddr << 8);

    uint32_t seq = ((uint32_t)readl(ccc + 0x2C) + 1) & 0xFFFF;
    if (!seq) seq = 1;

    writel_relaxed(cmd0, ccc + 0x104);
    writel_relaxed(seq | (first ? 0U : 0x20000000U), ccc + 0x104);
    writel_relaxed(high_paddr ? ((uint32_t)(paddr >> 16) & 0x000F0000U) : 0, ccc + 0x104);
    writel_relaxed((uint32_t)bit_len, ccc + 0x104);
    writel_relaxed((uint32_t)paddr, ccc + 0x104);

    int nwords = 5;
    if (high_bitlen) {
        writel_relaxed((uint32_t)(bit_len >> 32), ccc + 0x104);
        nwords = 6;
    }

    writel(avail - nwords, ccc + 0x100);

    if (first)
        ctx->reserved[0] = 1;

    if (ssr_poll_when(ccc + 0x2C, 0xFFFF, seq, SSR_TIMEOUT))
        return SSR_CCC_TIMEOUT;

    if ((int32_t)readl(ccc + 0x2C) < 0)
        return SSR_CCC_ERROR;

    return SSR_OK;
}

static int ssr_ccc_sha384_compress(ssr_sha384_ctx *ctx, const void *data, size_t len) {
    uintptr_t ccc = ctx->dma_base_address;
    bool first = (ctx->reserved[0] == 0);

    flush_dcache_range((uintptr_t)data, len);

    uint64_t paddr = (uint64_t)(uintptr_t)vaddr_to_paddr((void *)data);
    uint64_t bit_len = (uint64_t)len * 8;

    bool high_paddr = (paddr >> 32) != 0;
    bool high_bitlen = (len >> 29) != 0;

    uint32_t avail;
    do { avail = readl(ccc + 0x100); } while ((avail & 0xFF) < 6);

    uint32_t cmd0 = (high_bitlen ? 0x2000880U : 0x2000800U)
                  | ((uint32_t)high_paddr << 8)
                  | (first ? 0x30000000U : 0x31000000U);

    uint32_t seq = ((uint32_t)readl(ccc + 0x2C) + 1) & 0xFFFF;
    if (!seq) seq = 1;

    writel_relaxed(cmd0, ccc + 0x104);
    writel_relaxed(seq | (first ? 0U : 0x20000000U), ccc + 0x104);
    writel_relaxed(high_paddr ? ((uint32_t)(paddr >> 16) & 0x000F0000U) : 0, ccc + 0x104);
    writel_relaxed((uint32_t)bit_len, ccc + 0x104);
    writel_relaxed((uint32_t)paddr,   ccc + 0x104);

    int nwords = 5;
    if (high_bitlen) {
        writel_relaxed((uint32_t)(bit_len >> 32), ccc + 0x104);
        nwords = 6;
    }

    writel(avail - nwords, ccc + 0x100);

    if (first)
        ctx->reserved[0] = 1;

    if (ssr_poll_when(ccc + 0x2C, 0xFFFF, seq, SSR_TIMEOUT))
        return SSR_CCC_TIMEOUT;

    if ((int32_t)readl(ccc + 0x2C) < 0)
        return SSR_CCC_ERROR;

    return SSR_OK;
}

void ssr_ccc_sha_read_output(uintptr_t ccc, uint32_t *dst, int n) {
    for (int k = 0; k < n; k++)
        dst[k] = swab32(readl(ccc + 0x034 + (n - 1 - k) * 4));
}


int ssr_ccc_sha256_init(ssr_sha256_ctx *ctx) {
    if (!ctx)
        return SSR_KDF_ERR_NULL_PTR;
    memset(ctx, 0, sizeof(*ctx));
    ctx->dma_base_address = SSR_CCC_BASE;
    return SSR_OK;
}


int ssr_ccc_sha256_process(ssr_sha256_ctx *ctx, uint8_t *in, size_t inlen) {
    if (!ctx || !in)
        return SSR_KDF_ERR_NULL_PTR;

    size_t pos = (size_t)(ctx->data_len & 0x3F);
    ctx->data_len += inlen;

    if (pos != 0) {
        size_t fill = 64 - pos;
        if (inlen < fill) {
            memcpy(&ctx->buf.data_u8[pos], in, inlen);
            return SSR_OK;
        }
        memcpy(&ctx->buf.data_u8[pos], in, fill);
        int status = ssr_ccc_sha256_compress(ctx, ctx->buf.data_u8, 64);
        if (status) return status;
        in += fill;
        inlen -= fill;
    }

    if (inlen >= 64) {
        size_t full = inlen & ~(size_t)0x3F;
        int status = ssr_ccc_sha256_compress(ctx, in, full);
        if (status) return status;
        in += full;
        inlen -= full;
    }

    if (inlen)
        memcpy(&ctx->buf.data_u8[0], in, inlen);

    return SSR_OK;
}

int ssr_ccc_sha256_done(ssr_sha256_ctx *ctx, uint8_t *out) {
    if (!ctx || !out)
        return SSR_KDF_ERR_NULL_PTR;

    size_t pos = (size_t)(ctx->data_len & 0x3F);
    size_t pad_len = (pos < 56) ? 64 : 128;
    size_t len_off = pad_len - 8;
    uint64_t total = ctx->data_len;

    uint8_t pad[128];
    memset(pad, 0, pad_len);
    memcpy(pad, &ctx->buf.data_u8[0], pos);
    pad[pos] = 0x80;

    uint64_t bits = total * 8;
    for (int i = 7; i >= 0; i--) {
        pad[len_off + (size_t)i] = (uint8_t)(bits & 0xFF);
        bits >>= 8;
    }

    int status = ssr_ccc_sha256_compress(ctx, pad, pad_len);
    if (status) return status;

    ssr_ccc_sha_read_output(ctx->dma_base_address, ctx->hash_val, 8);
    memcpy(out, ctx->hash_val, 32);
    return SSR_OK;
}

int ssr_ccc_sha256(uint8_t *in, size_t inlen, uint8_t *out) {
    if (!in || !out)
        return SSR_KDF_ERR_NULL_PTR;

    uintptr_t ccc = SSR_CCC_BASE;

    flush_dcache_range((uintptr_t)in, inlen);
    uint64_t paddr = (uint64_t)(uintptr_t)vaddr_to_paddr(in);
    uint64_t bit_len = (uint64_t)inlen * 8;

    bool high_paddr = (paddr >> 32) != 0;
    bool high_bitlen = (bit_len >> 32) != 0;

    uint8_t type = CCC_SHA256_TYPE;

    uint32_t avail;
    do { avail = readl(ccc + 0x100); } while ((avail & 0xF8) == 0);

    uint32_t cmd0 = 0x30000000U
                  | ((uint32_t)(type & 3) << 25)
                  | ((uint32_t)(type >> 2 & 1) << 11)
                  | ((uint32_t)high_bitlen <<  7)
                  | ((uint32_t)high_paddr <<  8);

    uint32_t seq = ((uint32_t)readl(ccc + 0x2C) + 1) & 0xFFFF;
    if (!seq) seq = 1;

    writel_relaxed(cmd0, ccc + 0x104);
    writel_relaxed(seq | 0x80000000U, ccc + 0x104);
    writel_relaxed(high_paddr ? ((uint32_t)(paddr >> 16) & 0x000F0000U) : 0, ccc + 0x104);
    writel_relaxed((uint32_t)bit_len, ccc + 0x104);
    writel_relaxed((uint32_t)paddr, ccc + 0x104);

    int nwords = 5;
    if (high_bitlen) {
        writel_relaxed((uint32_t)(bit_len >> 32), ccc + 0x104);
        nwords = 6;
    }

    writel(avail - nwords, ccc + 0x100);

    if (ssr_poll_until(ccc + 0x2C, seq, 0xFFFF, SSR_TIMEOUT))
        return SSR_CCC_TIMEOUT;

    if ((int32_t)readl(ccc + 0x2C) >= 0)
        return SSR_CCC_ERROR;

    uint32_t buf[8];
    ssr_ccc_sha_read_output(ccc, buf, 8);
    memcpy(out, buf, 32);
    return SSR_OK;
}

int ssr_ccc_sha384_init(ssr_sha384_ctx *ctx) {
    if (!ctx)
        return SSR_KDF_ERR_NULL_PTR;
    memset(ctx, 0, sizeof(*ctx));
    ctx->dma_base_address = SSR_CCC_BASE;
    return SSR_OK;
}

int ssr_ccc_sha384_process(ssr_sha384_ctx *ctx, uint8_t *in, size_t inlen) {
    if (!ctx || !in)
        return SSR_KDF_ERR_NULL_PTR;

    size_t pos = (size_t)(ctx->data_len & 0x7F);
    ctx->data_len += inlen;

    if (pos != 0) {
        size_t fill = 128 - pos;
        if (inlen < fill) {
            memcpy(&ctx->buf.data_u8[pos], in, inlen);
            return SSR_OK;
        }
        memcpy(&ctx->buf.data_u8[pos], in, fill);
        int status = ssr_ccc_sha384_compress(ctx, ctx->buf.data_u8, 128);
        if (status) return status;
        in += fill;
        inlen -= fill;
    }

    if (inlen >= 128) {
        size_t full = inlen & ~(size_t)0x7F;
        int status = ssr_ccc_sha384_compress(ctx, in, full);
        if (status) return status;
        in += full;
        inlen -= full;
    }

    if (inlen)
        memcpy(&ctx->buf.data_u8[0], in, inlen);

    return SSR_OK;
}


int ssr_ccc_sha384_done(ssr_sha384_ctx *ctx, uint8_t *out) {
    if (!ctx || !out)
        return (int)SSR_KDF_ERR_NULL_PTR;

    size_t pos = (size_t)(ctx->data_len & 0x7F);
    size_t pad_len = (pos < 112) ? 128 : 256;
    size_t len_off = pad_len - 8;
    uint64_t total = ctx->data_len;

    uint8_t pad[256];
    memset(pad, 0, pad_len);
    memcpy(pad, &ctx->buf.data_u8[0], pos);
    pad[pos] = 0x80;

    uint64_t bits = total * 8;
    for (int i = 7; i >= 0; i--) {
        pad[len_off + (size_t)i] = (uint8_t)(bits & 0xFF);
        bits >>= 8;
    }

    int status = ssr_ccc_sha384_compress(ctx, pad, pad_len);
    if (status) return status;

    ssr_ccc_sha_read_output(ctx->dma_base_address, ctx->hash_val, 12);
    memcpy(out, ctx->hash_val, 48);
    return SSR_OK;
}


int ssr_ccc_sha384(uint8_t *in, size_t inlen, uint8_t *out) {
    if (!in || !out)
        return SSR_KDF_ERR_NULL_PTR;

    uintptr_t ccc = SSR_CCC_BASE;

    flush_dcache_range((uintptr_t)in, inlen);
    uint64_t paddr = (uint64_t)(uintptr_t)vaddr_to_paddr(in);
    uint64_t bit_len = (uint64_t)inlen * 8;

    bool high_paddr = (paddr >> 32) != 0;
    bool high_bitlen = (inlen >> 29) != 0;

    uint8_t type = CCC_SHA384_TYPE;

    uint32_t avail;
    do { avail = readl(ccc + 0x100); } while ((avail & 0xF8) == 0);

    uint32_t cmd0 = 0x30000000U
                  | ((uint32_t)(type & 3)  << 25)
                  | ((uint32_t)(type >> 2 & 1) << 11)
                  | ((uint32_t)high_bitlen <<  7)
                  | ((uint32_t)high_paddr <<  8);

    uint32_t seq = ((uint32_t)readl(ccc + 0x2C) + 1) & 0xFFFF;
    if (!seq) seq = 1;

    writel_relaxed(cmd0, ccc + 0x104);
    writel_relaxed(seq | 0x80000000U, ccc + 0x104);
    writel_relaxed(high_paddr ? ((uint32_t)(paddr >> 16) & 0x000F0000U) : 0, ccc + 0x104);
    writel_relaxed((uint32_t)bit_len, ccc + 0x104);
    writel_relaxed((uint32_t)paddr, ccc + 0x104);

    int nwords = 5;
    if (high_bitlen) {
        writel_relaxed((uint32_t)(bit_len >> 32), ccc + 0x104);
        nwords = 6;
    }

    writel(avail - nwords, ccc + 0x100);

    if (ssr_poll_until(ccc + 0x2C, seq, 0xFFFF, SSR_TIMEOUT))
        return SSR_CCC_TIMEOUT;

    if ((int32_t)readl(ccc + 0x2C) >= 0)
        return SSR_CCC_ERROR;

    uint32_t buf[12];
    ssr_ccc_sha_read_output(ccc, buf, 12);
    memcpy(out, buf, 48);
    return SSR_OK;
}
