/* The program headers of an i686-svr4-musl executable.
 *
 * The kernel of UNIX System V/386 Release 4.0 refuses an executable that has a
 * PT_PHDR entry and no PT_INTERP one (`elfexec': the entry is for the run-time
 * linker), and it refuses after the old image is gone, so the program is killed
 * as it starts. A static link writes PT_PHDR unless the headers are stated, as
 * here: the same three segments the default link makes, the file's headers
 * inside the first, and no PT_PHDR.
 */
PHDRS
{
  rodata PT_LOAD FILEHDR PHDRS FLAGS(4);
  text   PT_LOAD FLAGS(5);
  data   PT_LOAD FLAGS(6);
  unwind PT_GNU_EH_FRAME FLAGS(4);
  stack  PT_GNU_STACK FLAGS(6);
}

SECTIONS
{
  . = 0x400000 + SIZEOF_HEADERS;

  .rodata           : { *(.rodata .rodata.*) } :rodata
  .gcc_except_table : { *(.gcc_except_table .gcc_except_table.*) }
  .eh_frame_hdr     : { *(.eh_frame_hdr) } :rodata :unwind
  .eh_frame         : { KEEP(*(.eh_frame)) } :rodata

  /* A segment begins in a page of its own, at the offset its file position has. */
  . = ALIGN(CONSTANT(MAXPAGESIZE)) + (. & (CONSTANT(MAXPAGESIZE) - 1));
  .text             : { *(.text .text.*) } :text
  PROVIDE(etext = .); PROVIDE(_etext = .);

  . = ALIGN(CONSTANT(MAXPAGESIZE)) + (. & (CONSTANT(MAXPAGESIZE) - 1));
  .preinit_array    : { KEEP(*(.preinit_array)) } :data
  .init_array       : { KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*))) KEEP(*(.init_array)) }
  .fini_array       : { KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*))) KEEP(*(.fini_array)) }
  .data.rel.ro      : { *(.data.rel.ro .data.rel.ro.*) }
  .data             : { *(.data .data.*) }
  .got              : { *(.got) }
  .got.plt          : { *(.got.plt) }
  PROVIDE(edata = .); PROVIDE(_edata = .);
  PROVIDE(__bss_start = .);
  .bss              : { *(.bss .bss.*) *(COMMON) }
  PROVIDE(end = .); PROVIDE(_end = .);
}
