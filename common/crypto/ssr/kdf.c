/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <libc.h>
#include <crypto/ssr/kdf.h>
#include <crypto/ssr/top.h>


static void kdf_write_window(uintptr_t dst, const uint8_t *data, uint32_t len, uint32_t words, bool rtl) {
    uint32_t staging[17] = {0};
    uint32_t wc;

    if (rtl) {
        wc = words;
        memcpy((uint8_t *)staging + wc * 4 - len, data, len);
    } else {
        wc = (len + 3) / 4;
        memcpy(staging, data, len);
    }

    for (uint32_t i = 0; i < wc; i++)
        writel_relaxed(swab32(staging[wc - 1 - i]), dst + i * 4);
}

uint32_t ssr_kdf_kbkdf_cmac_counter(SsrKdfKeySel key_src, const uint8_t *sw_key, uint32_t key_alg,
                                     const uint8_t *in, uint32_t inlen,
                                     uint8_t *out, uint32_t out_len)
{
    uint32_t fin[SSR_KDF_CMAC_FIN_WORDS] = {0};
    uint32_t out_buf[16];
    uint32_t blocks, cmd, status;
    uint8_t key_size;

    if (key_alg > 2)
        return SSR_KDF_ERR_BAD_CONFIG;
    if (!out)
        return SSR_KDF_ERR_NULL_PTR;
    if (out_len > 64)
        return SSR_KDF_ERR_BAD_SIZE;
    if (!in)
        return SSR_KDF_ERR_NULL_PTR;
    if (inlen >= SSR_KDF_CMAC_FIN_WORDS * 4)
        return SSR_KDF_ERR_BAD_SIZE;

    blocks = (out_len + 15) / 16;
    key_size = SSR_AES_KEY_SIZE(key_alg);

    memcpy((uint8_t *)fin + sizeof(fin) - inlen, in, inlen);
    fin[0] |= inlen;

    for (int i = SSR_KDF_CMAC_FIN_WORDS - 1; i >= 0; i--)
        writel_relaxed(swab32(fin[i]), SSR_KDF_CMAC_FIN(i));

    if (key_src == SW_KEY) {
        if (!sw_key)
            return SSR_KDF_ERR_NULL_PTR;

        kdf_write_window(SSR_KDF_CMAC_LBL7, sw_key, key_size, SSR_KDF_CMAC_LBL_WORDS, true);
    }

    cmd = SSR_KDF_CMAC_CMD_START
        | FIELD_PREP(SSR_KDF_CMAC_CMD_KEY_ALG, key_alg)
        | FIELD_PREP(SSR_KDF_CMAC_CMD_KEY_SRC, (uint32_t)key_src)
        | FIELD_PREP(SSR_KDF_CMAC_CMD_OUT_BLOCKS, blocks);

    writel(cmd, SSR_KDF_CMAC_CMD);

    if (ssr_poll_when(SSR_KDF_CMAC_CMD, SSR_KDF_CMAC_CMD_START, SSR_KDF_CMAC_CMD_START, SSR_TIMEOUT))
        return SSR_KDF_ERR_TIMEOUT;

    status = readl(SSR_KDF_CMAC_STS);
    if (FIELD_GET(SSR_KDF_CMAC_STS_ERR, status))
        return SSR_KDF_ERR_HW_UNKNOWN;

    for (uint32_t n = 0; n < 16; n++)
        out_buf[n] = swab32(readl_relaxed(SSR_KDF_CMAC_OUT(n)));

    memcpy(out, out_buf, out_len);
    return SSR_OK;
}


uint32_t ssr_kdf_hkdf_sha256(const uint8_t *ikm,  uint32_t ikm_len,
                               const uint8_t *info, uint32_t info_len,
                               const uint8_t *salt, uint32_t salt_len,
                               uint8_t *out,  uint32_t out_len)
{
    bool no_salt = (salt == NULL);
    uint32_t cmd, status;

    if (!out || !ikm || !info)
        return SSR_KDF_ERR_NULL_PTR;
    if (!no_salt && !salt_len)
        return SSR_KDF_ERR_NULL_PTR;
    if (ikm_len  > SSR_KDF_HKDF_IKM_MAX)
        return SSR_KDF_ERR_BAD_SIZE;
    if (info_len > SSR_KDF_HKDF_INFO_MAX)
        return SSR_KDF_ERR_BAD_SIZE;
    if (!no_salt && salt_len > SSR_KDF_HKDF_SALT_MAX)
        return SSR_KDF_ERR_BAD_SIZE;
    if (out_len > SSR_KDF_HKDF_OUT_MAX || (out_len & 3))
        return SSR_KDF_ERR_BAD_SIZE;

    if (!no_salt)
        kdf_write_window(SSR_KDF_HKDF_SALT7, salt, salt_len,  0, false);

    kdf_write_window(SSR_KDF_HKDF_IKM15, ikm, ikm_len,  0, false);
    kdf_write_window(SSR_KDF_HKDF_INFO11, info, info_len, 0, false);

    cmd = SSR_KDF_HKDF_CMD_START
        | SSR_KDF_HKDF_CMD_FIXED
        | (no_salt ? SSR_KDF_HKDF_CMD_NO_SALT : 0U)
        | FIELD_PREP(SSR_KDF_HKDF_CMD_IKM_LEN,  ikm_len)
        | FIELD_PREP(SSR_KDF_HKDF_CMD_SALT_LEN, no_salt ? 0U : salt_len)
        | FIELD_PREP(SSR_KDF_HKDF_CMD_INFO_LEN, info_len);

    writel(cmd, SSR_KDF_HKDF_CMD);

    if (ssr_poll_when(SSR_KDF_HKDF_CMD, SSR_KDF_HKDF_CMD_START, SSR_KDF_HKDF_CMD_START, SSR_TIMEOUT))
        return SSR_KDF_ERR_TIMEOUT;

    status = readl(SSR_KDF_HKDF_STS);
    if (FIELD_GET(SSR_KDF_HKDF_STS_ERR, status))
        return SSR_KDF_ERR_HW_UNKNOWN;

    for (uint32_t n = 0; n < out_len / 4; n++)
        ((uint32_t *)out)[n] = swab32(readl_relaxed(SSR_KDF_HKDF_OUT(n)));

    return SSR_OK;
}
