#include <drivers/uart.h>

volatile uintptr_t uart_base = DEFAULT_UART_BASE;

void mtk_uart_putc(int ch) {
    while (!(*(volatile uint32_t*)(uart_base + UART_LSR_OFFSET) & UART_LSR_THRE));
    *(volatile uint32_t*)(uart_base + UART_THR_OFFSET) = ch;
}

void uart_putc(int ch, void *ctx) {
    (void)ctx;
    if (ch == '\n') {
        mtk_uart_putc('\r');
    }
    mtk_uart_putc(ch);
}

void mtk_uart_set_base(uintptr_t base) {
    uart_base = base;
}
