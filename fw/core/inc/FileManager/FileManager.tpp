/**
 * @file     FileManager.tpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     18.01.2025
 */

#ifndef FILEMANAGER_TPP
#define FILEMANAGER_TPP

#include <eventpoll.h>

#include <algorithm>
#include <cerrno>
#include <cstring>

template <size_t N_NODES, int NMAX_FD>
template <typename... Args>
constexpr FileManager<N_NODES, NMAX_FD>::FileManager(const Args &...args)
    : _nodes{args...}, _files{} {
  static_assert(sizeof...(args) > 0, "NodeTable is empty");
  static_assert(N_NODES == sizeof...(args), "Mismatched number of nodes");
  static_assert(NMAX_FD > NRESERVED_FD, "OFileTable is too small");
}

template <size_t N_NODES, int NMAX_FD>
typename std::array<Node, N_NODES>::const_pointer
FileManager<N_NODES, NMAX_FD>::_getNode(const char *name) const {
  auto it = std::find_if(_nodes.begin(), _nodes.end(), [&](const Node &node) {
    return !strcmp(name, node.name);
  });
  return it == _nodes.cend() ? nullptr : &(*it);
}

template <size_t N_NODES, int NMAX_FD>
int FileManager<N_NODES, NMAX_FD>::stdStreamAttach(int fd, const char *name,
                                                   int flags) {
  /* Only for standard streams */
  if (fd < 0 || fd >= NRESERVED_FD) return -EBADF;

  auto node = _getNode(name);
  if (!node) return -ENOENT;

  _files[fd] = {.node = node,
                .ofile = {.mode = (fd == STDIN_FILENO) ? FREAD : FWRITE,
                          .flags = flags & ~O_ACCMODE,
                          .pos = 0}};

  /* Invoke driver's open() and free _files' entry in case of error */
  auto ret = _files[fd].node->cdev.open(_files[fd].ofile);
  if (ret < 0) _files[fd].node = nullptr;

  return ret;
}

