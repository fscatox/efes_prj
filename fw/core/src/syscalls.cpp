/**
 * @file     syscalls.cpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     08.01.2025
 * @see      https://sourceware.org/newlib/libc.html
 *
 * Graceful failure for unused services is provided by libnosys stubs
 *   libgloss/libnosys/environ.c  // Pointer to a list of env. variables
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
 *   libgloss/libnosys/isatty.c   // Output stream is a terminal?
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

#include <cerrno>

#include "debug.h"
#include "main.h"
#include "stm32f4xx.h"

/*
 * Newlib provides a macro definition for errno as part of the support
 * for reentrant routines. Reentrant wrappers of the system calls take care
 * of capturing the global errno in a way consistent with the implementation
 */
#undef errno
extern int errno;

/* Symbols from linker script */
extern unsigned char end[];

/*
 * Global variable linked with the core register used as stack pointer
 * (see: https://github.com/ARM-software/abi-aa/blob/main/aapcs32/aapcs32.rst
 *       https://gcc.gnu.org/onlinedocs/gcc/Global-Register-Variables.html )
 */
register unsigned char *stack_ptr asm("sp");

/* System calls */
static auto &fm = File_Manager();

/* (prevent name mangling for these symbols) */
extern "C" {

/* Memory management */
void *_sbrk(ptrdiff_t incr) {
  static unsigned char *heap_end = end;
  unsigned char *prev_heap_end = heap_end;

  if (heap_end + incr > stack_ptr) {
    errno = ENOMEM;
    return reinterpret_cast<void*>(-1);
  }

  heap_end += incr;
  return prev_heap_end;
}

/* Logs the event for debugging and attempts a system reset */
void _exit([[maybe_unused]] int status) {

  /* wait for ongoing debug/error transmissions to complete */
  PRINTE("Status: %d", status);
  if (status == -1)
    perror("Errno: ");

  close(STDERR_FILENO);
  close(STDOUT_FILENO);

  /* force system reset */
  NVIC_SystemReset();
  while (true);
}

/*
 * Abstractions to access device files
 */

constexpr int wrapCall(int ret) {
  if (ret < 0) {
    errno = -ret;
    return -1;
  }
  return ret;
}

int _close(int fd) {
  return wrapCall(fm.close(fd));
}

int _fstat(int fd, struct stat *st) {
  return wrapCall(fm.fstat(fd, st));
}

int _link(const char *old_name, const char *new_name) {
  return wrapCall(FileManagerType::link(old_name, new_name));
}

off_t _lseek(int fd, off_t offset, int whence) {
  return wrapCall(fm.lseek(fd, offset, whence));
}

int _open(const char *name, int flags, mode_t mode) {
  return wrapCall(fm.open(name, flags, mode));
}

int _read(int fd, void *buf, size_t count) {
  return wrapCall(fm.read(fd, buf, count));
}

int _stat(const char *name, struct stat *st) {
  return wrapCall(fm.stat(name, st));
}

int _unlink(const char *name) {
  return wrapCall(FileManagerType::unlink(name));
}

int _write(int fd, const void *buf, size_t count) {
  return wrapCall(fm.write(fd, buf, count));
}
  
}