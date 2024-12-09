void omap3630_init_late(void);
void am35xx_init_late(void);
void ti81xx_init_late(void);
void omap4430_init_late(void);
int omap2_common_pm_late_init(void);

#if defined(CONFIG_SOC_OMAP2420) || defined(CONFIG_SOC_OMAP2430)
void omap2xxx_restart(char mode, const char *cmd);