/**
 * @file     IFile.h
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     12.01.2025
 */

#ifndef IFILE_H
#define IFILE_H

#include <cerrno>
#include <sys/types.h>

/* Represents an open file */
struct OFile {
  int mode;
  int flags;
  off_t pos;
};

class IFile {
public:
  virtual ~IFile() = default;

  virtual int open([[maybe_unused]] OFile &ofile) { return 0; }
  virtual int close([[maybe_unused]] OFile &ofile) { return 0; }
  virtual off_t llseek([[maybe_unused]] OFile &ofile,
                       [[maybe_unused]] off_t offset,
                       [[maybe_unused]] int whence) {
    return -ENOTSUP;
  }
  virtual ssize_t read([[maybe_unused]] OFile &ofile,
                       [[maybe_unused]] char *buf,
                       [[maybe_unused]] size_t count,
                       [[maybe_unused]] off_t &pos) {
    return -ENOTSUP;
  }
  virtual ssize_t write([[maybe_unused]] OFile &ofile,
                        [[maybe_unused]] const char *buf,
                        [[maybe_unused]] size_t count,
                        [[maybe_unused]] off_t &pos) {
    return -ENOTSUP;
  }

protected:
  IFile() = default;
};

#endif // IFILE_H
