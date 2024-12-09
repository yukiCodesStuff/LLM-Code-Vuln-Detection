/* 375 */	CALL(sys_setns)
		CALL(sys_process_vm_readv)
		CALL(sys_process_vm_writev)
		CALL(sys_kcmp)
		CALL(sys_finit_module)
#ifndef syscalls_counted
.equ syscalls_padding, ((NR_syscalls + 3) & ~3) - NR_syscalls
#define syscalls_counted