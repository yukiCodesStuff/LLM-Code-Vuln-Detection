static int svm_cpu_init(int cpu)
{
	struct svm_cpu_data *sd;

	sd = kzalloc(sizeof(struct svm_cpu_data), GFP_KERNEL);
	if (!sd)
		return -ENOMEM;
	sd->cpu = cpu;
	sd->save_area = alloc_page(GFP_KERNEL);
	if (!sd->save_area)
		goto free_cpu_data;

	if (svm_sev_enabled()) {
		sd->sev_vmcbs = kmalloc_array(max_sev_asid + 1,
					      sizeof(void *),
					      GFP_KERNEL);
		if (!sd->sev_vmcbs)
			goto free_save_area;
	}

	per_cpu(svm_data, cpu) = sd;

	return 0;

free_save_area:
	__free_page(sd->save_area);
free_cpu_data:
	kfree(sd);
	return -ENOMEM;

}

static bool valid_msr_intercept(u32 index)
	kvm_mmu_set_mmio_spte_mask(mask, mask, PT_WRITABLE_MASK | PT_USER_MASK);
}

static void svm_hardware_teardown(void)
{
	int cpu;

	if (svm_sev_enabled()) {
		bitmap_free(sev_asid_bitmap);
		bitmap_free(sev_reclaim_asid_bitmap);

		sev_flush_asids();
	}

	for_each_possible_cpu(cpu)
		svm_cpu_uninit(cpu);

	__free_pages(pfn_to_page(iopm_base >> PAGE_SHIFT), IOPM_ALLOC_ORDER);
	iopm_base = 0;
}

static __init int svm_hardware_setup(void)
{
	int cpu;
	struct page *iopm_pages;
	return 0;

err:
	svm_hardware_teardown();
	return r;
}

static void init_seg(struct vmcb_seg *seg)
{
	seg->selector = 0;
	seg->attrib = SVM_SELECTOR_P_MASK | SVM_SELECTOR_S_MASK |
	struct vmcb *vmcb = svm->vmcb;
	bool activated = kvm_vcpu_apicv_active(vcpu);

	if (!avic)
		return;

	if (activated) {
		/**
		 * During AVIC temporary deactivation, guest could update
		 * APIC ID, DFR and LDR registers, which would not be trapped
	return;
}

static int svm_deliver_avic_intr(struct kvm_vcpu *vcpu, int vec)
{
	if (!vcpu->arch.apicv_active)
		return -1;

	kvm_lapic_set_irr(vec, vcpu->arch.apic);
	smp_mb__after_atomic();

	if (avic_vcpu_is_running(vcpu)) {
		put_cpu();
	} else
		kvm_vcpu_wake_up(vcpu);

	return 0;
}

static bool svm_dy_apicv_has_pending_interrupt(struct kvm_vcpu *vcpu)
{
	.cpu_has_kvm_support = has_svm,
	.disabled_by_bios = is_disabled,
	.hardware_setup = svm_hardware_setup,
	.hardware_unsetup = svm_hardware_teardown,
	.check_processor_compatibility = svm_check_processor_compat,
	.hardware_enable = svm_hardware_enable,
	.hardware_disable = svm_hardware_disable,
	.cpu_has_accelerated_tpr = svm_cpu_has_accelerated_tpr,
	.run = svm_vcpu_run,
	.handle_exit = handle_exit,
	.skip_emulated_instruction = skip_emulated_instruction,
	.update_emulated_instruction = NULL,
	.set_interrupt_shadow = svm_set_interrupt_shadow,
	.get_interrupt_shadow = svm_get_interrupt_shadow,
	.patch_hypercall = svm_patch_hypercall,
	.set_irq = svm_set_irq,