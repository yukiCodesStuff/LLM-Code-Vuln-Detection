#include <asm/alternative.h>
#include <asm/fpu/xstate.h>
#include <asm/trace/mpx.h>
#include <asm/mpx.h>
#include <asm/vm86.h>
#include <asm/umip.h>

		regs->ip = (unsigned long)general_protection;
		regs->sp = (unsigned long)&gpregs->orig_ax;

		return;
	}
#endif
