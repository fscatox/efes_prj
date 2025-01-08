/**
 * @file     syscalls.c
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     08.01.2025
 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include "stm32f4xx.h"

/* Symbols defined in linker script */
extern char end[];

/* Define a global register variable and associate it with a specified register
 * (see: https://gcc.gnu.org/onlinedocs/gcc/Global-Register-Variables.html) */
register char *stack_ptr asm ("sp");

void *_sbrk (ptrdiff_t incr) {
  static char *heap_end = end;
  char *prev_heap_end = heap_end;

  if (heap_end + incr > stack_ptr) {
    errno = ENOMEM;
    return (void *) -1;
  }

  heap_end += incr;
  return (void *) prev_heap_end;
}

void _exit(int status) {
  (void)status;
  NVIC_SystemReset();
}