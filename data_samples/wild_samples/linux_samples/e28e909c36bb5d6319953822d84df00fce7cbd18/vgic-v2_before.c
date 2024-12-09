static const struct vgic_ops vgic_v2_ops = {
	.get_lr			= vgic_v2_get_lr,
	.set_lr			= vgic_v2_set_lr,
	.get_elrsr		= vgic_v2_get_elrsr,
	.get_eisr		= vgic_v2_get_eisr,
	.clear_eisr		= vgic_v2_clear_eisr,
	.get_interrupt_status	= vgic_v2_get_interrupt_status,
	.enable_underflow	= vgic_v2_enable_underflow,
	.disable_underflow	= vgic_v2_disable_underflow,
	.get_vmcr		= vgic_v2_get_vmcr,
	.set_vmcr		= vgic_v2_set_vmcr,
	.enable			= vgic_v2_enable,
};

static struct vgic_params vgic_v2_params;

static void vgic_cpu_init_lrs(void *params)
{
	struct vgic_params *vgic = params;
	int i;

	for (i = 0; i < vgic->nr_lr; i++)
		writel_relaxed(0, vgic->vctrl_base + GICH_LR0 + (i * 4));
}
{
	int ret;
	struct vgic_params *vgic = &vgic_v2_params;
	const struct resource *vctrl_res = &gic_kvm_info->vctrl;
	const struct resource *vcpu_res = &gic_kvm_info->vcpu;

	if (!gic_kvm_info->maint_irq) {
		kvm_err("error getting vgic maintenance irq\n");
		ret = -ENXIO;
		goto out;
	}