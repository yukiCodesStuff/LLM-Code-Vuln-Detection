
#include <asm/kvm_hyp.h>

#ifdef CONFIG_KVM_NEW_VGIC
extern struct vgic_global kvm_vgic_global_state;
#define vgic_v2_params kvm_vgic_global_state
#else
extern struct vgic_params vgic_v2_params;
#endif

static void __hyp_text save_maint_int_state(struct kvm_vcpu *vcpu,
					    void __iomem *base)
{
	struct vgic_v2_cpu_if *cpu_if = &vcpu->arch.vgic_cpu.vgic_v2;
	int nr_lr = (kern_hyp_va(&vgic_v2_params))->nr_lr;
	u32 eisr0, eisr1;
	int i;
	bool expect_mi;

static void __hyp_text save_elrsr(struct kvm_vcpu *vcpu, void __iomem *base)
{
	struct vgic_v2_cpu_if *cpu_if = &vcpu->arch.vgic_cpu.vgic_v2;
	int nr_lr = (kern_hyp_va(&vgic_v2_params))->nr_lr;
	u32 elrsr0, elrsr1;

	elrsr0 = readl_relaxed(base + GICH_ELRSR0);
	if (unlikely(nr_lr > 32))
static void __hyp_text save_lrs(struct kvm_vcpu *vcpu, void __iomem *base)
{
	struct vgic_v2_cpu_if *cpu_if = &vcpu->arch.vgic_cpu.vgic_v2;
	int nr_lr = (kern_hyp_va(&vgic_v2_params))->nr_lr;
	int i;

	for (i = 0; i < nr_lr; i++) {
		if (!(vcpu->arch.vgic_cpu.live_lrs & (1UL << i)))
	struct vgic_v2_cpu_if *cpu_if = &vcpu->arch.vgic_cpu.vgic_v2;
	struct vgic_dist *vgic = &kvm->arch.vgic;
	void __iomem *base = kern_hyp_va(vgic->vctrl_base);
	int nr_lr = (kern_hyp_va(&vgic_v2_params))->nr_lr;
	int i;
	u64 live_lrs = 0;

	if (!base)
		return;


	for (i = 0; i < nr_lr; i++)
		if (cpu_if->vgic_lr[i] & GICH_LR_STATE)
			live_lrs |= 1UL << i;