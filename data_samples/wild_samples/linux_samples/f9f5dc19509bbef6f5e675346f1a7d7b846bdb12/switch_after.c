	do {
		/* Jump in the fire! */
		exit_code = __guest_enter(vcpu, host_ctxt);

		/* And we're baaack! */
	} while (fixup_guest_exit(vcpu, &exit_code));

	fp_enabled = __fpsimd_enabled_nvhe();

	__sysreg_save_state_nvhe(guest_ctxt);
	__sysreg32_save_state(vcpu);
	__timer_disable_traps(vcpu);
	__hyp_vgic_save_state(vcpu);

	__deactivate_traps(vcpu);
	__deactivate_vm(vcpu);

	__sysreg_restore_state_nvhe(host_ctxt);

	if (fp_enabled) {
		__fpsimd_save_state(&guest_ctxt->gp_regs.fp_regs);
		__fpsimd_restore_state(&host_ctxt->gp_regs.fp_regs);
		__fpsimd_save_fpexc32(vcpu);
	}