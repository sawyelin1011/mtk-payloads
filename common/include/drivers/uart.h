/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#ifndef UART_H
#define UART_H

#include <stdint.h>

#define DEFAULT_UART_BASE 0x11002000

#define UART_THR_OFFSET     0x00
#define UART_LSR_OFFSET     0x14
#define UART_LSR_THRE       0x20

void uart_putc(int ch, void *ctx);
void mtk_uart_set_base(uintptr_t base);

#endif //UART_H
