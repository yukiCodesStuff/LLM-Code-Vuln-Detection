__SYSCALL(__NR_pidfd_open, sys_pidfd_open)
#define __NR_clone3 435
__SYSCALL(__NR_clone3, sys_clone3)

/*
 * Please add new compat syscalls above this comment and update
 * __NR_compat_syscalls in asm/unistd.h.