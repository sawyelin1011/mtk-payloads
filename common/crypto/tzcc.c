/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#include <stdint.h>
#include <libc.h>
#include <crypto/tzcc.h>
#include <crypto/key_derive.h>

static uintptr_t tzcc_base = 0;

void set_tzcc_base(uintptr_t base) {
    tzcc_base = base;
}

uintptr_t get_tzcc_base(void) {
    return tzcc_base;
}

int tzcc_key_derive(const uint8_t *label, uint32_t label_len,
                    const uint8_t *ctx, uint32_t ctx_len,
                    uint8_t *out, uint32_t out_len)
{
    if (tzcc_base == 0)
        return -1;

    SBROM_ClockEnable();

    int status = SBROM_KeyDerivation(tzcc_base, ROOT_KEY,
                                     label, label_len,
                                     ctx, ctx_len,
                                     out, out_len);

    SBROM_ClockDisable();
    return status;
}
