#include <asm/kvm_asm.h>
#include <asm/kvm_mmu.h>

/* These are for GICv2 emulation only */
#define GICH_LR_VIRTUALID		(0x3ffUL << 0)
#define GICH_LR_PHYSID_CPUID_SHIFT	(10)
#define GICH_LR_PHYSID_CPUID		(7UL << GICH_LR_PHYSID_CPUID_SHIFT)
#define ICH_LR_VIRTUALID_MASK		(BIT_ULL(32) - 1)

static u32 ich_vtr_el2;

static struct vgic_lr vgic_v3_get_lr(const struct kvm_vcpu *vcpu, int lr)
{
	u64 val = vcpu->arch.vgic_cpu.vgic_v3.vgic_lr[lr];

	if (vcpu->kvm->arch.vgic.vgic_model == KVM_DEV_TYPE_ARM_VGIC_V3)
		lr_desc.irq = val & ICH_LR_VIRTUALID_MASK;
	else
		lr_desc.irq = val & GICH_LR_VIRTUALID;

	lr_desc.source = 0;