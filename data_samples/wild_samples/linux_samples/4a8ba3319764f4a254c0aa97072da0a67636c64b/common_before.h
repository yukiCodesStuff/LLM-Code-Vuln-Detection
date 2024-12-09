{
}
#endif

/* This gets called from mach-omap2/io.c, do not call this */
void __init omap2_set_globals_tap(u32 class, void __iomem *tap);

void __init omap242x_map_io(void);
void __init omap243x_map_io(void);
void __init omap3_map_io(void);
void __init am33xx_map_io(void);
void __init omap4_map_io(void);
void __init omap5_map_io(void);
void __init ti81xx_map_io(void);

/* omap_barriers_init() is OMAP4 only */
void omap_barriers_init(void);

/**
 * omap_test_timeout - busy-loop, testing a condition
 * @cond: condition to test until it evaluates to true
 * @timeout: maximum number of microseconds in the timeout
 * @index: loop index (integer)
 *
 * Loop waiting for @cond to become true or until at least @timeout
 * microseconds have passed.  To use, define some integer @index in the
 * calling code.  After running, if @index == @timeout, then the loop has
 * timed out.
 */
#define omap_test_timeout(cond, timeout, index)			\
({								\
	for (index = 0; index < timeout; index++) {		\
		if (cond)					\
			break;					\
		udelay(1);					\
	}							\
})

extern struct device *omap2_get_mpuss_device(void);
extern struct device *omap2_get_iva_device(void);
extern struct device *omap2_get_l3_device(void);
extern struct device *omap4_get_dsp_device(void);

void omap_gic_of_init(void);

#ifdef CONFIG_CACHE_L2X0
extern void __iomem *omap4_get_l2cache_base(void);
#endif

struct device_node;

#ifdef CONFIG_SMP
extern void __iomem *omap4_get_scu_base(void);
#else
static inline void __iomem *omap4_get_scu_base(void)
{
	return NULL;
}
{
	return NULL;
}
#endif

extern void gic_dist_disable(void);
extern void gic_dist_enable(void);
extern bool gic_dist_disabled(void);
extern void gic_timer_retrigger(void);
extern void omap_smc1(u32 fn, u32 arg);
extern void __iomem *omap4_get_sar_ram_base(void);
extern void omap_do_wfi(void);

#ifdef CONFIG_SMP
/* Needed for secondary core boot */
extern void omap4_secondary_startup(void);
extern void omap4460_secondary_startup(void);
extern u32 omap_modify_auxcoreboot0(u32 set_mask, u32 clear_mask);
extern void omap_auxcoreboot_addr(u32 cpu_addr);
extern u32 omap_read_auxcoreboot0(void);

extern void omap4_cpu_die(unsigned int cpu);

extern struct smp_operations omap4_smp_ops;

extern void omap5_secondary_startup(void);
#endif

#if defined(CONFIG_SMP) && defined(CONFIG_PM)
extern int omap4_mpuss_init(void);
extern int omap4_enter_lowpower(unsigned int cpu, unsigned int power_state);
extern int omap4_finish_suspend(unsigned long cpu_state);
extern void omap4_cpu_resume(void);
extern int omap4_hotplug_cpu(unsigned int cpu, unsigned int power_state);
#else
static inline int omap4_enter_lowpower(unsigned int cpu,
					unsigned int power_state)
{
	cpu_do_idle();
	return 0;
}