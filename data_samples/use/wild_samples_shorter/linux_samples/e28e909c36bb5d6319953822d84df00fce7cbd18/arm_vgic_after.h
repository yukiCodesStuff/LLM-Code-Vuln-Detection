#ifndef __ASM_ARM_KVM_VGIC_H
#define __ASM_ARM_KVM_VGIC_H

#ifdef CONFIG_KVM_NEW_VGIC
#include <kvm/vgic/vgic.h>
#else

#include <linux/kernel.h>
#include <linux/kvm.h>
#include <linux/irqreturn.h>
#include <linux/spinlock.h>
struct irq_phys_map {
	u32			virt_irq;
	u32			phys_irq;
};

struct irq_phys_map_entry {
	struct list_head	entry;
	unsigned long   *active_shared;
	unsigned long   *pend_act_shared;

	/* CPU vif control registers for world switch */
	union {
		struct vgic_v2_cpu_if	vgic_v2;
		struct vgic_v3_cpu_if	vgic_v3;
int kvm_vgic_inject_irq(struct kvm *kvm, int cpuid, unsigned int irq_num,
			bool level);
int kvm_vgic_inject_mapped_irq(struct kvm *kvm, int cpuid,
			       unsigned int virt_irq, bool level);
void vgic_v3_dispatch_sgi(struct kvm_vcpu *vcpu, u64 reg);
int kvm_vgic_vcpu_pending_irq(struct kvm_vcpu *vcpu);
int kvm_vgic_map_phys_irq(struct kvm_vcpu *vcpu, int virt_irq, int phys_irq);
int kvm_vgic_unmap_phys_irq(struct kvm_vcpu *vcpu, unsigned int virt_irq);
bool kvm_vgic_map_is_active(struct kvm_vcpu *vcpu, unsigned int virt_irq);

#define irqchip_in_kernel(k)	(!!((k)->arch.vgic.in_kernel))
#define vgic_initialized(k)	(!!((k)->arch.vgic.nr_cpus))
#define vgic_ready(k)		((k)->arch.vgic.ready)
#define vgic_valid_spi(k, i)	(((i) >= VGIC_NR_PRIVATE_IRQS) && \
				 ((i) < (k)->arch.vgic.nr_irqs))

int vgic_v2_probe(const struct gic_kvm_info *gic_kvm_info,
		  const struct vgic_ops **ops,
		  const struct vgic_params **params);
}
#endif

#endif	/* old VGIC include */
#endif