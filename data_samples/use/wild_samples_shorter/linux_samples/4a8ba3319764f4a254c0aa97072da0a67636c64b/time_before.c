
struct xen_clock_event_device {
	struct clock_event_device evt;
	char *name;
};
static DEFINE_PER_CPU(struct xen_clock_event_device, xen_clock_events) = { .evt.irq = -1 };

static irqreturn_t xen_timer_interrupt(int irq, void *dev_id)
	if (evt->irq >= 0) {
		unbind_from_irqhandler(evt->irq, NULL);
		evt->irq = -1;
		kfree(per_cpu(xen_clock_events, cpu).name);
		per_cpu(xen_clock_events, cpu).name = NULL;
	}
}

void xen_setup_timer(int cpu)
{
	char *name;
	struct clock_event_device *evt;
	int irq;

	evt = &per_cpu(xen_clock_events, cpu).evt;
	WARN(evt->irq >= 0, "IRQ%d for CPU%d is already allocated\n", evt->irq, cpu);
	if (evt->irq >= 0)
		xen_teardown_timer(cpu);

	printk(KERN_INFO "installing Xen timer for CPU %d\n", cpu);

	name = kasprintf(GFP_KERNEL, "timer%d", cpu);
	if (!name)
		name = "<timer kasprintf failed>";

	irq = bind_virq_to_irqhandler(VIRQ_TIMER, cpu, xen_timer_interrupt,
				      IRQF_PERCPU|IRQF_NOBALANCING|IRQF_TIMER|
				      IRQF_FORCE_RESUME|IRQF_EARLY_RESUME,
				      name, NULL);
	(void)xen_set_irq_priority(irq, XEN_IRQ_PRIORITY_MAX);

	memcpy(evt, xen_clockevent, sizeof(*evt));

	evt->cpumask = cpumask_of(cpu);
	evt->irq = irq;
	per_cpu(xen_clock_events, cpu).name = name;
}


void xen_setup_cpu_clockevents(void)
{
	BUG_ON(preemptible());

	clockevents_register_device(this_cpu_ptr(&xen_clock_events.evt));
}

void xen_timer_resume(void)