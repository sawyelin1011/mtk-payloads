/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy, R0rt1z2
 */

#include <stdint.h>
#include <stddef.h>

#include <libc.h>
#include <debug.h>
#include <heap.h>
#include <logo.h>
#include <commands.h>

static void (*const volatile register_major_command)(const char *, const char *, HHANDLE) = (void *)0x11111111;
static void (*const volatile dagent_command_loop2)(void) = (void *)0x22222222;
static volatile uintptr_t heap_struct = 0x33333333;

__attribute__((section(".text.start"))) void main(void)
{
    printf("%s", banner);

    heap_dump(heap_struct);
    printf("\n");
    heap_fix(heap_struct);
    printf("\n");
    heap_dump(heap_struct);

    register_major_command("CMD:BOOT-TO", "1", cmd_boot_to);
    register_major_command("CMD:EXP-CALL-FUNC", "1", cmd_call_function);
    register_major_command("CMD:EXP-PATCH-MEM", "1", cmd_patch_mem);

    dagent_command_loop2();
}
