				   struct device_attribute *attr, char *buf);
extern ssize_t cpu_show_spec_store_bypass(struct device *dev,
					  struct device_attribute *attr, char *buf);

extern __printf(4, 5)
struct device *cpu_device_create(struct device *parent, void *drvdata,
				 const struct attribute_group **groups,
static inline void cpuhp_report_idle_dead(void) { }
#endif /* #ifdef CONFIG_HOTPLUG_CPU */

#endif /* _LINUX_CPU_H_ */