/**
 * @file     syscalls.c
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     08.01.2025
 * @see      https://sourceware.org/newlib/libc.html
 */

#include "stm32f4xx.h"

#include <stddef.h>
#include <errno.h>

/*
 * Newlib provides a macro definition for errno as part of the support
 * for reentrant routines. Reentrant wrappers of the system calls take care
 * of capturing the global errno in a way consistent with the implementation
 */
#undef errno
extern int errno;

/* Symbols defined in linker script */
extern unsigned char end[];

/*
 * Global variable linked with the core register used as stack pointer
 * (see: https://github.com/ARM-software/abi-aa/blob/main/aapcs32/aapcs32.rst
 *       https://gcc.gnu.org/onlinedocs/gcc/Global-Register-Variables.html )
 */
register unsigned char *stack_ptr asm("sp");

/*
 * Memory management
 */

void *_sbrk(ptrdiff_t incr) {
  static unsigned char *heap_end = end;
  unsigned char *prev_heap_end = heap_end;

  if (heap_end + incr > stack_ptr) {
    errno = ENOMEM;
    return (void *)-1;
  }

  heap_end += incr;
  return prev_heap_end;
}

/*
 * Exit a program without cleaning up files
 * (initiates a system reset request)
 */
void _exit(int status) {
  (void)status;
  NVIC_SystemReset();
}

/*
 * Graceful failure for unused services is provided by libnosys stubs
 *
 * See:
 *   libgloss/libnosys/environ.c  // Pointer to a list of environment
 * variables
 *                                // @return { 0 }
 *
 *   libgloss/libnosys/execve.c   // Transfer control to a new process
 *                                // @return -1, sets ENOMEM
 *
 *   libgloss/libnosys/fork.c     // Create a new process
 *                                // @return -1, sets EAGAIN
 *
 *   libgloss/libnosys/getpid.c   // Process ID
 *                                // @return 1
 *
 *   libgloss/libnosys/isatty.c   // Query whether output stream is a
 * terminal
 *                                // @return 1
 *
 *   libgloss/libnosys/kill.c     // Send a signal
 *                                // @return -1, sets EINVAL
 *
 *   libgloss/libnosys/wait.c     // Wait for a child process
 *                                // @return -1, sets ECHILD
 *
 *   libgloss/libnosys/times.c    // Timing information for current process
 *                                // @return -1
 */
