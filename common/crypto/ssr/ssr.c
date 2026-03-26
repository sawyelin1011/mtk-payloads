/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <libc.h>
#include <crypto/ssr/registers.h>
#include <crypto/ssr/top.h>
#include <crypto/ssr/kdf.h>
#include <crypto/ssr/pka.h>
#include <crypto/ssr/ccc.h>
#include <crypto/ssr/ssr.h>

volatile uintptr_t ssr_base;

int ssr_hw_init(void) {
    int status = ssr_cc_init();
    return status;
}

uint64_t ssr_lcs_get(void) {
    ssr_ccc_clk(true);
    ssr_kdf_clk(true);
    uint64_t val = ssr_get_lcs();
    ssr_kdf_clk(false);
    ssr_ccc_clk(false);
    return val;
}

void set_ssr_base(uintptr_t base_addr) {
    ssr_base = (base_addr & ~0xFFFFUL);
}

uintptr_t get_ssr_base(void) {
    return ssr_base;
}

int ssr_key_derive(const uint8_t *label, uint32_t label_len,
                          const uint8_t *context, uint32_t context_len,
                          uint8_t *out, uint32_t out_len)
{
    uint8_t  input[SSR_KDF_CMAC_FIN_MAX];
    uint8_t *p = input;
    int status;

    if (!label || !context || !out)
        return SSR_KDF_ERR_NULL_PTR;
    if (!label_len || !context_len || !out_len)
        return SSR_KDF_ERR_NULL_PTR;
    if (label_len >= 65 || context_len >= 65)
        return SSR_KDF_ERR_BAD_SIZE;
    if (label_len + context_len >= 65)
        return SSR_KDF_ERR_BAD_SIZE;

    /* TODO: Verify policy against LCS */

    memcpy(p, label, label_len);
    p += label_len;

    *p++ = 0x00;

    memcpy(p, context, context_len);
    p += context_len;

    *p = (uint8_t)(out_len << 3);

    ssr_kdf_clk(true);
    status = ssr_kdf_kbkdf_cmac_counter(
            FUSE_KDR,
            NULL,
            AES_256,
            input,
            (uint32_t)(p - input) + 1,
            out,
            out_len);

    ssr_kdf_clk(false);

    return status;
}

int ssr_rsa_2048(uint32_t* sign, uint8_t* exp, uint32_t* modulus, uint32_t* result) {
    ssr_pka_clk(true);
    int status = pka_rsa_modexp(SSR_PKA_RSA_MODE_2048, result, sign, exp, modulus, PKA_RSA_SIGN_TIMEOUT);
    ssr_pka_clk(false);

    return status;
}

int ssr_rsa_3072(uint32_t* sign, uint8_t* exp, uint32_t* modulus, uint32_t* result) {
    ssr_pka_clk(true);
    int status = pka_rsa_modexp(SSR_PKA_RSA_MODE_3072, result, sign, exp, modulus, PKA_RSA_SIGN_TIMEOUT);
    ssr_pka_clk(false);
    return status;
}

int ssr_ecdh_gen_key_p256(uint8_t *priv_key, uint8_t *pub_x, uint8_t *pub_y) {
    const uint32_t *ops[] = { (uint32_t *)priv_key, NULL, NULL, NULL, NULL, NULL };
    ssr_pka_clk(true);
    int status = pka_ecc_op(SSR_PKA_ECC_CURVE_NIST_P256, SSR_PKA_OP_ECC_GENKEY, ops, 1, (uint32_t *)pub_x, (uint32_t *)pub_y);
    ssr_pka_clk(false);
    return status;
}

int ssr_ecdh_gen_key_p384(uint8_t *priv_key, uint8_t *pub_x, uint8_t *pub_y) {
    const uint32_t *ops[] = { (uint32_t *)priv_key, NULL, NULL, NULL, NULL, NULL };
    ssr_pka_clk(true);
    int status = pka_ecc_op(SSR_PKA_ECC_CURVE_NIST_P384, SSR_PKA_OP_ECC_GENKEY, ops, 1, (uint32_t *)pub_x, (uint32_t *)pub_y);
    ssr_pka_clk(false);
    return status;
}

int ssr_ecdsa_sign_p256(uint8_t *priv_key, uint8_t *ephem_k, uint8_t *hash, uint8_t *sign_r, uint8_t *sign_s) {
    const uint32_t *ops[] = { (uint32_t *)priv_key, NULL, (uint32_t *)ephem_k, NULL, (uint32_t *)hash, NULL };
    ssr_pka_clk(true);
    int status = pka_ecc_op(SSR_PKA_ECC_CURVE_NIST_P256, SSR_PKA_OP_ECC_SIGN_P256, ops, 3, (uint32_t *)sign_r, (uint32_t *)sign_s);
    ssr_pka_clk(false);
    return status;
}

int ssr_ecdsa_sign_p384(uint8_t *priv_key, uint8_t *ephem_k, uint8_t *hash, uint8_t *sign_r, uint8_t *sign_s) {
    const uint32_t *ops[] = { (uint32_t *)priv_key, NULL, (uint32_t *)ephem_k, NULL, (uint32_t *)hash, NULL };
    ssr_pka_clk(true);
    int status = pka_ecc_op(SSR_PKA_ECC_CURVE_NIST_P384, SSR_PKA_OP_ECC_SIGN_P384, ops, 3, (uint32_t *)sign_r, (uint32_t *)sign_s);
    ssr_pka_clk(false);
    return status;
}

