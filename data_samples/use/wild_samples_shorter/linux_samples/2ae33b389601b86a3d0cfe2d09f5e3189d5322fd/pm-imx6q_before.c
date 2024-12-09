#include "common.h"
#include "hardware.h"

extern unsigned long phys_l2x0_saved_regs;

static int imx6q_suspend_finish(unsigned long val)
{
	cpu_do_idle();
	return 0;

void __init imx6q_pm_init(void)
{
	/*
	 * The l2x0 core code provides an infrastucture to save and restore
	 * l2x0 registers across suspend/resume cycle.  But because imx6q
	 * retains L2 content during suspend and needs to resume L2 before
	 * MMU is enabled, it can only utilize register saving support and
	 * have to take care of restoring on its own.  So we save physical
	 * address of the data structure used by l2x0 core to save registers,
	 * and later restore the necessary ones in imx6q resume entry.
	 */
#ifdef CONFIG_CACHE_L2X0
	phys_l2x0_saved_regs = __pa(&l2x0_saved_regs);
#endif

	suspend_set_ops(&imx6q_pm_ops);
}