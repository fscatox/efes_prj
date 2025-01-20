/**
 * @file     main.cpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     18.01.2025
 */

#include "IFile.h"
#include <cerrno>

off_t IFile::llseek([[maybe_unused]] OFile &ofile,
                    [[maybe_unused]] off_t offset,
                    [[maybe_unused]] int whence) {
  return -ENOTSUP;
}

ssize_t IFile::read([[maybe_unused]] const OFile &ofile,
                    [[maybe_unused]] void *buf, [[maybe_unused]] size_t count,
                    [[maybe_unused]] off_t &pos) {
  return -ENOTSUP;
}

ssize_t IFile::write([[maybe_unused]] const OFile &ofile,
                     [[maybe_unused]] const void *buf,
                     [[maybe_unused]] size_t count,
                     [[maybe_unused]] off_t &pos) {
  return -ENOTSUP;
}

int IFile::open([[maybe_unused]] const OFile &ofile) {
  return 0;
}

int IFile::close([[maybe_unused]] const OFile &ofile) {
  return 0;
}
