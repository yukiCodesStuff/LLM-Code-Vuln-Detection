{
	unsigned i;

	for (i = 0; i < max_cpus; ++i)
		set_cpu_present(i, true);
}

void __init smp_init_cpus(void)
{
	unsigned i;
	unsigned int ncpus = get_core_count();
	unsigned int core_id = get_core_id();

	pr_info("%s: Core Count = %d\n", __func__, ncpus);
	pr_info("%s: Core Id = %d\n", __func__, core_id);

	for (i = 0; i < ncpus; ++i)
		set_cpu_possible(i, true);
}

void __init smp_prepare_boot_cpu(void)
{
	unsigned int cpu = smp_processor_id();
	BUG_ON(cpu != 0);
	cpu_asid_cache(cpu) = ASID_USER_FIRST;
}

void __init smp_cpus_done(unsigned int max_cpus)
{
}

static int boot_secondary_processors = 1; /* Set with xt-gdb via .xt-gdb */
static DECLARE_COMPLETION(cpu_running);

void secondary_start_kernel(void)
{
	struct mm_struct *mm = &init_mm;
	unsigned int cpu = smp_processor_id();

	init_mmu();

#ifdef CONFIG_DEBUG_KERNEL
	if (boot_secondary_processors == 0) {
		pr_debug("%s: boot_secondary_processors:%d; Hanging cpu:%d\n",
			__func__, boot_secondary_processors, cpu);
		for (;;)
			__asm__ __volatile__ ("waiti " __stringify(LOCKLEVEL));
	}
{
	unsigned i;
	unsigned int ncpus = get_core_count();
	unsigned int core_id = get_core_id();

	pr_info("%s: Core Count = %d\n", __func__, ncpus);
	pr_info("%s: Core Id = %d\n", __func__, core_id);

	for (i = 0; i < ncpus; ++i)
		set_cpu_possible(i, true);
}

void __init smp_prepare_boot_cpu(void)
{
	unsigned int cpu = smp_processor_id();
	BUG_ON(cpu != 0);
	cpu_asid_cache(cpu) = ASID_USER_FIRST;
}

void __init smp_cpus_done(unsigned int max_cpus)
{
}

static int boot_secondary_processors = 1; /* Set with xt-gdb via .xt-gdb */
static DECLARE_COMPLETION(cpu_running);

void secondary_start_kernel(void)
{
	struct mm_struct *mm = &init_mm;
	unsigned int cpu = smp_processor_id();

	init_mmu();

#ifdef CONFIG_DEBUG_KERNEL
	if (boot_secondary_processors == 0) {
		pr_debug("%s: boot_secondary_processors:%d; Hanging cpu:%d\n",
			__func__, boot_secondary_processors, cpu);
		for (;;)
			__asm__ __volatile__ ("waiti " __stringify(LOCKLEVEL));
	}
{
	unsigned long timeout = jiffies + msecs_to_jiffies(1000);
	unsigned long ccount;
	int i;

#ifdef CONFIG_HOTPLUG_CPU
	cpu_start_id = cpu;
	system_flush_invalidate_dcache_range(
			(unsigned long)&cpu_start_id, sizeof(cpu_start_id));
#endif
	smp_call_function_single(0, mx_cpu_start, (void *)cpu, 1);

	for (i = 0; i < 2; ++i) {
		do
			ccount = get_ccount();
		while (!ccount);

		cpu_start_ccount = ccount;

		while (time_before(jiffies, timeout)) {
			mb();
			if (!cpu_start_ccount)
				break;
		}

		if (cpu_start_ccount) {
			smp_call_function_single(0, mx_cpu_stop,
					(void *)cpu, 1);
			cpu_start_ccount = 0;
			return -EIO;
		}
	}
	for (i = 0; i < 2; ++i) {
		do
			ccount = get_ccount();
		while (!ccount);

		cpu_start_ccount = ccount;

		while (time_before(jiffies, timeout)) {
			mb();
			if (!cpu_start_ccount)
				break;
		}
{
	int ret = 0;

	if (cpu_asid_cache(cpu) == 0)
		cpu_asid_cache(cpu) = ASID_USER_FIRST;

	start_info.stack = (unsigned long)task_pt_regs(idle);
	wmb();

	pr_debug("%s: Calling wakeup_secondary(cpu:%d, idle:%p, sp: %08lx)\n",
			__func__, cpu, idle, start_info.stack);

	ret = boot_secondary(cpu, idle);
	if (ret == 0) {
		wait_for_completion_timeout(&cpu_running,
				msecs_to_jiffies(1000));
		if (!cpu_online(cpu))
			ret = -EIO;
	}
	while (time_before(jiffies, timeout)) {
		system_invalidate_dcache_range((unsigned long)&cpu_start_id,
				sizeof(cpu_start_id));
		if (cpu_start_id == -cpu) {
			platform_cpu_kill(cpu);
			return;
		}
	}