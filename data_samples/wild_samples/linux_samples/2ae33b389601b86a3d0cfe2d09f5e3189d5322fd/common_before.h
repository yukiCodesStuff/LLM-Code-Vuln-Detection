{
	return 0;
}
#endif

extern void omap2_init_common_infrastructure(void);

extern void omap2_sync32k_timer_init(void);
extern void omap3_sync32k_timer_init(void);
extern void omap3_secure_sync32k_timer_init(void);
extern void omap3_gp_gptimer_timer_init(void);
extern void omap3_am33xx_gptimer_timer_init(void);
extern void omap4_local_timer_init(void);
extern void omap5_realtime_timer_init(void);

void omap2420_init_early(void);
void omap2430_init_early(void);
void omap3430_init_early(void);
void omap35xx_init_early(void);
void omap3630_init_early(void);
void omap3_init_early(void);	/* Do not use this one */
void am33xx_init_early(void);
void am35xx_init_early(void);
void ti81xx_init_early(void);
void am33xx_init_early(void);
void omap4430_init_early(void);
void omap5_init_early(void);
void omap3_init_late(void);	/* Do not use this one */
void omap4430_init_late(void);
void omap2420_init_late(void);
void omap2430_init_late(void);
void omap3430_init_late(void);
void omap35xx_init_late(void);
void omap3630_init_late(void);
void am35xx_init_late(void);
void ti81xx_init_late(void);
void omap4430_init_late(void);
int omap2_common_pm_late_init(void);

#if defined(CONFIG_SOC_OMAP2420) || defined(CONFIG_SOC_OMAP2430)
void omap2xxx_restart(char mode, const char *cmd);
#else
static inline void omap2xxx_restart(char mode, const char *cmd)
{
}