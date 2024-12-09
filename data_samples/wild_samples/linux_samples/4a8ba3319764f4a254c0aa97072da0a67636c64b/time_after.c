struct xen_clock_event_device {
	struct clock_event_device evt;
	char name[16];
};
static DEFINE_PER_CPU(struct xen_clock_event_device, xen_clock_events) = { .evt.irq = -1 };

static irqreturn_t xen_timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = this_cpu_ptr(&xen_clock_events.evt);
	irqreturn_t ret;

	ret = IRQ_NONE;
	if (evt->event_handler) {
		evt->event_handler(evt);
		ret = IRQ_HANDLED;
	}

	do_stolen_accounting();

	return ret;
}

void xen_teardown_timer(int cpu)
{
	struct clock_event_device *evt;
	BUG_ON(cpu == 0);
	evt = &per_cpu(xen_clock_events, cpu).evt;

	if (evt->irq >= 0) {
		unbind_from_irqhandler(evt->irq, NULL);
		evt->irq = -1;
	}
}

void xen_setup_timer(int cpu)
{
	struct xen_clock_event_device *xevt = &per_cpu(xen_clock_events, cpu);
	struct clock_event_device *evt = &xevt->evt;
	int irq;

	WARN(evt->irq >= 0, "IRQ%d for CPU%d is already allocated\n", evt->irq, cpu);
	if (evt->irq >= 0)
		xen_teardown_timer(cpu);

	printk(KERN_INFO "installing Xen timer for CPU %d\n", cpu);

	snprintf(xevt->name, sizeof(xevt->name), "timer%d", cpu);

	irq = bind_virq_to_irqhandler(VIRQ_TIMER, cpu, xen_timer_interrupt,
				      IRQF_PERCPU|IRQF_NOBALANCING|IRQF_TIMER|
				      IRQF_FORCE_RESUME|IRQF_EARLY_RESUME,
				      xevt->name, NULL);
	(void)xen_set_irq_priority(irq, XEN_IRQ_PRIORITY_MAX);

	memcpy(evt, xen_clockevent, sizeof(*evt));

	evt->cpumask = cpumask_of(cpu);
	evt->irq = irq;
}


void xen_setup_cpu_clockevents(void)
{
	clockevents_register_device(this_cpu_ptr(&xen_clock_events.evt));
}

void xen_timer_resume(void)
{
	int cpu;

	pvclock_resume();

	if (xen_clockevent != &xen_vcpuop_clockevent)
		return;

	for_each_online_cpu(cpu) {
		if (HYPERVISOR_vcpu_op(VCPUOP_stop_periodic_timer, cpu, NULL))
			BUG();
	}
}

static const struct pv_time_ops xen_time_ops __initconst = {
	.sched_clock = xen_clocksource_read,
};

static void __init xen_time_init(void)
{
	int cpu = smp_processor_id();
	struct timespec tp;

	clocksource_register_hz(&xen_clocksource, NSEC_PER_SEC);

	if (HYPERVISOR_vcpu_op(VCPUOP_stop_periodic_timer, cpu, NULL) == 0) {
		/* Successfully turned off 100Hz tick, so we have the
		   vcpuop-based timer interface */
		printk(KERN_DEBUG "Xen: using vcpuop timer interface\n");
		xen_clockevent = &xen_vcpuop_clockevent;
	}

	/* Set initial system time with full resolution */
	xen_read_wallclock(&tp);
	do_settimeofday(&tp);

	setup_force_cpu_cap(X86_FEATURE_TSC);

	xen_setup_runstate_info(cpu);
	xen_setup_timer(cpu);
	xen_setup_cpu_clockevents();

	if (xen_initial_domain())
		pvclock_gtod_register_notifier(&xen_pvclock_gtod_notifier);
}

void __init xen_init_time_ops(void)
{
	pv_time_ops = xen_time_ops;

	x86_init.timers.timer_init = xen_time_init;
	x86_init.timers.setup_percpu_clockev = x86_init_noop;
	x86_cpuinit.setup_percpu_clockev = x86_init_noop;

	x86_platform.calibrate_tsc = xen_tsc_khz;
	x86_platform.get_wallclock = xen_get_wallclock;
	/* Dom0 uses the native method to set the hardware RTC. */
	if (!xen_initial_domain())
		x86_platform.set_wallclock = xen_set_wallclock;
}

