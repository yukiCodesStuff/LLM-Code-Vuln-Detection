#include <linux/of_irq.h>
#include <linux/of_address.h>

#include <asm/smp_plat.h>
#include <asm/smp_twd.h>
#include <asm/localtimer.h>

/* set up by the platform code */
	struct device_node *np;
	int err;

	if (!is_smp() || !setup_max_cpus)
		return;

	np = of_find_matching_node(NULL, twd_of_match);
	if (!np)
		return;
