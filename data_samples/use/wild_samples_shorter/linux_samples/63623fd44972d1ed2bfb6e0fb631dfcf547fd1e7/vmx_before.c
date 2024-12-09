static bool __read_mostly fasteoi = 1;
module_param(fasteoi, bool, S_IRUGO);

static bool __read_mostly enable_apicv = 1;
module_param(enable_apicv, bool, S_IRUGO);

/*
 * If nested=1, nested virtualization is supported, i.e., guests may use
					   vmx->guest_msrs[i].mask);

	}
	if (vmx->guest_state_loaded)
		return;

	host_state = &vmx->loaded_vmcs->host_state;
	return 1;
}

static void vmx_clear_hlt(struct kvm_vcpu *vcpu)
{
	/*
	 * Ensure that we clear the HLT state in the VMCS.  We don't need to
 * 2. If target vcpu isn't running(root mode), kick it to pick up the
 * interrupt from PIR in next vmentry.
 */
static void vmx_deliver_posted_interrupt(struct kvm_vcpu *vcpu, int vector)
{
	struct vcpu_vmx *vmx = to_vmx(vcpu);
	int r;

	r = vmx_deliver_nested_posted_interrupt(vcpu, vector);
	if (!r)
		return;

	if (pi_test_and_set_pir(vector, &vmx->pi_desc))
		return;

	/* If a previous notification has sent the IPI, nothing to do.  */
	if (pi_test_and_set_on(&vmx->pi_desc))
		return;

	if (!kvm_vcpu_trigger_posted_interrupt(vcpu, false))
		kvm_vcpu_kick(vcpu);
}

/*
 * Set up the vmcs's constant host-state fields, i.e., host-state fields that
		vmcs_write32(PLE_WINDOW, vmx->ple_window);
	}

	if (vmx->nested.need_vmcs12_to_shadow_sync)
		nested_sync_vmcs12_to_shadow(vcpu);

	if (kvm_register_is_dirty(vcpu, VCPU_REGS_RSP))
		vmcs_writel(GUEST_RSP, vcpu->arch.regs[VCPU_REGS_RSP]);
	if (kvm_register_is_dirty(vcpu, VCPU_REGS_RIP))

	if (nested)
		nested_vmx_setup_ctls_msrs(&vmx->nested.msrs,
					   vmx_capability.ept,
					   kvm_vcpu_apicv_active(vcpu));
	else
		memset(&vmx->nested.msrs, 0, sizeof(vmx->nested.msrs));

	vmx->nested.posted_intr_nv = -1;
	if (setup_vmcs_config(&vmcs_conf, &vmx_cap) < 0)
		return -EIO;
	if (nested)
		nested_vmx_setup_ctls_msrs(&vmcs_conf.nested, vmx_cap.ept,
					   enable_apicv);
	if (memcmp(&vmcs_config, &vmcs_conf, sizeof(struct vmcs_config)) != 0) {
		printk(KERN_ERR "kvm: CPU %d feature inconsistency!\n",
				smp_processor_id());
		return -EIO;
	to_vmx(vcpu)->req_immediate_exit = true;
}

static int vmx_check_intercept(struct kvm_vcpu *vcpu,
			       struct x86_instruction_info *info,
			       enum x86_intercept_stage stage)
{
	struct vmcs12 *vmcs12 = get_vmcs12(vcpu);
	struct x86_emulate_ctxt *ctxt = &vcpu->arch.emulate_ctxt;

	/*
	 * RDPID causes #UD if disabled through secondary execution controls.
	 * Because it is marked as EmulateOnUD, we need to intercept it here.
	 */
	if (info->intercept == x86_intercept_rdtscp &&
	    !nested_cpu_has2(vmcs12, SECONDARY_EXEC_RDTSCP)) {
		ctxt->exception.vector = UD_VECTOR;
		ctxt->exception.error_code_valid = false;
		return X86EMUL_PROPAGATE_FAULT;
	}

	/* TODO: check more intercepts... */
	return X86EMUL_CONTINUE;
}

#ifdef CONFIG_X86_64
/* (a << shift) / divisor, return 1 if overflow otherwise 0 */

	if (nested) {
		nested_vmx_setup_ctls_msrs(&vmcs_config.nested,
					   vmx_capability.ept, enable_apicv);

		r = nested_vmx_hardware_setup(kvm_vmx_exit_handlers);
		if (r)
			return r;

	.run = vmx_vcpu_run,
	.handle_exit = vmx_handle_exit,
	.skip_emulated_instruction = skip_emulated_instruction,
	.set_interrupt_shadow = vmx_set_interrupt_shadow,
	.get_interrupt_shadow = vmx_get_interrupt_shadow,
	.patch_hypercall = vmx_patch_hypercall,
	.set_irq = vmx_inject_irq,