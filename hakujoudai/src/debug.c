/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy, R0rt1z2
 */

#include <stdarg.h>
#include <stdint.h>

#define NANOPRINTF_IMPLEMENTATION
#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_ALT_FORM_FLAG 0

#include <nanoprintf.h>
#include <mmio.h>

#define MTK_UART_THR       0x00
#define MTK_UART_LSR       0x14
#define MTK_UART_LSR_THRE  (1 << 5)

static volatile uintptr_t MTK_UART_BASE = 0x88888888;

static inline void uart_write(uint32_t offset, uint32_t value)
{
    *(volatile uint32_t *)(MTK_UART_BASE + offset) = value;
}

static inline uint32_t uart_read(uint32_t offset)
{
    return *(volatile uint32_t *)(MTK_UART_BASE + offset);
}


static void uart_putc(int ch, void *ctx)
{
    (void)ctx;
    if (ch == '\n') {
        while (!(uart_read(MTK_UART_LSR) & MTK_UART_LSR_THRE));
        uart_write(MTK_UART_THR, '\r');
    }
    while (!(uart_read(MTK_UART_LSR) & MTK_UART_LSR_THRE));
    uart_write(MTK_UART_THR, ch);
}

int printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = npf_vpprintf(&uart_putc, NULL, fmt, args);
    va_end(args);
    return ret;
}


void hexdump(const void *data, size_t size, uint64_t base_addr)
{
    const uint8_t *bytes = (const uint8_t *)data;

    for (size_t i = 0; i < size; i += 16) {
        printf("%016lx: ", base_addr + i);

        for (size_t j = 0; j < 16; j++) {
            if (i + j < size)
                printf("%02x ", bytes[i + j]);
            else
                printf("   ");
        }

        printf(" |");
        for (size_t j = 0; j < 16 && i + j < size; j++) {
            uint8_t c = bytes[i + j];
            printf("%c", (c >= 32 && c < 127) ? c : '.');
        }
        printf("|\n");
    }
}
