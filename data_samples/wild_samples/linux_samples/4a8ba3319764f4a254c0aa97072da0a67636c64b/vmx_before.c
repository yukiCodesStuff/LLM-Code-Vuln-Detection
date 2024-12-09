{
	int r = -ENOMEM, i, msr;

	rdmsrl_safe(MSR_EFER, &host_efer);

	for (i = 0; i < ARRAY_SIZE(vmx_msr_index); ++i)
		kvm_define_shared_msr(i, vmx_msr_index[i]);

	vmx_io_bitmap_a = (unsigned long *)__get_free_page(GFP_KERNEL);
	if (!vmx_io_bitmap_a)
		return r;

	vmx_io_bitmap_b = (unsigned long *)__get_free_page(GFP_KERNEL);
	if (!vmx_io_bitmap_b)
		goto out;

	vmx_msr_bitmap_legacy = (unsigned long *)__get_free_page(GFP_KERNEL);
	if (!vmx_msr_bitmap_legacy)
		goto out1;

	vmx_msr_bitmap_legacy_x2apic =
				(unsigned long *)__get_free_page(GFP_KERNEL);
	if (!vmx_msr_bitmap_legacy_x2apic)
		goto out2;

	vmx_msr_bitmap_longmode = (unsigned long *)__get_free_page(GFP_KERNEL);
	if (!vmx_msr_bitmap_longmode)
		goto out3;

	vmx_msr_bitmap_longmode_x2apic =
				(unsigned long *)__get_free_page(GFP_KERNEL);
	if (!vmx_msr_bitmap_longmode_x2apic)
		goto out4;
	vmx_vmread_bitmap = (unsigned long *)__get_free_page(GFP_KERNEL);
	if (!vmx_vmread_bitmap)
		goto out5;

	vmx_vmwrite_bitmap = (unsigned long *)__get_free_page(GFP_KERNEL);
	if (!vmx_vmwrite_bitmap)
		goto out6;

	memset(vmx_vmread_bitmap, 0xff, PAGE_SIZE);
	memset(vmx_vmwrite_bitmap, 0xff, PAGE_SIZE);

	/*
	 * Allow direct access to the PC debug port (it is often used for I/O
	 * delays, but the vmexits simply slow things down).
	 */
	memset(vmx_io_bitmap_a, 0xff, PAGE_SIZE);
	clear_bit(0x80, vmx_io_bitmap_a);

	memset(vmx_io_bitmap_b, 0xff, PAGE_SIZE);

	memset(vmx_msr_bitmap_legacy, 0xff, PAGE_SIZE);
	memset(vmx_msr_bitmap_longmode, 0xff, PAGE_SIZE);

	vmx_disable_intercept_for_msr(MSR_FS_BASE, false);
	vmx_disable_intercept_for_msr(MSR_GS_BASE, false);
	vmx_disable_intercept_for_msr(MSR_KERNEL_GS_BASE, true);
	vmx_disable_intercept_for_msr(MSR_IA32_SYSENTER_CS, false);
	vmx_disable_intercept_for_msr(MSR_IA32_SYSENTER_ESP, false);
	vmx_disable_intercept_for_msr(MSR_IA32_SYSENTER_EIP, false);
	vmx_disable_intercept_for_msr(MSR_IA32_BNDCFGS, true);

	memcpy(vmx_msr_bitmap_legacy_x2apic,
			vmx_msr_bitmap_legacy, PAGE_SIZE);
	memcpy(vmx_msr_bitmap_longmode_x2apic,
			vmx_msr_bitmap_longmode, PAGE_SIZE);

	if (enable_apicv) {
		for (msr = 0x800; msr <= 0x8ff; msr++)
			vmx_disable_intercept_msr_read_x2apic(msr);

		/* According SDM, in x2apic mode, the whole id reg is used.
		 * But in KVM, it only use the highest eight bits. Need to
		 * intercept it */
		vmx_enable_intercept_msr_read_x2apic(0x802);
		/* TMCCT */
		vmx_enable_intercept_msr_read_x2apic(0x839);
		/* TPR */
		vmx_disable_intercept_msr_write_x2apic(0x808);
		/* EOI */
		vmx_disable_intercept_msr_write_x2apic(0x80b);
		/* SELF-IPI */
		vmx_disable_intercept_msr_write_x2apic(0x83f);
	}
	else {
		kvm_x86_ops->hwapic_irr_update = NULL;
		kvm_x86_ops->deliver_posted_interrupt = NULL;
		kvm_x86_ops->sync_pir_to_irr = vmx_sync_pir_to_irr_dummy;
	}

	if (nested)
		nested_vmx_setup_ctls_msrs();

	return alloc_kvm_area();

out7:
	free_page((unsigned long)vmx_vmwrite_bitmap);
out6:
	free_page((unsigned long)vmx_vmread_bitmap);
out5:
	free_page((unsigned long)vmx_msr_bitmap_longmode_x2apic);
out4:
	free_page((unsigned long)vmx_msr_bitmap_longmode);
out3:
	free_page((unsigned long)vmx_msr_bitmap_legacy_x2apic);
out2:
	free_page((unsigned long)vmx_msr_bitmap_legacy);
out1:
	free_page((unsigned long)vmx_io_bitmap_b);
out:
	free_page((unsigned long)vmx_io_bitmap_a);

    return r;
}

