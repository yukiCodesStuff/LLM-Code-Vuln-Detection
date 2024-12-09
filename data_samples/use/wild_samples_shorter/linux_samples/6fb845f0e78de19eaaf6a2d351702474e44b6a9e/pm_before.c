#include <linux/suspend.h>
#include <asm/suspend.h>
#include "smc.h"

static int tango_pm_powerdown(unsigned long arg)
{
	tango_suspend(__pa_symbol(cpu_resume));
	.valid = suspend_valid_only_mem,
};

static int __init tango_pm_init(void)
{
	suspend_set_ops(&tango_pm_ops);
	return 0;
}

late_initcall(tango_pm_init);