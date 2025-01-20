/**
 * @file     FileManager.hpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     12.01.2025
 */

#ifndef FILEMANAGER_HPP
#define FILEMANAGER_HPP

#include <array>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "IFile.h"

#define NRESERVED_FD (STDERR_FILENO + 1)
#define VALID_OPEN_FLAGS ((O_RDONLY | O_WRONLY | O_RDWR) | (O_APPEND | O_NONBLOCK))

/* Binds a filename to a resource with IFile interface */
struct Node {
  const char *const name;
  IFile &cdev;
};

template <uint8_t N_NODES, uint8_t NMAX_FD = N_NODES + NRESERVED_FD>
class FileManager {
public:
  /* Construct NodeTable, inferring N_NODES */
  template <typename... Args>
  constexpr explicit FileManager(const Args &...args);

  /* Fill _files[fd] and invoke open() for standard streams */
  int stdStreamAttach(int fd, const char *name, int flags = 0);

  /* System calls */
  int open(const char *name, int flags, mode_t mode = 0);
  int close(int fd);

  int stat(const char *name, struct stat *st);
  int fstat(int fd, struct stat *st);
  static int link(const char *old_name, const char *new_name);
  static int unlink(const char *name);

  off_t lseek(int fd, off_t offset, int whence);
  int read(int fd, void *buf, size_t count);
  int write(int fd, const void *buf, size_t count);

private:
  using NodeTable = std::array<Node, N_NODES>;

  struct OFileEntry {
    typename NodeTable::const_pointer node;
    OFile ofile;
  };
  using OFileTable = std::array<OFileEntry, NMAX_FD>;

  typename NodeTable::const_pointer _getNode(const char *name) const;

  NodeTable _nodes;
  OFileTable _files;
};

/* Infers N_NODES from ctor arguments */
template <typename... Args>
FileManager(const Args &...args) -> FileManager<sizeof...(args)>;

#include "FileManager.tpp"

#endif // FILEMANAGER_HPP