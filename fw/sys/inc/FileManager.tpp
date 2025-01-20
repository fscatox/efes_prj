/**
 * @file     FileManager.tpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     18.01.2025
 */

#ifndef FILEMANAGER_TPP
#define FILEMANAGER_TPP

#include <algorithm>
#include <cerrno>
#include <cstring>

template <uint8_t N_NODES, uint8_t NMAX_FD>
template <typename... Args>
constexpr FileManager<N_NODES, NMAX_FD>::FileManager(const Args &...args)
    : _nodes{args...}, _files{} {
  static_assert(sizeof...(args) > 0, "NodeTable is empty");
  static_assert(N_NODES == sizeof...(args), "Mismatched number of nodes");
  static_assert(NMAX_FD > NRESERVED_FD, "OFileTable is too small");
}

template <uint8_t N_NODES, uint8_t NMAX_FD>
typename std::array<Node, N_NODES>::const_pointer
FileManager<N_NODES, NMAX_FD>::_getNode(const char *name) const {
  auto it = std::find_if(_nodes.begin(), _nodes.end(), [&](const Node &node) {
    return !strcmp(name, node.name);
  });
  return it == _nodes.cend() ? nullptr : &(*it);
}

template <uint8_t N_NODES, uint8_t NMAX_FD>
int FileManager<N_NODES, NMAX_FD>::stdStreamAttach(int fd, const char *name,
                                                   int flags) {

  /* Only for standard streams */
  if (fd < 0 || fd >= NRESERVED_FD)
    return -EBADF;

  auto node = _getNode(name);
  if (!node)
    return -ENOENT;

  _files[fd] = {.node = node,
                .ofile = {.mode = (fd == STDIN_FILENO) ? FREAD : FWRITE,
                          .flags = flags & ~O_ACCMODE,
                          .pos = 0}};

  /* Invoke driver's open() and free _files' entry in case of error */
  auto ret = _files[fd].node->cdev.open(_files[fd].ofile);
  if (ret < 0)
    _files[fd].node = nullptr;

  return ret;
}

template <uint8_t N_NODES, uint8_t NMAX_FD>
int FileManager<N_NODES, NMAX_FD>::open(const char *name, int flags,
                                        mode_t mode) {

  /* Fail if unsupported flags are set */
  if ((flags & ~VALID_OPEN_FLAGS) || mode)
    return -EINVAL;

  auto node = _getNode(name);
  if (!node)
    return -ENOENT;

  /* Look for a free fd past the reserved ones */
  auto it = std::find_if(_files.begin() + NRESERVED_FD, _files.end(),
                         [](const OFileEntry &entry) { return !entry.node; });
  if (it == _files.end())
    return -EMFILE;

  *it = {
      .node = node,
      .ofile = {
          .mode = (flags & O_ACCMODE) + 1, /* FREAD, FWRITE, or FREAD|FWRITE */
          .flags = flags & ~O_ACCMODE,     /* FAPPEND, or FNONBLOCK */
          .pos = 0,
      }};

  /* Invoke driver's open() and free _files' entry in case of error */
  auto ret = it->node->cdev.open(it->ofile);
  if (ret < 0) {
    it->node = nullptr;
    return ret;
  }

  /* fd is the index of the match in _files */
  return std::distance(_files.begin(), it);
}

template <uint8_t N_NODES, uint8_t NMAX_FD>
int FileManager<N_NODES, NMAX_FD>::close(int fd) {
  if (fd < 0 || fd >= NMAX_FD || !_files[fd].node)
    return -EBADF;

  auto ret = _files[fd].node->cdev.close(_files[fd].ofile);
  _files[fd].node = nullptr;

  return ret;
}

template <uint8_t N_NODES, uint8_t NMAX_FD>
int FileManager<N_NODES, NMAX_FD>::stat(const char *name, struct stat *st) {
  auto node = _getNode(name);
  if (!node)
    return -ENOENT;

  st->st_ino = std::distance(_nodes.cbegin(), node);
  st->st_mode = S_IFCHR;
  st->st_nlink = 1;
  return 0;
}

template <uint8_t N_NODES, uint8_t NMAX_FD>
int FileManager<N_NODES, NMAX_FD>::fstat(int fd, struct stat *st) {
  if (fd < 0 || fd >= NMAX_FD || !_files[fd].node)
    return -EBADF;

  st->st_ino = std::distance(_nodes.cbegin(), _files[fd].node);
  st->st_mode = S_IFCHR;
  st->st_nlink = 1;
  return 0;
}

template <uint8_t N_NODES, uint8_t NMAX_FD>
int FileManager<N_NODES, NMAX_FD>::link([[maybe_unused]] const char *old_name,
                                        [[maybe_unused]] const char *new_name) {
  return -ENOTSUP;
}

template <uint8_t N_NODES, uint8_t NMAX_FD>
int FileManager<N_NODES, NMAX_FD>::unlink([[maybe_unused]] const char *name) {
  return -ENOTSUP;
}

template <uint8_t N_NODES, uint8_t NMAX_FD>
off_t FileManager<N_NODES, NMAX_FD>::lseek(int fd, off_t offset, int whence) {
  if (fd < 0 || fd >= NMAX_FD || !_files[fd].node)
    return -EBADF;

  return _files[fd].node->cdev.llseek(_files[fd].ofile, offset, whence);
}

template <uint8_t N_NODES, uint8_t NMAX_FD>
int FileManager<N_NODES, NMAX_FD>::read(int fd, void *buf, size_t count) {
  if (fd < 0 || fd >= NMAX_FD || !_files[fd].node ||
      !(_files[fd].ofile.mode & FREAD))
    return -EBADF;

  return _files[fd].node->cdev.read(_files[fd].ofile, buf, count,
                               _files[fd].ofile.pos);
}

template <uint8_t N_NODES, uint8_t NMAX_FD>
int FileManager<N_NODES, NMAX_FD>::write(int fd, const void *buf,
                                         size_t count) {
  if (fd < 0 || fd >= NMAX_FD || !_files[fd].node ||
      !(_files[fd].ofile.mode & FWRITE))
    return -EBADF;

  return _files[fd].node->cdev.write(_files[fd].ofile, buf, count,
                                _files[fd].ofile.pos);
}

#endif // FILEMANAGER_TPP