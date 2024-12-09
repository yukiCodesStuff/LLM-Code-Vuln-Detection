	.enable			= vgic_v2_enable,
};

static struct vgic_params vgic_v2_params;

static void vgic_cpu_init_lrs(void *params)
{
	struct vgic_params *vgic = params;
	const struct resource *vctrl_res = &gic_kvm_info->vctrl;
	const struct resource *vcpu_res = &gic_kvm_info->vcpu;

	if (!gic_kvm_info->maint_irq) {
		kvm_err("error getting vgic maintenance irq\n");
		ret = -ENXIO;
		goto out;