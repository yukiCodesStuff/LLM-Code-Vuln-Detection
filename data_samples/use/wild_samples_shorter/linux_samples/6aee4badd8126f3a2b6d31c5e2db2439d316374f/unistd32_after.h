__SYSCALL(__NR_pidfd_open, sys_pidfd_open)
#define __NR_clone3 435
__SYSCALL(__NR_clone3, sys_clone3)
#define __NR_openat2 437
__SYSCALL(__NR_openat2, sys_openat2)

/*
 * Please add new compat syscalls above this comment and update
 * __NR_compat_syscalls in asm/unistd.h.