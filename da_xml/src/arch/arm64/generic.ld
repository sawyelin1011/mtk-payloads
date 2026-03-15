OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)

ENTRY(main)

SECTIONS
{
  . = 0x68000000;

  .text : {
    *(.text.main)
    *(.text*)
  }

  .rodata : {
    *(.rodata*)
  }

  .data : {
    *(.data*)
  }

  .bss ALIGN(32) : {
      __bss_start = .;
      *(.bss*)
      *(COMMON)
      __bss_end = .;
  }

  /DISCARD/ : {
    *(.comment*) *(.note*) *(.eh_frame*) *(.ARM*) *(.plt*) *(.got*)
  }
}