#ifdef CONFIG_XEN_PVHVM
static void xen_hvm_setup_cpu_clockevents(void)
{
	int cpu = smp_processor_id();
	xen_setup_runstate_info(cpu);
	/*
	 * xen_setup_timer(cpu) - snprintf is bad in atomic context. Hence
	 * doing it xen_hvm_cpu_notify (which gets called by smp_init during
	 * early bootup and also during CPU hotplug events).
	 */
	xen_setup_cpu_clockevents();
}

void __init xen_hvm_init_time_ops(void)
{
	/* vector callback is needed otherwise we cannot receive interrupts
	 * on cpu > 0 and at this point we don't know how many cpus are
	 * available */
	if (!xen_have_vector_callback)
		return;
	if (!xen_feature(XENFEAT_hvm_safe_pvclock)) {
		printk(KERN_INFO "Xen doesn't support pvclock on HVM,"
				"disable pv timer\n");
		return;
	}

	pv_time_ops = xen_time_ops;
	x86_init.timers.setup_percpu_clockev = xen_time_init;
	x86_cpuinit.setup_percpu_clockev = xen_hvm_setup_cpu_clockevents;

	x86_platform.calibrate_tsc = xen_tsc_khz;
	x86_platform.get_wallclock = xen_get_wallclock;
	x86_platform.set_wallclock = xen_set_wallclock;
}
#endif
	if (evt->irq >= 0) {
		unbind_from_irqhandler(evt->irq, NULL);
		evt->irq = -1;
	}
}

void xen_setup_timer(int cpu)
{
	struct xen_clock_event_device *xevt = &per_cpu(xen_clock_events, cpu);
	struct clock_event_device *evt = &xevt->evt;
	int irq;

	WARN(evt->irq >= 0, "IRQ%d for CPU%d is already allocated\n", evt->irq, cpu);
	if (evt->irq >= 0)
		xen_teardown_timer(cpu);

	printk(KERN_INFO "installing Xen timer for CPU %d\n", cpu);

	snprintf(xevt->name, sizeof(xevt->name), "timer%d", cpu);

	irq = bind_virq_to_irqhandler(VIRQ_TIMER, cpu, xen_timer_interrupt,
				      IRQF_PERCPU|IRQF_NOBALANCING|IRQF_TIMER|
				      IRQF_FORCE_RESUME|IRQF_EARLY_RESUME,
				      xevt->name, NULL);
	(void)xen_set_irq_priority(irq, XEN_IRQ_PRIORITY_MAX);

	memcpy(evt, xen_clockevent, sizeof(*evt));

	evt->cpumask = cpumask_of(cpu);
	evt->irq = irq;
}


void xen_setup_cpu_clockevents(void)
{
	clockevents_register_device(this_cpu_ptr(&xen_clock_events.evt));
}

void xen_timer_resume(void)
{
	int cpu;

	pvclock_resume();

	if (xen_clockevent != &xen_vcpuop_clockevent)
		return;

	for_each_online_cpu(cpu) {
		if (HYPERVISOR_vcpu_op(VCPUOP_stop_periodic_timer, cpu, NULL))
			BUG();
	}
}

static const struct pv_time_ops xen_time_ops __initconst = {
	.sched_clock = xen_clocksource_read,
};

static void __init xen_time_init(void)
{
	int cpu = smp_processor_id();
	struct timespec tp;

	clocksource_register_hz(&xen_clocksource, NSEC_PER_SEC);

	if (HYPERVISOR_vcpu_op(VCPUOP_stop_periodic_timer, cpu, NULL) == 0) {
		/* Successfully turned off 100Hz tick, so we have the
		   vcpuop-based timer interface */
		printk(KERN_DEBUG "Xen: using vcpuop timer interface\n");
		xen_clockevent = &xen_vcpuop_clockevent;
	}