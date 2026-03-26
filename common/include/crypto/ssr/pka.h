/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#pragma once

#include <stdint.h>

int pka_rsa_modexp(uint32_t mode, void *result, const void *base, const void *exp, const void *mod, uint32_t timeout);
int pka_ecc_op(uint32_t curve, uint32_t op_code, const uint32_t *operands[], int n_operands, uint32_t *out_x, uint32_t *out_y);
