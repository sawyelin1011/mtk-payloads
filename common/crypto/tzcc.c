/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#include <libc.h>
#include <crypto/tzcc.h>
#include <crypto/key_derive.h>

static uintptr_t tzcc_base;

void set_tzcc_base(uintptr_t base) {
    tzcc_base = base;
}

uintptr_t get_tzcc_base(void) {
    return tzcc_base;
}

uint32_t key_derive(KeyType key_type, uint8_t* out, uint32_t len) {
    uint8_t rpmb_label[8] = { 'R','P','M','B',' ','K','E','Y' };
    uint8_t rpmb_context[4] = { 'S','A','S','I' };
    uint8_t fde_label[8] = { 'S','Q','N','C','!','L','F','Z' };
    uint8_t fde_context[4] = { 'T','B','T','J' };

    uint8_t *label;
    uint8_t *context;

    if (key_type == RPMB_KEY) {
        label = rpmb_label;
        context = rpmb_context;
    } else if (key_type == FDE_KEY) {
        label = fde_label;
        context = fde_context;
    } else {
        memset(out, 0, len);
        return 0;
    }

    SBROM_ClockEnable();

    SBROM_KeyDerivation(tzcc_base, ROOT_KEY,
                               label, sizeof(rpmb_label),
                               context, sizeof(rpmb_context),
                               out, len);

    SBROM_ClockDisable();

    return 0;
}
