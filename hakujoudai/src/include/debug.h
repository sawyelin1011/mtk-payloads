/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy, R0rt1z2
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

int printf(const char *fmt, ...);
void hexdump(const void *data, size_t size, uint64_t base_addr);
