__SYSCALL(__NR_clone3, sys_clone3)
#endif

#define __NR_openat2 437
__SYSCALL(__NR_openat2, sys_openat2)

#undef __NR_syscalls
#define __NR_syscalls 438

/*
 * 32 bit systems traditionally used different
 * syscalls for off_t and loff_t arguments, while