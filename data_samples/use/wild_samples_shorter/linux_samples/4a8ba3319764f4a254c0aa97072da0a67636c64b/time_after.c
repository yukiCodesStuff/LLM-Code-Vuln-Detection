
struct xen_clock_event_device {
	struct clock_event_device evt;
	char name[16];
};
static DEFINE_PER_CPU(struct xen_clock_event_device, xen_clock_events) = { .evt.irq = -1 };

static irqreturn_t xen_timer_interrupt(int irq, void *dev_id)
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