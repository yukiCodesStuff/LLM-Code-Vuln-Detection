#include <linux/workqueue.h>

struct arch_timer_kvm {
	/* Is the timer enabled */
	bool			enabled;

	/* Virtual offset */
	cycle_t			cntvoff;
};

	/* Timer IRQ */
	struct kvm_irq_level		irq;

	/* VGIC mapping */
	struct irq_phys_map		*map;

	/* Active IRQ state caching */
	bool				active_cleared_last;
};

int kvm_timer_hyp_init(void);
void kvm_timer_enable(struct kvm *kvm);
void kvm_timer_init(struct kvm *kvm);
int kvm_timer_vcpu_reset(struct kvm_vcpu *vcpu,
			 const struct kvm_irq_level *irq);
void kvm_timer_vcpu_init(struct kvm_vcpu *vcpu);