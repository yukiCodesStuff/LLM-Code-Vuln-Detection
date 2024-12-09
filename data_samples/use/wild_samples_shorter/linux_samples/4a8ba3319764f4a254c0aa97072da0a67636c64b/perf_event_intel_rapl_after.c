
#define RAPL_CNTR_WIDTH 32 /* 32-bit rapl counters */

#define RAPL_EVENT_ATTR_STR(_name, v, str)				\
static struct perf_pmu_events_attr event_attr_##v = {			\
	.attr		= __ATTR(_name, 0444, rapl_sysfs_show, NULL),	\
	.id		= 0,						\
	.event_str	= str,						\
};

struct rapl_pmu {
	spinlock_t	 lock;
	int		 hw_unit;  /* 1/2^hw_unit Joule */
	int		 n_active; /* number of active events */
	.attrs = rapl_pmu_attrs,
};

static ssize_t rapl_sysfs_show(struct device *dev,
			       struct device_attribute *attr,
			       char *page)
{
	struct perf_pmu_events_attr *pmu_attr = \
		container_of(attr, struct perf_pmu_events_attr, attr);

	if (pmu_attr->event_str)
		return sprintf(page, "%s", pmu_attr->event_str);

	return 0;
}

RAPL_EVENT_ATTR_STR(energy-cores, rapl_cores, "event=0x01");
RAPL_EVENT_ATTR_STR(energy-pkg  ,   rapl_pkg, "event=0x02");
RAPL_EVENT_ATTR_STR(energy-ram  ,   rapl_ram, "event=0x03");
RAPL_EVENT_ATTR_STR(energy-gpu  ,   rapl_gpu, "event=0x04");

RAPL_EVENT_ATTR_STR(energy-cores.unit, rapl_cores_unit, "Joules");
RAPL_EVENT_ATTR_STR(energy-pkg.unit  ,   rapl_pkg_unit, "Joules");
RAPL_EVENT_ATTR_STR(energy-ram.unit  ,   rapl_ram_unit, "Joules");
RAPL_EVENT_ATTR_STR(energy-gpu.unit  ,   rapl_gpu_unit, "Joules");

/*
 * we compute in 0.23 nJ increments regardless of MSR
 */
RAPL_EVENT_ATTR_STR(energy-cores.scale, rapl_cores_scale, "2.3283064365386962890625e-10");
RAPL_EVENT_ATTR_STR(energy-pkg.scale,     rapl_pkg_scale, "2.3283064365386962890625e-10");
RAPL_EVENT_ATTR_STR(energy-ram.scale,     rapl_ram_scale, "2.3283064365386962890625e-10");
RAPL_EVENT_ATTR_STR(energy-gpu.scale,     rapl_gpu_scale, "2.3283064365386962890625e-10");

static struct attribute *rapl_events_srv_attr[] = {
	EVENT_PTR(rapl_cores),
	EVENT_PTR(rapl_pkg),