/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#include <stdint.h>
#include <libc.h>
#include <debug.h>
#include <sej.h>
#include <drivers/uart.h>
#include <logo.h>
#include <hal.h>
#include <commands.h>


extern uint8_t __bss_start, __bss_end;

int (*register_device_ctrl)(uint32_t /*ctrl_code*/, HHANDLE /*handle*/)=(const void*)0x11111111;


__attribute__ ((section(".text.main"))) int main(void) {
    // Clear BSS or good luck getting static working hehe
    memset(&__bss_start, 0, &__bss_end - &__bss_start);
    // Init UART print callback before we print the banner
    printf_register_cb(uart_putc);

    // Banner time!!
    printf(banner);

    printf("> Initializing SEJ context\n");
    init_sej_ctx();

    printf("> Registering commands\n");
    register_device_ctrl(0xF0000,(void*)cmd_ack);
    register_device_ctrl(0xF0001,(void*)cmd_setup_da_ctx);
    register_device_ctrl(0xF0002,(void*)cmd_readmem);
    register_device_ctrl(0xF0003,(void*)cmd_writemem);
    register_device_ctrl(0xF0004,(void*)cmd_readregister);
    register_device_ctrl(0xF0005,(void*)cmd_writeregister);
    register_device_ctrl(0xF0006,(void*)cmd_key_derive);
    register_device_ctrl(0xF0007,(void*)cmd_sej_aes);
    register_device_ctrl(0xF0008,(void*)cmd_rpmb_init);
    register_device_ctrl(0xF0009,(void*)cmd_rpmb_read);
    register_device_ctrl(0xF000A,(void*)cmd_rpmb_write);

    printf("\nAll done! See you on the other side :)\n\n\n");

    return 0;
}
