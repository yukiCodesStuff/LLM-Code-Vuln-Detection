__le32 il_add_beacon_time(struct il_priv *il, u32 base, u32 addon,
			  u32 beacon_interval);

#ifdef CONFIG_PM_SLEEP
extern const struct dev_pm_ops il_pm_ops;

#define IL_LEGACY_PM_OPS	(&il_pm_ops)

#else /* !CONFIG_PM_SLEEP */

#define IL_LEGACY_PM_OPS	NULL

#endif /* !CONFIG_PM_SLEEP */

/*****************************************************
*  Error Handling Debugging
******************************************************/