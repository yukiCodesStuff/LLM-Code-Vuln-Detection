
#include "soc.h"
#include "common.h"
#include "powerdomain.h"
#include "omap-secure.h"

#define REALTIME_COUNTER_BASE				0x48243200
	void __iomem *base;
	static struct clk *sys_clk;
	unsigned long rate;
	unsigned int reg, num, den;

	base = ioremap(REALTIME_COUNTER_BASE, SZ_32);
	if (!base) {
		pr_err("%s: ioremap failed\n", __func__);
	}

	rate = clk_get_rate(sys_clk);
	/* Numerator/denumerator values refer TRM Realtime Counter section */
	switch (rate) {
	case 1200000:
		num = 64;
		den = 125;
		break;
	case 1300000:
		num = 768;
		den = 1625;
		break;
	case 19200000:
		num = 192;
		den = 625;
		break;
	case 2600000:
		num = 384;
		den = 1625;
		break;
	case 2700000:
		num = 256;
		den = 1125;
		break;
	case 38400000:
		break;
	}

	/* Program numerator and denumerator registers */
	reg = readl_relaxed(base + INCREMENTER_NUMERATOR_OFFSET) &
			NUMERATOR_DENUMERATOR_MASK;
	reg |= num;
	reg |= den;
	writel_relaxed(reg, base + INCREMENTER_DENUMERATOR_RELOAD_OFFSET);

	arch_timer_freq = (rate / den) * num;
	set_cntfreq();

	iounmap(base);
}