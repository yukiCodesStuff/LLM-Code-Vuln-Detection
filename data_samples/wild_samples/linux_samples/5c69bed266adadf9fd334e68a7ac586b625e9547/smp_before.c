{
	play_dead_common();
	HYPERVISOR_vcpu_op(VCPUOP_down, smp_processor_id(), NULL);
	cpu_bringup();
	/*
	 * Balance out the preempt calls - as we are running in cpu_idle
	 * loop which has been called at bootup from cpu_bringup_and_idle.
	 * The cpucpu_bringup_and_idle called cpu_bringup which made a
	 * preempt_disable() So this preempt_enable will balance it out.
	 */
	preempt_enable();
}

#else /* !CONFIG_HOTPLUG_CPU */
static int xen_cpu_disable(void)
{
	return -ENOSYS;
}

static void xen_cpu_die(unsigned int cpu)
{
	BUG();
}

static void xen_play_dead(void)
{
	BUG();
}

#endif
static void stop_self(void *v)
{
	int cpu = smp_processor_id();

	/* make sure we're not pinning something down */
	load_cr3(swapper_pg_dir);
	/* should set up a minimal gdt */

	set_cpu_online(cpu, false);

	HYPERVISOR_vcpu_op(VCPUOP_down, cpu, NULL);
	BUG();
}

static void xen_stop_other_cpus(int wait)
{
	smp_call_function(stop_self, NULL, wait);
}

static void xen_smp_send_reschedule(int cpu)
{
	xen_send_IPI_one(cpu, XEN_RESCHEDULE_VECTOR);
}

static void __xen_send_IPI_mask(const struct cpumask *mask,
			      int vector)
{
	unsigned cpu;

	for_each_cpu_and(cpu, mask, cpu_online_mask)
		xen_send_IPI_one(cpu, vector);
}

static void xen_smp_send_call_function_ipi(const struct cpumask *mask)
{
	int cpu;

	__xen_send_IPI_mask(mask, XEN_CALL_FUNCTION_VECTOR);

	/* Make sure other vcpus get a chance to run if they need to. */
	for_each_cpu(cpu, mask) {
		if (xen_vcpu_stolen(cpu)) {
			HYPERVISOR_sched_op(SCHEDOP_yield, NULL);
			break;
		}
	}