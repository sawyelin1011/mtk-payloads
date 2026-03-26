/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <libc.h>
#include <crypto/ssr/registers.h>
#include <crypto/ssr/top.h>
#include <crypto/ssr/pka.h>

const uint32_t ecc_domain_p256[8] = {
    0xd835c65a, 0xe7933aaa, 0x55bdebb3, 0xbc869876,
    0xb0061d65, 0xf6b053cc, 0x3e3cce3b, 0x4b60d227
};

const uint32_t ecc_domain_p384[12] = {
    0xa72f31b3, 0xe4e73ee2, 0x6b058e98, 0x192df8e3,
    0x6e9c1d18, 0x124181fe, 0x8f081403, 0x5a871350,
    0x8d3956c6, 0x9dd12e8a, 0xedc8852a, 0xef2aecd3
};

 void pka_write(uintptr_t win_base, const uint32_t *src, int nwords) {
    volatile uint32_t *dst = (volatile uint32_t *)win_base;
    for (int i = nwords - 1; i >= 0; i--)
        *dst++ = swab32(src[i]);
}

void pka_read(uintptr_t win_base, uint32_t *dst, int nwords) {
    const volatile uint32_t *src = (const volatile uint32_t *)win_base;
    for (int i = nwords - 1; i >= 0; i--) {
        uint32_t val = *src++;
        dst[i] = swab32(val);
    }
}

int pka_rsa_modexp(uint32_t mode, void *result, const void *base,
                           const void *exponent, const void *modulus,
                           uint32_t timeout)
{
    if (!result || !base || !exponent || !modulus)
        return SSR_KDF_ERR_NULL_PTR;
    if (mode >= 4)
        return SSR_PKA_MODE_OUT_OF_RANGE;

    int key_words = (int)(mode + 1) * 32;

    writel(SSR_PKA_OP_RSA_MODEXP, SSR_PKA_OP_TYPE);
    writel(mode, SSR_PKA_RSA_KEY_IDX);
    writel(0, SSR_PKA_RSA_KEY_ZERO);

    pka_write(SSR_PKA_OP_A, exponent, key_words);
    pka_write(SSR_PKA_OP_B, modulus, key_words);
    pka_write(SSR_PKA_OP_C, base, key_words);

    writel(0, SSR_PKA_START);

    if (ssr_poll_until(SSR_PKA_DONE, 1, 1, timeout))
        return SSR_PKA_TIMEOUT;

    writel(1, SSR_PKA_RESULT_ACK);
    pka_read(SSR_PKA_OP_C, result, key_words);

    /* Some reset operation perhaps? */
    uint32_t ctrl = readl(SSR_PKA_CTRL);
    writel(ctrl & ~2U, SSR_PKA_CTRL);
    writel((ctrl & ~2U) | 2U, SSR_PKA_CTRL);

    return (int)SSR_OK;
}

void pka_ecc_push_operand(uintptr_t fifo, const uint32_t *src, int nwords) {
    for (int i = nwords - 1; i >= 0; i--)
        writel(swab32(src[i]), fifo);

    for (int i = nwords; i < 13; i++)
        writel(0, fifo);
}

void pka_ecc_push_domain(const uint32_t *table, int word_count) {
    for (int i = word_count - 1; i >= 0; i--)
        writel(swab32(table[i]), SSR_PKA_ECC_DOM_FIFO);
    for (int i = word_count; i < 19; i++)
        writel(0, SSR_PKA_ECC_DOM_FIFO);
}

void pka_ecc_pop_operand(uintptr_t fifo, uint32_t *dst, int nwords) {
    for (int i = nwords - 1; i >= 0; i--)
        dst[i] = swab32(readl(fifo));
}


int pka_ecc_op(uint32_t curve, uint32_t op_code,
                       const uint32_t *operands[], int n_operands,
                       uint32_t *out_x, uint32_t *out_y)
{
    const uint32_t *dom;
    int wc;

    if (curve == SSR_PKA_ECC_CURVE_NIST_P256) {
        dom = ecc_domain_p256;
        wc  = SSR_PKA_ECC_P256_WORDS;
    } else if (curve == SSR_PKA_ECC_CURVE_NIST_P384) {
        dom = ecc_domain_p384;
        wc  = SSR_PKA_ECC_P384_WORDS;
    } else {
        return SSR_PKA_ECC_INVALID_CURVE;
    }

    writel(op_code, SSR_PKA_OP_TYPE);
    writel(curve, SSR_PKA_ECC_CURVE);

    for (int i = 0; i < n_operands; i++) {
        if (operands[i])
            pka_ecc_push_operand(SSR_PKA_ECC_OP_FIFO, operands[i], wc);
    }

    pka_ecc_push_domain(dom, wc);

    writel(0, SSR_PKA_START);

    if (ssr_poll_until(SSR_PKA_DONE, 1, 1, SSR_TIMEOUT))
        return SSR_PKA_TIMEOUT;

    writel(1, SSR_PKA_RESULT_ACK);

    if (out_x)
        pka_ecc_pop_operand(SSR_PKA_ECC_OP_FIFO, out_x, wc);
    if (out_y)
        pka_ecc_pop_operand(SSR_PKA_ECC_DOM_FIFO, out_y, wc);

    uint32_t ctrl = readl(SSR_PKA_CTRL);
    writel(ctrl & ~2U, SSR_PKA_CTRL);
    writel((ctrl & ~2U) | 2U, SSR_PKA_CTRL);

    return SSR_OK;
}
