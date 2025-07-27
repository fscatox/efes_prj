/* File          : stm32f401xe.ld
 * Author        : Fabio Scatozza <s315216@studenti.polito.it>
 * Date          : 28.12.2024
 * Description   : Linker script for STM32F401xE (512 kB FLASH, 96 kB SRAM)
 *
 * Architecture  : ARMv7E-M (DSP extension to ARMv7-M)
 *                 FPv4-SP-D16-M (sp extension based on ARMv7-A/R VFPv4-D16)
 *                 Instruction set is Thumb
 *
 * References    : CMSIS v6.1.0
 *                 STMicroelectronics/STM32CubeF4 v1.28.1
 *                 git://sourceware.org/git/newlib-cygwin.git
 *                 (revision: 7923059bff6c120c6fb74b63c7553ea345c0a8f3)
 */

/* check minimum ram space at linking time for heap and stack */
PROVIDE(MIN_STACK_SIZE = 1K);
PROVIDE(MIN_HEAP_SIZE = 3K);

MEMORY
{
  /* sectors of the flash main memory block */
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 16K*4 + 64K + 128K*2
  /* sectors dedicated to non-volatile storage */
  NVS   (w!x) : ORIGIN = 0x08060000, LENGTH = 128K
  /* embedded SRAM */
  RAM   (w!x) : ORIGIN = 0x20000000, LENGTH = 96K
}

/* start executing from the reset handler */
ENTRY(Reset_Handler)

SECTIONS
{
  /* vector table seen at 0x0000_0000 */
  .isr_vector : { KEEP(*(.isr_vector)) } >FLASH

  /* C/C++ (legacy) runtime initialization */
  .init : { KEEP(*(.init)) } >FLASH

  /* executable code */
  .text : { *(.text .text.* .gnu.linkonce.t.*) } >FLASH

  /* C/C++ (legacy) runtime termination */
  .fini : { KEEP(*(.fini)) } >FLASH

  /* read-only data */
  .rodata : ALIGN(4)
  {
    *(.rodata .rodata.* .gnu.linkonce.r.*)
  } >FLASH

  /* frame unwinding */
  .ARM.extab : ALIGN(4)
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } >FLASH

  .ARM : ALIGN(4)
  {
    PROVIDE(__exidx_start = .);
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    PROVIDE(__exidx_end = .);
  } >FLASH

  .eh_frame : ALIGN(4) { *(.eh_frame) } >FLASH
  
  .debug_frame : ALIGN(4) { *(.debug_frame) } >FLASH

  /*
   * C/C++ runtime initialization
   * {__preinit, __init, __fini}_array_{start, end} are exported for newlib
   * see: __libc_init_array()
   */
  .preinit_array (READONLY) : ALIGN(4)
  {
    PROVIDE(__preinit_array_start = .);
    KEEP(*(.preinit_array))
    PROVIDE(__preinit_array_end = .);
  } >FLASH

  .init_array (READONLY) : ALIGN(4)
  {
    PROVIDE(__init_array_start = .);
    KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)))
    KEEP(*(SORT_BY_INIT_PRIORITY(.init_array*)))
    PROVIDE(__init_array_end = .);
  } >FLASH

  .fini_array (READONLY) : ALIGN(4)
  {
    PROVIDE(__fini_array_start = .);
    KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*)))
    KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array*)))
    PROVIDE(__fini_array_end = .);
  } >FLASH

  /* initialized data
   * the symbols are used by the startup to copy the lma content into the vma region
   * both VMA and LMA are adjusted to fit the alignment requirement */
  .data : ALIGN(4)
  {
    PROVIDE(__data_vstart = .);
    *(.data .data.* .gnu.linkonce.d.*)
    /* extend to a word boundary if populated */
    . = ALIGN(. != 0 ? 4 : 1);
  } >RAM AT> FLASH

  PROVIDE(__data_lstart = LOADADDR(.data));
  PROVIDE(__data_size = SIZEOF(.data));

  /* zeroed data
   * the symbols are used by crt0 to zero fill the vma region
   */
  .bss : ALIGN(4) {
    PROVIDE(__bss_start__ = .);
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(COMMON)
    /* extend to a word boundary if populated */
    . = ALIGN(. != 0 ? 4 : 1);
    PROVIDE(__bss_end__ = .);
  } >RAM

  /* remaining ram is shared for stack and heap use
   * heap/stack collisions are detected dynamically, with no __heap_limit to honour
   *
   *   - heap: ascending, from end
   *
   *   - stack: full-descending, from __stack_base
   *     single main stack, used in Thread and Handler modes
   *     AAPCS requires the stack to be 8B-aligned at public interfaces
   *     on reset, CCR->STKALIGN is set: on exception entry the stack is 8B-aligned
   *
   * newlib crt0 explicitly loads sp with __stack
   * see: _start(): line 346
   */

  PROVIDE(__stack_base = (ORIGIN(RAM) + LENGTH(RAM)) & ~7);
  PROVIDE(__stack = __stack_base);

  .heap_stack (NOLOAD) : ALIGN(4)
  {
    PROVIDE(end = ABSOLUTE(.));
    . = __stack_base - end;
  } >RAM

  ASSERT(SIZEOF(.heap_stack) >= MIN_HEAP_SIZE + MIN_STACK_SIZE, "Error: No room for heap and stack")

  /DISCARD/ :
  {
    *(.comment)
    *(.ARM.attributes)

    /* GNU indirect functions (ifunc)
     * compiled sources won't use indirection (-fno.plt) */
    *(.rel.iplt)
    *(.iplt)
    *(.igot.plt)

    /* linker stubs for other architecture profiles */
    *(.glue_7t)
    *(.glue_7)
    *(.vfp11_veneer)
    *(.v4_bx)

  }
}

