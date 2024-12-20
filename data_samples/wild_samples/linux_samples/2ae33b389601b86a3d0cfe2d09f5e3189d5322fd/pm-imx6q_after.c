/*
 * Copyright 2011 Freescale Semiconductor, Inc.
 * Copyright 2011 Linaro Ltd.
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <linux/init.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/suspend.h>
#include <asm/cacheflush.h>
#include <asm/proc-fns.h>
#include <asm/suspend.h>
#include <asm/hardware/cache-l2x0.h>

#include "common.h"
#include "hardware.h"

static int imx6q_suspend_finish(unsigned long val)
{
	cpu_do_idle();
	return 0;
}
static const struct platform_suspend_ops imx6q_pm_ops = {
	.enter = imx6q_pm_enter,
	.valid = suspend_valid_only_mem,
};

void __init imx6q_pm_init(void)
{
	suspend_set_ops(&imx6q_pm_ops);
}