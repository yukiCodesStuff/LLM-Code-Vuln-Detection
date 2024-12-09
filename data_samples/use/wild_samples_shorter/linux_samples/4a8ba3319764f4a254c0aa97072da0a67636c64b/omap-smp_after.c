#include <linux/irqchip/arm-gic.h>

#include <asm/smp_scu.h>
#include <asm/virt.h>

#include "omap-secure.h"
#include "omap-wakeupgen.h"
#include <asm/cputype.h>
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