static __exit void hardware_unsetup(void)
{
	free_page((unsigned long)vmx_msr_bitmap_legacy_x2apic);
	free_page((unsigned long)vmx_msr_bitmap_longmode_x2apic);
	free_page((unsigned long)vmx_msr_bitmap_legacy);
	free_page((unsigned long)vmx_msr_bitmap_longmode);
	free_page((unsigned long)vmx_io_bitmap_b);
	free_page((unsigned long)vmx_io_bitmap_a);
	free_page((unsigned long)vmx_vmwrite_bitmap);
	free_page((unsigned long)vmx_vmread_bitmap);

	free_kvm_area();
}

/*
 * Indicate a busy-waiting vcpu in spinlock. We do not enable the PAUSE
 * exiting, so only get here on cpu with PAUSE-Loop-Exiting.
 */
static int handle_pause(struct kvm_vcpu *vcpu)
{
	if (ple_gap)
		grow_ple_window(vcpu);

	skip_emulated_instruction(vcpu);
	kvm_vcpu_on_spin(vcpu);

	return 1;
}

static int handle_nop(struct kvm_vcpu *vcpu)
{
	skip_emulated_instruction(vcpu);
	return 1;
}

static int handle_mwait(struct kvm_vcpu *vcpu)
{
	printk_once(KERN_WARNING "kvm: MWAIT instruction emulated as NOP!\n");
	return handle_nop(vcpu);
}

static int handle_monitor(struct kvm_vcpu *vcpu)
{
	printk_once(KERN_WARNING "kvm: MONITOR instruction emulated as NOP!\n");
	return handle_nop(vcpu);
}

/*
 * To run an L2 guest, we need a vmcs02 based on the L1-specified vmcs12.
 * We could reuse a single VMCS for all the L2 guests, but we also want the
 * option to allocate a separate vmcs02 for each separate loaded vmcs12 - this
 * allows keeping them loaded on the processor, and in the future will allow
 * optimizations where prepare_vmcs02 doesn't need to set all the fields on
 * every entry if they never change.
 * So we keep, in vmx->nested.vmcs02_pool, a cache of size VMCS02_POOL_SIZE
 * (>=0) with a vmcs02 for each recently loaded vmcs12s, most recent first.
 *
 * The following functions allocate and free a vmcs02 in this pool.
 */

/* Get a VMCS from the pool to use as vmcs02 for the current vmcs12. */
static struct loaded_vmcs *nested_get_current_vmcs02(struct vcpu_vmx *vmx)
{
	struct vmcs02_list *item;
	list_for_each_entry(item, &vmx->nested.vmcs02_pool, list)
		if (item->vmptr == vmx->nested.current_vmptr) {
			list_move(&item->list, &vmx->nested.vmcs02_pool);
			return &item->vmcs02;
		}