extern void at91_gpio_suspend(void);
extern void at91_gpio_resume(void);

#ifdef CONFIG_PINCTRL_AT91
extern void at91_pinctrl_gpio_suspend(void);
extern void at91_pinctrl_gpio_resume(void);
#else
static inline void at91_pinctrl_gpio_suspend(void) {}
static inline void at91_pinctrl_gpio_resume(void) {}
#endif

#endif	/* __ASSEMBLY__ */

#endif