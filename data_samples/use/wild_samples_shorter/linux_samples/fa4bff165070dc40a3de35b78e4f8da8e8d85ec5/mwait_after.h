#include <linux/sched/idle.h>

#include <asm/cpufeature.h>
#include <asm/nospec-branch.h>

#define MWAIT_SUBSTATE_MASK		0xf
#define MWAIT_CSTATE_MASK		0xf
#define MWAIT_SUBSTATE_SIZE		4

static inline void __mwait(unsigned long eax, unsigned long ecx)
{
	mds_idle_clear_cpu_buffers();

	/* "mwait %eax, %ecx;" */
	asm volatile(".byte 0x0f, 0x01, 0xc9;"
		     :: "a" (eax), "c" (ecx));
}
static inline void __mwaitx(unsigned long eax, unsigned long ebx,
			    unsigned long ecx)
{
	/* No MDS buffer clear as this is AMD/HYGON only */

	/* "mwaitx %eax, %ebx, %ecx;" */
	asm volatile(".byte 0x0f, 0x01, 0xfb;"
		     :: "a" (eax), "b" (ebx), "c" (ecx));
}

static inline void __sti_mwait(unsigned long eax, unsigned long ecx)
{
	mds_idle_clear_cpu_buffers();

	trace_hardirqs_on();
	/* "mwait %eax, %ecx;" */
	asm volatile("sti; .byte 0x0f, 0x01, 0xc9;"
		     :: "a" (eax), "c" (ecx));