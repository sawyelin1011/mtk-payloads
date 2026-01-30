/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy, R0rt1z2
 */

.syntax unified
.arm

.section .text.start
.global _entry

_entry:
    @ calculate the load base
    adr r4, _entry

    @ patch the GOT
    ldr r0, =__got_start
    add r0, r0, r4
    ldr r1, =__got_end
    add r1, r1, r4

.Lgot_loop:
    cmp r0, r1
    bge .Lgot_done
    ldr r2, [r0]
    add r2, r2, r4
    str r2, [r0], #4
    b .Lgot_loop

.Lgot_done:
    b main
