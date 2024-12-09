#include <linux/workqueue.h>

struct arch_timer_kvm {
	/* Virtual offset */
	cycle_t			cntvoff;
};

	/* Timer IRQ */
	struct kvm_irq_level		irq;

	/* Active IRQ state caching */
	bool				active_cleared_last;

	/* Is the timer enabled */
	bool			enabled;
};

int kvm_timer_hyp_init(void);
int kvm_timer_enable(struct kvm_vcpu *vcpu);
void kvm_timer_init(struct kvm *kvm);
int kvm_timer_vcpu_reset(struct kvm_vcpu *vcpu,
			 const struct kvm_irq_level *irq);
void kvm_timer_vcpu_init(struct kvm_vcpu *vcpu);