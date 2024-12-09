int perf_event_disable(int fd);
int perf_event_reset(int fd);

#if !defined(__GLIBC_PREREQ) || !__GLIBC_PREREQ(2, 30)
#include <unistd.h>
#include <sys/syscall.h>
