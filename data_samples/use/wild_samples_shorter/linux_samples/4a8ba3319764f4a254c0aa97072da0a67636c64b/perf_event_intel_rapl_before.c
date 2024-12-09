
#define RAPL_CNTR_WIDTH 32 /* 32-bit rapl counters */

struct rapl_pmu {
	spinlock_t	 lock;
	int		 hw_unit;  /* 1/2^hw_unit Joule */
	int		 n_active; /* number of active events */
	.attrs = rapl_pmu_attrs,
};

EVENT_ATTR_STR(energy-cores, rapl_cores, "event=0x01");
EVENT_ATTR_STR(energy-pkg  ,   rapl_pkg, "event=0x02");
EVENT_ATTR_STR(energy-ram  ,   rapl_ram, "event=0x03");
EVENT_ATTR_STR(energy-gpu  ,   rapl_gpu, "event=0x04");

EVENT_ATTR_STR(energy-cores.unit, rapl_cores_unit, "Joules");
EVENT_ATTR_STR(energy-pkg.unit  ,   rapl_pkg_unit, "Joules");
EVENT_ATTR_STR(energy-ram.unit  ,   rapl_ram_unit, "Joules");
EVENT_ATTR_STR(energy-gpu.unit  ,   rapl_gpu_unit, "Joules");

/*
 * we compute in 0.23 nJ increments regardless of MSR
 */
EVENT_ATTR_STR(energy-cores.scale, rapl_cores_scale, "2.3283064365386962890625e-10");
EVENT_ATTR_STR(energy-pkg.scale,     rapl_pkg_scale, "2.3283064365386962890625e-10");
EVENT_ATTR_STR(energy-ram.scale,     rapl_ram_scale, "2.3283064365386962890625e-10");
EVENT_ATTR_STR(energy-gpu.scale,     rapl_gpu_scale, "2.3283064365386962890625e-10");

static struct attribute *rapl_events_srv_attr[] = {
	EVENT_PTR(rapl_cores),
	EVENT_PTR(rapl_pkg),