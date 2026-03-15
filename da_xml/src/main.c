/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#include <stdint.h>
#include <stddef.h>
#include <sej.h>
#include <drivers/uart.h>
#include <protocol_functions.h>
#include <commands.h>
#include <logo.h>

extern uint8_t __bss_start, __bss_end;

__attribute__((section(".text.main"))) int main(void) {
    // Zero out BSS or printf callbacks won't work :D
    memset(&__bss_start, 0, &__bss_end - &__bss_start);

    // Register UART callback
    printf_register_cb(uart_putc);

    // BANNER TIME!!!
    printf(banner);

    printf("> Initializing SEJ context...\n");
    init_sej_ctx();

    printf("> Registering commands...\n");

    const char *ver1 = "1";
    register_major_command(CMD_ACK, ver1, (HHANDLE)cmd_ack);
    register_major_command(CMD_DA_CTX, ver1, (HHANDLE)cmd_da_ctx);
    register_major_command(CMD_READMEM, ver1, (HHANDLE)cmd_readmem);
    register_major_command(CMD_WRITEMEM, ver1, (HHANDLE)cmd_writemem);
    register_major_command(CMD_READREGISTER, ver1, (HHANDLE)cmd_readregister);
    register_major_command(CMD_WRITEREGISTER, ver1, (HHANDLE)cmd_writeregister);
    register_major_command(CMD_KEY_DERIVE, ver1, (HHANDLE)cmd_key_derive);
    register_major_command(CMD_SEJ, ver1, (HHANDLE)cmd_sej_aes);
    register_major_command(CMD_RPMB_INIT, ver1, (HHANDLE)cmd_rpmb_init);
    register_major_command(CMD_RPMB_READ, ver1, (HHANDLE)cmd_rpmb_read);
    register_major_command(CMD_RPMB_WRITE, ver1, (HHANDLE)cmd_rpmb_write);

    printf("\nAll done! See you on the other side :)\n\n\n");

    return 0;
}
