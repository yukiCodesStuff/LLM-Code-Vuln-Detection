				   struct device_attribute *attr, char *buf);
extern ssize_t cpu_show_spec_store_bypass(struct device *dev,
					  struct device_attribute *attr, char *buf);
extern ssize_t cpu_show_l1tf(struct device *dev,
			     struct device_attribute *attr, char *buf);

extern __printf(4, 5)
struct device *cpu_device_create(struct device *parent, void *drvdata,
				 const struct attribute_group **groups,
static inline void cpuhp_report_idle_dead(void) { }
#endif /* #ifdef CONFIG_HOTPLUG_CPU */

enum cpuhp_smt_control {
	CPU_SMT_ENABLED,
	CPU_SMT_DISABLED,
	CPU_SMT_FORCE_DISABLED,
	CPU_SMT_NOT_SUPPORTED,
};

#if defined(CONFIG_SMP) && defined(CONFIG_HOTPLUG_SMT)
extern enum cpuhp_smt_control cpu_smt_control;
extern void cpu_smt_disable(bool force);
extern void cpu_smt_check_topology_early(void);
extern void cpu_smt_check_topology(void);
#else
# define cpu_smt_control		(CPU_SMT_ENABLED)
static inline void cpu_smt_disable(bool force) { }
static inline void cpu_smt_check_topology_early(void) { }
static inline void cpu_smt_check_topology(void) { }
#endif

#endif /* _LINUX_CPU_H_ */