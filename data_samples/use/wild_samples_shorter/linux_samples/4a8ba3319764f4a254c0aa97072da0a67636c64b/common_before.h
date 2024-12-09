extern struct device *omap2_get_l3_device(void);
extern struct device *omap4_get_dsp_device(void);

void omap_gic_of_init(void);

#ifdef CONFIG_CACHE_L2X0
extern void __iomem *omap4_get_l2cache_base(void);
extern struct smp_operations omap4_smp_ops;

extern void omap5_secondary_startup(void);
#endif

#if defined(CONFIG_SMP) && defined(CONFIG_PM)
extern int omap4_mpuss_init(void);