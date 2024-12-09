#include <asm/x86_init.h>
#include <asm/reboot.h>
#include <asm/cache.h>
#include <asm/nospec-branch.h>

#define CREATE_TRACE_POINTS
#include <trace/events/nmi.h>

		write_cr2(this_cpu_read(nmi_cr2));
	if (this_cpu_dec_return(nmi_state))
		goto nmi_restart;

	if (user_mode(regs))
		mds_user_clear_cpu_buffers();
}
NOKPROBE_SYMBOL(do_nmi);

void stop_nmi(void)