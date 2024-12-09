	if (kvm_register_clock("primary cpu clock")) {
		hv_clock = NULL;
		memblock_free(mem, size);
		return;
	}
	pv_time_ops.sched_clock = kvm_clock_read;
	x86_platform.calibrate_tsc = kvm_get_tsc_khz;
	x86_platform.get_wallclock = kvm_get_wallclock;
	x86_platform.set_wallclock = kvm_set_wallclock;
#ifdef CONFIG_X86_LOCAL_APIC
	x86_cpuinit.early_percpu_clock_init =
		kvm_setup_secondary_clock;
#endif
	x86_platform.save_sched_clock_state = kvm_save_sched_clock_state;
	x86_platform.restore_sched_clock_state = kvm_restore_sched_clock_state;
	machine_ops.shutdown  = kvm_shutdown;
#ifdef CONFIG_KEXEC
	machine_ops.crash_shutdown  = kvm_crash_shutdown;
#endif
	kvm_get_preset_lpj();
	clocksource_register_hz(&kvm_clock, NSEC_PER_SEC);
	pv_info.name = "KVM";

	if (kvm_para_has_feature(KVM_FEATURE_CLOCKSOURCE_STABLE_BIT))
		pvclock_set_flags(PVCLOCK_TSC_STABLE_BIT);
}

int __init kvm_setup_vsyscall_timeinfo(void)
{
#ifdef CONFIG_X86_64
	int cpu;
	int ret;
	u8 flags;
	struct pvclock_vcpu_time_info *vcpu_time;
	unsigned int size;

	if (!hv_clock)
		return 0;

	size = PAGE_ALIGN(sizeof(struct pvclock_vsyscall_time_info)*NR_CPUS);

	cpu = get_cpu();

	vcpu_time = &hv_clock[cpu].pvti;
	flags = pvclock_read_flags(vcpu_time);

	if (!(flags & PVCLOCK_TSC_STABLE_BIT)) {
		put_cpu();
		return 1;
	}