
#include <asm/fpsimd.h>
#include <asm/hw_breakpoint.h>
#include <asm/ptrace.h>
#include <asm/types.h>

#ifdef __KERNEL__
/* Free all resources held by a thread. */
extern void release_thread(struct task_struct *);

/* Prepare to copy thread state - unlazy all lazy status */
#define prepare_to_copy(tsk)	do { } while (0)

unsigned long get_wchan(struct task_struct *p);

#define cpu_relax()			barrier()
#define cpu_relax_lowlatency()                cpu_relax()