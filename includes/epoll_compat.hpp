#ifndef EPOLL_COMPAT_HPP
#define EPOLL_COMPAT_HPP

/*
  macOS compatibility layer for a minimal subset of Linux epoll used by this project.
  On Linux, include <sys/epoll.h> instead of this header.
*/

#ifdef __APPLE__

#include <sys/types.h>
#include <sys/event.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef union epoll_data {
	void	*ptr;
	int		fd;
	unsigned int u32;
	unsigned long long u64;
} epoll_data_t;

struct epoll_event {
	unsigned int	events;
	epoll_data_t		data;
};

/* event masks (subset) */
#ifndef EPOLLIN
#define EPOLLIN  0x001
#endif
#ifndef EPOLLOUT
#define EPOLLOUT 0x004
#endif
#ifndef EPOLLHUP
#define EPOLLHUP 0x010
#endif

/* control ops */
#ifndef EPOLL_CTL_ADD
#define EPOLL_CTL_ADD 1
#endif
#ifndef EPOLL_CTL_DEL
#define EPOLL_CTL_DEL 2
#endif
#ifndef EPOLL_CTL_MOD
#define EPOLL_CTL_MOD 3
#endif

int epoll_create1(int flags);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

#ifdef __cplusplus
}
#endif

#endif /* __APPLE__ */

#endif /* EPOLL_COMPAT_HPP */
