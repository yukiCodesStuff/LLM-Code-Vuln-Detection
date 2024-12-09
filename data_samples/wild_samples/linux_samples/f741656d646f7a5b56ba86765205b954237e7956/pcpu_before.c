	struct xen_platform_op op = {
		.cmd                   = XENPF_get_cpuinfo,
		.interface_version     = XENPF_INTERFACE_VERSION,
		.u.pcpu_info.xen_cpuid = cpu,
	};

	ret = HYPERVISOR_dom0_op(&op);
	if (ret)
		return ret;

	info = &op.u.pcpu_info;
	if (max_cpu)
		*max_cpu = info->max_present;

	pcpu = get_pcpu(cpu);

	/*
	 * Only those at cpu present map has its sys interface.
	 */
	if (info->flags & XEN_PCPU_FLAGS_INVALID) {
		if (pcpu)
			unregister_and_remove_pcpu(pcpu);
		return 0;
	}