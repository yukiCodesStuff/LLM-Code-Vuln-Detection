{
}
#endif

#ifdef CONFIG_ATH9K_LEGACY_RATE_CONTROL
int ath_rate_control_register(void);
void ath_rate_control_unregister(void);
#else
static inline int ath_rate_control_register(void)
{
	return 0;
}