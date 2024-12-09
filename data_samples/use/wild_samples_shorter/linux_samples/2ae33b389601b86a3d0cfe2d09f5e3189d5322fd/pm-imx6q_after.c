#include "common.h"
#include "hardware.h"

static int imx6q_suspend_finish(unsigned long val)
{
	cpu_do_idle();
	return 0;

void __init imx6q_pm_init(void)
{
	suspend_set_ops(&imx6q_pm_ops);
}