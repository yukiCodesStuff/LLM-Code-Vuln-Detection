__le32 il_add_beacon_time(struct il_priv *il, u32 base, u32 addon,
			  u32 beacon_interval);

#ifdef CONFIG_PM
extern const struct dev_pm_ops il_pm_ops;

#define IL_LEGACY_PM_OPS	(&il_pm_ops)

#else /* !CONFIG_PM */

#define IL_LEGACY_PM_OPS	NULL

#endif /* !CONFIG_PM */

/*****************************************************
*  Error Handling Debugging
******************************************************/