int ssr_ecdsa_vfy_p256(uint8_t *pub_x, uint8_t *pub_y, uint8_t *hash, uint8_t *sig_r, uint8_t *sig_s, uint8_t *sig_r_prime) {
    const uint32_t *ops[] = { (uint32_t *)sig_r, (uint32_t *)pub_x, (uint32_t *)pub_y, (uint32_t *)sig_s, (uint32_t *)hash, NULL };
    ssr_pka_clk(true);
    int status = pka_ecc_op(SSR_PKA_ECC_CURVE_NIST_P256, SSR_PKA_OP_ECC_VERIFY_P256, ops, 5, (uint32_t *)sig_r_prime, NULL);
    ssr_pka_clk(false);
    return status;
}

int ssr_ecdsa_vfy_p384(uint8_t *pub_x, uint8_t *pub_y, uint8_t *hash, uint8_t *sig_r, uint8_t *sig_s, uint8_t *sig_r_prime) {
    const uint32_t *ops[] = { (uint32_t *)sig_r, (uint32_t *)pub_x, (uint32_t *)pub_y, (uint32_t *)sig_s, (uint32_t *)hash, NULL };
    ssr_pka_clk(true);
    int status = pka_ecc_op(SSR_PKA_ECC_CURVE_NIST_P384, SSR_PKA_OP_ECC_VERIFY_P384, ops, 5, (uint32_t *)sig_r_prime, NULL);
    ssr_pka_clk(false);
    return status;
}

int ssr_sha256(uint8_t* in, size_t inlen, uint8_t* out) {
    int status = 0;

    if (in == NULL) {
        return 2;
    }
    if (inlen == 0) {
        return 1;
    }

    ssr_ccc_clk(true);
    ssr_ccc_set_clk_rate(4);
    status = ssr_ccc_sha256(in, inlen, out);
    ssr_ccc_set_clk_rate(0);
    ssr_ccc_clk(false);
    return status;
}

int ssr_sha256_init(ssr_sha256_ctx* ctx) {
    int status = 0;

    if (ctx == NULL) {
        return -1;
    }

    ssr_ccc_clk(true);
    ssr_ccc_set_clk_rate(4);
    status = ssr_ccc_sha256_init(ctx);
    if (status != 0) {
        ssr_ccc_set_clk_rate(0);
        ssr_ccc_clk(false);
    }

    return status;
}

int ssr_sha256_process(ssr_sha256_ctx* ctx, uint8_t* in, size_t inlen) {
    int status = 2;

    if (ctx != NULL && in != NULL) {
        if (inlen == 0) {
            return 1;
        }

        if (0xfffff0 < inlen) {
            do {
                status = ssr_ccc_sha256_process(ctx, in, 0xfffff0);
                if (status != 0) goto err;

                inlen -= 0xfffff0;
                in += 0xfffff0;
            } while (0xfffff0 < inlen);
        }

        status = ssr_ccc_sha256_process(ctx, in, inlen);
        if (status == 0) {
            return SSR_OK;
        }
    }

err:
    ssr_ccc_set_clk_rate(0);
    ssr_ccc_clk(false);

    return status;
}

int ssr_sha256_done(ssr_sha256_ctx *ctx, uint8_t* out) {
    int status = 0;

    if (ctx != NULL && out != NULL) {
        status = ssr_ccc_sha256_done(ctx, out);
    }

    ssr_ccc_set_clk_rate(0);
    ssr_ccc_clk(false);
    return status;
}

int ssr_sha384(uint8_t* in, size_t inlen, uint8_t* out) {
    int status = 0;

    if (in == NULL) {
        return 2;
    }
    if (inlen == 0) {
        return 1;
    }

    ssr_ccc_clk(true);
    ssr_ccc_set_clk_rate(4);
    status = ssr_ccc_sha384(in, inlen, out);
    ssr_ccc_set_clk_rate(0);
    ssr_ccc_clk(false);
    return status;
}

int ssr_sha384_init(ssr_sha384_ctx* ctx) {
    int status = 0;

    if (ctx == NULL) {
        return -1;
    }

    ssr_ccc_clk(true);
    ssr_ccc_set_clk_rate(4);
    status = ssr_ccc_sha384_init(ctx);
    if (status != 0) {
        ssr_ccc_set_clk_rate(0);
        ssr_ccc_clk(false);
    }

    return status;
}

int ssr_sha384_process(ssr_sha384_ctx* ctx, uint8_t* in, size_t inlen) {
    int status = 2;

    if (ctx != NULL && in != NULL) {
        if (inlen == 0) {
            return 1;
        }

        if (0xfffff0 < inlen) {
            do {
                status = ssr_ccc_sha384_process(ctx, in, 0xfffff0);
                if (status != 0) goto err;

                inlen -= 0xfffff0;
                in += 0xfffff0;
            } while (0xfffff0 < inlen);
        }

        status = ssr_ccc_sha384_process(ctx, in, inlen);
        if (status == 0) {
            return SSR_OK;
        }
    }

err:
    ssr_ccc_set_clk_rate(0);
    ssr_ccc_clk(false);

    return status;
}

int ssr_sha384_done(ssr_sha384_ctx *ctx, uint8_t* out) {
    int status = 0;

    if (ctx != NULL && out != NULL) {
        status = ssr_ccc_sha384_done(ctx, out);
    }

    ssr_ccc_set_clk_rate(0);
    ssr_ccc_clk(false);
    return status;
}
