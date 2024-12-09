	play_dead_common();
	HYPERVISOR_vcpu_op(VCPUOP_down, smp_processor_id(), NULL);
	cpu_bringup();
}

#else /* !CONFIG_HOTPLUG_CPU */
static int xen_cpu_disable(void)