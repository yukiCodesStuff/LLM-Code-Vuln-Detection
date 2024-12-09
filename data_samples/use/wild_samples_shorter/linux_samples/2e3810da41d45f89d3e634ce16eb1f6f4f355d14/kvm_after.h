
#if defined(CONFIG_HAVE_KVM_IRQFD)

#ifdef kvm_irqchips
#define kvm_ack_irq_string "irqchip %s pin %u"
#define kvm_ack_irq_parm  __print_symbolic(__entry->irqchip, kvm_irqchips), __entry->pin
#else
#define kvm_ack_irq_string "irqchip %d pin %u"
#define kvm_ack_irq_parm  __entry->irqchip, __entry->pin
#endif

TRACE_EVENT(kvm_ack_irq,
	TP_PROTO(unsigned int irqchip, unsigned int pin),
	TP_ARGS(irqchip, pin),

		__entry->pin		= pin;
	),

	TP_printk(kvm_ack_irq_string, kvm_ack_irq_parm)
);

#endif /* defined(CONFIG_HAVE_KVM_IRQFD) */
