		/* And we're baaack! */
	} while (fixup_guest_exit(vcpu, &exit_code));

	fp_enabled = __fpsimd_enabled_nvhe();

	__sysreg_save_state_nvhe(guest_ctxt);
	__sysreg32_save_state(vcpu);