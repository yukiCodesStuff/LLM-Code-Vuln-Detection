
extern void sys_ni_syscall(void);

const sys_call_ptr_t sys_call_table[] ____cacheline_aligned = {
	/*
	 * Smells like a compiler bug -- it doesn't work
	 * when the & below is removed.
	 */