#include <linux/suspend.h>
#include <linux/export.h>
#include <linux/smp.h>

#include <asm/pgtable.h>
#include <asm/proto.h>
#include <asm/mtrr.h>
	do_fpu_end();
	x86_platform.restore_sched_clock_state();
	mtrr_bp_restore();
}

/* Needed by apm.c */
void restore_processor_state(void)