int perf_event_disable(int fd);
int perf_event_reset(int fd);

struct perf_event_read {
	__u64 nr;
	__u64 l1d_misses;
};

#if !defined(__GLIBC_PREREQ) || !__GLIBC_PREREQ(2, 30)
#include <unistd.h>
#include <sys/syscall.h>
