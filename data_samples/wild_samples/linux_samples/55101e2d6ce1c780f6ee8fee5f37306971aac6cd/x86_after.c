{
	unsigned long old_cr4 = kvm_read_cr4(vcpu);
	unsigned long pdptr_bits = X86_CR4_PGE | X86_CR4_PSE |
				   X86_CR4_PAE | X86_CR4_SMEP;
	if (cr4 & CR4_RESERVED_BITS)
		return 1;

	if (!guest_cpuid_has_xsave(vcpu) && (cr4 & X86_CR4_OSXSAVE))
		return 1;

	if (!guest_cpuid_has_smep(vcpu) && (cr4 & X86_CR4_SMEP))
		return 1;

	if (!guest_cpuid_has_smap(vcpu) && (cr4 & X86_CR4_SMAP))
		return 1;

	if (!guest_cpuid_has_fsgsbase(vcpu) && (cr4 & X86_CR4_FSGSBASE))
		return 1;

	if (is_long_mode(vcpu)) {
		if (!(cr4 & X86_CR4_PAE))
			return 1;
	} else if (is_paging(vcpu) && (cr4 & X86_CR4_PAE)
		   && ((cr4 ^ old_cr4) & pdptr_bits)
		   && !load_pdptrs(vcpu, vcpu->arch.walk_mmu,
				   kvm_read_cr3(vcpu)))
		return 1;

	if ((cr4 & X86_CR4_PCIDE) && !(old_cr4 & X86_CR4_PCIDE)) {
		if (!guest_cpuid_has_pcid(vcpu))
			return 1;

		/* PCID can not be enabled when cr3[11:0]!=000H or EFER.LMA=0 */
		if ((kvm_read_cr3(vcpu) & X86_CR3_PCID_MASK) || !is_long_mode(vcpu))
			return 1;
	}

	if (kvm_x86_ops->set_cr4(vcpu, cr4))
		return 1;

	if (((cr4 ^ old_cr4) & pdptr_bits) ||
	    (!(cr4 & X86_CR4_PCIDE) && (old_cr4 & X86_CR4_PCIDE)))
		kvm_mmu_reset_context(vcpu);

	if ((cr4 ^ old_cr4) & X86_CR4_SMAP)
		update_permission_bitmask(vcpu, vcpu->arch.walk_mmu, false);

	if ((cr4 ^ old_cr4) & X86_CR4_OSXSAVE)
		kvm_update_cpuid(vcpu);

	return 0;
}
	if ((cr4 & X86_CR4_PCIDE) && !(old_cr4 & X86_CR4_PCIDE)) {
		if (!guest_cpuid_has_pcid(vcpu))
			return 1;

		/* PCID can not be enabled when cr3[11:0]!=000H or EFER.LMA=0 */
		if ((kvm_read_cr3(vcpu) & X86_CR3_PCID_MASK) || !is_long_mode(vcpu))
			return 1;
	}

	if (kvm_x86_ops->set_cr4(vcpu, cr4))
		return 1;

	if (((cr4 ^ old_cr4) & pdptr_bits) ||
	    (!(cr4 & X86_CR4_PCIDE) && (old_cr4 & X86_CR4_PCIDE)))
		kvm_mmu_reset_context(vcpu);

	if ((cr4 ^ old_cr4) & X86_CR4_SMAP)
		update_permission_bitmask(vcpu, vcpu->arch.walk_mmu, false);

	if ((cr4 ^ old_cr4) & X86_CR4_OSXSAVE)
		kvm_update_cpuid(vcpu);

	return 0;
}
EXPORT_SYMBOL_GPL(kvm_set_cr4);

int kvm_set_cr3(struct kvm_vcpu *vcpu, unsigned long cr3)
{
	if (cr3 == kvm_read_cr3(vcpu) && !pdptrs_changed(vcpu)) {
		kvm_mmu_sync_roots(vcpu);
		kvm_mmu_flush_tlb(vcpu);
		return 0;
	}
{
	struct timespec ts;

	ktime_get_ts(&ts);
	monotonic_to_bootbased(&ts);
	return timespec_to_ns(&ts);
}

#ifdef CONFIG_X86_64
static atomic_t kvm_guest_has_master_clock = ATOMIC_INIT(0);
#endif

static DEFINE_PER_CPU(unsigned long, cpu_tsc_khz);
unsigned long max_tsc_khz;

static inline u64 nsec_to_cycles(struct kvm_vcpu *vcpu, u64 nsec)
{
	return pvclock_scale_delta(nsec, vcpu->arch.virtual_tsc_mult,
				   vcpu->arch.virtual_tsc_shift);
}

static u32 adjust_tsc_khz(u32 khz, s32 ppm)
{
	u64 v = (u64)khz * (1000000 + ppm);
	do_div(v, 1000000);
	return v;
}

static void kvm_set_tsc_khz(struct kvm_vcpu *vcpu, u32 this_tsc_khz)
{
	u32 thresh_lo, thresh_hi;
	int use_scaling = 0;

	/* tsc_khz can be zero if TSC calibration fails */
	if (this_tsc_khz == 0)
		return;

	/* Compute a scale to convert nanoseconds in TSC cycles */
	kvm_get_time_scale(this_tsc_khz, NSEC_PER_SEC / 1000,
			   &vcpu->arch.virtual_tsc_shift,
			   &vcpu->arch.virtual_tsc_mult);
	vcpu->arch.virtual_tsc_khz = this_tsc_khz;

	/*
	 * Compute the variation in TSC rate which is acceptable
	 * within the range of tolerance and decide if the
	 * rate being applied is within that bounds of the hardware
	 * rate.  If so, no scaling or compensation need be done.
	 */
	thresh_lo = adjust_tsc_khz(tsc_khz, -tsc_tolerance_ppm);
	thresh_hi = adjust_tsc_khz(tsc_khz, tsc_tolerance_ppm);
	if (this_tsc_khz < thresh_lo || this_tsc_khz > thresh_hi) {
		pr_debug("kvm: requested TSC rate %u falls outside tolerance [%u,%u]\n", this_tsc_khz, thresh_lo, thresh_hi);
		use_scaling = 1;
	}
{
	u32 access = ((kvm_x86_ops->get_cpl(vcpu) == 3) ? PFERR_USER_MASK : 0)
		| (write ? PFERR_WRITE_MASK : 0);

	if (vcpu_match_mmio_gva(vcpu, gva)
	    && !permission_fault(vcpu, vcpu->arch.walk_mmu,
				 vcpu->arch.access, access)) {
		*gpa = vcpu->arch.mmio_gfn << PAGE_SHIFT |
					(gva & (PAGE_SIZE - 1));
		trace_vcpu_match_mmio(gva, *gpa, write, false);
		return 1;
	}