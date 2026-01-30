OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_entry)

SECTIONS
{
  . = 0;

  .text : {
    *(.text.start)
    *(.text .text.* .gnu.linkonce.t.*)
  }

  .rodata : {
    *(.rodata .rodata.* .gnu.linkonce.r.*)
  }

  .data : {
    *(.data .data.* .gnu.linkonce.d.*)
  }

  .got : {
    __got_start = .;
    *(.got .got.*)
    __got_end = .;
  }

  .got.plt : {
    *(.got.plt)
  }

  .bss : {
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(COMMON)
  }

  /DISCARD/ : {
    *(.interp)
    *(.dynsym)
    *(.dynstr)
    *(.hash)
    *(.dynamic)
    *(.comment)
    *(.ARM.exidx*)
    *(.ARM.attributes*)
    *(.rel*)
    *(.rela*)
  }
}
