/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#include <crypto/ssr/registers.h>
#include <crypto/ssr/top.h>
#include <mmio.h>

static inline int set_bit_mask(uintptr_t addr, uint8_t val, uint32_t mask) {
    if (val > 1) return SSR_ERR_INVALID_PARAMETER;

    if (val)
        setbits(addr, mask);
    else
        clrbits(addr, mask);

    return 0;
}

int ssr_cc_init(void) {
    writel(SSR_INIT_MAGIC1, CCC_HW_INIT_CFG0);
    writel(SSR_INIT_MAGIC2, CCC_HW_INIT_CFG1);

    // Self test

    return 0;
}

void ssr_cc_skip_keyslot_init(void) {
    uint32_t val = readl(CCC_SKIP_KS_INIT) | 1;
    writel(val, CCC_SKIP_KS_INIT);
}

int ssr_rng_clk(bool enable) {
    return set_bit_mask(SSR_BASE, enable, SSR_CLK_RNG);
}

int ssr_ccc_clk(bool enable) {
    return set_bit_mask(SSR_BASE, enable, SSR_CLK_CCC);
}

int ssr_kdf_clk(bool enable) {
    return set_bit_mask(SSR_BASE, enable, SSR_CLK_KDF);
}

int ssr_pka_clk(bool enable) {
    return set_bit_mask(SSR_BASE, enable, SSR_CLK_PKA);
}

// TODO: Make these use the correct registers for other SOCs like
// MT6899. Currently these only work on MT6878 and MT6985

int ssr_rng_set_clk_rate(uint8_t rate) {
    if (rate > 3) {
        return SSR_ERR_INVALID_RANGE;
    }

    setbits(CLK_CFG_17_CLR, 0x7F);

    if (rate != 0) {
        setbits(CLK_CFG_17_SET, (uint32_t)rate);
    }

    setbits(CLK_CFG_UPDATE2, 0x40);

    return 0;
}

int ssr_ccc_set_clk_rate(uint8_t rate) {
    if (rate > 5) {
        return SSR_ERR_INVALID_RANGE;
    }

    setbits(CLK_CFG_16_CLR, 0x7F0000);

    if (rate != 0) {
        setbits(CLK_CFG_16_SET, (uint32_t)rate << 16);
    }

    setbits(CLK_CFG_UPDATE2, 0x10);

    return 0;
}

int ssr_kdf_set_clk_rate(uint8_t rate) {
    if (rate > 5) {
        return SSR_ERR_INVALID_RANGE;
    }

    setbits(CLK_CFG_16_CLR, 0x7F000000);

    if (rate != 0) {
        setbits(CLK_CFG_16_SET, (uint32_t)rate << 24);
    }

    setbits(CLK_CFG_UPDATE2, 0x20);

    return 0;
}

int ssr_pka_set_clk_rate(uint8_t rate) {
    if (rate > 5) {
        return SSR_ERR_INVALID_RANGE;
    }

    setbits(CLK_CFG_16_CLR, 0x7F00);

    if (rate != 0) {
        setbits(CLK_CFG_16_SET, (uint32_t)rate << 8);
    }

    setbits(CLK_CFG_UPDATE2, 0x08);

    return 0;
}

// Lifecycle state
uint64_t ssr_get_lcs(void) {
    return (readl(SSR_LCS) >> 13) & 0xF;
}

uintptr_t vaddr_to_paddr(void* in) {
    return (uintptr_t)in;
}
