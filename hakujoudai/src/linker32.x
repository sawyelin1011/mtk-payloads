/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy, R0rt1z2
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_entry)

SECTIONS
{
    . = 0;

    /* Code section - entry point MUST come first */
    .text : {
        KEEP(*(.text.entry))
        *(.text.start)
        *(.text .text.* .gnu.linkonce.t.*)
        *(.glue_7)
        *(.glue_7t)
        *(.vfp11_veneer)
        *(.v4_bx)
        . = ALIGN(4);
    }

    .rodata : {
        *(.rodata .rodata.* .gnu.linkonce.r.*)
        . = ALIGN(4);
    }

    .data : {
        *(.data .data.* .gnu.linkonce.d.*)
        . = ALIGN(4);
    }

    .got : {
        . = ALIGN(4);
        __got_start = .;
        *(.got)
        *(.got.*)
        __got_end = .;
        . = ALIGN(4);
    }

    .got.plt : {
        *(.got.plt)
        . = ALIGN(4);
    }

    .bss : {
        __bss_start = .;
        *(.bss .bss.* .gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    }

    . = ALIGN(4);
    __payload_end = .;

    /DISCARD/ : {
        *(.interp)
        *(.dynsym)
        *(.dynstr)
        *(.hash)
        *(.gnu.hash)
        *(.dynamic)
        *(.comment)
        *(.note*)
        *(.ARM.exidx*)
        *(.ARM.extab*)
        *(.ARM.attributes*)
        *(.eh_frame*)
        *(.eh_frame_hdr*)
        *(.rel*)
        *(.rela*)
        *(.plt*)
        *(.iplt*)
        *(.igot*)
    }
}
