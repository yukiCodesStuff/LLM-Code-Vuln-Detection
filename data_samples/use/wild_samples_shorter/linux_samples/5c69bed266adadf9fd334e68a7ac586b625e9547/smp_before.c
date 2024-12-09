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