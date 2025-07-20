/**
 * @file     poll.h
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     18.07.2025
 * @see      https://github.com/torvalds/linux/blob/master/include/uapi/asm-generic/poll.h
 */

#ifndef POLL_H
#define POLL_H

#define POLLIN		0x0001
#define POLLPRI		0x0002
#define POLLOUT		0x0004
#define POLLERR		0x0008
#define POLLHUP		0x0010
#define POLLNVAL	0x0020

#define POLLRDNORM	0x0040
#define POLLRDBAND	0x0080
#ifndef POLLWRNORM
#define POLLWRNORM	0x0100
#endif
#ifndef POLLWRBAND
#define POLLWRBAND	0x0200
#endif
#ifndef POLLMSG
#define POLLMSG		0x0400
#endif
#ifndef POLLREMOVE
#define POLLREMOVE	0x1000
#endif
#ifndef POLLRDHUP
#define POLLRDHUP       0x2000
#endif

#endif //POLL_H
