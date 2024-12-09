/*
 * OMAP4 SMP source file. It contains platform specific functions
 * needed for the linux smp kernel.
 *
 * Copyright (C) 2009 Texas Instruments, Inc.
 *
 * Author:
 *      Santosh Shilimkar <santosh.shilimkar@ti.com>
 *
 * Platform file needed for the OMAP4 SMP. This file is based on arm
 * realview smp platform.
 * * Copyright (c) 2002 ARM Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/device.h>
#include <linux/smp.h>
#include <linux/io.h>
#include <linux/irqchip/arm-gic.h>

#include <asm/smp_scu.h>
#include <asm/virt.h>

#include "omap-secure.h"
#include "omap-wakeupgen.h"
#include <asm/cputype.h>

#include "soc.h"
#include "iomap.h"
#include "common.h"
#include "clockdomain.h"
#include "pm.h"

#define CPU_MASK		0xff0ffff0
#define CPU_CORTEX_A9		0x410FC090
#define CPU_CORTEX_A15		0x410FC0F0

#define OMAP5_CORE_COUNT	0x2

/* SCU base address */
static void __iomem *scu_base;

static DEFINE_SPINLOCK(boot_lock);

void __iomem *omap4_get_scu_base(void)
{
	return scu_base;
}
{
	void *startup_addr = omap4_secondary_startup;
	void __iomem *base = omap_get_wakeupgen_base();

	/*
	 * Initialise the SCU and wake up the secondary core using
	 * wakeup_secondary().
	 */
	if (scu_base)
		scu_enable(scu_base);

	if (cpu_is_omap446x())
		startup_addr = omap4460_secondary_startup;

	/*
	 * Write the address of secondary startup routine into the
	 * AuxCoreBoot1 where ROM code will jump and start executing
	 * on secondary core once out of WFE
	 * A barrier is added to ensure that write buffer is drained
	 */
	if (omap_secure_apis_support())
		omap_auxcoreboot_addr(virt_to_phys(startup_addr));
	else
		/*
		 * If the boot CPU is in HYP mode then start secondary
		 * CPU in HYP mode as well.
		 */
		if ((__boot_cpu_mode & MODE_MASK) == HYP_MODE)
			writel_relaxed(virt_to_phys(omap5_secondary_hyp_startup),
				       base + OMAP_AUX_CORE_BOOT_1);
		else
			writel_relaxed(virt_to_phys(omap5_secondary_startup),
				       base + OMAP_AUX_CORE_BOOT_1);

}

struct smp_operations omap4_smp_ops __initdata = {
	.smp_init_cpus		= omap4_smp_init_cpus,
	.smp_prepare_cpus	= omap4_smp_prepare_cpus,
	.smp_secondary_init	= omap4_secondary_init,
	.smp_boot_secondary	= omap4_boot_secondary,
#ifdef CONFIG_HOTPLUG_CPU
	.cpu_die		= omap4_cpu_die,
#endif
};