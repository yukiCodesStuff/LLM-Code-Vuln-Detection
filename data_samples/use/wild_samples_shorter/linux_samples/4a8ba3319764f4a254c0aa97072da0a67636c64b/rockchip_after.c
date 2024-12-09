#include <linux/init.h>
#include <linux/of_platform.h>
#include <linux/irqchip.h>
#include <linux/clk-provider.h>
#include <linux/clocksource.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/hardware/cache-l2x0.h>
#include "core.h"

#define RK3288_GRF_SOC_CON0 0x244

static void __init rockchip_timer_init(void)
{
	if (of_machine_is_compatible("rockchip,rk3288")) {
		struct regmap *grf;

		/*
		 * Disable auto jtag/sdmmc switching that causes issues
		 * with the mmc controllers making them unreliable
		 */
		grf = syscon_regmap_lookup_by_compatible("rockchip,rk3288-grf");
		if (!IS_ERR(grf))
			regmap_write(grf, RK3288_GRF_SOC_CON0, 0x10000000);
		else
			pr_err("rockchip: could not get grf syscon\n");
	}

	of_clk_init(NULL);
	clocksource_of_init();
}

static void __init rockchip_dt_init(void)
{
	of_platform_populate(NULL, of_default_bus_match_table, NULL, NULL);
	platform_device_register_simple("cpufreq-dt", 0, NULL, 0);
DT_MACHINE_START(ROCKCHIP_DT, "Rockchip Cortex-A9 (Device Tree)")
	.l2c_aux_val	= 0,
	.l2c_aux_mask	= ~0,
	.init_time	= rockchip_timer_init,
	.dt_compat	= rockchip_board_dt_compat,
	.init_machine	= rockchip_dt_init,
MACHINE_END