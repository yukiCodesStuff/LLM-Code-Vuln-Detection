#include <linux/kvm.h>
#include <linux/kvm_host.h>
#include <linux/interrupt.h>

#include <clocksource/arm_arch_timer.h>
#include <asm/arch_timer.h>


	timer->active_cleared_last = false;
	timer->irq.level = new_level;
	trace_kvm_timer_update_irq(vcpu->vcpu_id, timer->map->virt_irq,
				   timer->irq.level);
	ret = kvm_vgic_inject_mapped_irq(vcpu->kvm, vcpu->vcpu_id,
					 timer->map,
					 timer->irq.level);
	WARN_ON(ret);
}

	 * because the guest would never see the interrupt.  Instead wait
	 * until we call this function from kvm_timer_flush_hwstate.
	 */
	if (!vgic_initialized(vcpu->kvm))
		return -ENODEV;

	if (kvm_timer_should_fire(vcpu) != timer->irq.level)
		kvm_timer_update_irq(vcpu, !timer->irq.level);
	* to ensure that hardware interrupts from the timer triggers a guest
	* exit.
	*/
	if (timer->irq.level || kvm_vgic_map_is_active(vcpu, timer->map))
		phys_active = true;
	else
		phys_active = false;

	/*
	 * We want to avoid hitting the (re)distributor as much as
	 * possible, as this is a potentially expensive MMIO access
	if (timer->active_cleared_last && !phys_active)
		return;

	ret = irq_set_irqchip_state(timer->map->irq,
				    IRQCHIP_STATE_ACTIVE,
				    phys_active);
	WARN_ON(ret);

			 const struct kvm_irq_level *irq)
{
	struct arch_timer_cpu *timer = &vcpu->arch.timer_cpu;
	struct irq_phys_map *map;

	/*
	 * The vcpu timer irq number cannot be determined in
	 * kvm_timer_vcpu_init() because it is called much before
	timer->cntv_ctl = 0;
	kvm_timer_update_state(vcpu);

	/*
	 * Tell the VGIC that the virtual interrupt is tied to a
	 * physical interrupt. We do that once per VCPU.
	 */
	map = kvm_vgic_map_phys_irq(vcpu, irq->irq, host_vtimer_irq);
	if (WARN_ON(IS_ERR(map)))
		return PTR_ERR(map);

	timer->map = map;
	return 0;
}

void kvm_timer_vcpu_init(struct kvm_vcpu *vcpu)
	struct arch_timer_cpu *timer = &vcpu->arch.timer_cpu;

	timer_disarm(timer);
	if (timer->map)
		kvm_vgic_unmap_phys_irq(vcpu, timer->map);
}

void kvm_timer_enable(struct kvm *kvm)
{
	if (kvm->arch.timer.enabled)
		return;

	/*
	 * There is a potential race here between VCPUs starting for the first
	 * time, which may be enabling the timer multiple times.  That doesn't
	 * the arch timers are enabled.
	 */
	if (timecounter && wqueue)
		kvm->arch.timer.enabled = 1;
}

void kvm_timer_init(struct kvm *kvm)
{