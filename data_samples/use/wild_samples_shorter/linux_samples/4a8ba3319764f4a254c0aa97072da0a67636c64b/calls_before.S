		CALL(sys_getrandom)
/* 385 */	CALL(sys_memfd_create)
		CALL(sys_bpf)
#ifndef syscalls_counted
.equ syscalls_padding, ((NR_syscalls + 3) & ~3) - NR_syscalls
#define syscalls_counted
#endif