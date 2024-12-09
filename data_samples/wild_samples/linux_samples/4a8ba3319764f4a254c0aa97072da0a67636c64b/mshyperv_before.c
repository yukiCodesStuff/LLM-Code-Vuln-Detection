static struct clocksource hyperv_cs = {
	.name		= "hyperv_clocksource",
	.rating		= 400, /* use this when running on Hyperv*/
	.read		= read_hv_clock,
	.mask		= CLOCKSOURCE_MASK(64),
};

static void __init ms_hyperv_init_platform(void)
{
	/*
	 * Extract the features and hints
	 */
	ms_hyperv.features = cpuid_eax(HYPERV_CPUID_FEATURES);
	ms_hyperv.hints    = cpuid_eax(HYPERV_CPUID_ENLIGHTMENT_INFO);

	printk(KERN_INFO "HyperV: features 0x%x, hints 0x%x\n",
	       ms_hyperv.features, ms_hyperv.hints);

#ifdef CONFIG_X86_LOCAL_APIC
	if (ms_hyperv.features & HV_X64_MSR_APIC_FREQUENCY_AVAILABLE) {
		/*
		 * Get the APIC frequency.
		 */
		u64	hv_lapic_frequency;

		rdmsrl(HV_X64_MSR_APIC_FREQUENCY, hv_lapic_frequency);
		hv_lapic_frequency = div_u64(hv_lapic_frequency, HZ);
		lapic_timer_frequency = hv_lapic_frequency;
		printk(KERN_INFO "HyperV: LAPIC Timer Frequency: %#x\n",
				lapic_timer_frequency);
	}
#endif

	if (ms_hyperv.features & HV_X64_MSR_TIME_REF_COUNT_AVAILABLE)
		clocksource_register_hz(&hyperv_cs, NSEC_PER_SEC/100);

#ifdef CONFIG_X86_IO_APIC
	no_timer_check = 1;
#endif

}