template <size_t N_NODES, int NMAX_FD>
int FileManager<N_NODES, NMAX_FD>::open(const char *name, int flags,
                                        [[maybe_unused]] mode_t mode) {
  /* File creation is not supported */
  flags &= ~O_CREAT;

  /* Fail if unsupported flags are set */
  if (flags & ~VALID_OPEN_FLAGS) return -EINVAL;

  auto node = _getNode(name);
  if (!node) return -ENOENT;

  /* Look for a free fd past the reserved ones */
  auto it = std::find_if(_files.begin() + NRESERVED_FD, _files.end(),
                         [](const OFileEntry &entry) { return !entry.node; });
  if (it == _files.end()) return -EMFILE;

  *it = {
      .node = node,
      .ofile = {
          .mode = (flags & O_ACCMODE) + 1, /* FREAD, FWRITE, or FREAD|FWRITE */
          .flags = flags & ~O_ACCMODE,     /* FTRUNC, FAPPEND, or FNONBLOCK */
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

template <size_t N_NODES, int NMAX_FD>
int FileManager<N_NODES, NMAX_FD>::close(int fd) {
  if (fd < 0 || fd >= NMAX_FD || !_files[fd].node) return -EBADF;

  auto ret = _files[fd].node->cdev.close(_files[fd].ofile);
  _files[fd].node = nullptr;

  return ret;
}

template <size_t N_NODES, int NMAX_FD>
int FileManager<N_NODES, NMAX_FD>::stat(const char *name, struct stat *st) {
  auto node = _getNode(name);
  if (!node) return -ENOENT;

  st->st_ino = std::distance(_nodes.cbegin(), node);
  st->st_mode = S_IFCHR;
  st->st_nlink = 1;
  return 0;
}

template <size_t N_NODES, int NMAX_FD>
int FileManager<N_NODES, NMAX_FD>::fstat(int fd, struct stat *st) {
  if (fd < 0 || fd >= NMAX_FD || !_files[fd].node) return -EBADF;

  st->st_ino = std::distance(_nodes.cbegin(), _files[fd].node);
  st->st_mode = S_IFCHR;
  st->st_nlink = 1;
  return 0;
}

template <size_t N_NODES, int NMAX_FD>
int FileManager<N_NODES, NMAX_FD>::link([[maybe_unused]] const char *old_name,
                                        [[maybe_unused]] const char *new_name) {
  return -ENOSYS;
}

template <size_t N_NODES, int NMAX_FD>
int FileManager<N_NODES, NMAX_FD>::unlink([[maybe_unused]] const char *name) {
  return -ENOSYS;
}

template <size_t N_NODES, int NMAX_FD>
off_t FileManager<N_NODES, NMAX_FD>::lseek(int fd, off_t offset, int whence) {
  if (fd < 0 || fd >= NMAX_FD || !_files[fd].node) return -EBADF;

  return _files[fd].node->cdev.llseek(_files[fd].ofile, offset, whence);
}

template <size_t N_NODES, int NMAX_FD>
int FileManager<N_NODES, NMAX_FD>::read(int fd, void *buf, size_t count) {
  if (fd < 0 || fd >= NMAX_FD || !_files[fd].node ||
      !(_files[fd].ofile.mode & FREAD))
    return -EBADF;

  return _files[fd].node->cdev.read(_files[fd].ofile, static_cast<char *>(buf),
                                    count, _files[fd].ofile.pos);
}

template <size_t N_NODES, int NMAX_FD>
int FileManager<N_NODES, NMAX_FD>::write(int fd, const void *buf,
                                         size_t count) {
  if (fd < 0 || fd >= NMAX_FD || !_files[fd].node ||
      !(_files[fd].ofile.mode & FWRITE))
    return -EBADF;

  return _files[fd].node->cdev.write(_files[fd].ofile,
                                     static_cast<const char *>(buf), count,
                                     _files[fd].ofile.pos);
}

template <size_t N_NODES, int NMAX_FD>
int FileManager<N_NODES, NMAX_FD>::vfcntl(int fd, int cmd, va_list vlist) {
  if (fd < 0 || fd >= NMAX_FD || !_files[fd].node) return -EBADF;

  auto &ofile = _files[fd].ofile;
  auto &flags = ofile.flags;

  switch (cmd) {
    case F_GETFL:
      return flags;

    case F_SETFL: {
      constexpr auto SETFL_MASK = O_APPEND | O_NONBLOCK;
      auto arg = va_arg(vlist, unsigned int);
      const auto flipped_bits = arg ^ flags;

      /* O_APPEND cannot be cleared if the file is marked as append-only
       * amd the file is open for write */
      if ((flipped_bits & O_APPEND) && (ofile.mode & FWRITE)) return -EPERM;

      flags = (flags & ~SETFL_MASK) | (arg & SETFL_MASK);
      return 0;
    }

    default:
      return -EINVAL;
  }
}

template <size_t N_NODES, int NMAX_FD>
int FileManager<N_NODES, NMAX_FD>::select(int n, fd_set *inp, fd_set *outp,
                                          fd_set *exp, timeval *tvp) {
  constexpr auto POLLIN_SET =
      EPOLLRDNORM | EPOLLRDBAND | EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLNVAL;
  constexpr auto POLLOUT_SET =
      EPOLLWRBAND | EPOLLWRNORM | EPOLLOUT | EPOLLERR | EPOLLNVAL;
  constexpr auto POLLEX_SET = EPOLLPRI | EPOLLNVAL;

  /* Support for blocking indefinitely or not at all */
  bool timed_out = false;
  if (tvp && !((timed_out = (tvp->tv_sec == 0) && (tvp->tv_usec == 0))))
    return -ENOTSUP;

  if (n < 0) return -EINVAL;
  if (n > NMAX_FD) n = NMAX_FD;

  /*
   * A set of file descriptors is represented with the `fd_set` type.
   * The inclusion of a file descriptor in the set corresponds to setting a bit
   * in a bitmap. While declaring the type, the maximum number of representable
   * file descriptors is taken to be `FD_SETSIZE`, and the data type for the
   * bitmap is chosen to be `fd_mask`. Accordingly, the fd_set is an array of
   * ceil(FD_SETSIZE / fd_mask::nbits) bitmaps, with the straightforward
   * mapping such that the file descriptor fd is in bitmap (fd / fd_mask::nbits)
   * at location (fd % fd_mask::nbits).
   *
   * Now, the argument n is "the highest-numbered file descriptor in any of
   * the three sets plus 1": therefore, it corresponds to the exact number of
   * bits used in the inp, outp, and exp set representations.
   *
   * (For the returns, it is therefore possible to reduce the memory footprint
   * compared to the `fd_set` type)
   */
  fd_set ret_in{}, ret_out{}, ret_ex{};

  /*
   * The return is the total number of bits that were set in the returned sets
   * (corresponding to file descriptors that were found ready in that class)
   */
  auto set_cnt = 0;

  /*
   * The readiness of a file descriptor is investigated with a VFS operation
   *   `__poll_t (*poll) (struct file *, struct poll_table_struct *)`
   * Monitoring of such readiness is implemented with a polling loop
   */
  while (true) {
    auto inp_ = inp->__fds_bits;
    auto outp_ = outp->__fds_bits;
    auto exp_ = exp->__fds_bits;

    auto rinp = ret_in.__fds_bits;
    auto routp = ret_out.__fds_bits;
    auto rexp = ret_ex.__fds_bits;

    /* Over bitmaps */
    for (auto i = 0; i < n; ++rinp, ++routp, ++rexp, ++inp_, ++outp_, ++exp_) {
      constexpr int bitmap_nbits = sizeof(fd_mask) * 8;

      const auto in = *inp_;
      const auto out = *outp_;
      const auto ex = *exp_;
      const auto all = in | out | ex;

      if (!all) /* Skip entire bitmap */
        i += bitmap_nbits;
      else {
        /* Within a bitmap */
        fd_mask bit_sel = 1;
        for (auto j = 0; i < n && j < bitmap_nbits; ++j, ++i, bit_sel <<= 1) {
          if (all & bit_sel) {
            /*
             * Bind to driver's file operation, which returns the ready state
             * for reading, writing, and exceptional conditions.
             * Based on this information, the return bitmaps and the total bit
             * set count are updated.
             */
            const auto mask = !_files[i].node
                                  ? EPOLLNVAL
                                  : _files[i].node->cdev.poll(_files[i].ofile);

            if ((mask & POLLIN_SET) && (in & bit_sel)) {
              *rinp |= bit_sel;
              set_cnt++;
            }
            if ((mask & POLLOUT_SET) && (out & bit_sel)) {
              *routp |= bit_sel;
              set_cnt++;
            }
            if ((mask & POLLEX_SET) && (ex & bit_sel)) {
              *rexp |= bit_sel;
              set_cnt++;
            }
          }
        }
      }
    }

    if (set_cnt || timed_out) {
      *inp = ret_in;
      *outp = ret_out;
      *exp = ret_ex;
      return set_cnt;
    }
  }
}

#endif  // FILEMANAGER_TPP