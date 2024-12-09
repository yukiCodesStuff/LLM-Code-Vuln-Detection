#include <asm/alternative.h>
#include <asm/fpu/xstate.h>
#include <asm/trace/mpx.h>
#include <asm/nospec-branch.h>
#include <asm/mpx.h>
#include <asm/vm86.h>
#include <asm/umip.h>

		regs->ip = (unsigned long)general_protection;
		regs->sp = (unsigned long)&gpregs->orig_ax;

		/*
		 * This situation can be triggered by userspace via
		 * modify_ldt(2) and the return does not take the regular
		 * user space exit, so a CPU buffer clear is required when
		 * MDS mitigation is enabled.
		 */
		mds_user_clear_cpu_buffers();
		return;
	}
#endif
