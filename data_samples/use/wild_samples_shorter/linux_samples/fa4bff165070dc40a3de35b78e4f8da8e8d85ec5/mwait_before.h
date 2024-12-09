#include <linux/sched/idle.h>

#include <asm/cpufeature.h>

#define MWAIT_SUBSTATE_MASK		0xf
#define MWAIT_CSTATE_MASK		0xf
#define MWAIT_SUBSTATE_SIZE		4

static inline void __mwait(unsigned long eax, unsigned long ecx)
{
	/* "mwait %eax, %ecx;" */
	asm volatile(".byte 0x0f, 0x01, 0xc9;"
		     :: "a" (eax), "c" (ecx));
}
static inline void __mwaitx(unsigned long eax, unsigned long ebx,
			    unsigned long ecx)
{
	/* "mwaitx %eax, %ebx, %ecx;" */
	asm volatile(".byte 0x0f, 0x01, 0xfb;"
		     :: "a" (eax), "b" (ebx), "c" (ecx));
}

static inline void __sti_mwait(unsigned long eax, unsigned long ecx)
{
	trace_hardirqs_on();
	/* "mwait %eax, %ecx;" */
	asm volatile("sti; .byte 0x0f, 0x01, 0xc9;"
		     :: "a" (eax), "c" (ecx));