
#include "soc.h"
#include "common.h"
#include "control.h"
#include "powerdomain.h"
#include "omap-secure.h"

#define REALTIME_COUNTER_BASE				0x48243200
	void __iomem *base;
	static struct clk *sys_clk;
	unsigned long rate;
	unsigned int reg;
	unsigned long long num, den;

	base = ioremap(REALTIME_COUNTER_BASE, SZ_32);
	if (!base) {
		pr_err("%s: ioremap failed\n", __func__);
	}

	rate = clk_get_rate(sys_clk);

	if (soc_is_dra7xx()) {
		/*
		 * Errata i856 says the 32.768KHz crystal does not start at
		 * power on, so the CPU falls back to an emulated 32KHz clock
		 * based on sysclk / 610 instead. This causes the master counter
		 * frequency to not be 6.144MHz but at sysclk / 610 * 375 / 2
		 * (OR sysclk * 75 / 244)
		 *
		 * This affects at least the DRA7/AM572x 1.0, 1.1 revisions.
		 * Of course any board built without a populated 32.768KHz
		 * crystal would also need this fix even if the CPU is fixed
		 * later.
		 *
		 * Either case can be detected by using the two speedselect bits
		 * If they are not 0, then the 32.768KHz clock driving the
		 * coarse counter that corrects the fine counter every time it
		 * ticks is actually rate/610 rather than 32.768KHz and we
		 * should compensate to avoid the 570ppm (at 20MHz, much worse
		 * at other rates) too fast system time.
		 */
		reg = omap_ctrl_readl(DRA7_CTRL_CORE_BOOTSTRAP);
		if (reg & DRA7_SPEEDSELECT_MASK) {
			num = 75;
			den = 244;
			goto sysclk1_based;
		}
	}

	/* Numerator/denumerator values refer TRM Realtime Counter section */
	switch (rate) {
	case 12000000:
		num = 64;
		den = 125;
		break;
	case 13000000:
		num = 768;
		den = 1625;
		break;
	case 19200000:
		num = 192;
		den = 625;
		break;
	case 26000000:
		num = 384;
		den = 1625;
		break;
	case 27000000:
		num = 256;
		den = 1125;
		break;
	case 38400000:
		break;
	}

sysclk1_based:
	/* Program numerator and denumerator registers */
	reg = readl_relaxed(base + INCREMENTER_NUMERATOR_OFFSET) &
			NUMERATOR_DENUMERATOR_MASK;
	reg |= num;
	reg |= den;
	writel_relaxed(reg, base + INCREMENTER_DENUMERATOR_RELOAD_OFFSET);

	arch_timer_freq = DIV_ROUND_UP_ULL(rate * num, den);
	set_cntfreq();

	iounmap(base);
}