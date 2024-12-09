
	update_permission_bitmask(vcpu, context, true);
	update_pkru_bitmask(vcpu, context, true);
	reset_rsvds_bits_mask_ept(vcpu, context, execonly);
	reset_ept_shadow_zero_bits_mask(vcpu, context, execonly);
}
EXPORT_SYMBOL_GPL(kvm_init_shadow_ept_mmu);