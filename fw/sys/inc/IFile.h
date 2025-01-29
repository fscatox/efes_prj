/**
 * @file     IFile.h
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     12.01.2025
 */

#ifndef IFILE_H
#define IFILE_H

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

  virtual int open(OFile &ofile);
  virtual int close(OFile &ofile);
  virtual off_t llseek(OFile &ofile, off_t offset, int whence);
  virtual ssize_t read(OFile &ofile, char *buf, size_t count, off_t &pos);
  virtual ssize_t write(OFile &ofile, const char *buf, size_t count, off_t &pos);

protected:
  IFile() = default;
};

#endif // IFILE_H
