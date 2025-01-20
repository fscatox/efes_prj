/**
 * @file     main.h
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     05.01.2025
 */

#ifndef MAIN_H
#define MAIN_H

extern "C" {

#include <cerrno>
#include <sys/types.h>

/*
 * Clock configuration
 */

#define HCLK_FREQUENCY_HZ 64000000

void systemClockConfig();

/*
 * System calls for filesystem access
 */

#undef errno
extern int errno;

#define RET_ERRNO(x)  \
  do {                \
    if ((x) < 0) {    \
      errno = -(x);   \
      return -1;      \
    }                 \
    return x;         \
  } while (false)

int _close(int fd);
int _fstat(int fd, struct stat *st);
int _link(const char *old_name, const char *new_name);
off_t _lseek(int fd, off_t offset, int whence);
int _open(const char *name, int flags, mode_t mode);
int _read(int fd, void *buf, size_t count);
int _stat(const char *name, struct stat *st);
int _unlink(const char *name);
int _write(int fd, const void *buf, size_t count);

}

#endif // MAIN_H
