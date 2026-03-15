/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>
#include <stddef.h>

#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_ALT_FORM_FLAG 1

#include <nanoprintf.h>

/*
 * Maximum number of callbacks that can be registered simultaneously.
 * Increase this value if more sinks are needed.
 */
#define PRINTF_MAX_CALLBACKS 8

typedef void (*printf_putc_cb)(int c, void *ctx);

/*
 * Register a callback that will be invoked for every character
 * produced by printf
 */
int printf_register_cb(printf_putc_cb cb);

/*
 * Unregister the first occurrence of cb from the callback table.
 */
int printf_unregister_cb(printf_putc_cb cb);

/* Remove every registered callback at once. */
void printf_unregister_all_cbs(void);

int printf(const char *fmt, ...);
void hexdump(const void *data, size_t size, uint64_t base_addr);
void bytes_to_hex(const uint8_t *in, size_t len, char *out);
int hex_to_bytes(const char *hex, uint8_t *out, size_t out_len);

#endif // DEBUG_H
