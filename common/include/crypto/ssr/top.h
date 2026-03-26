/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <mmio.h>

#define ssr_poll(addr, val, cond, timeout) \
    ({  \
        uint64_t __iters = 0; \
        uint64_t __max_iters = (uint64_t)(timeout) * 1000; \
        int __ret = 0; \
        do { \
            if (__iters > __max_iters) { \
                __ret = 0x7249; \
                break; \
            } \
            __iters++; \
            for (volatile int __d = 0; __d < 100; __d++); \
            (val) = readl(addr); \
        } while(cond); \
        __ret; \
    })

#define ssr_poll_when(addr, mask, expected, timeout) \
    ({ \
        uint32_t __v; \
        readl_poll_timeout((addr), __v, ((__v) & (mask)) == (expected), (timeout)); \
    })

#define ssr_poll_until(addr, mask, expected, timeout) \
    ({ \
        uint32_t __val; \
        ssr_poll((addr), __val, ((__val) & (mask)) != (expected), (timeout)); \
    })

int ssr_cc_init(void);
void ssr_cc_skip_keyslot_init(void);
int ssr_rng_clk(bool enable);
int ssr_ccc_clk(bool enable);
int ssr_kdf_clk(bool enable);
int ssr_pka_clk(bool enable);
int ssr_rng_set_clk_rate(uint8_t rate);
int ssr_ccc_set_clk_rate(uint8_t rate);
int ssr_kdf_set_clk_rate(uint8_t rate);
int ssr_pka_set_clk_rate(uint8_t rate);
uint64_t ssr_get_lcs(void);
uintptr_t vaddr_to_paddr(void* in);
