__SYSCALL(__NR_clone3, sys_clone3)
#endif

#undef __NR_syscalls
#define __NR_syscalls 436

/*
 * 32 bit systems traditionally used different
 * syscalls for off_t and loff_t arguments, while