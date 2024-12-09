	if (!guest_cpuid_has_smep(vcpu) && (cr4 & X86_CR4_SMEP))
		return 1;

	if (!guest_cpuid_has_smap(vcpu) && (cr4 & X86_CR4_SMAP))
		return 1;

	if (!guest_cpuid_has_fsgsbase(vcpu) && (cr4 & X86_CR4_FSGSBASE))
		return 1;

	if (is_long_mode(vcpu)) {
	    (!(cr4 & X86_CR4_PCIDE) && (old_cr4 & X86_CR4_PCIDE)))
		kvm_mmu_reset_context(vcpu);

	if ((cr4 ^ old_cr4) & X86_CR4_SMAP)
		update_permission_bitmask(vcpu, vcpu->arch.walk_mmu, false);

	if ((cr4 ^ old_cr4) & X86_CR4_OSXSAVE)
		kvm_update_cpuid(vcpu);

	return 0;
{
	struct timespec ts;

	ktime_get_ts(&ts);
	monotonic_to_bootbased(&ts);
	return timespec_to_ns(&ts);
}
		| (write ? PFERR_WRITE_MASK : 0);

	if (vcpu_match_mmio_gva(vcpu, gva)
	    && !permission_fault(vcpu, vcpu->arch.walk_mmu,
				 vcpu->arch.access, access)) {
		*gpa = vcpu->arch.mmio_gfn << PAGE_SHIFT |
					(gva & (PAGE_SIZE - 1));
		trace_vcpu_match_mmio(gva, *gpa, write, false);
		return 1;