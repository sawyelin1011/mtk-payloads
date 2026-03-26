#pragma once

#include <crypto/sbrom/sbrom.h>
#include <stdint.h>

void set_tzcc_base(uintptr_t base);
uintptr_t get_tzcc_base(void);
int tzcc_key_derive(const uint8_t *label,uint32_t label_len, const uint8_t *ctx, uint32_t ctx_len, uint8_t *out, uint32_t out_len);
