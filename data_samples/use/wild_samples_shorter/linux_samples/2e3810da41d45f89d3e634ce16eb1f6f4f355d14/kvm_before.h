
#if defined(CONFIG_HAVE_KVM_IRQFD)

TRACE_EVENT(kvm_ack_irq,
	TP_PROTO(unsigned int irqchip, unsigned int pin),
	TP_ARGS(irqchip, pin),

		__entry->pin		= pin;
	),

#ifdef kvm_irqchips
	TP_printk("irqchip %s pin %u",
		  __print_symbolic(__entry->irqchip, kvm_irqchips),
		 __entry->pin)
#else
	TP_printk("irqchip %d pin %u", __entry->irqchip, __entry->pin)
#endif
);

#endif /* defined(CONFIG_HAVE_KVM_IRQFD) */
